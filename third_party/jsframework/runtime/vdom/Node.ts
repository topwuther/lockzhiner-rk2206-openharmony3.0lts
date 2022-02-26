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

import Document from './Document';
import Watcher from '../main/reactivity/watcher';

/**
 * This class generate an unique id.
 */
class IDGenerator {
  private static _id: number = 0;

  /**
   * Return an unique id, which value is a string converted from number.
   * @return {string} A string converted from number.
   */
  public static getUniqueId(): string {
    this._id++;
    return this._id.toString();
  }
}

/**
 * Enum for NodeTypes
 * @enum {number}
 */
/* eslint-disable no-unused-vars */
enum NodeType {
  /**
   * Element type
   */
  Element = 1,

  /**
   * Comment type
   */
  Comment = 8
}
/* eslint-enable no-unused-vars */

/**
 * Node is base class for Element and Comment.<br>
 * It has basic method for setting nodeId, getting children, etc...
 */
class Node {
  public static NodeType = NodeType;

  protected _depth: number;
  protected _nodeId: string;
  protected _nodeType: number;
  protected _ref: string;
  protected _nextSibling: Node;
  protected _previousSibling: Node;
  protected _watchers: Watcher[];
  protected _destroyHook: () => void;
  protected _type: string;
  protected _parentNode: Node;
  protected _ownerDocument: Document;
  protected _docId: string;

  constructor() {
    this._nodeId = IDGenerator.getUniqueId();
    this._ref = this.nodeId;
    this._nextSibling = null;
    this._previousSibling = null;
    this._parentNode = null;
    this._watchers = [];
    this._destroyHook = null;
  }

  /**
   * Parent node.
   * @type {Node}
   */
  public set parentNode(newParentNode: Node) {
    this._parentNode = newParentNode;
  }

  public get parentNode() {
    return this._parentNode;
  }

  /**
   * Watchers for recativity in a Node.
   * @type {Watcher[]}
   */
  public get watchers() {
    return this._watchers;
  }

  public set watchers(newWatchers: Watcher[]) {
    this._watchers = newWatchers;
  }

  /**
   * Node ID.
   * @type {string}
   */
  public get nodeId() {
    return this._nodeId;
  }

  public set nodeId(newNodeId: string) {
    this._nodeId = newNodeId;
  }

  /**
   * Node type.
   */
  public get nodeType() {
    return this._nodeType;
  }

  public set nodeType(newNodeType: NodeType) {
    this._nodeType = newNodeType;
  }

  /**
   * Destroy hook.
   * @type {Function}
   */
  public set destroyHook(hook: () => void) {
    this._destroyHook = hook;
  }

  public get destroyHook() {
    return this._destroyHook;
  }

  /**
   * The level from current node to root element.
   * @type {number}
   */
  public set depth(newValue: number) {
    this._depth = newValue;
  }

  public get depth() {
    return this._depth;
  }

  /**
   * <p>XML tag name, like div, button.</p>
   * <p>If node type is NodeType.Comment, it's value is "comment".</p>
   * @type {string}
   */
  public set type(newType: string) {
    this._type = newType;
  }

  public get type() {
    return this._type;
  }

  /**
   * <p>Node Reference, it's value is same as nodeId, It will send to native.</p>
   * <p>Document element's ref is "_documentElement", root element's ref is "root".</p>
   * @type {string}
   */
  public set ref(newRef: string) {
    this._ref = newRef;
  }

  public get ref() {
    return this._ref;
  }

  /**
   * Next sibling node.
   * @type {Node}
   */
  public set nextSibling(nextSibling: Node) {
    this._nextSibling = nextSibling;
  }

  public get nextSibling() {
    return this._nextSibling;
  }

  /**
   * Previous sibling node.
   * @type {Node}
   */
  public set previousSibling(previousSibling: Node) {
    this._previousSibling = previousSibling;
  }

  public get previousSibling() {
    return this._previousSibling;
  }

  /**
   * Document reference which this element belong to.
   * @type {Document}
   */
  public set ownerDocument(doc: Document) {
    this._ownerDocument = doc;
  }

  public get ownerDocument() {
    return this._ownerDocument;
  }

  /**
   * ID of the document which element belong to.
   * @type {string}
   */
  public get docId() {
    return this._docId;
  }

  public set docId(value: string) {
    this._docId = value;
  }

  /**
   * Destroy current node, and remove itself form nodeMap.
   */
  public destroy(): void {
    this._nextSibling = null;
    this._previousSibling = null;
    this._parentNode = null;
    this._watchers = null;
    this._destroyHook = null;
    this._ownerDocument = null;
  }
}

export default Node;
