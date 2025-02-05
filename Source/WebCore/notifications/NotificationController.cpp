/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
#include "NotificationController.h"

#if ENABLE(NOTIFICATIONS)

#include "NotificationPresenter.h"
#include <wtf/PassOwnPtr.h>

namespace WebCore {

NotificationController::NotificationController(Page* page, NotificationPresenter* client)
    : m_page(page)
    , m_client(client)
{
}

NotificationController::~NotificationController()
{
    if (m_client)
        m_client->notificationControllerDestroyed();
}

PassOwnPtr<NotificationController> NotificationController::create(Page* page, NotificationPresenter* client)
{
    return adoptPtr(new NotificationController(page, client));
}

NotificationPresenter* NotificationController::clientFrom(Page* page)
{
    if (NotificationController* controller = NotificationController::from(page))
        return controller->client();
    return 0;
}

const AtomicString& NotificationController::supplementName()
{
    DEFINE_STATIC_LOCAL(AtomicString, name, ("NotificationController"));
    return name;
}

void provideNotification(Page* page, NotificationPresenter* client)
{
    NotificationController::provideTo(page, NotificationController::supplementName(), NotificationController::create(page, client));
}

} // namespace WebCore

#endif // ENABLE(NOTIFICATIONS)
