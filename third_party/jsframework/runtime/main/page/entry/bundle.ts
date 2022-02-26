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
 * 2021.01.08 - Add i18n and dpi service.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import {
  isComponent,
  isModule,
  removePrefix,
  Log
} from '../../../utils/index';
import {
  registerCustomComponent,
  requireModule
} from '../register';
import { appMap } from '../../app/map';
import Vm from '../../model/index';
import Page from '../index';
import {updateDpi, updateLocale} from '../../app';

/**
 * Parse page code.
 * @param {Page} page - Page.
 * @param {string} name - Name of page.
 * @param {*[]} args
 */
export const defineFn = function(page: Page, name?: string, ...args: any[] | null): void {
  Log.debug(`Define a component ${name}.`);

  const parseContent: Function = args[1];
  let bundleContent: object = null;
  const moduleContent = { exports: {} };

  // Function to obtain bundle content.
  if (parseContent) {
    const pageRequire = (name: string) : any => {
      if (isModule(name)) {
        const packageName = page.packageName;
        const appFunction = (): Page => {
          if (page && page.doc) {
            return page;
          }
          if (packageName === 'notset') {
            return page;
          }
          const appPage: Page = appMap[packageName].getAppInstance();
          return appPage || page;
        };
        const moduleName: string = removePrefix(name);
        return requireModule(appFunction, moduleName);
      }
    };
    parseContent(pageRequire, moduleContent.exports, moduleContent);
    bundleContent = moduleContent.exports;
  }
  if (isComponent(name)) {
    const componetName: string = removePrefix(name);
    registerCustomComponent(page, componetName, bundleContent);
  }
};

/**
 * Create i18n and dpi service, a new Vm.
 * @param {Page} page
 * @param {string} name - Name of page.
 * @param {*} config
 * @param {*} data
 * @return {*}
 */
export function bootstrap(page: Page, name: string, data: any): any {
  Log.debug(`Bootstrap for ${name}.`);

  // Check component name.
  let componentName: string;
  if (isComponent(name)) {
    componentName = removePrefix(name);
  } else {
    return new Error(`Wrong component name: ${name}.`);
  }

  // Set i18n and dpi data.
  if (global && global.aceapp && page.options && page.options.i18n) {
    updateLocale(page.options.i18n);
  }
  if (global && global.aceapp && page.options && page.options.resourcesConfiguration) {
    updateDpi(page.options.resourcesConfiguration);
  }

  // Start i18n service.
  if (global && global.aceapp && global.aceapp._i18n_data_ && page.i18nService) {
    const I18nService: any = page.i18nService;
    global.aceapp.i18n = new I18nService(global.aceapp._i18n_data_);
  }

  // Start dpi service.
  if (global && global.aceapp && global.aceapp._dpi_data_ && page.dpiService) {
    const DpiService: any = page.dpiService;
    global.aceapp.dpi = new DpiService(global.aceapp._dpi_data_);
  }

  // Create a new Vm and mark rootVm.
  page.vm = new Vm(componentName, null, { _app: page, _rootVm: true }, null, data, null);
}
