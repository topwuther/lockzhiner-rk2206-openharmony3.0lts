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

/**
 * @fileOverview Document & Element Helpers.
 *
 * required:
 * Document#: createElement, createComment, getRef
 * Element#: appendChild, insertBefore, removeChild, nextSibling
 */
import Vm from './index';
import Element from '../../vdom/Element';
import Comment from '../../vdom/Comment';
import Node from '../../vdom/Node';
import { FragBlockInterface, isBlock } from './compiler';
import { emitSubVmLife } from './pageLife';

/**
 * Create a body by type.
 * @param {Vm} vm - Vm object.
 * @param {string} type - Element type.
 * @return {Node} Body of Node by type.
 */
export function createBody(vm: Vm, type: string): Node {
  const doc = vm.app.doc;
  return doc.createBody(type);
}

/**
 * Create an element by type
 * @param {Vm} vm - Vm object.
 * @param {string} type - Element type.
 * @return {Element} Element of Node by type.
 */
export function createElement(vm: Vm, type: string): Element {
  const doc = vm.app.doc;
  return doc.createElement(type);
}

/**
 * Create and return a frag block for an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} element - Element object.
 * @return {FragBlockInterface} New block.
 */
export function createBlock(vm: Vm, element: Element | FragBlockInterface): FragBlockInterface {
  const start = createBlockStart(vm);
  const end = createBlockEnd(vm);
  const blockId = lastestBlockId++;
  const newBlock: FragBlockInterface = {start, end, blockId};
  if (isBlock(element)) {
    let updateMark = element.updateMark;
    if (updateMark) {
      if (isBlock(updateMark)) {
        updateMark = updateMark.end;
      }
      element.element.insertAfter(end, updateMark);
      element.element.insertAfter(start, updateMark);
      element.updateMark = end;
    } else {
      element.element.insertBefore(start, element.end);
      element.element.insertBefore(end, element.end);
    }
    newBlock.element = element.element;
  } else {
    element.appendChild(start);
    element.appendChild(end);
    newBlock.element = element;
    element.block = newBlock;
  }
  return newBlock;
}

let lastestBlockId = 1;

/**
 * Create and return a block starter.
 * @param {Vm} vm - Vm object.
 * @return {Comment} A block starter.
 */
function createBlockStart(vm: Vm): Comment {
  const doc = vm.app.doc;
  const anchor = doc.createComment('start');
  return anchor;
}

/**
 * Create and return a block ender.
 * @param {Vm} vm - Vm object.
 * @return {Comment} A block starter.
 */
function createBlockEnd(vm: Vm): Comment {
  const doc = vm.app.doc;
  const anchor = doc.createComment('end');
  anchor.destroyHook = function() {
    if (anchor.watchers !== undefined) {
      anchor.watchers.forEach(function(watcher) {
        watcher.teardown();
      });
      anchor.watchers = [];
    }
  };
  return anchor;
}

/**
 * Attach target to a certain dest using appendChild by default.
 * @param {Element} target - If the dest is a frag block then insert before the ender.
 * @param {FragBlockInterface | Element} dest - A certain dest.
 * @return {*}
 */
export function attachTarget(target: Element, dest: FragBlockInterface | Element): any {
  if (isBlock(dest)) {
    const before = dest.end;
    const after = dest.updateMark;
    if (dest.children) {
      dest.children.push(target);
    }
    if (after) {
      const signal = moveTarget(target, after);
      if (isBlock(target)) {
        dest.updateMark = target.end;
      } else {
        dest.updateMark = target;
      }
      return signal;
    } else if (isBlock(target)) {
      dest.element.insertBefore(target.start, before);
      dest.element.insertBefore(target.end, before);
    } else {
      return dest.element.insertBefore(target, before);
    }
  } else {
    if (isBlock(target)) {
      dest.appendChild(target.start);
      dest.appendChild(target.end);
    } else {
      return dest.appendChild(target);
    }
  }
}

/**
 * Move target before a certain element. The target maybe block or element.
 * @param {Element | FragBlockInterface} target - Block or element.
 * @param {Node} after - Node object after moving.
 * @return {*}
 */
export function moveTarget(target: Element | FragBlockInterface, after: Node): any {
  if (isBlock(target)) {
    return moveBlock(target, after);
  }
  return moveElement(target, after);
}

/**
 * Move element before a certain element.
 * @param {Element} element - Element object.
 * @param {Node} after - Node object after moving.
 * @return {*}
 */
function moveElement(element: Element, after: Node): any {
  const parent = after.parentNode as Element;
  if (parent && parent.children.indexOf(after) !== -1) {
    return parent.insertAfter(element, after);
  }
}

/**
 * Move all elements of the block before a certain element.
 * @param {FragBlockInterface} fragBlock - Frag block.
 * @param {Node} after - Node object after moving.
 */
function moveBlock(fragBlock: FragBlockInterface, after: Node): any {
  const parent = after.parentNode as Element;
  if (parent) {
    let el = fragBlock.start as Node;
    let signal;
    const group = [el];
    while (el && el !== fragBlock.end) {
      el = el.nextSibling;
      group.push(el);
    }
    let temp = after;
    group.every((el) => {
      signal = parent.insertAfter(el, temp);
      temp = el;
      return signal !== -1;
    });
    return signal;
  }
}

/**
 * Remove target from DOM tree.
 * @param {Element | FragBlockInterface} target - If the target is a frag block then call _removeBlock
 * @param {boolean} [preserveBlock] - Preserve block.
 */
export function removeTarget(target: Element | FragBlockInterface, preserveBlock?: boolean): void {
  if (!preserveBlock) {
    preserveBlock = false;
  }
  if (isBlock(target)) {
    removeBlock(target, preserveBlock);
  } else {
    removeElement(target);
  }
  if (target.vm) {
    target.vm.$emit('hook:onDetached');
    emitSubVmLife(target.vm, 'onDetached');
    target.vm.$emit('hook:destroyed');
  }
}

/**
 * Remove an element.
 * @param {Element | Comment} target - Target element.
 */
function removeElement(target: Element | Comment): void {
  const parent = target.parentNode as Element;
  if (parent) {
    parent.removeChild(target);
  }
}

/**
 * Remove a frag block.
 * @param {FragBlockInterface} fragBlock - Frag block.
 * @param {boolean} [preserveBlock] - If preserve block.
 */
function removeBlock(fragBlock: FragBlockInterface, preserveBlock?: boolean): void {
  if (!preserveBlock) {
    preserveBlock = false;
  }
  const result = [];
  let el = fragBlock.start.nextSibling;
  while (el && el !== fragBlock.end) {
    result.push(el);
    el = el.nextSibling;
  }
  if (!preserveBlock) {
    removeElement(fragBlock.start);
  }
  result.forEach((el) => {
    removeElement(el);
  });
  if (!preserveBlock) {
    removeElement(fragBlock.end);
  }
}
