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

import { interceptCallback } from '../main/manage/event/callbackIntercept';
import Element from './Element';

/**
 * This factory class create native components classes, like div, image, text, etc...<br>
 * Each class is extends from Element, and saved in static property nativeElementClassMap.
 */
class NativeElementClassFactory {
  /**
   * Map for native element class
   * @type {Map}
   * @static
   */
  public static nativeElementClassMap: Map<string, any> = new Map();

  /**
   * Create a native element class form native, and saved in nativeElementClassMap.
   * @param {string} tagName - Name of element class.
   * @param {Function[]} methods - Prototype methods of element class.
   * @static
   */
  public static createNativeElementClass(tagName: string, methods: any[]) {
    // Skip when no special component methods.
    if (!methods || !methods.length) {
      return;
    }

    class NativeElement extends Element {
      constructor(props) {
        super(tagName, props, true);
      }
    }

    // Add methods to prototype.
    methods.forEach(methodName => {
      Object.defineProperty(NativeElement.prototype, methodName, {
        configurable: true,
        enumerable: true,
        get: function moduleGetter() {
          return (...args: any) => {
            const taskCenter = this.getTaskCenter(this.docId);
            if (taskCenter) {
              // support aceapp callback style
              args = interceptCallback(args);
              if (methodName === 'scrollTo' && args[0].id) {
                args[0].id = findEl(this, args[0].id);
              }
              const ret = taskCenter.send('component', {
                ref: this.ref,
                component: tagName,
                method: methodName
              }, args);
              return ret;
            }
          };
        }
      });
    });

    // Add to element type map.
    this.nativeElementClassMap.set(tagName, NativeElement);
  }
}

function findEl(parent, id) {
  if (!parent) {
    return;
  }
  if (parent.id === id) {
    return parent.ref;
  }
  let ans;
  const children = parent.children;
  if (children) {
    for (const child in children) {
      ans = ans || findEl(children[child], id);
    }
  }
  return ans;
}

export default NativeElementClassFactory;
