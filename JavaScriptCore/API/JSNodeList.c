// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#include "JSNode.h"
#include "JSNodeList.h"
#include "UnusedParam.h"

static JSValueRef JSNodeListPrototype_item(JSContextRef context, JSObjectRef object, JSObjectRef thisObject, size_t argc, JSValueRef argv[], JSValueRef* exception)
{
    if (argc > 0) {
        NodeList* nodeList = JSObjectGetPrivate(thisObject);
        assert(nodeList);
        Node* node = NodeList_item(nodeList, JSValueToNumber(context, argv[0], exception));
        if (node)
            return JSNode_new(context, node);
    }
    
    return JSValueMakeUndefined();
}

static JSStaticFunction JSNodeListPrototype_staticFunctions[] = {
    { "item", JSNodeListPrototype_item, kJSPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

static JSClassRef JSNodeListPrototype_class(JSContextRef context)
{
    static JSClassRef jsClass;
    if (!jsClass) {
        jsClass = JSClassCreate(NULL, JSNodeListPrototype_staticFunctions, &kJSObjectCallbacksNone, NULL);
    }
    
    return jsClass;
}

static JSValueRef JSNodeList_length(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
    UNUSED_PARAM(context);
    
    NodeList* nodeList = JSObjectGetPrivate(thisObject);
    assert(nodeList);
    return JSValueMakeNumber(NodeList_length(nodeList));
}

static JSStaticValue JSNodeList_staticValues[] = {
    { "length", JSNodeList_length, NULL, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { 0, 0, 0, 0 }
};

static JSValueRef JSNodeList_getProperty(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
    NodeList* nodeList = JSObjectGetPrivate(thisObject);
    assert(nodeList);
    double index = JSValueToNumber(context, JSValueMakeString(propertyName), exception);
    unsigned uindex = index;
    if (uindex == index) { // false for NaN
        Node* node = NodeList_item(nodeList, uindex);
        if (node) {
            return JSNode_new(context, node);
        }
    }
    
    return NULL;
}

static void JSNodeList_finalize(JSObjectRef thisObject)
{
    NodeList* nodeList = JSObjectGetPrivate(thisObject);
    assert(nodeList);
    NodeList_deref(nodeList);
}

static JSClassRef JSNodeList_class(JSContextRef context)
{
    static JSClassRef jsClass;
    if (!jsClass) {
        JSObjectCallbacks callbacks = kJSObjectCallbacksNone;
        callbacks.getProperty = JSNodeList_getProperty;
        callbacks.finalize = JSNodeList_finalize;
        
        jsClass = JSClassCreate(JSNodeList_staticValues, NULL, &callbacks, NULL);
    }
    
    return jsClass;
}

static JSObjectRef JSNodeList_prototype(JSContextRef context)
{
    static JSObjectRef prototype;
    if (!prototype) {
        prototype = JSObjectMake(context, JSNodeListPrototype_class(context), NULL);
        JSValueProtect(prototype);
    }
    return prototype;
}

JSObjectRef JSNodeList_new(JSContextRef context, NodeList* nodeList)
{
    NodeList_ref(nodeList);
    
    JSObjectRef jsNodeList = JSObjectMake(context, JSNodeList_class(context), JSNodeList_prototype(context));
    JSObjectSetPrivate(jsNodeList, nodeList);
    return jsNodeList;
}
