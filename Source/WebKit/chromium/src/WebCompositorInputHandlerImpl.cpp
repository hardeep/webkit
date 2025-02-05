/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "WebCompositorInputHandlerImpl.h"

#include "WebCompositorImpl.h"
#include "WebCompositorInputHandlerClient.h"
#include "WebInputEvent.h"
#include "WebKit.h"
#include "platform/WebKitPlatformSupport.h"
#include "cc/CCProxy.h"
#include <wtf/ThreadingPrimitives.h>

using namespace WebCore;

namespace WebCore {

PassOwnPtr<CCInputHandler> CCInputHandler::create(CCInputHandlerClient* inputHandlerClient)
{
    return WebKit::WebCompositorInputHandlerImpl::create(inputHandlerClient);
}

}

namespace WebKit {

// These statics may only be accessed from the compositor thread.
int WebCompositorInputHandlerImpl::s_nextAvailableIdentifier = 1;
HashSet<WebCompositorInputHandlerImpl*>* WebCompositorInputHandlerImpl::s_compositors = 0;

WebCompositor* WebCompositorInputHandler::fromIdentifier(int identifier)
{
    return static_cast<WebCompositor*>(WebCompositorInputHandlerImpl::fromIdentifier(identifier));
}

PassOwnPtr<WebCompositorInputHandlerImpl> WebCompositorInputHandlerImpl::create(WebCore::CCInputHandlerClient* inputHandlerClient)
{
    return adoptPtr(new WebCompositorInputHandlerImpl(inputHandlerClient));
}

WebCompositorInputHandler* WebCompositorInputHandlerImpl::fromIdentifier(int identifier)
{
    ASSERT(WebCompositorImpl::initialized());
    ASSERT(CCProxy::isImplThread());

    if (!s_compositors)
        return 0;

    for (HashSet<WebCompositorInputHandlerImpl*>::iterator it = s_compositors->begin(); it != s_compositors->end(); ++it) {
        if ((*it)->identifier() == identifier)
            return *it;
    }
    return 0;
}

WebCompositorInputHandlerImpl::WebCompositorInputHandlerImpl(CCInputHandlerClient* inputHandlerClient)
    : m_client(0)
    , m_identifier(s_nextAvailableIdentifier++)
    , m_inputHandlerClient(inputHandlerClient)
#ifndef NDEBUG
    , m_expectScrollUpdateEnd(false)
    , m_expectPinchUpdateEnd(false)
#endif
    , m_scrollStarted(false)
{
    ASSERT(CCProxy::isImplThread());

    if (!s_compositors)
        s_compositors = new HashSet<WebCompositorInputHandlerImpl*>;
    s_compositors->add(this);
}

WebCompositorInputHandlerImpl::~WebCompositorInputHandlerImpl()
{
    ASSERT(CCProxy::isImplThread());
    if (m_client)
        m_client->willShutdown();

    ASSERT(s_compositors);
    s_compositors->remove(this);
    if (!s_compositors->size()) {
        delete s_compositors;
        s_compositors = 0;
    }
}

void WebCompositorInputHandlerImpl::setClient(WebCompositorInputHandlerClient* client)
{
    ASSERT(CCProxy::isImplThread());
    // It's valid to set a new client if we've never had one or to clear the client, but it's not valid to change from having one client to a different one.
    ASSERT(!m_client || !client);
    m_client = client;
}

void WebCompositorInputHandlerImpl::handleInputEvent(const WebInputEvent& event)
{
    ASSERT(CCProxy::isImplThread());
    ASSERT(m_client);

    if (event.type == WebInputEvent::MouseWheel) {
        const WebMouseWheelEvent& wheelEvent = *static_cast<const WebMouseWheelEvent*>(&event);
        CCInputHandlerClient::ScrollStatus scrollStatus = m_inputHandlerClient->scrollBegin(IntPoint(wheelEvent.x, wheelEvent.y), CCInputHandlerClient::Wheel);
        switch (scrollStatus) {
        case CCInputHandlerClient::ScrollStarted:
            m_inputHandlerClient->scrollBy(IntSize(-wheelEvent.deltaX, -wheelEvent.deltaY));
            m_inputHandlerClient->scrollEnd();
            m_client->didHandleInputEvent();
            return;
        case CCInputHandlerClient::ScrollIgnored:
            m_client->didNotHandleInputEvent(false /* sendToWidget */);
            return;
        case CCInputHandlerClient::ScrollFailed:
            break;
        }
    } else if (event.type == WebInputEvent::GestureScrollBegin) {
        ASSERT(!m_scrollStarted);
        ASSERT(!m_expectScrollUpdateEnd);
#ifndef NDEBUG
        m_expectScrollUpdateEnd = true;
#endif
        const WebGestureEvent& gestureEvent = *static_cast<const WebGestureEvent*>(&event);
        CCInputHandlerClient::ScrollStatus scrollStatus = m_inputHandlerClient->scrollBegin(IntPoint(gestureEvent.x, gestureEvent.y), CCInputHandlerClient::Gesture);
        switch (scrollStatus) {
        case CCInputHandlerClient::ScrollStarted:
            m_scrollStarted = true;
            m_client->didHandleInputEvent();
            return;
        case CCInputHandlerClient::ScrollIgnored:
            m_client->didNotHandleInputEvent(false /* sendToWidget */);
            return;
        case CCInputHandlerClient::ScrollFailed:
            break;
        }
    } else if (event.type == WebInputEvent::GestureScrollUpdate) {
        ASSERT(m_expectScrollUpdateEnd);
        if (m_scrollStarted) {
            const WebGestureEvent& gestureEvent = *static_cast<const WebGestureEvent*>(&event);
            m_inputHandlerClient->scrollBy(IntSize(-gestureEvent.deltaX, -gestureEvent.deltaY));
            m_client->didHandleInputEvent();
            return;
        }
    } else if (event.type == WebInputEvent::GestureScrollEnd) {
        ASSERT(m_expectScrollUpdateEnd);
#ifndef NDEBUG
        m_expectScrollUpdateEnd = false;
#endif
        if (m_scrollStarted) {
            m_inputHandlerClient->scrollEnd();
            m_client->didHandleInputEvent();
            m_scrollStarted = false;
            return;
        }
    } else if (event.type == WebInputEvent::GesturePinchBegin) {
        ASSERT(!m_expectPinchUpdateEnd);
#ifndef NDEBUG
        m_expectPinchUpdateEnd = true;
#endif
        m_inputHandlerClient->pinchGestureBegin();
        m_client->didHandleInputEvent();
        return;
    } else if (event.type == WebInputEvent::GesturePinchEnd) {
        ASSERT(m_expectPinchUpdateEnd);
#ifndef NDEBUG
        m_expectPinchUpdateEnd = false;
#endif
        m_inputHandlerClient->pinchGestureEnd();
        m_client->didHandleInputEvent();
        return;
    } else if (event.type == WebInputEvent::GesturePinchUpdate) {
        ASSERT(m_expectPinchUpdateEnd);
        const WebGestureEvent& gestureEvent = *static_cast<const WebGestureEvent*>(&event);
        m_inputHandlerClient->pinchGestureUpdate(gestureEvent.deltaX, IntPoint(gestureEvent.x, gestureEvent.y));
        m_client->didHandleInputEvent();
        return;
    }
    m_client->didNotHandleInputEvent(true /* sendToWidget */);
}

int WebCompositorInputHandlerImpl::identifier() const
{
    ASSERT(CCProxy::isImplThread());
    return m_identifier;
}

void WebCompositorInputHandlerImpl::willDraw(double frameBeginTimeMs)
{
}

}
