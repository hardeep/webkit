/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2008, 2009 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ClipboardChromium.h"

#include "CachedImage.h"
#include "ChromiumDataObject.h"
#include "ClipboardMimeTypes.h"
#include "ClipboardUtilitiesChromium.h"
#include "DataTransferItemChromium.h"
#include "DataTransferItemListChromium.h"
#include "Document.h"
#include "DragData.h"
#include "Element.h"
#include "ExceptionCode.h"
#include "File.h"
#include "FileList.h"
#include "Frame.h"
#include "HTMLNames.h"
#include "HTMLParserIdioms.h"
#include "Image.h"
#include "MIMETypeRegistry.h"
#include "NamedNodeMap.h"
#include "PlatformSupport.h"
#include "Range.h"
#include "RenderImage.h"
#include "StringCallback.h"
#include "markup.h"

#include <wtf/text/WTFString.h>

namespace WebCore {

namespace {

// These wrapper classes invalidate a DataTransferItem/DataTransferItemList when the associated
// Clipboard object goes out of scope.
class DataTransferItemListPolicyWrapper : public DataTransferItemList {
public:
    static PassRefPtr<DataTransferItemListPolicyWrapper> create(
        PassRefPtr<ClipboardChromium>, PassRefPtr<DataTransferItemListChromium>);

    virtual size_t length() const;
    virtual PassRefPtr<DataTransferItem> item(unsigned long index);
    virtual void deleteItem(unsigned long index, ExceptionCode&);
    virtual void clear();
    virtual void add(const String& data, const String& type, ExceptionCode&);
    virtual void add(PassRefPtr<File>);

private:
    DataTransferItemListPolicyWrapper(PassRefPtr<ClipboardChromium>, PassRefPtr<DataTransferItemListChromium>);

    RefPtr<ClipboardChromium> m_clipboard;
    RefPtr<DataTransferItemListChromium> m_list;
};

class DataTransferItemPolicyWrapper : public DataTransferItem {
public:
    static PassRefPtr<DataTransferItemPolicyWrapper> create(
        PassRefPtr<ClipboardChromium>, PassRefPtr<DataTransferItem>);

    virtual String kind() const;
    virtual String type() const;

    virtual void getAsString(PassRefPtr<StringCallback>) const;
    virtual PassRefPtr<Blob> getAsFile() const;

private:
    DataTransferItemPolicyWrapper(PassRefPtr<ClipboardChromium>, PassRefPtr<DataTransferItem>);

    RefPtr<ClipboardChromium> m_clipboard;
    RefPtr<DataTransferItem> m_item;
};

PassRefPtr<DataTransferItemListPolicyWrapper> DataTransferItemListPolicyWrapper::create(
    PassRefPtr<ClipboardChromium> clipboard, PassRefPtr<DataTransferItemListChromium> list)
{
    return adoptRef(new DataTransferItemListPolicyWrapper(clipboard, list));
}

size_t DataTransferItemListPolicyWrapper::length() const
{
    if (m_clipboard->policy() == ClipboardNumb)
        return 0;
    return m_list->length();
}

PassRefPtr<DataTransferItem> DataTransferItemListPolicyWrapper::item(unsigned long index)
{
    if (m_clipboard->policy() == ClipboardNumb)
        return 0;
    RefPtr<DataTransferItem> item = m_list->item(index);
    if (!item)
        return 0;
    return DataTransferItemPolicyWrapper::create(m_clipboard, item);
}

void DataTransferItemListPolicyWrapper::deleteItem(unsigned long index, ExceptionCode& ec)
{
    if (m_clipboard->policy() != ClipboardWritable) {
        ec = INVALID_STATE_ERR;
        return;
    }
    // FIXME: We handle all the exceptions here, so we don't need to propogate ec.
    m_list->deleteItem(index, ec);
}

void DataTransferItemListPolicyWrapper::clear()
{
    if (m_clipboard->policy() != ClipboardWritable)
        return;
    m_list->clear();
}

void DataTransferItemListPolicyWrapper::add(const String& data, const String& type, ExceptionCode& ec)
{
    if (m_clipboard->policy() != ClipboardWritable)
        return;
    m_list->add(data, type, ec);
}

void DataTransferItemListPolicyWrapper::add(PassRefPtr<File> file)
{
    if (m_clipboard->policy() != ClipboardWritable)
        return;
    m_list->add(file);
}

DataTransferItemListPolicyWrapper::DataTransferItemListPolicyWrapper(
    PassRefPtr<ClipboardChromium> clipboard, PassRefPtr<DataTransferItemListChromium> list)
    : m_clipboard(clipboard)
    , m_list(list)
{
}

PassRefPtr<DataTransferItemPolicyWrapper> DataTransferItemPolicyWrapper::create(
    PassRefPtr<ClipboardChromium> clipboard, PassRefPtr<DataTransferItem> item)
{
    return adoptRef(new DataTransferItemPolicyWrapper(clipboard, item));
}

String DataTransferItemPolicyWrapper::kind() const
{
    if (m_clipboard->policy() == ClipboardNumb)
        return String();
    return m_item->kind();
}

String DataTransferItemPolicyWrapper::type() const
{
    if (m_clipboard->policy() == ClipboardNumb)
        return String();
    return m_item->type();
}

void DataTransferItemPolicyWrapper::getAsString(PassRefPtr<StringCallback> callback) const
{
    if (m_clipboard->policy() != ClipboardReadable && m_clipboard->policy() != ClipboardWritable)
        return;

    m_item->getAsString(callback);
}

PassRefPtr<Blob> DataTransferItemPolicyWrapper::getAsFile() const
{
    if (m_clipboard->policy() != ClipboardReadable && m_clipboard->policy() != ClipboardWritable)
        return 0;

    return m_item->getAsFile();
}

DataTransferItemPolicyWrapper::DataTransferItemPolicyWrapper(
    PassRefPtr<ClipboardChromium> clipboard, PassRefPtr<DataTransferItem> item)
    : m_clipboard(clipboard)
    , m_item(item)
{
}

} // namespace

using namespace HTMLNames;

// We provide the IE clipboard types (URL and Text), and the clipboard types specified in the WHATWG Web Applications 1.0 draft
// see http://www.whatwg.org/specs/web-apps/current-work/ Section 6.3.5.3

static String normalizeType(const String& type)
{
    String cleanType = type.stripWhiteSpace().lower();
    if (cleanType == mimeTypeText || cleanType.startsWith(mimeTypeTextPlainEtc))
        return mimeTypeTextPlain;
    return cleanType;
}

PassRefPtr<Clipboard> Clipboard::create(ClipboardAccessPolicy policy, DragData* dragData, Frame* frame)
{
    return ClipboardChromium::create(DragAndDrop, dragData->platformData(), policy, frame);
}

ClipboardChromium::ClipboardChromium(ClipboardType clipboardType,
                                     PassRefPtr<ChromiumDataObject> dataObject,
                                     ClipboardAccessPolicy policy,
                                     Frame* frame)
    : Clipboard(policy, clipboardType)
    , m_dataObject(dataObject)
    , m_frame(frame)
    , m_originalSequenceNumber(PlatformSupport::clipboardSequenceNumber(currentPasteboardBuffer()))
    , m_dragStorageUpdated(true)
{
}

ClipboardChromium::~ClipboardChromium()
{
}

PassRefPtr<ClipboardChromium> ClipboardChromium::create(ClipboardType clipboardType,
    PassRefPtr<ChromiumDataObject> dataObject, ClipboardAccessPolicy policy, Frame* frame)
{
    return adoptRef(new ClipboardChromium(clipboardType, dataObject, policy, frame));
}

void ClipboardChromium::clearData(const String& type)
{
    if (policy() != ClipboardWritable || !m_dataObject)
        return;

    m_dragStorageUpdated = true;
    m_dataObject->clearData(normalizeType(type));

    ASSERT_NOT_REACHED();
}

void ClipboardChromium::clearAllData()
{
    if (policy() != ClipboardWritable)
        return;

    m_dragStorageUpdated = true;
    m_dataObject->clearAll();
}

String ClipboardChromium::getData(const String& type) const
{
    bool ignoredSuccess = false;
    if (policy() != ClipboardReadable || !m_dataObject)
        return String();

    if (isForCopyAndPaste() && platformClipboardChanged())
        return String();

    return m_dataObject->getData(normalizeType(type), ignoredSuccess);
}

bool ClipboardChromium::setData(const String& type, const String& data)
{
    if (policy() != ClipboardWritable)
        return false;

    m_dragStorageUpdated = true;
    return m_dataObject->setData(normalizeType(type), data);
}

bool ClipboardChromium::platformClipboardChanged() const
{
    return PlatformSupport::clipboardSequenceNumber(currentPasteboardBuffer()) != m_originalSequenceNumber;
}

// extensions beyond IE's API
HashSet<String> ClipboardChromium::types() const
{
    HashSet<String> results;
    if (policy() != ClipboardReadable && policy() != ClipboardTypesReadable)
        return results;

    if (!m_dataObject)
        return results;

    results = m_dataObject->types();

    if (m_dataObject->containsFilenames())
        results.add(mimeTypeFiles);

    return results;
}

PassRefPtr<FileList> ClipboardChromium::files() const
{
    if (policy() != ClipboardReadable)
        return FileList::create();

    if (!m_dataObject)
        return FileList::create();

    const Vector<String>& filenames = m_dataObject->filenames();
    RefPtr<FileList> fileList = FileList::create();
    for (size_t i = 0; i < filenames.size(); ++i)
        fileList->append(File::create(filenames.at(i)));

    return fileList.release();
}

void ClipboardChromium::setDragImage(CachedImage* image, Node* node, const IntPoint& loc)
{
    if (policy() != ClipboardImageWritable && policy() != ClipboardWritable)
        return;

    if (m_dragImage)
        m_dragImage->removeClient(this);
    m_dragImage = image;
    if (m_dragImage)
        m_dragImage->addClient(this);

    m_dragLoc = loc;
    m_dragImageElement = node;
}

void ClipboardChromium::setDragImage(CachedImage* img, const IntPoint& loc)
{
    setDragImage(img, 0, loc);
}

void ClipboardChromium::setDragImageElement(Node* node, const IntPoint& loc)
{
    setDragImage(0, node, loc);
}

DragImageRef ClipboardChromium::createDragImage(IntPoint& loc) const
{
    DragImageRef result = 0;
    if (m_dragImageElement) {
        if (m_frame) {
            result = m_frame->nodeImage(m_dragImageElement.get());
            loc = m_dragLoc;
        }
    } else if (m_dragImage) {
        result = createDragImageFromImage(m_dragImage->image());
        loc = m_dragLoc;
    }
    return result;
}

static CachedImage* getCachedImage(Element* element)
{
    // Attempt to pull CachedImage from element
    ASSERT(element);
    RenderObject* renderer = element->renderer();
    if (!renderer || !renderer->isImage())
        return 0;

    RenderImage* image = toRenderImage(renderer);
    if (image->cachedImage() && !image->cachedImage()->errorOccurred())
        return image->cachedImage();

    return 0;
}

static void writeImageToDataObject(ChromiumDataObject* dataObject, Element* element,
                                   const KURL& url)
{
    // Shove image data into a DataObject for use as a file
    CachedImage* cachedImage = getCachedImage(element);
    if (!cachedImage || !cachedImage->imageForRenderer(element->renderer()) || !cachedImage->isLoaded())
        return;

    SharedBuffer* imageBuffer = cachedImage->imageForRenderer(element->renderer())->data();
    if (!imageBuffer || !imageBuffer->size())
        return;

    dataObject->setFileContent(imageBuffer);

    // Determine the filename for the file contents of the image.
    String filename = cachedImage->response().suggestedFilename();
    if (filename.isEmpty())
        filename = url.lastPathComponent();
    if (filename.isEmpty())
        filename = element->getAttribute(altAttr);
    else {
        // Strip any existing extension. Assume that alt text is usually not a filename.
        int extensionIndex = filename.reverseFind('.');
        if (extensionIndex != -1)
            filename.truncate(extensionIndex);
    }

    String extension = MIMETypeRegistry::getPreferredExtensionForMIMEType(
        cachedImage->response().mimeType());
    extension = extension.isEmpty() ? emptyString() : "." + extension;

    ClipboardChromium::validateFilename(filename, extension);

    dataObject->setFileContentFilename(filename + extension);
    dataObject->setFileExtension(extension);
}

void ClipboardChromium::declareAndWriteDragImage(Element* element, const KURL& url, const String& title, Frame* frame)
{
    if (!m_dataObject)
        return;

    m_dragStorageUpdated = true;
    m_dataObject->setData(mimeTypeURL, url);
    m_dataObject->setUrlTitle(title);

    // Write the bytes in the image to the file format.
    writeImageToDataObject(m_dataObject.get(), element, url);

    // Put img tag on the clipboard referencing the image
    m_dataObject->setData(mimeTypeTextHTML, createMarkup(element, IncludeNode, 0, ResolveAllURLs));
}

void ClipboardChromium::writeURL(const KURL& url, const String& title, Frame*)
{
    if (!m_dataObject)
        return;
    ASSERT(!url.isEmpty());

    m_dragStorageUpdated = true;
    m_dataObject->setData(mimeTypeURL, url);
    m_dataObject->setUrlTitle(title);

    // The URL can also be used as plain text.
    m_dataObject->setData(mimeTypeTextPlain, url.string());

    // The URL can also be used as an HTML fragment.
    m_dataObject->setData(mimeTypeTextHTML, urlToMarkup(url, title));
    m_dataObject->setHtmlBaseUrl(url);
}

void ClipboardChromium::writeRange(Range* selectedRange, Frame* frame)
{
    ASSERT(selectedRange);
    if (!m_dataObject)
         return;

    m_dragStorageUpdated = true;
    m_dataObject->setData(mimeTypeTextHTML, createMarkup(selectedRange, 0, AnnotateForInterchange, false, ResolveNonLocalURLs));
    m_dataObject->setHtmlBaseUrl(frame->document()->url());

    String str = frame->editor()->selectedText();
#if OS(WINDOWS)
    replaceNewlinesWithWindowsStyleNewlines(str);
#endif
    replaceNBSPWithSpace(str);
    m_dataObject->setData(mimeTypeTextPlain, str);
}

void ClipboardChromium::writePlainText(const String& text)
{
    if (!m_dataObject)
        return;

    String str = text;
#if OS(WINDOWS)
    replaceNewlinesWithWindowsStyleNewlines(str);
#endif
    replaceNBSPWithSpace(str);

    m_dragStorageUpdated = true;
    m_dataObject->setData(mimeTypeTextPlain, str);
}

bool ClipboardChromium::hasData()
{
    ASSERT(isForDragAndDrop());
    if (!m_dataObject)
        return false;

    return m_dataObject->hasData();
}

#if ENABLE(DATA_TRANSFER_ITEMS)
PassRefPtr<DataTransferItemList> ClipboardChromium::items()
{
    if (!m_dataObject)
        // Return an unassociated empty list.
        return DataTransferItemListChromium::create(this, m_frame->document()->scriptExecutionContext());

    if (!m_itemList)
        m_itemList = DataTransferItemListChromium::create(this, m_frame->document()->scriptExecutionContext());

    // FIXME: According to the spec, we are supposed to return the same collection of items each
    // time. We now return a wrapper that always wraps the *same* set of items, so JS shouldn't be
    // able to tell, but we probably still want to fix this.
    return DataTransferItemListPolicyWrapper::create(this, m_itemList);
}

// FIXME: integrate ChromiumDataObject and DataTransferItemList rather than holding them separately and keeping them synced.
void ClipboardChromium::mayUpdateItems(Vector<RefPtr<DataTransferItem> >& items)
{
    if (!items.isEmpty() && !storageHasUpdated())
        return;

    items.clear();

    ScriptExecutionContext* scriptExecutionContext = m_frame->document()->scriptExecutionContext();

    if (isForCopyAndPaste() && policy() == ClipboardReadable) {
        // Iterate through the types and add them.
        HashSet<String> types = m_dataObject->types();
        for (HashSet<String>::const_iterator it = types.begin(); it != types.end(); ++it)
            items.append(DataTransferItemChromium::createFromPasteboard(this, scriptExecutionContext, *it));
        return;
    }

    bool success = false;
    String plainText = m_dataObject->getData(mimeTypeTextPlain, success);
    if (success)
        items.append(DataTransferItemChromium::create(this, scriptExecutionContext, plainText, mimeTypeTextPlain));

    success = false;
    String htmlText = m_dataObject->getData(mimeTypeTextHTML, success);
    if (success)
        items.append(DataTransferItemChromium::create(this, scriptExecutionContext, htmlText, mimeTypeTextHTML));

    if (m_dataObject->containsFilenames()) {
        const Vector<String>& filenames = m_dataObject->filenames();
        for (Vector<String>::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
            items.append(DataTransferItemChromium::create(this, scriptExecutionContext, File::create(*it)));
    }
    m_dragStorageUpdated = false;
}

bool ClipboardChromium::storageHasUpdated() const
{
    return (isForCopyAndPaste() && platformClipboardChanged()) || (isForDragAndDrop() && m_dragStorageUpdated);
}

#endif

} // namespace WebCore
