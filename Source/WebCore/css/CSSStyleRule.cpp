/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2005, 2006, 2008 Apple Inc. All rights reserved.
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
#include "CSSStyleRule.h"

#include "CSSParser.h"
#include "CSSSelector.h"
#include "CSSStyleSheet.h"
#include "Document.h"
#include "StylePropertySet.h"
#include "StyleRule.h"
#include <wtf/text/StringBuilder.h>

namespace WebCore {

CSSStyleRule::CSSStyleRule(CSSStyleSheet* parent, int line)
    : CSSRule(parent, CSSRule::STYLE_RULE)
    , m_styleRule(adoptPtr(new StyleRule(line, this)))
{
}

CSSStyleRule::~CSSStyleRule()
{
    if (m_styleRule->properties())
        m_styleRule->properties()->clearParentRule(this);
    cleanup();
}

CSSStyleDeclaration* CSSStyleRule::style() const
{
    return m_styleRule->properties()->ensureRuleCSSStyleDeclaration(this);
}

typedef HashMap<const CSSStyleRule*, String> SelectorTextCache;
static SelectorTextCache& selectorTextCache()
{
    DEFINE_STATIC_LOCAL(SelectorTextCache, cache, ());
    return cache;
}

inline void CSSStyleRule::cleanup()
{
    if (hasCachedSelectorText()) {
        selectorTextCache().remove(this);
        setHasCachedSelectorText(false);
    }
}

String CSSStyleRule::generateSelectorText() const
{
    StringBuilder builder;
    for (CSSSelector* s = m_styleRule->selectorList().first(); s; s = CSSSelectorList::next(s)) {
        if (s != m_styleRule->selectorList().first())
            builder.append(", ");
        builder.append(s->selectorText());
    }
    return builder.toString();
}

String CSSStyleRule::selectorText() const
{
    if (hasCachedSelectorText()) {
        ASSERT(selectorTextCache().contains(this));
        return selectorTextCache().get(this);
    }

    ASSERT(!selectorTextCache().contains(this));
    String text = generateSelectorText();
    selectorTextCache().set(this, text);
    setHasCachedSelectorText(true);
    return text;
}

void CSSStyleRule::setSelectorText(const String& selectorText)
{
    Document* doc = 0;
    if (CSSStyleSheet* styleSheet = parentStyleSheet())
        doc = styleSheet->findDocument();
    if (!doc)
        return;

    CSSParser p;
    CSSSelectorList selectorList;
    p.parseSelector(selectorText, doc, selectorList);
    if (!selectorList.first())
        return;

    String oldSelectorText = this->selectorText();
    m_styleRule->adoptSelectorList(selectorList);

    if (hasCachedSelectorText()) {
        ASSERT(selectorTextCache().contains(this));
        selectorTextCache().set(this, generateSelectorText());
    }

    if (this->selectorText() == oldSelectorText)
        return;

    doc->styleSelectorChanged(DeferRecalcStyle);
}

String CSSStyleRule::cssText() const
{
    String result = selectorText();

    result += " { ";
    result += m_styleRule->properties()->asText();
    result += "}";

    return result;
}

} // namespace WebCore
