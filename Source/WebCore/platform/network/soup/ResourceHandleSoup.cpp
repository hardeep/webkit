/*
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Xan Lopez <xan@gnome.org>
 * Copyright (C) 2008, 2010 Collabora Ltd.
 * Copyright (C) 2009 Holger Hans Peter Freyther
 * Copyright (C) 2009 Gustavo Noronha Silva <gns@gnome.org>
 * Copyright (C) 2009 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2009, 2010, 2011 Igalia S.L.
 * Copyright (C) 2009 John Kjellberg <john.kjellberg@power.alstom.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "ResourceHandle.h"

#include "Base64.h"
#include "CachedResourceLoader.h"
#include "ChromeClient.h"
#include "CookieJarSoup.h"
#include "FileSystem.h"
#include "Frame.h"
#include "GOwnPtrSoup.h"
#include "HTTPParsers.h"
#include "Logging.h"
#include "MIMETypeRegistry.h"
#include "NotImplemented.h"
#include "Page.h"
#include "ResourceError.h"
#include "ResourceHandleClient.h"
#include "ResourceHandleInternal.h"
#include "ResourceResponse.h"
#include "SharedBuffer.h"
#include "TextEncoding.h"
#include <errno.h>
#include <fcntl.h>
#include <gio/gio.h>
#include <glib.h>
#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#include <libsoup/soup-request-http.h>
#include <libsoup/soup-requester.h>
#include <libsoup/soup.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wtf/gobject/GRefPtr.h>
#include <wtf/text/CString.h>

#if ENABLE(BLOB)
#include "BlobData.h"
#include "BlobRegistryImpl.h"
#include "BlobStorageData.h"
#endif

namespace WebCore {

#define READ_BUFFER_SIZE 8192

class WebCoreSynchronousLoader : public ResourceHandleClient {
    WTF_MAKE_NONCOPYABLE(WebCoreSynchronousLoader);
public:
    WebCoreSynchronousLoader(ResourceError&, ResourceResponse &, Vector<char>&);
    ~WebCoreSynchronousLoader();

    virtual void didReceiveResponse(ResourceHandle*, const ResourceResponse&);
    virtual void didReceiveData(ResourceHandle*, const char*, int, int encodedDataLength);
    virtual void didFinishLoading(ResourceHandle*, double /*finishTime*/);
    virtual void didFail(ResourceHandle*, const ResourceError&);

    void run();

private:
    ResourceError& m_error;
    ResourceResponse& m_response;
    Vector<char>& m_data;
    bool m_finished;
    GRefPtr<GMainLoop> m_mainLoop;
};

WebCoreSynchronousLoader::WebCoreSynchronousLoader(ResourceError& error, ResourceResponse& response, Vector<char>& data)
    : m_error(error)
    , m_response(response)
    , m_data(data)
    , m_finished(false)
{
    m_mainLoop = adoptGRef(g_main_loop_new(0, false));
}

WebCoreSynchronousLoader::~WebCoreSynchronousLoader()
{
}

void WebCoreSynchronousLoader::didReceiveResponse(ResourceHandle*, const ResourceResponse& response)
{
    m_response = response;
}

void WebCoreSynchronousLoader::didReceiveData(ResourceHandle*, const char* data, int length, int)
{
    m_data.append(data, length);
}

void WebCoreSynchronousLoader::didFinishLoading(ResourceHandle*, double)
{
    g_main_loop_quit(m_mainLoop.get());
    m_finished = true;
}

void WebCoreSynchronousLoader::didFail(ResourceHandle* handle, const ResourceError& error)
{
    m_error = error;
    didFinishLoading(handle, 0);
}

void WebCoreSynchronousLoader::run()
{
    if (!m_finished)
        g_main_loop_run(m_mainLoop.get());
}

static void cleanupSoupRequestOperation(ResourceHandle*, bool isDestroying);
static void sendRequestCallback(GObject*, GAsyncResult*, gpointer);
static void readCallback(GObject*, GAsyncResult*, gpointer);
static void closeCallback(GObject*, GAsyncResult*, gpointer);
static bool startNonHTTPRequest(ResourceHandle*, KURL);

ResourceHandleInternal::~ResourceHandleInternal()
{
}

SoupSession* ResourceHandleInternal::soupSession()
{
    return (m_context && m_context->isValid()) ? m_context->soupSession() : ResourceHandle::defaultSession();
}

ResourceHandle::~ResourceHandle()
{
    cleanupSoupRequestOperation(this, true);
}

static void ensureSessionIsInitialized(SoupSession* session)
{
    if (g_object_get_data(G_OBJECT(session), "webkit-init"))
        return;

    if (session == ResourceHandle::defaultSession()) {
        SoupCookieJar* jar = SOUP_COOKIE_JAR(soup_session_get_feature(session, SOUP_TYPE_COOKIE_JAR));
        if (!jar)
            soup_session_add_feature(session, SOUP_SESSION_FEATURE(defaultCookieJar()));
        else
            setDefaultCookieJar(jar);
    }

    if (!soup_session_get_feature(session, SOUP_TYPE_LOGGER) && LogNetwork.state == WTFLogChannelOn) {
        SoupLogger* logger = soup_logger_new(static_cast<SoupLoggerLogLevel>(SOUP_LOGGER_LOG_BODY), -1);
        soup_session_add_feature(session, SOUP_SESSION_FEATURE(logger));
        g_object_unref(logger);
    }

    if (!soup_session_get_feature(session, SOUP_TYPE_REQUESTER)) {
        SoupRequester* requester = soup_requester_new();
        soup_session_add_feature(session, SOUP_SESSION_FEATURE(requester));
        g_object_unref(requester);
    }

    g_object_set_data(G_OBJECT(session), "webkit-init", reinterpret_cast<void*>(0xdeadbeef));
}

void ResourceHandle::prepareForURL(const KURL& url)
{
    GOwnPtr<SoupURI> soupURI(soup_uri_new(url.string().utf8().data()));
    if (!soupURI)
        return;
    soup_session_prepare_for_uri(ResourceHandle::defaultSession(), soupURI.get());
}

// Called each time the message is going to be sent again except the first time.
// It's used mostly to let webkit know about redirects.
static void restartedCallback(SoupMessage* msg, gpointer data)
{
    ResourceHandle* handle = static_cast<ResourceHandle*>(data);
    if (!handle)
        return;
    ResourceHandleInternal* d = handle->getInternal();
    if (d->m_cancelled)
        return;

    GOwnPtr<char> uri(soup_uri_to_string(soup_message_get_uri(msg), false));
    String location = String::fromUTF8(uri.get());
    KURL newURL = KURL(handle->firstRequest().url(), location);

    ResourceRequest request = handle->firstRequest();
    ResourceResponse response;
    request.setURL(newURL);
    request.setHTTPMethod(msg->method);
    response.updateFromSoupMessage(msg);

    // Should not set Referer after a redirect from a secure resource to non-secure one.
    if (!request.url().protocolIs("https") && protocolIs(request.httpReferrer(), "https")) {
        request.clearHTTPReferrer();
        soup_message_headers_remove(msg->request_headers, "Referer");
    }

    if (d->client())
        d->client()->willSendRequest(handle, request, response);

    if (d->m_cancelled)
        return;

    // Update the first party in case the base URL changed with the redirect
    String firstPartyString = request.firstPartyForCookies().string();
    if (!firstPartyString.isEmpty()) {
        GOwnPtr<SoupURI> firstParty(soup_uri_new(firstPartyString.utf8().data()));
        soup_message_set_first_party(d->m_soupMessage.get(), firstParty.get());
    }
}

static void wroteBodyDataCallback(SoupMessage*, SoupBuffer* buffer, gpointer data)
{
    RefPtr<ResourceHandle> handle = static_cast<ResourceHandle*>(data);
    if (!handle)
        return;

    ASSERT(buffer);
    ResourceHandleInternal* internal = handle->getInternal();
    internal->m_bodyDataSent += buffer->length;

    if (internal->m_cancelled)
        return;
    ResourceHandleClient* client = handle->client();
    if (!client)
        return;

    client->didSendData(handle.get(), internal->m_bodyDataSent, internal->m_bodySize);
}

static void cleanupSoupRequestOperation(ResourceHandle* handle, bool isDestroying = false)
{
    ResourceHandleInternal* d = handle->getInternal();

    if (d->m_soupRequest)
        d->m_soupRequest.clear();

    if (d->m_inputStream)
        d->m_inputStream.clear();

    d->m_cancellable.clear();

    if (d->m_soupMessage) {
        g_signal_handlers_disconnect_matched(d->m_soupMessage.get(), G_SIGNAL_MATCH_DATA,
                                             0, 0, 0, 0, handle);
        d->m_soupMessage.clear();
    }

    if (d->m_buffer) {
        g_slice_free1(READ_BUFFER_SIZE, d->m_buffer);
        d->m_buffer = 0;
    }

    if (!isDestroying)
        handle->deref();
}

static ResourceError convertSoupErrorToResourceError(GError* error, SoupRequest* request, SoupMessage* message = 0)
{
    ASSERT(error);
    ASSERT(request);

    GOwnPtr<char> uri(soup_uri_to_string(soup_request_get_uri(request), FALSE));
    if (message && SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code)) {
        return ResourceError(g_quark_to_string(SOUP_HTTP_ERROR),
                             static_cast<gint>(message->status_code),
                             uri.get(),
                             String::fromUTF8(message->reason_phrase));
    }

    // Non-transport errors are handled differently.
    return ResourceError(g_quark_to_string(G_IO_ERROR),
                         error->code,
                         uri.get(),
                         String::fromUTF8(error->message));
}

static void sendRequestCallback(GObject* source, GAsyncResult* res, gpointer data)
{
    RefPtr<ResourceHandle> handle = static_cast<ResourceHandle*>(data);

    ResourceHandleInternal* d = handle->getInternal();
    ResourceHandleClient* client = handle->client();
    SoupMessage* soupMessage = d->m_soupMessage.get();

    if (d->m_cancelled || !client) {
        cleanupSoupRequestOperation(handle.get());
        return;
    }

    GOwnPtr<GError> error;
    GInputStream* in = soup_request_send_finish(d->m_soupRequest.get(), res, &error.outPtr());
    if (error) {
        client->didFail(handle.get(), convertSoupErrorToResourceError(error.get(), d->m_soupRequest.get(), soupMessage));
        cleanupSoupRequestOperation(handle.get());
        return;
    }

    d->m_inputStream = adoptGRef(in);
    d->m_buffer = static_cast<char*>(g_slice_alloc(READ_BUFFER_SIZE));

    if (soupMessage) {
        if (handle->shouldContentSniff() && soupMessage->status_code != SOUP_STATUS_NOT_MODIFIED) {
            const char* sniffedType = soup_request_get_content_type(d->m_soupRequest.get());
            d->m_response.setSniffedContentType(sniffedType);
        }
        d->m_response.updateFromSoupMessage(soupMessage);

        if (d->m_defersLoading)
            soup_session_pause_message(d->soupSession(), soupMessage);
    } else {
        d->m_response.setURL(handle->firstRequest().url());
        const gchar* contentType = soup_request_get_content_type(d->m_soupRequest.get());
        d->m_response.setMimeType(extractMIMETypeFromMediaType(contentType));
        d->m_response.setTextEncodingName(extractCharsetFromMediaType(contentType));
        d->m_response.setExpectedContentLength(soup_request_get_content_length(d->m_soupRequest.get()));
    }

    client->didReceiveResponse(handle.get(), d->m_response);

    if (d->m_cancelled) {
        cleanupSoupRequestOperation(handle.get());
        return;
    }

    g_input_stream_read_async(d->m_inputStream.get(), d->m_buffer, READ_BUFFER_SIZE,
                              G_PRIORITY_DEFAULT, d->m_cancellable.get(), readCallback, handle.get());
}

static bool addFileToSoupMessageBody(SoupMessage* message, const String& fileNameString, size_t offset, size_t lengthToSend, unsigned long& totalBodySize)
{
    GOwnPtr<GError> error;
    CString fileName = fileSystemRepresentation(fileNameString);
    GMappedFile* fileMapping = g_mapped_file_new(fileName.data(), false, &error.outPtr());
    if (error)
        return false;

    gsize bufferLength = lengthToSend;
    if (!lengthToSend)
        bufferLength = g_mapped_file_get_length(fileMapping);
    totalBodySize += bufferLength;

    SoupBuffer* soupBuffer = soup_buffer_new_with_owner(g_mapped_file_get_contents(fileMapping) + offset,
                                                        bufferLength,
                                                        fileMapping,
                                                        reinterpret_cast<GDestroyNotify>(g_mapped_file_unref));
    soup_message_body_append_buffer(message->request_body, soupBuffer);
    soup_buffer_free(soupBuffer);
    return true;
}

#if ENABLE(BLOB)
static bool blobIsOutOfDate(const BlobDataItem& blobItem)
{
    ASSERT(blobItem.type == BlobDataItem::File);
    if (blobItem.expectedModificationTime == BlobDataItem::doNotCheckFileChange)
        return false;

    time_t fileModificationTime;
    if (!getFileModificationTime(blobItem.path, fileModificationTime))
        return true;

    return fileModificationTime != static_cast<time_t>(blobItem.expectedModificationTime);
}

static void addEncodedBlobItemToSoupMessageBody(SoupMessage* message, const BlobDataItem& blobItem, unsigned long& totalBodySize)
{
    if (blobItem.type == BlobDataItem::Data) {
        totalBodySize += blobItem.length;
        soup_message_body_append(message->request_body, SOUP_MEMORY_TEMPORARY,
                                 blobItem.data->data() + blobItem.offset, blobItem.length);
        return;
    }

    ASSERT(blobItem.type == BlobDataItem::File);
    if (blobIsOutOfDate(blobItem))
        return;

    addFileToSoupMessageBody(message,
                             blobItem.path,
                             blobItem.offset,
                             blobItem.length == BlobDataItem::toEndOfFile ? 0 : blobItem.length,
                             totalBodySize);
}

static void addEncodedBlobToSoupMessageBody(SoupMessage* message, const FormDataElement& element, unsigned long& totalBodySize)
{
    RefPtr<BlobStorageData> blobData = static_cast<BlobRegistryImpl&>(blobRegistry()).getBlobDataFromURL(KURL(ParsedURLString, element.m_blobURL));
    if (!blobData)
        return;

    for (size_t i = 0; i < blobData->items().size(); ++i)
        addEncodedBlobItemToSoupMessageBody(message, blobData->items()[i], totalBodySize);
}
#endif // ENABLE(BLOB)

static bool addFormElementsToSoupMessage(SoupMessage* message, const char* contentType, FormData* httpBody, unsigned long& totalBodySize)
{
    soup_message_body_set_accumulate(message->request_body, FALSE);
    size_t numElements = httpBody->elements().size();
    for (size_t i = 0; i < numElements; i++) {
        const FormDataElement& element = httpBody->elements()[i];

        if (element.m_type == FormDataElement::data) {
            totalBodySize += element.m_data.size();
            soup_message_body_append(message->request_body, SOUP_MEMORY_TEMPORARY,
                                     element.m_data.data(), element.m_data.size());
            continue;
        }

        if (element.m_type == FormDataElement::encodedFile) {
            if (!addFileToSoupMessageBody(message ,
                                         element.m_filename,
                                         0 /* offset */,
                                         0 /* lengthToSend */,
                                         totalBodySize))
                return false;
            continue;
        }

#if ENABLE(BLOB)
        ASSERT(element.m_type == FormDataElement::encodedBlob);
        addEncodedBlobToSoupMessageBody(message, element, totalBodySize);
#endif
    }
    return true;
}

static bool startHTTPRequest(ResourceHandle* handle)
{
    ASSERT(handle);

    ResourceHandleInternal* d = handle->getInternal();

    SoupSession* session = d->soupSession();
    ensureSessionIsInitialized(session);
    SoupRequester* requester = SOUP_REQUESTER(soup_session_get_feature(session, SOUP_TYPE_REQUESTER));

    ResourceRequest request(handle->firstRequest());
    KURL url(request.url());
    url.removeFragmentIdentifier();
    request.setURL(url);

    GOwnPtr<GError> error;
    d->m_soupRequest = adoptGRef(soup_requester_request(requester, url.string().utf8().data(), &error.outPtr()));
    if (error) {
        d->m_soupRequest = 0;
        return false;
    }

    d->m_soupMessage = adoptGRef(soup_request_http_get_message(SOUP_REQUEST_HTTP(d->m_soupRequest.get())));
    if (!d->m_soupMessage)
        return false;

    SoupMessage* soupMessage = d->m_soupMessage.get();
    request.updateSoupMessage(soupMessage);

    if (!handle->shouldContentSniff())
        soup_message_disable_feature(soupMessage, SOUP_TYPE_CONTENT_SNIFFER);

    g_signal_connect(soupMessage, "restarted", G_CALLBACK(restartedCallback), handle);
    g_signal_connect(soupMessage, "wrote-body-data", G_CALLBACK(wroteBodyDataCallback), handle);

    String firstPartyString = request.firstPartyForCookies().string();
    if (!firstPartyString.isEmpty()) {
        GOwnPtr<SoupURI> firstParty(soup_uri_new(firstPartyString.utf8().data()));
        soup_message_set_first_party(soupMessage, firstParty.get());
    }

    FormData* httpBody = d->m_firstRequest.httpBody();
    CString contentType = d->m_firstRequest.httpContentType().utf8().data();
    if (httpBody && !httpBody->isEmpty()
        && !addFormElementsToSoupMessage(soupMessage, contentType.data(), httpBody, d->m_bodySize)) {
        // We failed to prepare the body data, so just fail this load.
        g_signal_handlers_disconnect_matched(soupMessage, G_SIGNAL_MATCH_DATA, 0, 0, 0, 0, handle);
        d->m_soupMessage.clear();
        return false;
    }

    // balanced by a deref() in cleanupSoupRequestOperation, which should always run
    handle->ref();

    // Make sure we have an Accept header for subresources; some sites
    // want this to serve some of their subresources
    if (!soup_message_headers_get_one(soupMessage->request_headers, "Accept"))
        soup_message_headers_append(soupMessage->request_headers, "Accept", "*/*");

    // Send the request only if it's not been explicitly deferred.
    if (!d->m_defersLoading) {
        d->m_cancellable = adoptGRef(g_cancellable_new());
        soup_request_send_async(d->m_soupRequest.get(), d->m_cancellable.get(), sendRequestCallback, handle);
    }

    return true;
}

bool ResourceHandle::start(NetworkingContext* context)
{
    ASSERT(!d->m_soupMessage);

    // The frame could be null if the ResourceHandle is not associated to any
    // Frame, e.g. if we are downloading a file.
    // If the frame is not null but the page is null this must be an attempted
    // load from an unload handler, so let's just block it.
    // If both the frame and the page are not null the context is valid.
    if (context && !context->isValid())
        return false;

    if (!(d->m_user.isEmpty() || d->m_pass.isEmpty())) {
        // If credentials were specified for this request, add them to the url,
        // so that they will be passed to NetworkRequest.
        KURL urlWithCredentials(firstRequest().url());
        urlWithCredentials.setUser(d->m_user);
        urlWithCredentials.setPass(d->m_pass);
        d->m_firstRequest.setURL(urlWithCredentials);
    }

    KURL url = firstRequest().url();
    String urlString = url.string();
    String protocol = url.protocol();

    // Used to set the authentication dialog toplevel; may be NULL
    d->m_context = context;

    if (equalIgnoringCase(protocol, "http") || equalIgnoringCase(protocol, "https")) {
        if (startHTTPRequest(this))
            return true;
    }

    if (startNonHTTPRequest(this, url))
        return true;

    // Error must not be reported immediately
    this->scheduleFailure(InvalidURLFailure);

    return true;
}

void ResourceHandle::cancel()
{
    d->m_cancelled = true;
    if (d->m_soupMessage)
        soup_session_cancel_message(d->soupSession(), d->m_soupMessage.get(), SOUP_STATUS_CANCELLED);
    else if (d->m_cancellable)
        g_cancellable_cancel(d->m_cancellable.get());
}

static bool hasBeenSent(ResourceHandle* handle)
{
    ResourceHandleInternal* d = handle->getInternal();

    return d->m_cancellable;
}

void ResourceHandle::platformSetDefersLoading(bool defersLoading)
{
    if (d->m_cancelled)
        return;

    // We only need to take action here to UN-defer loading.
    if (defersLoading)
        return;

    // We need to check for d->m_soupRequest because the request may
    // have raised a failure (for example invalid URLs). We cannot
    // simply check for d->m_scheduledFailure because it's cleared as
    // soon as the failure event is fired.
    if (!hasBeenSent(this) && d->m_soupRequest) {
        d->m_cancellable = adoptGRef(g_cancellable_new());
        soup_request_send_async(d->m_soupRequest.get(), d->m_cancellable.get(), sendRequestCallback, this);
        return;
    }

    if (d->m_deferredResult) {
        GRefPtr<GAsyncResult> asyncResult = adoptGRef(d->m_deferredResult.leakRef());
        readCallback(G_OBJECT(d->m_inputStream.get()), asyncResult.get(), this);
    }
}

bool ResourceHandle::loadsBlocked()
{
    return false;
}

bool ResourceHandle::willLoadFromCache(ResourceRequest&, Frame*)
{
    // Not having this function means that we'll ask the user about re-posting a form
    // even when we go back to a page that's still in the cache.
    notImplemented();
    return false;
}

void ResourceHandle::loadResourceSynchronously(NetworkingContext* context, const ResourceRequest& request, StoredCredentials /*storedCredentials*/, ResourceError& error, ResourceResponse& response, Vector<char>& data)
{
#if ENABLE(BLOB)
    if (request.url().protocolIs("blob")) {
        blobRegistry().loadResourceSynchronously(request, error, response, data);
        return;
    }
#endif

    WebCoreSynchronousLoader syncLoader(error, response, data);
    RefPtr<ResourceHandle> handle = create(context, request, &syncLoader, false /*defersLoading*/, false /*shouldContentSniff*/);
    if (!handle)
        return;

    // If the request has already failed, do not run the main loop, or else we'll block indefinitely.
    if (handle->d->m_scheduledFailureType != NoFailure)
        return;

    syncLoader.run();
}

static void closeCallback(GObject* source, GAsyncResult* res, gpointer data)
{
    RefPtr<ResourceHandle> handle = static_cast<ResourceHandle*>(data);

    ResourceHandleInternal* d = handle->getInternal();
    g_input_stream_close_finish(d->m_inputStream.get(), res, 0);
    cleanupSoupRequestOperation(handle.get());
}

static void readCallback(GObject* source, GAsyncResult* asyncResult, gpointer data)
{
    RefPtr<ResourceHandle> handle = static_cast<ResourceHandle*>(data);

    ResourceHandleInternal* d = handle->getInternal();
    ResourceHandleClient* client = handle->client();

    if (d->m_cancelled || !client) {
        cleanupSoupRequestOperation(handle.get());
        return;
    }

    if (d->m_defersLoading) {
        d->m_deferredResult = asyncResult;
        return;
    }

    GOwnPtr<GError> error;
    gssize bytesRead = g_input_stream_read_finish(d->m_inputStream.get(), asyncResult, &error.outPtr());
    if (error) {
        client->didFail(handle.get(), convertSoupErrorToResourceError(error.get(), d->m_soupRequest.get()));
        cleanupSoupRequestOperation(handle.get());
        return;
    }

    if (!bytesRead) {
        // We inform WebCore of load completion now instead of waiting for the input
        // stream to close because the input stream is closed asynchronously.
        client->didFinishLoading(handle.get(), 0);
        g_input_stream_close_async(d->m_inputStream.get(), G_PRIORITY_DEFAULT, 0, closeCallback, handle.get());
        return;
    }

    // It's mandatory to have sent a response before sending data
    ASSERT(!d->m_response.isNull());

    client->didReceiveData(handle.get(), d->m_buffer, bytesRead, bytesRead);

    // didReceiveData may cancel the load, which may release the last reference.
    if (d->m_cancelled || !client) {
        cleanupSoupRequestOperation(handle.get());
        return;
    }

    g_input_stream_read_async(d->m_inputStream.get(), d->m_buffer, READ_BUFFER_SIZE, G_PRIORITY_DEFAULT,
                              d->m_cancellable.get(), readCallback, handle.get());
}

static bool startNonHTTPRequest(ResourceHandle* handle, KURL url)
{
    ASSERT(handle);

    if (handle->firstRequest().httpMethod() != "GET" && handle->firstRequest().httpMethod() != "POST")
        return false;

    ResourceHandleInternal* d = handle->getInternal();

    SoupSession* session = d->soupSession();
    ensureSessionIsInitialized(session);
    SoupRequester* requester = SOUP_REQUESTER(soup_session_get_feature(session, SOUP_TYPE_REQUESTER));

    CString urlStr = url.string().utf8();

    GOwnPtr<GError> error;
    d->m_soupRequest = adoptGRef(soup_requester_request(requester, urlStr.data(), &error.outPtr()));
    if (error) {
        d->m_soupRequest = 0;
        return false;
    }

    // balanced by a deref() in cleanupSoupRequestOperation, which should always run
    handle->ref();

    // Send the request only if it's not been explicitly deferred.
    if (!d->m_defersLoading) {
        d->m_cancellable = adoptGRef(g_cancellable_new());
        soup_request_send_async(d->m_soupRequest.get(), d->m_cancellable.get(), sendRequestCallback, handle);
    }

    return true;
}

SoupSession* ResourceHandle::defaultSession()
{
    static SoupSession* session = 0;
    // Values taken from http://www.browserscope.org/  following
    // the rule "Do What Every Other Modern Browser Is Doing". They seem
    // to significantly improve page loading time compared to soup's
    // default values.
    static const int maxConnections = 35;
    static const int maxConnectionsPerHost = 6;

    if (!session) {
        session = soup_session_async_new();
        g_object_set(session,
                     SOUP_SESSION_MAX_CONNS, maxConnections,
                     SOUP_SESSION_MAX_CONNS_PER_HOST, maxConnectionsPerHost,
                     SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
                     SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_SNIFFER,
                     SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_PROXY_RESOLVER_DEFAULT,
                     NULL);
    }

    return session;
}

}
