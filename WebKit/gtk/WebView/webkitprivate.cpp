/*
 * Copyright (C) 2007 Holger Hans Peter Freyther
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

#include "webkitprivate.h"
#include "ChromeClientGtk.h"
#include "FrameLoader.h"
#include "FrameLoaderClientGtk.h"
#include "NotImplemented.h"

using namespace WebCore;

namespace WebKit {
void apply(WebKitSettings*, WebCore::Settings*)
{
    notImplemented();
}

WebKitSettings* create(WebCore::Settings*)
{
    notImplemented();
    return 0;
}

WebKitWebFrame* getFrameFromView(WebKitWebView* webView)
{
    return webkit_web_view_get_main_frame(webView);
}

WebKitWebView* getViewFromFrame(WebKitWebFrame* frame)
{
    return webkit_web_frame_get_web_view(frame);
}

WebCore::Frame* core(WebKitWebFrame* frame)
{
    if (!frame)
        return 0;

    WebKitWebFramePrivate* frame_data = WEBKIT_WEB_FRAME_GET_PRIVATE(frame);
    return frame_data ? frame_data->frame : 0;
}

WebKitWebFrame* kit(WebCore::Frame* coreFrame)
{
    if (!coreFrame)
        return 0;

    ASSERT(coreFrame->loader());
    WebKit::FrameLoaderClient* client = static_cast<WebKit::FrameLoaderClient*>(coreFrame->loader()->client());
    return client ? client->webFrame() : 0;
}

WebCore::Page* core(WebKitWebView* webView)
{
    if (!webView)
        return 0;

    WebKitWebViewPrivate* webViewData = WEBKIT_WEB_VIEW_GET_PRIVATE(webView);
    return webViewData ? webViewData->corePage : 0;
}

WebKitWebView* kit(WebCore::Page* corePage)
{
    if (!corePage)
        return 0;

    ASSERT(corePage->chrome());
    WebKit::ChromeClient* client = static_cast<WebKit::ChromeClient*>(corePage->chrome()->client());
    return client ? client->webView() : 0;
}
}
