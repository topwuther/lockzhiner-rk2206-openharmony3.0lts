/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import Element from './Element';
import Node from './Node';
import { Log } from '../utils';

/**
 * Document element is the root element in a Document.
 * @extends Element
 */
class DocumentElement extends Element {
  constructor(type: string = 'div', props: object = {}, isExtended?: boolean) {
    super(type, props, isExtended);
  }

  /**
   * Destroy this document element.
   */
  public destroy() {
    const doc = this._ownerDocument;
    if (doc) {
      delete this.docId;
      delete doc.nodeMap[this.nodeId];
    }
    super.destroy();
  }

  /**
   * Append a child node.
   * @param {Node} node - Target node.
   * @param {number} before - The node next to the target position.
   * @override
   */
  public appendChild(node: Node, before?: Node): void {
    Log.debug(`DocumentElement#appendChild, node = ${node}, before = ${before}.`);

    if (this.pureChildren.length > 0 || node.parentNode) {
      return;
    }
    const children = this.children;
    const beforeIndex = children.indexOf(before);
    if (beforeIndex < 0) {
      children.push(node);
    } else {
      children.splice(beforeIndex, 0, node);
    }

    if (node.nodeType === Node.NodeType.Element) {
      const element = node as Element;
      if (element.role === 'body') {
        element.docId = this.id;
        element.ownerDocument = this.ownerDocument;
        element.parentNode = this;
        this.linkChild(element);
      } else {
        element.children.forEach(child => {
          child.parentNode = element;
        });
        const document = this.ownerDocument;
        document.setElementToBody(element);
        element.docId = document.id;
        element.ownerDocument = document;
        this.linkChild(this);
        delete this.ownerDocument.nodeMap[element.nodeId];
      }
      this.pureChildren.push(element);
      this.ownerDocument.sentBodyToNative(element);
    } else {
      node.parentNode = this;
      this.ownerDocument.nodeMap[node.ref] = node;
    }
  }
}

export default DocumentElement;
