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
 * 2021.01.08 - Add init global data and hook life cycle of App.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import {
  isModule,
  removePrefix,
  isApplication,
  removeApplicationPrefix,
  Log
} from '../../utils/index';
import {
  registerCustomComponent,
  requireModule
} from '../page/register';
import { appMap } from './map';
import { updateLocale, updateDpi } from './index';
import Page from '../page/index';
import { App } from './App';

const APP_LIFE_CYCLE_TYPES: string[] = ['onCreate', 'onError', 'onDestroy', 'onShow', 'onHide'];

export let CSS_INHERITANCE: string[];

/**
 * Parse app page code.
 * @param {Page} page
 * @param {string} packageName - PackageName of App.
 * @param {string} name - Name of page.
 * @param {*[]} args
 */
export const defineFn = function(page: Page, packageName: string, name?: string, ...args: any[] | null): void {
  Log.debug(`Define a page ${name}.`);
  const parseContent: Function = args[1];
  let bundleContent: any = null;

  // Function to obtain bundle content.
  if (parseContent) {
    const pageRequire = (name: string) : any => {
      if (isModule(name)) {
        const appFunction = (): Page => {
          const appPage: Page = appMap[packageName].getAppInstance();
          return appPage || page;
        };
        return requireModule(appFunction, removePrefix(name));
      }
    };
    const moduleContent = { exports: {} };
    parseContent(pageRequire, moduleContent.exports, moduleContent);
    bundleContent = moduleContent.exports;

    let minPlatformVersion: number = 5;
    if (bundleContent.manifest) {
      minPlatformVersion = bundleContent.manifest.minPlatformVersion;
    }
    CSS_INHERITANCE = minPlatformVersion > 5 ?
      ['fontFamily', 'fontWeight', 'fontSize', 'fontStyle', 'textAlign', 'lineHeight', 'letterSpacing', 'color', 'visibility'] :
      [];
  }

  // Apply bundleContent.
  if (isApplication(name)) {
    const componetName: string = removeApplicationPrefix(name);
    registerCustomComponent(page, componetName, bundleContent);
  }
};

/**
 * Set i18n and dpi data, hook life cycle of App.
 * @param {Page} page
 * @param {string} packageName - PackageName of App.
 * @param {string} name - Name of page.
 * @param {*} config
 * @param {*} data
 * @return {*}
 */
export function bootstrap(page: Page, packageName: string, name: string, config: any, data: any): any {
  Log.debug(`Bootstrap for ${name}.`);
  Log.debug(`${config} ${data}`);

  // Check component name.
  let componentName: string;
  if (isApplication(name)) {
    componentName = removeApplicationPrefix(name);
  } else {
    return new Error(`Wrong component name: ${name}.`);
  }

  // Init global data when page first load,
  // global.aceapp.$data means config.data in manifest.json, can add new data by this.$app.$data api.
  if (page.options && page.options.appCreate) {
    const getApp = function() {
      return global.aceapp;
    };
    global.getApp = getApp;
    global.aceapp = {};
    global.aceapp.$data = page.options.appGlobalData || {};

    // Set i18n and dpi data.
    if (page.options.i18n) {
      updateLocale(page.options.i18n);
    }
    if (page.options.resourcesConfiguration) {
      updateDpi(page.options.resourcesConfiguration);
    }
  }
  if (page.customComponentMap) {
    const app: App = appMap[packageName];
    if (app) {
      const options: object = page.customComponentMap[componentName] || {};
      const aceapp: any = global.aceapp || {};
      for (const key in options) {
        if (!isReserved(key)) {
          app[key] = options[key];
          aceapp[key] = options[key];
        }
      }
      aceapp.$def = aceapp;
      aceapp._def = aceapp.$def;
      aceapp._data = aceapp.$data;

      // Exit api to $app.
      aceapp.exit = function(): void {
      };
      APP_LIFE_CYCLE_TYPES.forEach((type) => {
        app.onEvent(`hook:${type}`, options[type]);
      });

      // Last fire on Create.
      Log.debug(`Page "onCreate" lifecycle in app(${app.packageName}).`);
      app.emitEvent('hook:onCreate');
    }
  }
}

/**
 * Check input param is onCreate or onDestroy.
 * @param {string} key
 * @return {boolean}
 */
function isReserved(key: string): boolean {
  if (key === 'onCreate' || key === 'onDestroy') {
    return true;
  }
  return false;
}

/**
 * Return timerAPIs.
 * @param {Page} page - Page
 * @return {Object}
 */
export function genTimerAPI(page: Page): object {
  const timerAPIs: object = {};

  // Timer APIs polyfill in native
  const timer: any = page.requireModule('timer');
  const animation = page.requireModule('animation');
  Object.assign(timerAPIs, {
    setTimeout: (...args) => {
      const handler = () => {
        args[0](...args.slice(2));
      };
      timer.setTimeout(handler, args[1]);
      return page.doc.taskCenter.callbackManager.currCallbackId.toString();
    },
    setInterval: (...args) => {
      const handler = () => {
        args[0](...args.slice(2));
      };
      timer.setInterval(handler, args[1]);
      return page.doc.taskCenter.callbackManager.currCallbackId.toString();
    },
    clearTimeout: (n) => {
      timer.clearTimeout(n);
      page.doc.taskCenter.callbackManager.remove(n);
    },
    clearInterval: (n) => {
      timer.clearInterval(n);
      page.doc.taskCenter.callbackManager.remove(n);
    },
    requestAnimationFrame: (...args) => {
      const handler = function(timestamp) {
        args[0](timestamp, ...args.slice(1));
      };
      animation.requestAnimationFrame(handler);
      return page.doc.taskCenter.callbackManager.currCallbackId.toString();
    },
    cancelAnimationFrame: (n) => {
      animation.cancelAnimationFrame(n);
    }
  });
  return timerAPIs;
}
