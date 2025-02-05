/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
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

#import <WebCore/DOMObject.h>

#if WEBKIT_VERSION_MAX_ALLOWED >= WEBKIT_VERSION_LATEST

@class DOMDocument;
@class DOMIDBKey;
@class DOMOptionsObject;
@class DOMSVGDocument;
@class DOMSVGPoint;
@class DOMTestObj;
@class DOMTestObjectAConstructor;
@class DOMTestObjectBConstructor;
@class DOMTestObjectCConstructor;
@class DOMa;
@class DOMb;
@class DOMbool;
@class DOMc;
@class DOMd;
@class DOMe;
@class NSString;
@protocol DOMEventListener;

enum {
#if ENABLE(Condition1)
    DOM_CONDITIONAL_CONST = 0,
#endif
    DOM_CONST_VALUE_0 = 0,
    DOM_CONST_VALUE_1 = 1,
    DOM_CONST_VALUE_2 = 2,
    DOM_CONST_VALUE_4 = 4,
    DOM_CONST_VALUE_8 = 8,
    DOM_CONST_VALUE_9 = -1,
    DOM_CONST_VALUE_10 = "my constant string",
    DOM_CONST_VALUE_11 = 0xffffffff,
    DOM_CONST_VALUE_12 = 0x01,
    DOM_CONST_VALUE_13 = 0X20,
    DOM_CONST_VALUE_14 = 0x1abc,
    DOM_CONST_JAVASCRIPT = 15
};

@interface DOMTestObj : DOMObject
- (int)readOnlyIntAttr;
- (NSString *)readOnlyStringAttr;
- (DOMTestObj *)readOnlyTestObjAttr;
- (short)shortAttr;
- (void)setShortAttr:(short)newShortAttr;
- (unsigned short)unsignedShortAttr;
- (void)setUnsignedShortAttr:(unsigned short)newUnsignedShortAttr;
- (int)intAttr;
- (void)setIntAttr:(int)newIntAttr;
- (long long)longLongAttr;
- (void)setLongLongAttr:(long long)newLongLongAttr;
- (unsigned long long)unsignedLongLongAttr;
- (void)setUnsignedLongLongAttr:(unsigned long long)newUnsignedLongLongAttr;
- (NSString *)stringAttr;
- (void)setStringAttr:(NSString *)newStringAttr;
- (DOMTestObj *)testObjAttr;
- (void)setTestObjAttr:(DOMTestObj *)newTestObjAttr;
- (DOMTestObj *)XMLObjAttr;
- (void)setXMLObjAttr:(DOMTestObj *)newXMLObjAttr;
- (BOOL)create;
- (void)setCreate:(BOOL)newCreate;
- (NSString *)reflectedStringAttr;
- (void)setReflectedStringAttr:(NSString *)newReflectedStringAttr;
- (int)reflectedIntegralAttr;
- (void)setReflectedIntegralAttr:(int)newReflectedIntegralAttr;
- (unsigned)reflectedUnsignedIntegralAttr;
- (void)setReflectedUnsignedIntegralAttr:(unsigned)newReflectedUnsignedIntegralAttr;
- (BOOL)reflectedBooleanAttr;
- (void)setReflectedBooleanAttr:(BOOL)newReflectedBooleanAttr;
- (NSString *)reflectedURLAttr;
- (void)setReflectedURLAttr:(NSString *)newReflectedURLAttr;
- (NSString *)reflectedStringAttr;
- (void)setReflectedStringAttr:(NSString *)newReflectedStringAttr;
- (int)reflectedCustomIntegralAttr;
- (void)setReflectedCustomIntegralAttr:(int)newReflectedCustomIntegralAttr;
- (BOOL)reflectedCustomBooleanAttr;
- (void)setReflectedCustomBooleanAttr:(BOOL)newReflectedCustomBooleanAttr;
- (NSString *)reflectedCustomURLAttr;
- (void)setReflectedCustomURLAttr:(NSString *)newReflectedCustomURLAttr;
- (int)attrWithGetterException;
- (void)setAttrWithGetterException:(int)newAttrWithGetterException;
- (int)attrWithSetterException;
- (void)setAttrWithSetterException:(int)newAttrWithSetterException;
- (NSString *)stringAttrWithGetterException;
- (void)setStringAttrWithGetterException:(NSString *)newStringAttrWithGetterException;
- (NSString *)stringAttrWithSetterException;
- (void)setStringAttrWithSetterException:(NSString *)newStringAttrWithSetterException;
- (int)customAttr;
- (void)setCustomAttr:(int)newCustomAttr;
- (int)withScriptStateAttribute;
- (void)setWithScriptStateAttribute:(int)newWithScriptStateAttribute;
- (DOMTestObj *)withScriptExecutionContextAttribute;
- (void)setWithScriptExecutionContextAttribute:(DOMTestObj *)newWithScriptExecutionContextAttribute;
- (DOMTestObj *)withScriptStateAttributeRaises;
- (void)setWithScriptStateAttributeRaises:(DOMTestObj *)newWithScriptStateAttributeRaises;
- (DOMTestObj *)withScriptExecutionContextAttributeRaises;
- (void)setWithScriptExecutionContextAttributeRaises:(DOMTestObj *)newWithScriptExecutionContextAttributeRaises;
- (DOMTestObj *)withScriptExecutionContextAndScriptStateAttribute;
- (void)setWithScriptExecutionContextAndScriptStateAttribute:(DOMTestObj *)newWithScriptExecutionContextAndScriptStateAttribute;
- (DOMTestObj *)withScriptExecutionContextAndScriptStateAttributeRaises;
- (void)setWithScriptExecutionContextAndScriptStateAttributeRaises:(DOMTestObj *)newWithScriptExecutionContextAndScriptStateAttributeRaises;
- (DOMTestObj *)withScriptExecutionContextAndScriptStateWithSpacesAttribute;
- (void)setWithScriptExecutionContextAndScriptStateWithSpacesAttribute:(DOMTestObj *)newWithScriptExecutionContextAndScriptStateWithSpacesAttribute;
- (DOMTestObj *)withScriptArgumentsAndCallStackAttribute;
- (void)setWithScriptArgumentsAndCallStackAttribute:(DOMTestObj *)newWithScriptArgumentsAndCallStackAttribute;
#if ENABLE(Condition1)
- (int)conditionalAttr1;
- (void)setConditionalAttr1:(int)newConditionalAttr1;
#endif
#if ENABLE(Condition1) && ENABLE(Condition2)
- (int)conditionalAttr2;
- (void)setConditionalAttr2:(int)newConditionalAttr2;
#endif
#if ENABLE(Condition1) || ENABLE(Condition2)
- (int)conditionalAttr3;
- (void)setConditionalAttr3:(int)newConditionalAttr3;
#endif
#if ENABLE(Condition1)
- (DOMTestObjectAConstructor *)conditionalAttr4;
- (void)setConditionalAttr4:(DOMTestObjectAConstructor *)newConditionalAttr4;
#endif
#if ENABLE(Condition1) && ENABLE(Condition2)
- (DOMTestObjectBConstructor *)conditionalAttr5;
- (void)setConditionalAttr5:(DOMTestObjectBConstructor *)newConditionalAttr5;
#endif
#if ENABLE(Condition1) || ENABLE(Condition2)
- (DOMTestObjectCConstructor *)conditionalAttr6;
- (void)setConditionalAttr6:(DOMTestObjectCConstructor *)newConditionalAttr6;
#endif
- (DOMDocument *)contentDocument;
- (DOMSVGPoint *)mutablePoint;
- (void)setMutablePoint:(DOMSVGPoint *)newMutablePoint;
- (DOMSVGPoint *)immutablePoint;
- (void)setImmutablePoint:(DOMSVGPoint *)newImmutablePoint;
- (float)strictFloat;
- (void)setStrictFloat:(float)newStrictFloat;
- (int)descriptionName;
- (int)idName;
- (void)setIdName:(int)newIdName;
- (NSString *)hashName;
- (void)voidMethod;
- (void)voidMethodWithArgs:(int)intArg strArg:(NSString *)strArg objArg:(DOMTestObj *)objArg;
- (int)intMethod;
- (int)intMethodWithArgs:(int)intArg strArg:(NSString *)strArg objArg:(DOMTestObj *)objArg;
- (DOMTestObj *)objMethod;
- (DOMTestObj *)objMethodWithArgs:(int)intArg strArg:(NSString *)strArg objArg:(DOMTestObj *)objArg;
- (DOMTestObj *)methodThatRequiresAllArgsAndThrows:(NSString *)strArg objArg:(DOMTestObj *)objArg;
- (void)serializedValue:(NSString *)serializedArg;
- (void)idbKey:(DOMIDBKey *)key;
- (void)optionsObject:(DOMOptionsObject *)oo ooo:(DOMOptionsObject *)ooo;
- (void)methodWithException;
- (void)customMethod;
- (void)customMethodWithArgs:(int)intArg strArg:(NSString *)strArg objArg:(DOMTestObj *)objArg;
- (void)addEventListener:(NSString *)type listener:(id <DOMEventListener>)listener useCapture:(BOOL)useCapture;
- (void)removeEventListener:(NSString *)type listener:(id <DOMEventListener>)listener useCapture:(BOOL)useCapture;
- (void)withScriptStateVoid;
- (DOMTestObj *)withScriptStateObj;
- (void)withScriptStateVoidException;
- (DOMTestObj *)withScriptStateObjException;
- (void)withScriptExecutionContext;
- (void)withScriptExecutionContextAndScriptState;
- (DOMTestObj *)withScriptExecutionContextAndScriptStateObjException;
- (DOMTestObj *)withScriptExecutionContextAndScriptStateWithSpaces;
- (void)withScriptArgumentsAndCallStack;
- (void)methodWithOptionalArg:(int)opt;
- (void)methodWithNonOptionalArgAndOptionalArg:(int)nonOpt opt:(int)opt;
- (void)methodWithNonOptionalArgAndTwoOptionalArgs:(int)nonOpt opt1:(int)opt1 opt2:(int)opt2;
- (void)methodWithOptionalString:(NSString *)str;
- (void)methodWithOptionalStringIsUndefined:(NSString *)str;
- (void)methodWithOptionalStringIsNullString:(NSString *)str;
#if ENABLE(Condition1)
- (NSString *)conditionalMethod1;
#endif
#if ENABLE(Condition1) && ENABLE(Condition2)
- (void)conditionalMethod2;
#endif
#if ENABLE(Condition1) || ENABLE(Condition2)
- (void)conditionalMethod3;
#endif
- (void)classMethod;
- (int)classMethodWithOptional:(int)arg;
#if ENABLE(Condition1)
- (void)overloadedMethod1:(int)arg;
#endif
#if ENABLE(Condition1)
- (void)overloadedMethod1:(NSString *)type;
#endif
- (DOMSVGDocument *)getSVGDocument;
- (void)convert1:(DOMa *);
- (void)convert2:(DOMb *);
- (void)convert3:(DOMc *);
- (void)convert4:(DOMd *);
- (void)convert5:(DOMe *);
- (DOMSVGPoint *)mutablePointFunction;
- (DOMSVGPoint *)immutablePointFunction;
- (void)orange;
- (DOMbool *)strictFunction:(NSString *)str a:(float)a b:(int)b;
@end

#endif
