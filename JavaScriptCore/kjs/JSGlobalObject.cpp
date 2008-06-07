/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron Zwarich (cwzwarich@uwaterloo.ca)
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
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
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
#include "JSGlobalObject.h"

#include "CodeBlock.h"
#include "array_object.h"
#include "bool_object.h"
#include "date_object.h"
#include "debugger.h"
#include "error_object.h"
#include "function_object.h"
#include "math_object.h"
#include "number_object.h"
#include "object_object.h"
#include "regexp_object.h"
#include "scope_chain_mark.h"
#include "string_object.h"

#if HAVE(SYS_TIME_H)
#include <sys/time.h>
#endif

#if PLATFORM(WIN_OS)
#include <windows.h>
#endif

#if PLATFORM(QT)
#include <QDateTime>
#endif

namespace KJS {

// Default number of ticks before a timeout check should be done.
static const int initialTickCountThreshold = 255;

// Preferred number of milliseconds between each timeout check
static const int preferredScriptCheckTimeInterval = 1000;

static inline void markIfNeeded(JSValue* v)
{
    if (v && !v->marked())
        v->mark();
}
    
// Returns the current time in milliseconds
// It doesn't matter what "current time" is here, just as long as
// it's possible to measure the time difference correctly.
static inline unsigned getCurrentTime()
{
#if HAVE(SYS_TIME_H)
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#elif PLATFORM(QT)
    QDateTime t = QDateTime::currentDateTime();
    return t.toTime_t() * 1000 + t.time().msec();
#elif PLATFORM(WIN_OS)
    return timeGetTime();
#else
#error Platform does not have getCurrentTime function
#endif
}

JSGlobalObject* JSGlobalObject::s_head = 0;

JSGlobalObject::~JSGlobalObject()
{
    ASSERT(JSLock::currentThreadIsHoldingLock());

    if (d()->debugger)
        d()->debugger->detach(this);

    d()->next->d()->prev = d()->prev;
    d()->prev->d()->next = d()->next;
    s_head = d()->next;
    if (s_head == this)
        s_head = 0;

    HashSet<ProgramCodeBlock*>::const_iterator end = codeBlocks().end();
    for (HashSet<ProgramCodeBlock*>::const_iterator it = codeBlocks().begin(); it != end; ++it)
        (*it)->globalObject = 0;
    
    delete d();
}

void JSGlobalObject::init(JSObject* thisValue)
{
    ASSERT(JSLock::currentThreadIsHoldingLock());

    if (s_head) {
        d()->prev = s_head;
        d()->next = s_head->d()->next;
        s_head->d()->next->d()->prev = this;
        s_head->d()->next = this;
    } else
        s_head = d()->next = d()->prev = this;

    resetTimeoutCheck();
    d()->timeoutTime = 0;
    d()->timeoutCheckCount = 0;

    d()->recursion = 0;
    d()->debugger = 0;
    
    d()->globalData = &JSGlobalData::threadInstance();

    d()->globalExec.set(new ExecState(this, thisValue, d()->globalScopeChain.node()));

    d()->pageGroupIdentifier = 0;

    reset(prototype());
}

void JSGlobalObject::put(ExecState* exec, const Identifier& propertyName, JSValue* value)
{
    if (symbolTablePut(propertyName, value))
        return;
    return JSVariableObject::put(exec, propertyName, value);
}

void JSGlobalObject::putWithAttributes(ExecState* exec, const Identifier& propertyName, JSValue* value, unsigned attributes)
{
    if (symbolTablePutWithAttributes(propertyName, value, attributes))
        return;

    JSValue* valueBefore = getDirect(propertyName);
    JSVariableObject::put(exec, propertyName, value);
    if (!valueBefore) {
        if (JSValue* valueAfter = getDirect(propertyName))
            putDirect(propertyName, valueAfter, attributes);
    }
}

void JSGlobalObject::defineGetter(ExecState* exec, const Identifier& propertyName, JSObject* getterFunc)
{
    PropertySlot slot;
    if (!symbolTableGet(propertyName, slot))
        JSVariableObject::defineGetter(exec, propertyName, getterFunc);
}

void JSGlobalObject::defineSetter(ExecState* exec, const Identifier& propertyName, JSObject* setterFunc)
{
    PropertySlot slot;
    if (!symbolTableGet(propertyName, slot))
        JSVariableObject::defineSetter(exec, propertyName, setterFunc);
}

static inline JSObject* lastInPrototypeChain(JSObject* object)
{
    JSObject* o = object;
    while (o->prototype()->isObject())
        o = static_cast<JSObject*>(o->prototype());
    return o;
}

void JSGlobalObject::reset(JSValue* prototype)
{
    // Clear before inititalizing, to avoid calling mark() on stale pointers --
    // which would be wasteful -- or uninitialized pointers -- which would be
    // dangerous. (The allocations below may cause a GC.)

    _prop.clear();
    registerFileStack().current()->clear();
    registerFileStack().current()->addGlobalSlots(1);
    symbolTable().clear();

    // Prototypes
    d()->functionPrototype = 0;
    d()->objectPrototype = 0;

    d()->arrayPrototype = 0;
    d()->stringPrototype = 0;
    d()->booleanPrototype = 0;
    d()->numberPrototype = 0;
    d()->datePrototype = 0;
    d()->regExpPrototype = 0;
    d()->errorPrototype = 0;
    
    d()->evalErrorPrototype = 0;
    d()->rangeErrorPrototype = 0;
    d()->referenceErrorPrototype = 0;
    d()->syntaxErrorPrototype = 0;
    d()->typeErrorPrototype = 0;
    d()->URIErrorPrototype = 0;

    // Constructors
    d()->objectConstructor = 0;
    d()->functionConstructor = 0;
    d()->arrayConstructor = 0;
    d()->stringConstructor = 0;
    d()->booleanConstructor = 0;
    d()->numberConstructor = 0;
    d()->dateConstructor = 0;
    d()->regExpConstructor = 0;
    d()->errorConstructor = 0;
    
    d()->evalErrorConstructor = 0;
    d()->rangeErrorConstructor = 0;
    d()->referenceErrorConstructor = 0;
    d()->syntaxErrorConstructor = 0;
    d()->typeErrorConstructor = 0;
    d()->URIErrorConstructor = 0;

    d()->evalFunction = 0;

    ExecState* exec = d()->globalExec.get();

    // Prototypes
    d()->functionPrototype = new FunctionPrototype(exec);
    d()->objectPrototype = new ObjectPrototype(exec, d()->functionPrototype);
    d()->functionPrototype->setPrototype(d()->objectPrototype);

    d()->arrayPrototype = new ArrayPrototype(exec, d()->objectPrototype);
    d()->stringPrototype = new StringPrototype(exec, d()->objectPrototype);
    d()->booleanPrototype = new BooleanPrototype(exec, d()->objectPrototype, d()->functionPrototype);
    d()->numberPrototype = new NumberPrototype(exec, d()->objectPrototype, d()->functionPrototype);
    d()->datePrototype = new DatePrototype(exec, d()->objectPrototype);
    d()->regExpPrototype = new RegExpPrototype(exec, d()->objectPrototype, d()->functionPrototype);
    d()->errorPrototype = new ErrorPrototype(exec, d()->objectPrototype, d()->functionPrototype);
    
    d()->evalErrorPrototype = new NativeErrorPrototype(exec, d()->errorPrototype, "EvalError", "EvalError");
    d()->rangeErrorPrototype = new NativeErrorPrototype(exec, d()->errorPrototype, "RangeError", "RangeError");
    d()->referenceErrorPrototype = new NativeErrorPrototype(exec, d()->errorPrototype, "ReferenceError", "ReferenceError");
    d()->syntaxErrorPrototype = new NativeErrorPrototype(exec, d()->errorPrototype, "SyntaxError", "SyntaxError");
    d()->typeErrorPrototype = new NativeErrorPrototype(exec, d()->errorPrototype, "TypeError", "TypeError");
    d()->URIErrorPrototype = new NativeErrorPrototype(exec, d()->errorPrototype, "URIError", "URIError");

    // Constructors
    d()->objectConstructor = new ObjectObjectImp(exec, d()->objectPrototype, d()->functionPrototype);
    d()->functionConstructor = new FunctionObjectImp(exec, d()->functionPrototype);
    d()->arrayConstructor = new ArrayObjectImp(exec, d()->functionPrototype, d()->arrayPrototype);
    d()->stringConstructor = new StringObjectImp(exec, d()->functionPrototype, d()->stringPrototype);
    d()->booleanConstructor = new BooleanObjectImp(exec, d()->functionPrototype, d()->booleanPrototype);
    d()->numberConstructor = new NumberObjectImp(exec, d()->functionPrototype, d()->numberPrototype);
    d()->dateConstructor = new DateObjectImp(exec, d()->functionPrototype, d()->datePrototype);
    d()->regExpConstructor = new RegExpObjectImp(exec, d()->functionPrototype, d()->regExpPrototype);
    d()->errorConstructor = new ErrorObjectImp(exec, d()->functionPrototype, d()->errorPrototype);
    
    d()->evalErrorConstructor = new NativeErrorImp(exec, d()->functionPrototype, d()->evalErrorPrototype);
    d()->rangeErrorConstructor = new NativeErrorImp(exec, d()->functionPrototype, d()->rangeErrorPrototype);
    d()->referenceErrorConstructor = new NativeErrorImp(exec, d()->functionPrototype, d()->referenceErrorPrototype);
    d()->syntaxErrorConstructor = new NativeErrorImp(exec, d()->functionPrototype, d()->syntaxErrorPrototype);
    d()->typeErrorConstructor = new NativeErrorImp(exec, d()->functionPrototype, d()->typeErrorPrototype);
    d()->URIErrorConstructor = new NativeErrorImp(exec, d()->functionPrototype, d()->URIErrorPrototype);
    
    d()->functionPrototype->putDirect(exec->propertyNames().constructor, d()->functionConstructor, DontEnum);

    d()->objectPrototype->putDirect(exec->propertyNames().constructor, d()->objectConstructor, DontEnum);
    d()->functionPrototype->putDirect(exec->propertyNames().constructor, d()->functionConstructor, DontEnum);
    d()->arrayPrototype->putDirect(exec->propertyNames().constructor, d()->arrayConstructor, DontEnum);
    d()->booleanPrototype->putDirect(exec->propertyNames().constructor, d()->booleanConstructor, DontEnum);
    d()->stringPrototype->putDirect(exec->propertyNames().constructor, d()->stringConstructor, DontEnum);
    d()->numberPrototype->putDirect(exec->propertyNames().constructor, d()->numberConstructor, DontEnum);
    d()->datePrototype->putDirect(exec->propertyNames().constructor, d()->dateConstructor, DontEnum);
    d()->regExpPrototype->putDirect(exec->propertyNames().constructor, d()->regExpConstructor, DontEnum);
    d()->errorPrototype->putDirect(exec->propertyNames().constructor, d()->errorConstructor, DontEnum);
    d()->evalErrorPrototype->putDirect(exec->propertyNames().constructor, d()->evalErrorConstructor, DontEnum);
    d()->rangeErrorPrototype->putDirect(exec->propertyNames().constructor, d()->rangeErrorConstructor, DontEnum);
    d()->referenceErrorPrototype->putDirect(exec->propertyNames().constructor, d()->referenceErrorConstructor, DontEnum);
    d()->syntaxErrorPrototype->putDirect(exec->propertyNames().constructor, d()->syntaxErrorConstructor, DontEnum);
    d()->typeErrorPrototype->putDirect(exec->propertyNames().constructor, d()->typeErrorConstructor, DontEnum);
    d()->URIErrorPrototype->putDirect(exec->propertyNames().constructor, d()->URIErrorConstructor, DontEnum);

    // Set global constructors

    // FIXME: These properties could be handled by a static hash table.

    putDirect("Object", d()->objectConstructor, DontEnum);
    putDirect("Function", d()->functionConstructor, DontEnum);
    putDirect("Array", d()->arrayConstructor, DontEnum);
    putDirect("Boolean", d()->booleanConstructor, DontEnum);
    putDirect("String", d()->stringConstructor, DontEnum);
    putDirect("Number", d()->numberConstructor, DontEnum);
    putDirect("Date", d()->dateConstructor, DontEnum);
    putDirect("RegExp", d()->regExpConstructor, DontEnum);
    putDirect("Error", d()->errorConstructor, DontEnum);
    putDirect("EvalError", d()->evalErrorConstructor);
    putDirect("RangeError", d()->rangeErrorConstructor);
    putDirect("ReferenceError", d()->referenceErrorConstructor);
    putDirect("SyntaxError", d()->syntaxErrorConstructor);
    putDirect("TypeError", d()->typeErrorConstructor);
    putDirect("URIError", d()->URIErrorConstructor);

    // Set global values.
    GlobalPropertyInfo staticGlobals[] = {
        GlobalPropertyInfo("Math", new MathObjectImp(exec, d()->objectPrototype), DontEnum | DontDelete),
        GlobalPropertyInfo("NaN", jsNaN(), DontEnum | DontDelete),
        GlobalPropertyInfo("Infinity", jsNumber(Inf), DontEnum | DontDelete),
        GlobalPropertyInfo("undefined", jsUndefined(), DontEnum | DontDelete)
    };

    addStaticGlobals(staticGlobals, sizeof(staticGlobals) / sizeof(GlobalPropertyInfo));

    // Set global functions.

    d()->evalFunction = new PrototypeReflexiveFunction(exec, d()->functionPrototype, 1, exec->propertyNames().eval, globalFuncEval, this);
    putDirectFunction(d()->evalFunction, DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 2, "parseInt", globalFuncParseInt), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "parseFloat", globalFuncParseFloat), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "isNaN", globalFuncIsNaN), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "isFinite", globalFuncIsFinite), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "escape", globalFuncEscape), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "unescape", globalFuncUnescape), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "decodeURI", globalFuncDecodeURI), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "decodeURIComponent", globalFuncDecodeURIComponent), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "encodeURI", globalFuncEncodeURI), DontEnum);
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "encodeURIComponent", globalFuncEncodeURIComponent), DontEnum);
#ifndef NDEBUG
    putDirectFunction(new PrototypeFunction(exec, d()->functionPrototype, 1, "kjsprint", globalFuncKJSPrint), DontEnum);
#endif

    // Set prototype, and also insert the object prototype at the end of the chain.

    setPrototype(prototype);
    lastInPrototypeChain(this)->setPrototype(d()->objectPrototype);
}

void JSGlobalObject::startTimeoutCheck()
{
    if (!d()->timeoutCheckCount)
        resetTimeoutCheck();
    
    ++d()->timeoutCheckCount;
}

void JSGlobalObject::stopTimeoutCheck()
{
    --d()->timeoutCheckCount;
}

void JSGlobalObject::resetTimeoutCheck()
{
    d()->tickCount = 0;
    d()->ticksUntilNextTimeoutCheck = initialTickCountThreshold;
    d()->timeAtLastCheckTimeout = 0;
    d()->timeExecuting = 0;
}

bool JSGlobalObject::checkTimeout()
{    
    d()->tickCount = 0;
    
    unsigned currentTime = getCurrentTime();

    if (!d()->timeAtLastCheckTimeout) {
        // Suspicious amount of looping in a script -- start timing it
        d()->timeAtLastCheckTimeout = currentTime;
        return false;
    }

    unsigned timeDiff = currentTime - d()->timeAtLastCheckTimeout;

    if (timeDiff == 0)
        timeDiff = 1;
    
    d()->timeExecuting += timeDiff;
    d()->timeAtLastCheckTimeout = currentTime;
    
    // Adjust the tick threshold so we get the next checkTimeout call in the interval specified in 
    // preferredScriptCheckTimeInterval
    d()->ticksUntilNextTimeoutCheck = (unsigned)((float)preferredScriptCheckTimeInterval / timeDiff) * d()->ticksUntilNextTimeoutCheck;

    // If the new threshold is 0 reset it to the default threshold. This can happen if the timeDiff is higher than the
    // preferred script check time interval.
    if (d()->ticksUntilNextTimeoutCheck == 0)
        d()->ticksUntilNextTimeoutCheck = initialTickCountThreshold;

    if (d()->timeoutTime && d()->timeExecuting > d()->timeoutTime) {
        if (shouldInterruptScript())
            return true;
        
        resetTimeoutCheck();
    }
    
    return false;
}

void JSGlobalObject::mark()
{
    JSVariableObject::mark();
    
    HashSet<ProgramCodeBlock*>::const_iterator end = codeBlocks().end();
    for (HashSet<ProgramCodeBlock*>::const_iterator it = codeBlocks().begin(); it != end; ++it)
        (*it)->mark();

    registerFileStack().mark();

    markIfNeeded(d()->globalExec->exception());

    markIfNeeded(d()->objectConstructor);
    markIfNeeded(d()->functionConstructor);
    markIfNeeded(d()->arrayConstructor);
    markIfNeeded(d()->booleanConstructor);
    markIfNeeded(d()->stringConstructor);
    markIfNeeded(d()->numberConstructor);
    markIfNeeded(d()->dateConstructor);
    markIfNeeded(d()->regExpConstructor);
    markIfNeeded(d()->errorConstructor);
    markIfNeeded(d()->evalErrorConstructor);
    markIfNeeded(d()->rangeErrorConstructor);
    markIfNeeded(d()->referenceErrorConstructor);
    markIfNeeded(d()->syntaxErrorConstructor);
    markIfNeeded(d()->typeErrorConstructor);
    markIfNeeded(d()->URIErrorConstructor);
    
    markIfNeeded(d()->evalFunction);
    
    markIfNeeded(d()->objectPrototype);
    markIfNeeded(d()->functionPrototype);
    markIfNeeded(d()->arrayPrototype);
    markIfNeeded(d()->booleanPrototype);
    markIfNeeded(d()->stringPrototype);
    markIfNeeded(d()->numberPrototype);
    markIfNeeded(d()->datePrototype);
    markIfNeeded(d()->regExpPrototype);
    markIfNeeded(d()->errorPrototype);
    markIfNeeded(d()->evalErrorPrototype);
    markIfNeeded(d()->rangeErrorPrototype);
    markIfNeeded(d()->referenceErrorPrototype);
    markIfNeeded(d()->syntaxErrorPrototype);
    markIfNeeded(d()->typeErrorPrototype);
    markIfNeeded(d()->URIErrorPrototype);
}

JSGlobalObject* JSGlobalObject::toGlobalObject(ExecState*) const
{
    return const_cast<JSGlobalObject*>(this);
}

ExecState* JSGlobalObject::globalExec()
{
    return d()->globalExec.get();
}

bool JSGlobalObject::isDynamicScope() const
{
    return true;
}


} // namespace KJS
