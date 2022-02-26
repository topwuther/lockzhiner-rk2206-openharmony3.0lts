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
 * 2021.01.08 - Remove docMap.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import Comment from './Comment';
import DocumentElement from './DocumentElement';
import Node from './Node';
import Element from './Element';
import { TaskCenter } from '../main/manage/event/TaskCenter';
import { Log } from '../utils';

/**
 * When a document is loaded into a application runtime, it becomes a document object.<br>
 * The document object is the root node of the vdom document.
 */
class Document {
  private _id: string;
  private _nodeMap: any;
  private _taskCenter: TaskCenter;
  private _documentElement: DocumentElement;
  private _body: Node;
  private _url: string

  constructor(id = '', url) {
    this._id = id;
    this._url = url;
    this._nodeMap = {};
    this._taskCenter = new TaskCenter(id);
    this._createDocumentElement();
  }

  /**
   * Body of this document.
   * @type {Node}
   * @readonly
   */
  public get body() {
    return this._body;
  }

  /**
   * ID of this document.
   * @type {string}
   * @readonly
   */
  public get id() {
    return this._id;
  }

  /**
   * Document element of this document.
   * @type {DocumentElement}
   * @readonly
   */
  public get documentElement() {
    return this._documentElement;
  }

  /**
   * url of this document (page).
   * @type {url}
   * @readonly
   */
  public get url() {
    return this._url;
  }

  /**
   * Node map of this document.
   * @type {Map}
   * @readonly
   */
  public get nodeMap() {
    return this._nodeMap;
  }

  /**
   * Task center of this document.
   * @type {TaskCenter}
   * @readonly
   */
  public get taskCenter() {
    return this._taskCenter;
  }

  /**
   * Set up body node.
   * @param {Node} el - Target element.
   */
  public setElementToBody(el: Element): void {
    el.role = 'body';
    el.depth = 1;
    delete this._nodeMap[el.nodeId];
    el.ref = '_root';
    this._nodeMap._root = el;
    this._body = el;
  }

  /**
   * Send body of this Document to native.
   * @param {Node} node - body element.
   */
  public sentBodyToNative(node: Element): void {
    const body = node.toJSON();
    if (this._taskCenter && typeof this._taskCenter.send === 'function') {
      this._taskCenter.send('dom', { action: 'createBody' }, [body]);
    }
  }

  /**
   * Get the node from nodeMap.
   * @param {string} ref - id of target node.
   * @return {object} node from node map.
   */
  public getRef(ref: string) {
    return this._nodeMap[ref];
  }

  /**
   * Create element of body.
   * @param {string} tagName - Tag name of body element.
   * @param {Object} options - Properties of element.
   * @return {Node} Body element.
   */
  public createBody(tagName: string, options?: any): Node {
    if (!this._body) {
      const el = new Element(tagName, options);
      this.setElementToBody(el);
    }
    return this._body;
  }

  /**
   * Create an element.
   * @param {string} tagName - Tag name of element.
   * @param {Object} options - Properties of element.
   * @return {Node} New element
   */
  public createElement(tagName: string, options?: any): Element {
    return new Element(tagName, options);
  }

  /**
   * Create an comment.
   * @param {string} commentText - Text of comment.
   * @return {object} comment
   */
  public createComment(commentText: string): Comment {
    return new Comment(commentText);
  }

  /**
   * Fire an event on specified element manually.
   * @param {Element} element - Event target element.
   * @param {string} eventType - Event name
   * @param {Object} eventObj - Event object.
   * @param {boolean} isDomChanges - if need to change dom
   * @param {object} options - Event options
   * @return {*} anything returned by handler function
   */
  public fireEvent(element: Element, eventType: string, eventObj: any, isDomChanges: boolean, options: any) {
    Log.debug(`Document#fireEvent, element = ${element}, eventType = ${eventType}, eventObj = ${eventObj}, isDomChanges = ${isDomChanges}.`);
    if (!element) {
      return;
    }
    eventObj = eventObj || {};
    eventObj.type = eventObj.type || eventType;
    eventObj.target = element;
    eventObj.currentTarget = element;
    eventObj.timestamp = Date.now();
    if (isDomChanges) {
      this._updateElement(element, isDomChanges);
    }
    let isBubble;
    const $root = this.getRef('_root');
    if ($root && $root.attr) {
      isBubble = $root.attr['bubble'] === 'true';
    }
    return element.fireEvent(eventType, eventObj, isBubble, options);
  }

  /**
   * Destroy current document, and remove itself form docMap.
   */
  public destroy() {
    this.taskCenter.destroyCallback();
    delete this._nodeMap;
    delete this._taskCenter;
  }

  /**
   * Create the document element.
   * @return {object} documentElement
   */
  private _createDocumentElement(): void {
    if (!this._documentElement) {
      const el = new DocumentElement('document');
      el.docId = this._id;
      el.ownerDocument = this;
      el.role = 'documentElement';
      el.depth = 0;
      el.ref = '_documentElement';
      this._nodeMap._documentElement = el;
      this._documentElement = el;
    }
  }

  private _updateElement(el: Element, changes: any): void {
    Log.debug(`Document#_updateElement, el = ${el}, changes = ${JSON.stringify(changes)}.`);
    const attrs = changes.attrs || {};
    for (const name in attrs) {
      el.setAttr(name, attrs[name], true);
    }
    const style = changes.style || {};
    for (const name in style) {
      el.setStyle(name, style[name], true);
    }
  }
}

export default Document;
