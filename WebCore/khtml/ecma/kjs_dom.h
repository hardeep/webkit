/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2004, 2005, 2006 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef KJS_DOM_H
#define KJS_DOM_H

#include "kjs_binding.h"

#include <qvaluelist.h>
#include "NodeListImpl.h"
#include "Shared.h"

namespace DOM {
    class AtomicString;
    class AttrImpl;
    class CharacterDataImpl;
    class DocumentTypeImpl;
    class DOMImplementationImpl;
    class ElementImpl;
    class EntityImpl;
    class NamedNodeMapImpl;
    class NotationImpl;
    class ProcessingInstructionImpl;
    class TextImpl;
}

namespace KJS {

  KJS_DEFINE_PROTOTYPE(DOMNodeProto)

  class DOMNode : public DOMObject {
  public:
    DOMNode(ExecState *exec, DOM::NodeImpl *n);
    virtual ~DOMNode();
    virtual bool toBoolean(ExecState *) const;
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    virtual void mark();
    virtual void put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, JSValue *value, int attr);
    DOM::NodeImpl *impl() const { return m_impl.get(); }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual JSValue *toPrimitive(ExecState *exec, JSType preferred = UndefinedType) const;
    virtual UString toString(ExecState *exec) const;
    void setListener(ExecState *exec, const DOM::AtomicString &eventType, JSValue *func) const;
    JSValue *getListener(const DOM::AtomicString &eventType) const;
    virtual void pushEventHandlerScope(ExecState *exec, ScopeChain &scope) const;

    enum { NodeName, NodeValue, NodeType, ParentNode, ParentElement,
           ChildNodes, FirstChild, LastChild, PreviousSibling, NextSibling, Item,
           Attributes, NamespaceURI, Prefix, LocalName, OwnerDocument, InsertBefore,
           ReplaceChild, RemoveChild, AppendChild, HasAttributes, HasChildNodes,
           CloneNode, Normalize, IsSupported, AddEventListener, RemoveEventListener,
           DispatchEvent, Contains, IsSameNode, IsEqualNode, TextContent,
           IsDefaultNamespace, LookupNamespaceURI, LookupPrefix,
           OnAbort, OnBlur, OnChange, OnClick, OnContextMenu, OnDblClick, OnDragDrop, OnError,
           OnDragEnter, OnDragOver, OnDragLeave, OnDrop, OnDragStart, OnDrag, OnDragEnd,
           OnBeforeCut, OnCut, OnBeforeCopy, OnCopy, OnBeforePaste, OnPaste, OnSelectStart,
           OnFocus, OnInput, OnKeyDown, OnKeyPress, OnKeyUp, OnLoad, OnMouseDown,
           OnMouseMove, OnMouseOut, OnMouseOver, OnMouseUp, OnMouseWheel, OnMove, OnReset,
           OnResize, OnScroll, OnSearch, OnSelect, OnSubmit, OnUnload,
           OffsetLeft, OffsetTop, OffsetWidth, OffsetHeight, OffsetParent,
           ClientWidth, ClientHeight, ScrollLeft, ScrollTop, ScrollWidth, ScrollHeight,
           ScrollIntoView, ScrollIntoViewIfNeeded };

  protected:
    // Constructor for inherited classes; doesn't set up a prototype.
    DOMNode(DOM::NodeImpl *n);
    RefPtr<DOM::NodeImpl> m_impl;
  };

  DOM::NodeImpl *toNode(JSValue *); // returns 0 if passed-in value is not a DOMNode object

  class DOMNodeList : public DOMObject {
  public:
    DOMNodeList(ExecState *, DOM::NodeListImpl *l) : m_impl(l) { }
    ~DOMNodeList();
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    virtual JSValue *callAsFunction(ExecState *exec, JSObject *thisObj, const List&args);
    virtual bool implementsCall() const { return true; }
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    enum { Length, Item };
    DOM::NodeListImpl *impl() const { return m_impl.get(); }

    virtual JSValue *toPrimitive(ExecState *exec, JSType preferred = UndefinedType) const;

  private:
    static JSValue *indexGetter(ExecState *exec, JSObject *, const Identifier&, const PropertySlot& slot);
    static JSValue *nameGetter(ExecState *exec, JSObject *, const Identifier&, const PropertySlot& slot);

    RefPtr<DOM::NodeListImpl> m_impl;
  };

  class DOMDocument : public DOMNode {
  public:
    DOMDocument(ExecState *exec, DOM::DocumentImpl *d);
    ~DOMDocument();
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, JSValue *value, int attr);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { DocType, Implementation, DocumentElement, 
           Charset, DefaultCharset, CharacterSet, ActualEncoding, InputEncoding,
           // Functions
           AdoptNode,
           CreateElement, CreateDocumentFragment, CreateTextNode, CreateComment,
           CreateCDATASection, CreateProcessingInstruction, CreateAttribute,
           CreateEntityReference, GetElementsByTagName, ImportNode, CreateElementNS,
           CreateAttributeNS, GetElementsByTagNameNS, GetElementById,
           CreateRange, CreateNodeIterator, CreateTreeWalker, DefaultView,
           CreateEvent, ElementFromPoint, StyleSheets, PreferredStylesheetSet, 
           SelectedStylesheetSet, GetOverrideStyle, ReadyState, 
           ExecCommand, QueryCommandEnabled, QueryCommandIndeterm, QueryCommandState, 
           QueryCommandSupported, QueryCommandValue };

  protected:
    // Constructor for inherited classes; doesn't set up a prototype.
    DOMDocument(DOM::DocumentImpl *d);
  };

  DOM::AttrImpl *toAttr(JSValue *); // returns 0 if passed-in value is not a DOMAttr object

  KJS_DEFINE_PROTOTYPE(DOMElementProto)

  class DOMElement : public DOMNode {
  public:
    DOMElement(ExecState *exec, DOM::ElementImpl *e);
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { TagName, ScrollByLines, ScrollByPages, ScrollIntoView };
  protected:
    // Constructor for inherited classes; doesn't set up a prototype.
    DOMElement(DOM::ElementImpl *e);
  private:
    static JSValue *attributeGetter(ExecState *exec, JSObject *, const Identifier&, const PropertySlot& slot);
  };

  DOM::ElementImpl *toElement(JSValue *); // returns 0 if passed-in value is not a DOMElement object

  DOM::DocumentTypeImpl *toDocumentType(JSValue *); // returns 0 if passed-in value is not a DOMDocumentType object

  class DOMNamedNodeMap : public DOMObject {
  public:
    DOMNamedNodeMap(ExecState *, DOM::NamedNodeMapImpl *m);
    ~DOMNamedNodeMap();
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState *) const { return true; }
    static const ClassInfo info;
    enum { GetNamedItem, SetNamedItem, RemoveNamedItem, Item,
           GetNamedItemNS, SetNamedItemNS, RemoveNamedItemNS };
    DOM::NamedNodeMapImpl *impl() const { return m_impl.get(); }
  private:
    static JSValue *lengthGetter(ExecState* exec, JSObject *, const Identifier&, const PropertySlot& slot);
    static JSValue *indexGetter(ExecState* exec, JSObject *, const Identifier&, const PropertySlot& slot);
    static JSValue *nameGetter(ExecState *exec, JSObject *, const Identifier&, const PropertySlot& slot);

    RefPtr<DOM::NamedNodeMapImpl> m_impl;
  };

  // Constructor for Node - constructor stuff not implemented yet
  class NodeConstructor : public DOMObject {
  public:
    NodeConstructor(ExecState *) { }
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  // Constructor for DOMException - constructor stuff not implemented yet
  class DOMExceptionConstructor : public DOMObject {
  public:
    DOMExceptionConstructor(ExecState *) { }
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    JSValue *getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  JSValue *getDOMDocumentNode(ExecState *exec, DOM::DocumentImpl *n);
  bool checkNodeSecurity(ExecState *exec, DOM::NodeImpl *n);
  JSValue *getRuntimeObject(ExecState *exec, DOM::NodeImpl *n);
  JSValue *getDOMNode(ExecState*, PassRefPtr<DOM::NodeImpl>);
  JSValue *getDOMNamedNodeMap(ExecState *exec, DOM::NamedNodeMapImpl *m);
  JSValue *getDOMNodeList(ExecState *exec, PassRefPtr<DOM::NodeListImpl>);
  JSValue *getDOMDOMImplementation(ExecState *exec, DOM::DOMImplementationImpl *i);
  JSObject *getNodeConstructor(ExecState *exec);
  JSObject *getDOMExceptionConstructor(ExecState *exec);

  // Internal class, used for the collection return by e.g. document.forms.myinput
  // when multiple nodes have the same name.
  class DOMNamedNodesCollection : public DOMObject {
  public:
    DOMNamedNodesCollection(ExecState *exec, const QValueList< RefPtr<DOM::NodeImpl> >& nodes );
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
  private:
    static JSValue *lengthGetter(ExecState* exec, JSObject *, const Identifier&, const PropertySlot& slot);
    static JSValue *indexGetter(ExecState* exec, JSObject *, const Identifier&, const PropertySlot& slot);

    QValueList< RefPtr<DOM::NodeImpl> > m_nodes;
  };

} // namespace

#endif
