/**
 * Copyright (C) 2006 Apple Computer, Inc.
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"
#include "PlatformString.h"

namespace WebCore {

String::String(CFStringRef str)
{
    if (!str)
        return;

    CFIndex size = CFStringGetLength(str);
    if (size == 0)
        m_impl = StringImpl::empty();
    else {
        Vector<UChar, 1024> buffer(size);
        CFStringGetCharacters(str, CFRangeMake(0, size), buffer.data());
        m_impl = new StringImpl(buffer.data(), size);
    }
}

String::String(NSString* str)
{
    if (!str)
        return;

    CFIndex size = CFStringGetLength(reinterpret_cast<CFStringRef>(str));
    if (size == 0)
        m_impl = StringImpl::empty();
    else {
        Vector<UChar, 1024> buffer(size);
        CFStringGetCharacters(reinterpret_cast<CFStringRef>(str), CFRangeMake(0, size), buffer.data());
        m_impl = new StringImpl(buffer.data(), size);
    }
}

}
