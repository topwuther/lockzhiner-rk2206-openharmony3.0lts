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
 * 2021.01.08 - The function 'standardization' is simpler and more accurate.
 * And extend function 'send' to fit framework.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import {
  Log,
  typof
} from '../../../utils/index';
import CallbackManager from './CallbackManager';

type OptionsType = Partial<Record<'action' | 'module' | 'method' | 'ref', string>>

/**
 * <p>Tasks processing center.</p>
 * <p>Instructs the Native module to perform operations based on the message sent by the listener.</p>
 * <p>Then the Native module invokes the callNative() callback function in sendTasks()<br>
 * to send the message to the Native module.</p>
 */
export class TaskCenter {
  public instanceId: string;
  public callbackManager: CallbackManager;

  constructor(id: string) {
    this.instanceId = id;
    this.callbackManager = new CallbackManager(id);
  }

  /**
   * Execute the consume() function from callbackManager class.
   * @param {number} callbackId - Callback id.
   * @param {Object} data - Data that needed.
   * @param {boolean} ifKeepAlive - If keepAlive is false, delete this callback.
   * @return {*}
   */
  public consumeCallback(callbackId: number, data: object, ifKeepAlive: boolean): any | Error {
    return this.callbackManager.consume(callbackId, data, ifKeepAlive);
  }

  /**
   * Execute the close() function from callbackManager class.
   * @param {number} callbackId - Callback id.
   */
  public destroyCallback(): void {
    return this.callbackManager.destroy();
  }

  /**
   * Execute the remove() function from callbackManager class.
   * @param {number} callbackId - Callback id.
   */
  public removeCallback(callbackId: number): void {
    this.callbackManager.remove(callbackId);
  }

  /**
   * Execute the isEmpty() function from callbackManager class.
   * @param {number} callbackId - Callback id.
   * @return {boolean} If callbacklist object is empty, return true. Otherwise return false.
   */
  public callbackIsEmpty(): boolean {
    return this.callbackManager.isEmpty();
  }

  /**
   * Standardizing a value. Specially, if the value is a function, generate a function id.
   * @param  {*} arg - Any type.
   * @return {*}
   */
  public standardization(arg: any): any {
    const type = typof(arg);
    if (type === 'object') {
      const ans = {};
      Object.keys(arg).forEach(key => {
        ans[key] = this.standardization(arg[key]);
      });
      return ans;
    } else if (type === 'function') {
      return this.callbackManager.add(arg).toString();
    } else if (type === 'array') {
      return arg.map(i => this.standardization(i));
    } else {
      return arg;
    }
  }

  /**
   * Instruct the Native module to perform operations based on the message sent by the listener.
   * @param {string} type - Such as dom, module and component.
   * @param {OptionsType} options - Include action, module and method.
   * @param {*} args - Args of a Vm.
   */
  public send(type: string, options: OptionsType, args: any): any {
    const {
      action,
      module,
      method
    } = options;
    if (type !== 'dom') {
      args = args.map(arg => this.standardization(arg));
    }
    switch (type) {
      case 'dom':
        if (typeof ace !== 'undefined' &&
            typeof ace.domCreateBody !== 'undefined' &&
            typeof ace.domAddElement !== 'undefined') {
          if (action === 'createBody') {
            ace.domCreateBody(
              0,
              args[0].type,
              args[0].attr,
              args[0].style,
              args[0].event
            );
          } else if (action === 'addElement') {
            ace.domAddElement(
              args[0],
              args[1].ref,
              args[1].type,
              args[1].attr,
              args[1].style,
              args[1].event,
              args[1].customComponent,
              args[2],
              this.instanceId
            );
          } else if (action === 'updateAttrs') {
            ace.updateElementAttrs(
              args[0],
              args[1],
              this.instanceId
            );
          } else if (action === 'updateStyle') {
            ace.updateElementStyles(
              args[0],
              args[1],
              this.instanceId
            );
          } else if (action === 'createFinish') {
            ace.onCreateFinish();
            return;
          } else if (action === 'updateFinish') {
            ace.onUpdateFinish();
            return;
          } else if (action === 'removeElement') {
            ace.removeElement(
              args[0],
              this.instanceId
            );
          } else {
            Log.error(
              'TaskCenter.js: send() unsupported action. IGNORING!'
            );
          }
          return;
        } else {
          Log.error(
            'TaskCenter.js: attempting acev1 method for calling native'
          );
          return;
        }

      case 'module':
        switch (module) {
          case 'system.fetch':
            if (method === 'fetch') {
              Log.error(
                'TaskCenter.js: send: module system.fetch. calling ace.fetch.'
              );
              ace.onFetchRequest(args[1], JSON.stringify(args));
              return;
            } else {
              Log.error(
                'TaskCenter.js: send: module system.fetch. unrecognized method. Ignoring.'
              );
            }
            break;
          case 'system.device':
            return ace.callNative(JSON.stringify(options), args[args.length - 1]);
          case 'system.router':
            return ace.callNative(JSON.stringify(options), JSON.stringify(args[0]));
          case 'system.prompt':
            return ace.callNative(JSON.stringify(options), JSON.stringify(args[0]));
          case 'system.app':
            return ace.callNative(JSON.stringify(options), args);
          case 'system.configuration':
            return ace.callNative(JSON.stringify(options), JSON.stringify(args[0]));
          case 'system.grid':
            return ace.callNative(JSON.stringify(options), args);
          case 'internal.jsResult':
            return ace.callNative(JSON.stringify(options), args);
          case 'timer':
            return ace.callNative(JSON.stringify(options), args);
          case 'system.offscreenCanvas':
            return ace.callNative(JSON.stringify(options), JSON.stringify(args[0]));
          case 'system.image':
            return ace.callNative(JSON.stringify(options), JSON.stringify(args[0]));
          case 'system.mediaquery':
            return ace.callNative(JSON.stringify(options), args);
          case 'animation':
            return ace.callNative(JSON.stringify(options), args[0]);
          case 'system.resource':
            return ace.callNative(JSON.stringify(options), args);
          case 'ohos.animator':
            return ace.callNative(JSON.stringify(options), JSON.stringify(args[0]));
          default:
            break;
        }
        break;

      case 'component':
        return ace.callComponent(options.ref, method, JSON.stringify(args));
      default:
        break;
    }
  }
}
