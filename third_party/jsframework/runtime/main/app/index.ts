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

import { Log } from '../../utils/index';
import {
  defineFn,
  bootstrap,
  genTimerAPI
} from './bundle';
import { appMap } from './map';
import { getPageGlobal } from './helper';
import { App } from './App';
import Page from '../page/index';
import { destroy } from '../page/api/index';
import { mockSystemPlugin } from '../extend/systemplugin/index';
import { compileBundle } from '../page/entry/init';
import { removePrefix } from '../util/index';
import { requireModule } from '../page/register';

/**
 * Device information for mediaQuery.
 */
export interface MediaQueryInfo {
  'orientation': string;
  'deviceType': string;
  'deviceWidth': string;
  'deviceHeight': string;
  'roundScreen': boolean;
  'width': string;
  'height': string;
  'isInit': boolean;
  'resolution': string;
  'aspectRatio': string;
  'darkMode': string;
}

/**
 * Information of App.
 */
export interface Options extends MediaQueryInfo {
  'appInstanceId': string;
  'packageName': string;
  'appCreate': boolean;
  'appCode': string | Function;
  'pcPreview': string;
  'resourcesConfiguration': object;
  'i18n': object;
  'language': string;
  'appGlobalData'?: object;
  'bundleUrl': string;
}

/**
 * Framework Services.
 */
export interface Services {
  service: object;
  I18n?: Function;
  dpi?: Function;
}

interface ParseOptions {
  $app_define$(...args: any[]): void; // eslint-disable-line camelcase
  $app_bootstrap$(name: string, config: any, _data: any): void; // eslint-disable-line camelcase
  $app_require$(name: string): void; // eslint-disable-line camelcase
}

const pageMap: Map<string, Page> = App.pageMap;

/**
 * Create app page, run jsbundle code.
 * @param {Page} page
 * @param {Options} options
 * @param {Object} data
 * @param {Services} services
 */
export function appCreate(page: Page, options: Options, data: object, services: Services): void {
  if (!options || !options.appCreate || !options.appCode) {
    return;
  }
  if (options.pcPreview && options.pcPreview === 'enable') {
    mockSystemPlugin();
  }
  const packageName: string = page.packageName;
  const appPage: Page = new Page(options.appInstanceId, options, packageName, data);
  pageMap.set(appPage.id, appPage);
  Log.debug(`Create a page with: ${packageName}.`);
  appMap[packageName] = new App(packageName, options.appInstanceId);
  const timerAPIs: object = genTimerAPI(appPage);
  appMap[packageName].setTimer(timerAPIs);
  const code = options.appCode;
  global.__appProto__ = getPageGlobal(packageName);

  // prepare page env methods
  const appDefine = (...args: any[]): void => defineFn(page, packageName, ...args);
  const appBootstrap = (name: string, config: any, _data: any): void => {
    bootstrap(page, packageName, name, config, _data || data);
    Log.debug(`After create a page(${page.id}).`);
  };

  // require in top app(instance)
  const appRequireModule = name => requireModule(appPage, removePrefix(name));
  const parseOptions: ParseOptions = {
    $app_define$: appDefine,
    $app_bootstrap$: appBootstrap,
    $app_require$: appRequireModule
  };

  // Compile js bundle code and get result.
  if (typeof code === 'function') {
    Log.info('call Function directly when appCreate');
    code.call(global, parseOptions);
  } else {
    // Function with code and use strict mode.
    const functionCode: string = `(function(global){\n\n"use strict";\n\n ${code} \n\n})(this.__appProto__)`;
    compileBundle(functionCode, 'app.js', parseOptions, timerAPIs, services);
  }
}

/**
 * Emit onError event.
 * @param {string} packageName
 * @param {*} errors
 */
export function appError(packageName: string, errors: any): void {
  Log.debug(`AppError an app with: ${packageName}.`);
  const app: App = appMap[packageName];
  if (!app) {
    Log.debug(`AppError an app error ${packageName}.`);
    return;
  }
  Log.debug(`AppError an app error ${packageName}.`);
  app.emitEvent('hook:onError', errors);
}

/**
 * Emit onShow event.
 * @param {string} packageName - Package name.
 */
export function appShow(packageName: string): void {
  Log.debug(`Show an app with: ${packageName}.`);
  const app: App = appMap[packageName];
  if (!app) {
    Log.debug(`Show an app error ${packageName}.`);
    return;
  }
  app.emitEvent('hook:onShow');
}

/**
 * Emit onHide event.
 * @param {string} packageName - Package name.
 */
export function appHide(packageName: string): void {
  Log.debug(`Hide an app with: ${packageName}.`);
  const app: App = appMap[packageName];
  if (!app) {
    Log.debug(`Hide an app error ${packageName}.`);
    return;
  }
  app.emitEvent('hook:onHide');
}

/**
 * Emit onDestroy event.
 * @param {string} packageName - Package name.
 */
export function appDestroy(packageName: string): void {
  Log.debug(`Destroy an app with: ${packageName}.`);
  const app: App = appMap[packageName];
  if (!app) {
    Log.error(`Destroy an app error ${packageName}.`);
    return;
  }
  app.emitEvent('hook:onDestroy');
  app.deleteGlobalKeys();
  delete appMap[packageName];
  const appPage: Page = pageMap.get(app.appInstanceId);
  if (appPage) {
    if (appPage.doc.taskCenter.callbackIsEmpty()) {
      appPage.callTasks([{
        module: 'internal.jsResult',
        method: 'appDestroyFinish',
        args: []
      }]);
      destroy(appPage);
      pageMap.delete(appPage.id);
    } else {
      appPage.destroyed = true;
    }
  }
}

/**
 * Init language resource.
 * @param {*} i18nData
 */
export function updateLocale(i18nData: any): void {
  if (i18nData) {
    global.aceapp._i18n_data_ = { messages: i18nData.resources };
  } else {
    global.aceapp._i18n_data_ = null;
  }
}

/**
 * Init image dpi.
 * @param {Object} dpiData
 */
export function updateDpi(dpiData: object): void {
  if (dpiData) {
    global.aceapp._dpi_data_ = { images: dpiData };
  } else {
    global.aceapp._dpi_data_ = null;
  }
}
