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
 * - init bundle
 *
 * corresponded with the API of page manager (framework.js)
 */

import {
  Log
} from '../../../utils/index';
import { removePrefix } from '../../util/index';
import {
  defineFn,
  bootstrap
} from './bundle';
import { updateActions } from '../api/misc';
import { getPageGlobal } from '../../app/helper';
import { Image } from '../Image';
import { OffscreenCanvas } from '../OffscreenCanvas';
import Page from '../index';
import { Services } from '../../app/index';
import { requireModule } from '../register';
import { App } from '../../app/App';

interface ParseOptions {
  $app_define$(...args: any[]): void; // eslint-disable-line camelcase
  $app_bootstrap$(name: string): void; // eslint-disable-line camelcase
  $app_require$(name: string): void; // eslint-disable-line camelcase
  Image(): void;
  OffscreenCanvas(width, height): void;
}

/**
 * Init a page by run code with data.
 * @param {Page} page
 * @param {string} code
 * @param {Object} data
 * @param {Services} services
 * @return {*}
 */
export function init(page: Page, code: string | Function, data: object, services: Services): any {
  Log.debug('Intialize a page with:\n', data);
  let result;

  // Methods to parse code.
  const pageDefine = (...args) => defineFn(page, ...args);
  const pageBoot = (name) => {
    result = bootstrap(page, name, data);
    updateActions(page);
    page.doc.taskCenter.send('dom', { action: 'createFinish' }, []);
    Log.debug(`After initialized a page(${page.id}).`);
  };

  const packageName = page.packageName;
  const appFunction = () => {
    if (page && page.doc) {
      return page;
    }
    // card not has packageName
    if (packageName === 'notset') {
      return page;
    }
    const instance = App.pageMap.get(page.id);
    return instance || page;
  };

  const pageRequireModule = name => requireModule(appFunction, removePrefix(name));

  const imageObj: () => Image = function() {
    return new Image(page);
  };
  const offscreenCanvasObj: (width, height) => OffscreenCanvas = function(width, height) {
    return new OffscreenCanvas(page, width, height);
  };
  const options: ParseOptions = {
    $app_define$: pageDefine,
    $app_bootstrap$: pageBoot,
    $app_require$: pageRequireModule,
    Image: imageObj,
    OffscreenCanvas: offscreenCanvasObj
  };

  // Support page global and init language.
  global.__appProto__ = getPageGlobal(page.packageName);
  global.language = page.options.language;

  let functionCode: string;
  if (typeof code !== 'function') {
    functionCode = `(function(global){\n\n"use strict";\n\n ${code} \n\n})(this.__appProto__)`;
  }

  // Compile js bundle code and get result.
  if (typeof code === 'function') {
    code.call(global, options);
  } else {
    compileBundle(functionCode, page.doc.url, options, services);
  }
  return result;
}

/**
 * Run bundle code by a new function.
 * @param {string} functionCode - Js bundle code.
 * @param {Object[]} args - Global methods for compile js bundle code.
 * @return {*}
 */
export function compileBundle(functionCode: string, file: string, ...args: object[]): any {
  const funcKeys: string[] = [];
  const funcValues: Function[] = [];
  args.forEach((module) => {
    for (const key in module) {
      funcKeys.push(key);
      funcValues.push(module[key]);
    }
  });

  // If failed to run code on native, then run code on framework.
  if (!compileBundleNative(funcKeys, funcValues, functionCode, file)) {
    const resolveFunction: Function = new Function(funcKeys.toString(), functionCode);
    return resolveFunction(...funcValues);
  }
}

/**
 * Call a new function generated on the V8 native side.
 * @param {string[]} funcKeys
 * @param {Function[]} funcValues
 * @param {string} functionCode
 * @return {boolean} Return true if no error occurred.
 */
function compileBundleNative(funcKeys: string[], funcValues: Function[], functionCode: string, file: string): boolean {
  if (typeof compileAndRunBundle !== 'function') {
    return false;
  }

  let isSuccess: boolean = false;
  const bundle: string = `(function (${funcKeys.toString()}) {${functionCode}})`;
  try {
    const compileFunction: Function = compileAndRunBundle(bundle, file);
    if (compileFunction && typeof compileFunction === 'function') {
      compileFunction(...funcValues);
      isSuccess = true;
    }
  } catch (e) {
    Log.error(e);
  }
  return isSuccess;
}
