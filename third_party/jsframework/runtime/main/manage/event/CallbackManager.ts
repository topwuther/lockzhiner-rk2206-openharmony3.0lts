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

export interface callbackObjInterface {
  [key: string]: Function;
}

/**
 * <p>Callback management of a certain page.</p>
 * <p>We can get the real callback that called from native by callback id which is unique for a callback.</p>
 */
export default class CallbackManager {
  public callbackMap: callbackObjInterface;
  public instanceId: string;
  public currCallbackId: number;

  constructor(instanceId: string) {
    this.instanceId = String(instanceId);
    this.currCallbackId = 0;
    this.callbackMap = {};
  }

  /**
   * Add a callback to callbacks object.
   * @param {*} callback - The callback from native.
   * @return {number} Last cllback id in object.
   */
  public add(callback: Function): number {
    this.currCallbackId++;
    this.callbackMap[this.currCallbackId] = callback;
    return this.currCallbackId;
  }

  /**
   * Remove a callback by callback id.
   * @param {number} callbackId - Callback id.
   * @return {Function} Callback that removed.
   */
  public remove(callbackId: number): Function {
    const callback: Function = this.callbackMap[callbackId];
    delete this.callbackMap[callbackId];
    return callback;
  }

  /**
   * Consume a callback by callback id.
   * @param {number} callbackId - Callback id.
   * @param {Object} data - Data that needed.
   * @param {boolean} ifKeepAlive - If keepAlive is false, delete this callback.
   * @return {*}
   */
  public consume(callbackId: number, data: object, ifKeepAlive: boolean): any | Error {
    const callback: Function = this.callbackMap[callbackId];
    if (typeof ifKeepAlive === 'undefined' || ifKeepAlive === false) {
      delete this.callbackMap[callbackId];
    }
    if (typeof callback === 'function') {
      return callback.call(null, data);
    }
    return new Error(`Invalid callback id '${callbackId}'.`);
  }

  /**
   * Clean all callbacks in callbackMap.
   */
  public destroy(): void {
    this.callbackMap = {};
  }

  /**
   * Check whether the callbacks object is empty.
   * @return {boolean} If callbacks object is empty, return true. Otherwise return false.
   */
  public isEmpty(): boolean {
    if (Object.keys(this.callbackMap).length === 0) {
      return true;
    }
    return false;
  }
}
