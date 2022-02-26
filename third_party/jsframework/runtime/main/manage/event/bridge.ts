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
 * 2021.01.08 - Add fireEventSync event to eventHandlers and page.destroyed judgment to 'receiveTasks'.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import { Log } from '../../../utils/index';
import { App } from '../../app/App';
import Page from '../../page';
import {
  fireEvent,
  callback,
  fireEventSync,
  destroy
} from '../../page/api/index';

const pageMap: Map<string, Page> = App.pageMap;

const eventHandlers = {
  /**
   * Invoke the fireEvent function.
   * @param {string} id - Page id.
   * @param {*} args - Args.
   */
  fireEvent: (id: string, ...args: any[]) => {
    return fireEvent(pageMap.get(id), ...args);
  },

  /**
   * Invoke the callback function.
   * @param {string} id - Page id.
   * @param {*} args - Args
   */
  callback: (id: string, ...args: any[]) => {
    return callback(pageMap.get(id), ...args);
  },

  /**
   * Invoke the fireEventSync function.
   * @param {string} id - Page id.
   * @param {*} args - Args.
   */
  fireEventSync: (id: string, ...args: any[]) => {
    return fireEventSync(pageMap.get(id), ...args);
  }
};

/**
 * Accept calls from native (event or callback).
 * @param {string} id - Page id.
 * @param {*} tasks list with `method` and `args`.
 * @return {*}
 */
export function receiveTasks(id: string, tasks: any[]): any[] | Error {
  id = id.toString();
  Log.debug(`ReceiveTasks id ${id}, tasks: ${JSON.stringify(tasks)}`);
  const page: Page = pageMap.get(id);
  if (page && Array.isArray(tasks)) {
    const results = [];
    tasks.forEach((task) => {
      const handler = eventHandlers[task.method];
      const args = [...task.args];
      if (typeof handler === 'function') {
        args.unshift(id);
        results.push(handler(...args));
      }
    });
    if (page.destroyed && page.doc.taskCenter.callbackIsEmpty()) {
      page.callTasks([{
        module: 'internal.jsResult',
        method: 'appDestroyFinish',
        args: []
      }]);
      destroy(page);
      pageMap.delete(id);
    }
    return results;
  }
  return new Error(`Invalid page id '${id}' or tasks.`);
}
