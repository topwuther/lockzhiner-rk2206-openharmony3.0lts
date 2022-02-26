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
 * @fileOverview
 * page controls from native
 *
 * - fire event
 * - callback
 * - refresh
 * - destroy
 *
 * corresponded with the API of page manager (framework.js)
 */

import {
  typof,
  Log
} from '../../../utils/index';
import Page from '../index';
import Element from '../../../vdom/Element';

/**
 * Destroy a page.
 * @param {Page} page
 */
export function destroy(page: Page): void {
  Log.debug(`Destroy a page(${page.id}).`);

  if (page.vm) {
    page.vm.destroy();
  }

  page.id = null;
  page.options = null;
  page.vm = null;
  page.doc.taskCenter.destroyCallback();
  page.doc.destroy();
  page.doc = null;
  page.customComponentMap = null;
  page.commonModules = null;
}

/**
 * Fire an event to update domChanges of a page.
 * @param {Page} page
 * @param {*} ref
 * @param {*} type
 * @param {Object} e
 * @param {Object} domChanges
 * @param {*[] | null} args
 * @return any
 */
export function fireEvent(page: Page, ref?: any, type?: any, e?: object, domChanges?: boolean, ...args: any[] | null): any {
  Log.debug(`Fire a '${type}' event on an element(${ref}) in page(${page.id}), args = ${JSON.stringify(args)}.`);
  if (Array.isArray(ref)) {
    ref.some((ref) => {
      return fireEvent(page, ref, type, e) !== false;
    });
    return;
  }
  const el: Element = page.doc.getRef(ref);
  if (el) {
    const options: {params?: any} = {};
    if (args) {
      options.params = [...args];
    }
    const result: any = page.doc.fireEvent(el, type, e, domChanges, options);
    page.differ.flush();
    page.doc.taskCenter.send('dom', { action: 'updateFinish' }, []);
    return result;
  }
  return new Error(`Invalid element reference '${ref}'.`);
}

/**
 * Update page content on a callback.
 * @param {Page}  page
 * @param {number}  callbackId
 * @param {Object} data
 * @param {boolean} ifKeepAlive
 * @return {*}
 */
export function callback(page: Page, callbackId?: number, data?: object, ifKeepAlive?: boolean): any {
  Log.debug(`runtime/main/page/api/misc.js: Invoke a callback(${callbackId}) with `
    + `${JSON.stringify(data)} in page(${page.id}) ifKeepAlive = ${ifKeepAlive}.`);
  const result: any = page.doc.taskCenter.consumeCallback(callbackId, data, ifKeepAlive);
  updateActions(page);
  page.doc.taskCenter.send('dom', { action: 'updateFinish' }, []);
  return result;
}

/**
 * Fire an event to update domChanges of a page.
 * @param {Page} page - page.
 * @param {*} - correspond to fireEvent args.
 * @return {*}
 */
export function fireEventSync(page: Page, ...args: any[]): any {
  const result: any = fireEvent(page, ...args);
  if (page && page.callTasks) {
    let callbackId = '';
    for (let i = 0; i < args.length; i++) {
      if (args[i]._callbackId !== undefined) {
        callbackId = args[i]._callbackId;
        break;
      }
    }
    let resultStr = result || {};
    if (Array.isArray(resultStr)) {
      resultStr = result[0] || {};
    }
    if (resultStr.constructor !== String) {
      resultStr = JSON.stringify(resultStr);
    }
    return page.callTasks([{
      module: 'internal.jsResult',
      method: 'callbackNative',
      args: [callbackId, resultStr]
    }]);
  }
}

/**
 * Collect all virtual-DOM mutations together and send them to renderer.
 * @param {Page} page
 */
export function updateActions(page: Page): void {
  page.differ.flush();
}

/**
 * Call all tasks from a page to renderer (native).
 * @param {object} page
 * @param {*} tasks
 * @return {*}
 */
export function callTasks(page: Page, tasks: any): any {
  let result: any;
  if (typof(tasks) !== 'array') {
    tasks = [tasks];
  }
  tasks.forEach(task => {
    result = page.doc.taskCenter.send(
      'module',
      {
        module: task.module,
        method: task.method
      },
      task.args
    );
  });
  return result;
}
