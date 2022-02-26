/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * 2021.01.08 - Move element's method from operation.js to Element class.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import { Log } from '../utils/index';
import Node from './Node';
import NativeElementClassFactory from './NativeElementClassFactory';
import Document from './Document';
import { TaskCenter } from '../main/manage/event/TaskCenter';
import { FragBlockInterface } from '../main/model/compiler';
import Vm from '../main/model';
import { CSS_INHERITANCE } from '../main/app/bundle';

/**
 * Element is a basic class to describe a tree node in vdom.
 * @extends Node
 */
class Element extends Node {
  private _style: any;
  private _classStyle: any;
  private _event: any;
  private _idStyle: any;
  private _tagStyle: any;
  private _id: string | null;
  private _classList: any[];
  private _block: FragBlockInterface;
  private _vm: Vm;
  private _isCustomComponent: boolean;
  private _inheritedStyle: object;

  protected _children: Node[];
  protected _pureChildren: Element[];
  protected _role: string;
  protected _attr: any;
  protected _dataSet: any;

  constructor(type = 'div', props:any = {}, isExtended: boolean = false) {
    super();
    const NativeElementClass = NativeElementClassFactory.nativeElementClassMap.get(type);
    if (NativeElementClass && !isExtended) {
      return new NativeElementClass(props);
    }

    this._nodeType = Node.NodeType.Element;
    this._type = type;
    this._attr = props.attr || {};
    this._style = props.style || {};
    this._classStyle = props.classStyle || {};
    this._event = {};
    this._idStyle = {};
    this._tagStyle = {};
    this._id = null;
    this._classList = [];
    this._children = [];
    this._pureChildren = [];
    this._isCustomComponent = false;
    this._inheritedStyle = {};
    this._dataSet = {};
  }

  /**
   * inherit sytle from parent
   * @type {Object}
   */
  public set inheritedStyle(inheritedStyle: object) {
    this._inheritedStyle = inheritedStyle;
  }

  public get inheritedStyle() {
    return this._inheritedStyle;
  }

  /**
   * Children array except comment node.
   * @type {Node[]}
   */
  public set pureChildren(newPureChildren: Element[]) {
    this._pureChildren = newPureChildren;
  }

  public get pureChildren() {
    return this._pureChildren;
  }

  /**
   * event of element
   * @type {Node[]}
   */
  public get event() {
    return this._event;
  }

  /**
   * Children array.
   * @type {Node[]}
   */
  public set children(newChildren: Node[]) {
    this._children = newChildren;
  }

  public get children() {
    return this._children;
  }

  /**
   * View model of this Element.
   * @type {Vm}
   */
  public get vm() {
    return this._vm;
  }

  public set vm(newVm: Vm) {
    this._vm = newVm;
  }

  /**
   * Style object of this Element, which keys is style name, and values is style values.
   * @type {JSON}
   */
  public get classStyle() {
    return this._classStyle;
  }

  /**
   * Block in this Element.
   * @type {FragBlock}
   */
  public get block() {
    return this._block;
  }

  public set block(newBlock: FragBlockInterface) {
    this._block = newBlock;
  }

  /**
   * Role of this element.
   * @type {string}
   */
  public set role(role: string) {
    this._role = role;
  }

  public get role() {
    return this._role;
  }

  /**
   * ID of this element.
   * @type {string}
   */
  public get id() {
    return this._id;
  }

  public set id(value) {
    this._id = value;
  }

  /**
   * Class list of this element.
   * @type {string[]}
   */
  public get classList() {
    return this._classList;
  }

  public set classList(value: string[]) {
    this._classList = value.slice(0);
  }

  /**
   * Attributes object of this Element.
   * @type {Object}
   */
  public set attr(attr: any) {
    this._attr = attr;
  }

  public get attr() {
    return this._attr;
  }

  /**
   * DataSet object of this Element.
   * @type {Object}
   */
  public set dataSet(dataSet: any) {
    this._dataSet = dataSet;
  }

  public get dataSet() {
    return this._dataSet;
  }

  /**
   * Flag of whether the element is the root of customeComponent.
   * @param {bollean}
   */
  public set isCustomComponent(isCustomComponent: boolean) {
    this._isCustomComponent = isCustomComponent;
  }

  public get isCustomComponent() {
    return this._isCustomComponent;
  }

  /**
   * Style object of this Element.
   * @type {Object}
   */
  public set style(style: any) {
    this._style = style;
  }

  public get style() {
    return this._style;
  }

  /**
   * Get TaskCenter instance by id.
   * @param {string} id
   * @return {TaskCenter} TaskCenter
   */
  public getTaskCenter(id: string): TaskCenter | null {
    const doc: Document = this._ownerDocument;
    if (doc && doc.taskCenter) {
      return doc.taskCenter;
    }
    return null;
  }

  /**
   * Establish the connection between parent and child node.
   * @param {Node} child - Target node.
   */
  public linkChild(child: Node): void {
    child.parentNode = this;
    if (this._docId) {
      child.docId = this._docId;
      child.ownerDocument = this._ownerDocument;
      child.ownerDocument.nodeMap[child.nodeId] = child;
      child.depth = this._depth + 1;
    }
    if (child.nodeType === Node.NodeType.Element) {
      const element = child as Element;
      element.children.forEach((grandChild: Node) => {
        element.linkChild(grandChild);
      });
    }
  }

  /**
   * Insert a node into list at the specified index.
   * @param {Node} target - Target node.
   * @param {number} newIndex - Target index.
   * @param {Object} [options] - options of insert method.
   * @param {boolean} [options.changeSibling=false] - If need to change sibling's index.
   * @param {boolean} [options.isInPureChildren=false] - If in pure children array or children array.
   * @return {number} New index of node.
   */
  public insertIndex(target: Node, newIndex: number, { changeSibling = false, isInPureChildren = false }): number {
    const list: Node[] = isInPureChildren ? this._pureChildren : this._children;
    if (newIndex < 0) {
      newIndex = 0;
    }
    const before: Node = list[newIndex - 1];
    const after: Node = list[newIndex];
    list.splice(newIndex, 0, target);
    if (changeSibling) {
      before && (before.nextSibling = target);
      target.previousSibling = before;
      target.nextSibling = after;
      after && (after.previousSibling = target);
    }
    return newIndex;
  }

  /**
   * Move the node to a new index in list.
   * @param {Node} target - Target node.
   * @param {number} newIndex - Target index.
   * @param {Object} [options] - options of insert method.
   * @param {boolean} [options.changeSibling=false] - If need to change sibling's index.
   * @param {boolean} [options.isInPureChildren=false] - If in pure children array or children array.
   * @return {number} New index of node.
   */
  public moveIndex(target: Node, newIndex: number, { changeSibling = false, isInPureChildren = false }): number {
    const list: Node[] = isInPureChildren ? this._pureChildren : this._children;
    const index: number = list.indexOf(target);
    if (index < 0) {
      return -1;
    }
    if (changeSibling) {
      const before: Node = list[index - 1];
      const after: Node = list[index + 1];
      before && (before.nextSibling = after);
      after && (after.previousSibling = before);
    }
    list.splice(index, 1);
    let newIndexAfter = newIndex;
    if (index <= newIndex) {
      newIndexAfter = newIndex - 1;
    }
    const beforeNew: Node = list[newIndexAfter - 1];
    const afterNew: Node = list[newIndexAfter];
    list.splice(newIndexAfter, 0, target);
    if (changeSibling) {
      if (beforeNew) {
        beforeNew.nextSibling = target;
      }
      target.previousSibling = beforeNew;
      target.nextSibling = afterNew;
      if (afterNew) {
        afterNew.previousSibling = target;
      }
    }
    if (index === newIndexAfter) {
      return -1;
    }
    return newIndex;
  }

  /**
   * Remove the node from list.
   * @param {Node} target - Target node.
   * @param {Object} [options] - options of insert method.
   * @param {boolean} [options.changeSibling=false] - If need to change sibling's index.
   * @param {boolean} [options.isInPureChildren=false] - If in pure children array or children array.
   */
  public removeIndex(target, { changeSibling = false, isInPureChildren = false}): void {
    const list: Node[] = isInPureChildren ? this._pureChildren : this._children;
    const index: number = list.indexOf(target);
    if (index < 0) {
      return;
    }
    if (changeSibling) {
      const before: Node = list[index - 1];
      const after: Node = list[index + 1];
      before && (before.nextSibling = after);
      after && (after.previousSibling = before);
    }
    list.splice(index, 1);
  }

  /**
   * Get the next sibling element.
   * @param {Node} node - Target node.
   * @return {Node} Next node of target node.
   */
  public nextElement(node: Node): Element {
    while (node) {
      if (node.nodeType === Node.NodeType.Element) {
        return node as Element;
      }
      node = node.nextSibling;
    }
  }

  /**
   * Get the previous sibling element.
   * @param {Node} node - Target node.
   * @return {Node} Previous node of target node.
   */
  public previousElement(node: Node): Element {
    while (node) {
      if (node.nodeType === Node.NodeType.Element) {
        return node as Element;
      }
      node = node.previousSibling;
    }
  }

  /**
   * Append a child node.
   * @param {Node} node - Target node.
   * @return {number} the signal sent by native
   */
  public appendChild(node: Node): void {
    if (node.parentNode && node.parentNode !== this) {
      return;
    }

    if (!node.parentNode) {
      this.linkChild(node as Element);
      this.insertIndex(node, this.children.length, { changeSibling: true });
      if (this.docId) {
        this.registerNode(node);
      }
      if (node.nodeType === Node.NodeType.Element) {
        const element = node as Element;
        this.insertIndex(element, this.pureChildren.length, { isInPureChildren: true });
        this.inheritStyle(node, true);
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, element.toJSON(), -1]
          );
        }
      }
    } else {
      this.moveIndex(node, this.children.length, { changeSibling: true });
      if (node.nodeType === Node.NodeType.Element) {
        const index = this.moveIndex(node, this.pureChildren.length, { isInPureChildren: true });
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter && index >= 0) {
          return taskCenter.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          );
        }
      }
    }
  }

  /**
   * Insert a node before specified node.
   * @param {Node} node - Target node.
   * @param {Node} before - The node next to the target position.
   * @return {number} the signal sent by native
   */
  public insertBefore(node: Node, before: Node): void {
    if (node.parentNode && node.parentNode !== this) {
      return;
    }
    if (node === before || node.nextSibling && node.nextSibling === before) {
      return;
    }
    // If before is not exist, return.
    if (this.children.indexOf(before) < 0) {
      return;
    }
    if (!node.parentNode) {
      this.linkChild(node as Element);
      this.insertIndex(node, this.children.indexOf(before), { changeSibling: true });
      if (this.docId) {
        this.registerNode(node);
      }
      if (node.nodeType === Node.NodeType.Element) {
        const element = node as Element;
        const pureBefore = this.nextElement(before);
        const index = this.insertIndex(
          element,
          pureBefore
            ? this.pureChildren.indexOf(pureBefore)
            : this.pureChildren.length,
          { isInPureChildren: true }
        );
        this.inheritStyle(node);
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, element.toJSON(), index]
          );
        }
      }
    } else {
      this.moveIndex(node, this.children.indexOf(before), { changeSibling: true });
      if (node.nodeType === Node.NodeType.Element) {
        const pureBefore = this.nextElement(before);
        const index = this.moveIndex(
          node,
          pureBefore
            ? this.pureChildren.indexOf(pureBefore)
            : this.pureChildren.length,
          { isInPureChildren: true}
        );
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter && index >= 0) {
          return taskCenter.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          );
        }
      }
    }
  }

  /**
   * Insert a node after specified node.
   * @param {Node} node - Target node.
   * @param {Node} after - The node in front of the target position.
   * @return {number} the signal sent by native
   */
  public insertAfter(node: Node, after: Node) {
    if (node.parentNode && node.parentNode !== this) {
      return;
    }
    if (node === after || node.previousSibling && node.previousSibling === after) {
      return;
    }
    if (!node.parentNode) {
      this.linkChild(node as Element);
      this.insertIndex(node, this.children.indexOf(after) + 1, { changeSibling: true });

      if (this.docId) {
        this.registerNode(node);
      }
      if (node.nodeType === Node.NodeType.Element) {
        const element = node as Element;
        const index = this.insertIndex(
          element,
          this.pureChildren.indexOf(this.previousElement(after)) + 1,
          { isInPureChildren: true }
        );
        this.inheritStyle(node);
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, element.toJSON(), index]
          );
        }
      }
    } else {
      this.moveIndex(node, this.children.indexOf(after) + 1, { changeSibling: true});
      if (node.nodeType === Node.NodeType.Element) {
        const index = this.moveIndex(
          node,
          this.pureChildren.indexOf(this.previousElement(after)) + 1,
          { isInPureChildren: true }
        );
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter && index >= 0) {
          return taskCenter.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          );
        }
      }
    }
  }

  /**
   * Remove a child node, and decide whether it should be destroyed.
   * @param {Node} node - Target node.
   * @param {boolean} [preserved=false] - If need to keep the target node.
   */
  public removeChild(node: Node, preserved: boolean = false): void {
    if (node.parentNode) {
      this.removeIndex(node, { changeSibling: true });
      if (node.nodeType === Node.NodeType.Element) {
        this.removeIndex(node, { isInPureChildren: true});
        const taskCenter = this.getTaskCenter(this.docId);
        if (taskCenter) {
          taskCenter.send(
            'dom',
            { action: 'removeElement' },
            [node.ref]
          );
        }
      }
    }
    if (!preserved) {
      node.destroy();
    }
  }

  /**
   * Clear all child nodes.
   */
  public clear(): void {
    const taskCenter: TaskCenter = this.getTaskCenter(this._docId);
    if (taskCenter) {
      this._pureChildren.forEach(child => {
        taskCenter.send('dom', { action: 'removeElement' }, [child.ref]);
      });
    }
    this._children.forEach(node => {
      node.destroy();
    });
    this._children.length = 0;
    this._pureChildren.length = 0;
  }

  /**
   * Set dataSet for an element.
   * @param {string} key - dataSet name.
   * @param {string} value - dataSet value.
   */
  public setData(key: string, value: string): void {
    this.dataSet[key] = value;
  }

  /**
   * Set an attribute, and decide whether the task should be send to native.
   * @param {string} key - Arribute name.
   * @param {string | number} value - Arribute value.
   * @param {boolean} [silent=false] - If use silent mode.
   */
  public setAttr(key: string, value: string | number, silent: boolean = false): void {
    if (this.attr[key] === value && silent !== false) {
      return;
    }
    // Because the data has been addressed in SetData
    if (key === 'data') {
      return;
    }
    this.attr[key] = value;
    const taskCenter = this.getTaskCenter(this.docId);
    if (!silent && taskCenter) {
      const result = {};
      result[key] = value;
      taskCenter.send('dom', { action: 'updateAttrs' }, [this.ref, result]);
    }
  }

  /**
   * Set a style property, and decide whether the task should be send to native.
   * @param {string} key - Style name.
   * @param {string | number} value - Style value.
   * @param {boolean} [silent=false] - If use silent mode.
   */
  public setStyle(key: string, value: string | number, silent: boolean = false): void {
    if (this.style[key] === value && silent !== false) {
      return;
    }
    this.style[key] = value;
    const taskCenter = this.getTaskCenter(this.docId);
    if (!silent && taskCenter) {
      const result = {};
      result[key] = value;
      taskCenter.send('dom', { action: 'updateStyle' }, [this.ref, this.toStyle()]);
      if (CSS_INHERITANCE.includes(key)) {
        this.broadcastStyle();
      }
    }
  }

  /**
   * Set style properties from class.
   * @param {object} classStyle - Style properties.
   */
  public setClassStyle(classStyle: any): void {
    let canUpdate: boolean = false;
    const taskCenter = this.getTaskCenter(this.docId);
    Object.keys(classStyle).forEach(key => {
      if (CSS_INHERITANCE.includes(key) && taskCenter) {
        if (!this.isSameStyle(this.classStyle[key], classStyle[key], key)) {
          canUpdate = true;
        }
      }
    });
    for (const key in this._classStyle) {
      this._classStyle[key] = '';
    }

    Object.assign(this._classStyle, classStyle);
    if (taskCenter) {
      taskCenter.send('dom', { action: 'updateStyle' }, [this.ref, this.toStyle()]);
      if (canUpdate) {
        this.broadcastStyle();
      }
    }
  }

  /**
   * Set style properties from class.
   * @param {object} classStyle - Style properties.
   */
  public setCustomFlag(): void {
    this._isCustomComponent = true;
  }

  /**
   * Set IdStyle properties from class.
   * @param {string} key - Style name.
   * @param {string|number} value - Style value.
   * @param {boolean} [silent=false] - If use silent mode.
   */
  public setIdStyle(key: string, value: string | number, silent: boolean = false): void {
    if (this._idStyle[key] === value && silent !== false) {
      return;
    }
    // if inline style has define return
    if (this.style[key]) {
      return;
    }
    this._idStyle[key] = value;
    const taskCenter = this.getTaskCenter(this.docId);
    if (!silent && taskCenter) {
      const result = {};
      result[key] = value;
      taskCenter.send('dom', { action: 'updateStyle' }, [this.ref, result]);
      if (CSS_INHERITANCE.includes(key)) {
        this.broadcastStyle();
      }
    }
  }

  /**
   * Set TagStyle properties from class.
   * @param {string} key - Style name.
   * @param {string|number} value - Style value.
   * @param {boolean} [silent=false] - If use silent mode.
   */
  public setTagStyle(key: string, value: string | number, silent: boolean = false): void {
    if (this._tagStyle[key] === value && silent !== false) {
      return;
    }
    // If inline id class style has define return.
    if (this.style[key] || this._idStyle[key] || this._classStyle[key]) {
      return;
    }
    this._tagStyle[key] = value;
    const taskCenter = this.getTaskCenter(this.docId);
    if (!silent && taskCenter) {
      const result = {};
      result[key] = value;
      taskCenter.send('dom', { action: 'updateStyle' }, [this.ref, result]);
    }
  }

  /**
   * Add an event handler.
   * @param {string} type - Event name.
   * @param {Function} handler - Event handler.
   * @param {Object} [params] - Event parameters.
   */
  public addEvent(type: string, handler?: Function, params?: any): void {
    if (!this._event) {
      this._event = {};
    }
    if (!this._event[type]) {
      this._event[type] = { handler, params };
      const taskCenter = this.getTaskCenter(this.docId);
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'addEvent' },
          [this.ref, type]
        );
      }
    }
  }

  /**
   * Remove an event handler.
   * @param {string} type - Event name
   */
  public removeEvent(type: string): void {
    if (this._event && this._event[type]) {
      delete this._event[type];
      const taskCenter = this.getTaskCenter(this.docId);
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'removeEvent' },
          [this.ref, type]
        );
      }
    }
  }

  /**
   * Fire an event manually.
   * @param {string} type - Event name.
   * @param {function} event - Event handler.
   * @param {boolean} isBubble - Whether or not event bubble
   * @param {boolean} [options] - Event options
   * @return {*} anything returned by handler function.
   */
  public fireEvent(type: string, event: any, isBubble?: boolean, options?: any) {
    Log.debug(`Element#fireEvent, type = ${type}, event = ${event}, isBubble = ${isBubble}, options = ${options}.`);
    const BUBBLE_EVENTS = [
      'mouse', 'click', 'longpress', 'touchstart',
      'touchmove', 'touchend', 'panstart', 'panmove',
      'panend', 'horizontalpan', 'verticalpan', 'swipe'
    ];
    let result = null;
    let isStopPropagation = false;
    const eventDesc = this._event[type];
    if (eventDesc && event) {
      const handler = eventDesc.handler;
      event.stopPropagation = () => {
        isStopPropagation = true;
      };
      if (options && options.params) {
        result = handler.call(this, event, ...options.params);
      } else {
        result = handler.call(this, event);
      }
    }

    if (!isStopPropagation && isBubble && BUBBLE_EVENTS.indexOf(type) !== -1) {
      if (this._parentNode) {
        const parentNode = this._parentNode as Element;
        event.currentTarget = parentNode;
        parentNode.fireEvent(type, event, isBubble); // no options
      }
    }

    return result;
  }

  /**
   * Get all styles of current element.
   * @return {object} style
   */
  public toStyle(): any {
    // Selector Specificity  inline > #id > .class > tag > inheritance.
    const style = Object.assign({}, this._inheritedStyle);
    this.assignStyle(style, this._tagStyle);
    this.assignStyle(style, this._classStyle);
    this.assignStyle(style, this._idStyle);
    this.assignStyle(style, this.style);
    return style;
  }

  /**
   * Assign style.
   * @param {*} src - Source style object.
   * @param {*} dest - Target style object.
   */
  public assignStyle(src: any, dest: any): void {
    if (dest) {
      const keys = Object.keys(dest);

      // Margin and padding style: the style should be empty in the first.
      keys.sort(function(style1, style2) {
        if (dest[style1] === '') {
          return 1;
        } else {
          return -1;
        }
      });
      let i = keys.length;
      while (i--) {
        const key = keys[i];
        const val = dest[key];
        if (val) {
          src[key] = val;
        } else {
          if ((val === '' || val === undefined) && src[key]) {
            return;
          }
          src[key] = val;
        }
      }
    }
  }

  /**
   * Convert current element to JSON like object.
   * @param {boolean} [ignoreChildren=false] - whether to ignore child nodes, default false
   * @return {JSON} JSON object of this element.
   */
  public toJSON(ignoreChildren = false): JSON {
    const result: any = {
      ref: this.ref,
      type: this._type,
      attr: this.attr,
      style: this.toStyle(),
      customComponent: this._isCustomComponent
    };
    const event = [];
    for (const type in this._event) {
      const { params } = this._event[type];
      if (!params) {
        event.push(type);
      } else {
        event.push({ type, params });
      }
    }
    if (event.length) {
      result.event = event;
    }
    if (!ignoreChildren && this._pureChildren.length) {
      result.children = this._pureChildren.map(child => child.toJSON());
    }
    if (this._id) {
      result.id = this._id;
    }
    return result;
  }

  /**
   * Convert to HML element tag string.
   * @override
   * @return {string} hml of this element.
   */
  public toString(): string {
    const id = this._id !== null ? this._id : '';
    return '<' + this._type +
        ' id =' + id +
        ' attr=' + JSON.stringify(this.attr) +
        ' style=' + JSON.stringify(this.toStyle()) + '>' +
        this.pureChildren.map((child) => child.toString()).join('') +
        '</' + this._type + '>';
  }

  /**
   * Destroy this element
   */
  public destroy() {
    Log.debug(`Element#destroy this._type = ${this._type}.`);
    if (this._event && this._event['detached']) {
      this.fireEvent('detached', {});
    }
    this._attr = {};
    this._style = {};
    this._classStyle = {};
    this._event = {};
    this._idStyle = {};
    this._tagStyle = {};
    this._classList.length = 0;

    if (this.destroyHook) {
      this.destroyHook();
      this.destroyHook = null;
    }
    if (this._children) {
      this._children.forEach((child: Node): void => {
        child.destroy();
      });
      this._children.length = 0;
    }
    if (this._pureChildren) {
      this._pureChildren.length = 0;
    }
    super.destroy();
  }

  /**
   * the judgement of whether the inherited style should update
   * @param {string | Array} oldClassStyle
   * @param {string | Array} newClassStyle
   * @param {string} key
   * @returns {boolean}
   */
  isSameStyle(oldClassStyle: string | any[], newClassStyle: string | any[], key: string) {
    if (key === 'fontFamily') {
      if (oldClassStyle[0].fontFamily === newClassStyle[0].fontFamily) {
        return true;
      }
    } else {
      if (oldClassStyle === newClassStyle) {
        return true;
      }
    }
    return false;
  }

  /**
   * iterate child node for updating inheritedstyle
   */
  broadcastStyle() {
    if (this.pureChildren) {
      for (const child in this.pureChildren) {
        this.pureChildren[child].setInheritedStyle();
        this.pureChildren[child].broadcastStyle();
      }
    }
  }

  /**
   * before update inherited style
   * clear up the inherited style
   */
  resetInheritedStyle() {
    this.inheritedStyle = {};
  }

  /**
   * inherited style from parent
   */
  public setInheritedStyle() {
    this.resetInheritedStyle();
    const parentNode: Element = this.parentNode as Element;
    parentNode.inheritStyle(this);
    const taskCenter = this.getTaskCenter(this.docId);
    if (taskCenter) {
      taskCenter.send(
        'dom',
        { action: 'updateStyle' },
        [this.ref, this.toStyle()]
      );
    }
  }

  /**
   * inherit style of parent.
   * @return {object} element
   */
  public inheritStyle(node, isFirst = false) {
    // for first render, save time
    const allStyle = this.toStyle();
    this.setChildStyle(allStyle, node._inheritedStyle);
  }

  /**
   * set inherited style to child
   * @param {object} parentStyle
   * @param {object} childStyle
   * @param {object} node
   */
  public setChildStyle(parentStyle, childStyle) {
    Object.keys(parentStyle).forEach(key => {
      if (CSS_INHERITANCE.includes(key)) {
        childStyle[key] = parentStyle[key];
      }
    });
  }

  private registerNode(node) {
    const doc = this._ownerDocument;
    doc.nodeMap[node.nodeId] = node;
  }
}

export default Element;
