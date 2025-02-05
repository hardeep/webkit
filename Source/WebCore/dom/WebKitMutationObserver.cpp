/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(MUTATION_OBSERVERS)

#include "WebKitMutationObserver.h"

#include "Document.h"
#include "ExceptionCode.h"
#include "MutationCallback.h"
#include "MutationObserverRegistration.h"
#include "MutationRecord.h"
#include "Node.h"
#include <wtf/ListHashSet.h>
#include <wtf/MainThread.h>

namespace WebCore {

PassRefPtr<WebKitMutationObserver> WebKitMutationObserver::create(PassRefPtr<MutationCallback> callback)
{
    return adoptRef(new WebKitMutationObserver(callback));
}

WebKitMutationObserver::WebKitMutationObserver(PassRefPtr<MutationCallback> callback)
    : m_callback(callback)
{
}

WebKitMutationObserver::~WebKitMutationObserver()
{
    ASSERT(m_registrations.isEmpty());
}

bool WebKitMutationObserver::validateOptions(MutationObserverOptions options)
{
    return (options & (Attributes | CharacterData | ChildList))
        && ((options & Attributes) || !(options & AttributeOldValue))
        && ((options & Attributes) || !(options & AttributeFilter))
        && ((options & CharacterData) || !(options & CharacterDataOldValue));
}

void WebKitMutationObserver::observe(Node* node, MutationObserverOptions options, const HashSet<AtomicString>& attributeFilter, ExceptionCode& ec)
{
    if (!node) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (!validateOptions(options)) {
        // FIXME: Revisit this once the spec specifies the exception type; SYNTAX_ERR may not be appropriate.
        ec = SYNTAX_ERR;
        return;
    }

    MutationObserverRegistration* registration = node->registerMutationObserver(this);
    registration->resetObservation(options, attributeFilter);

    node->document()->addMutationObserverTypes(registration->mutationTypes());
}

void WebKitMutationObserver::disconnect()
{
    m_records.clear();
    HashSet<MutationObserverRegistration*> registrations(m_registrations);
    for (HashSet<MutationObserverRegistration*>::iterator iter = registrations.begin(); iter != registrations.end(); ++iter)
        (*iter)->unregister();
}

void WebKitMutationObserver::observationStarted(MutationObserverRegistration* registration)
{
    ASSERT(!m_registrations.contains(registration));
    m_registrations.add(registration);
}

void WebKitMutationObserver::observationEnded(MutationObserverRegistration* registration)
{
    ASSERT(m_registrations.contains(registration));
    m_registrations.remove(registration);
}

typedef ListHashSet<RefPtr<WebKitMutationObserver> > MutationObserverSet;

static MutationObserverSet& activeMutationObservers()
{
    DEFINE_STATIC_LOCAL(MutationObserverSet, activeObservers, ());
    return activeObservers;
}

void WebKitMutationObserver::enqueueMutationRecord(PassRefPtr<MutationRecord> mutation)
{
    ASSERT(isMainThread());
    m_records.append(mutation);
    activeMutationObservers().add(this);
}

void WebKitMutationObserver::deliver()
{
    if (m_records.isEmpty())
        return;

    MutationRecordArray records;
    records.swap(m_records);

    for (HashSet<MutationObserverRegistration*>::iterator iter = m_registrations.begin(); iter != m_registrations.end(); ++iter)
        (*iter)->clearTransientRegistrations();

    m_callback->handleEvent(&records, this);
}

void WebKitMutationObserver::deliverAllMutations()
{
    ASSERT(isMainThread());
    static bool deliveryInProgress = false;
    if (deliveryInProgress)
        return;
    deliveryInProgress = true;

    while (!activeMutationObservers().isEmpty()) {
        MutationObserverSet::iterator iter = activeMutationObservers().begin();
        RefPtr<WebKitMutationObserver> observer = *iter;
        activeMutationObservers().remove(iter);
        observer->deliver();
    }

    deliveryInProgress = false;
}

} // namespace WebCore

#endif // ENABLE(MUTATION_OBSERVERS)
