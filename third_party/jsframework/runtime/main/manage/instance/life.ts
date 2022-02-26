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

import { destroy } from '../../page/api/index';
import { App } from '../../app/App';
import { resetTarget } from '../../reactivity/dep';
import Page from '../../page';
import { init as initApp } from '../../page/api/index';
import { appCreate, Options } from '../../app/index';

const pageMap: Map<string, Page> = App.pageMap;

/**
 * Create a page.
 * @param {string} id - Page id.
 * @param {string} code - JS Bundle code.
 * @param {Object} options - Options of a page instance.
 * @param {Object} data - Data that needed.
 * @param {*} env - Such as: { created, ... services }.
 * @return {Object | Error}
 */
export function createInstance(id: string, code: string, options: Options, data: object, env: any): object | Error {
  const { services } = env;
  const { I18n, dpi } = services;
  resetTarget();

  let page: Page = pageMap.get(id);
  let result: object;
  if (!page) {
    page = new Page(id, options, options.packageName, data);
    page.i18nService = I18n;
    page.dpiService = dpi;
    appCreate(page, options, data, services);
    pageMap.set(id, page);
    result = initApp(page, code, data, services);
  } else {
    result = new Error(`invalid page id "${id}"`);
  }
  return result;
}

/**
 * Destroy a page by id.
 * @param  {string} id - Page id.
 * @return {Object} The reset pageMap.
 */
export function destroyInstance(id: string): object {
  if (typeof markupState === 'function') {
    markupState();
  }
  resetTarget();
  const page: Page = pageMap.get(id);
  if (!page) {
    return new Error(`invalid page id '${id}'.`);
  }
  pageMap.delete(id);
  destroy(page);

  // Tell v8 to do a full Garbage Collection every eighteen times.
  const remainder: number = Math.round(Number(id)) % 18;
  if (!remainder) {
    if (typeof notifyTrimMemory === 'function') {
      notifyTrimMemory();
    }
  }
  return pageMap;
}
