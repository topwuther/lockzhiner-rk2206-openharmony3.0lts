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
 * 2021.01.08 - Add some utils.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import { hasOwn } from './index';

/**
 * Verify whether the variable is empty.
 * @param {*} any - The variable which should be verified.
 * @return {boolean} The result whether the variable is empty.
 */
export function isEmpty(any) {
  if (!any || typeof any !== 'object') {
    return true;
  }
  for (const item in any) {
    if (hasOwn(any, item)) {
      return false;
    }
  }
  return true;
}

/**
 * Verify whether the valiable is null or undefined.
 * @param {*} any - The valiable which should be verified.
 * @return {boolean} The result whether the variable is null or undefined.
 */
export function isNull(any: any): boolean {
  return any === null || any === undefined;
}

/**
 * Remove an item from an array.
 * @param {*[]} arr - The array from which removes an item.
 * @param {*} item - The item which to be removed.
 * @return {*} The item which has been removed.
 */
export function removeItem(array: any[], item: any) {
  if (!array.length) {
    return;
  }
  if (typeof item !== 'number') {
    item = array.indexOf(item);
  }
  if (item > -1) {
    array.splice(item, 1);
  }
}

/**
* Find the value of the key.
* @param {string} key - The key.
* @param {Object} message - The object which to be checked.
* @return {*} The value of the key.
*/
export function getValue(key: string, message: object): any {
  const keys: string[] = key.split('.');
  if (keys.length === 0) {
    return null;
  }
  let value = message;
  for (const i in keys) {
    value = value[keys[i]];
    if (isNull(value)) {
      return null;
    }
  }
  return value;
}

/**
 * This class provide log.
 */
export class Log {
  /**
   * Provide debug log.
   * @param {*[]} message - The debug message.
   * @example
   * Log.debug('This is a debug message.');
   */
  public static debug(...message: any[]): void {
    aceConsole.debug('[JS Framework] (debug) %s', message);
  }

  /**
   * Provide info log.
   * @param {*[]} message - The info message.
   * @example
   * Log.info('This is an info message.');
   */
  public static info(...message: any[]): void {
    aceConsole.info('[JS Framework] (info) %s', message);
  }

  /**
   * Provide warn log.
   * @param {*[]} message - The warn message.
   * @example
   * Log.warn('This is a warn message.');
   */
  public static warn(...message: any[]): void {
    aceConsole.warn('[JS Framework] (warn) %s', message);
  }

  /**
   * Provide error log.
   * @param {*[]} message - The error message.
   * @example
   * Log.error('This is an error message.');
   */
  public static error(...message: any[]): void {
    aceConsole.error('[JS Framework] (error) %s', message);
  }
}
