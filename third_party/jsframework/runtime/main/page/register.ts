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
 * 2021.01.08 - Strengthen init modules, getter and setter of modules.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

/*
 * native module register
 */
import { Log } from '../../utils/index';
import { interceptCallback } from '../manage/event/callbackIntercept';
import { getPluginModule } from '../extend/mediaquery/plugins';
import Page from './index';
import Document from '../../vdom/Document';
import { TaskCenter } from '../manage/event/TaskCenter';

let nativeModulesPkg: object = {};

/**
 * for UT
 */
export function getModule(moduleName) {
  return nativeModulesPkg[moduleName];
}

/**
 * for UT
 */
export function allModules() {
  return nativeModulesPkg;
}

/**
 * for UT
 */
export function clearModules() {
  nativeModulesPkg = {};
}

/**
 * Init modules for an app page.
 * @param {Object} modules
 */
export function initModules(modules: object): void {
  for (const moduleName in modules) {
    /* Obtains the module package name.
     * If modulename does not contain the package name, the default package name is _global_.
     */
    const s: string[] = moduleName.split('.');
    let pkg: string = '_global_';
    let cls: string;
    if (s.length === 1) {
      cls = s[0];
    } else {
      pkg = s[0];
      cls = s[1];
    }
    let clss: object = nativeModulesPkg[pkg];
    if (!clss) {
      clss = {};
      nativeModulesPkg[pkg] = clss;
    }
    let methods: object = clss[cls];
    if (!methods) {
      methods = {};
      clss[cls] = methods;
    }

    // Push each non-existed new method.
    for (let method of modules[moduleName]) {
      method = {
        name: method
      };
      if (!methods[method.name]) {
        methods[method.name] = method;
      }
    }
  }
}

/**
 * Get a module of methods for an app page.
 * @param {Page | Function} app
 * @param {string} name
 * @return {Object}
 */
export function requireModule(app: Page | Function, name: string): object {
  const s: string[] = name.split('.');
  let pkg: string = s[0];
  let cls: string = s[1];
  const moduleName: string = s[1];
  let clss: object = nativeModulesPkg[pkg];

  // If can not found pkg from nativeModulesPkg, then use '_global_' as pkg to find again.
  if (!clss && !cls) {
    cls = pkg;
    pkg = '_global_';
    clss = nativeModulesPkg[pkg];
  }
  if (cls) {
    const target: object = {};
    const methods: object = clss[cls];
    bind(app, target, methods, name, true);
    if (pkg && (pkg === 'system' || pkg === '@system')) {
      const module: object = getPluginModule(target, moduleName);
      if (module) {
        return module;
      }
    }
    return target;
  } else {
    const target: object = {};
    for (const clsName in clss) {
      target[clsName] = {};
      const methods: object = clss[clsName];
      bind(app, target[clsName], methods, name + '.' + clsName, true);
    }
    return target;
  }
}

function isGlobalModule(name: string) {
  const clss: object = nativeModulesPkg['_global_'];
  const globalModules: string[] = Object.keys(clss);
  if (globalModules.indexOf(name) === -1) {
    return false;
  }
  return true;
}

function bind(app: Page | Function, target: object, methods: object, moduleName: string, needPromise: boolean) {
  for (const methodName in methods) {
    Object.defineProperty(target, methodName, {
      configurable: true,
      enumerable: true,
      get: function moduleGetter() {
        if (this._modifyMethods && this._modifyMethods[methodName] &&
             typeof this._modifyMethods[methodName] === 'function') {
          return this._modifyMethods[methodName];
        }
        return (...args) => {
          let promise;
          if (!isGlobalModule(moduleName)) {
            const ret: any = interceptCallback(args, needPromise);
            args = needPromise ? ret.args : ret;
            promise = needPromise ? ret.promise : undefined;
          }
          const appInstance: Page = typeof app === 'function' ? app() : app;
          if (moduleName === 'system.router' && methodName === 'getParams') {
            if (appInstance.routerParams) {
              return appInstance.routerParams.paramsData;
            }
          }
          const ret: any = appInstance.callTasks({
            module: moduleName,
            method: methodName,
            args: args
          });

          // API return exception.
          if (ret && ret.__EXCEPTION__) {
            const e: any = new Error();
            e.code = ret.code;
            e.message = ret.message;
            throw e;
          }
          if (ret && ret.__PROMISE__ && promise) {
            return promise.promise;
          } else {
            const taskCenter: Document | TaskCenter = appInstance.doc && appInstance.doc.taskCenter;
            if (taskCenter) {
              if (args[0] && args[0].__onlyPromise) {
                taskCenter.removeCallback(args[0].__callbackId);
              } else if (args.length > 1 && args[1] && args[1].__onlyPromise) {
                taskCenter.removeCallback(args[1].__callbackId);
              }
            }
            return ret;
          }
        };
      },
      set: function moduleSetter(value: Function) {
        if (typeof value === 'function') {
          this._modifyMethods = this._modifyMethods || {};
          this._modifyMethods[methodName] = value;
        }
      }
    });
  }
}

/**
 * Register a custom component options.
 * @param {Page} app
 * @param {string} name
 * @param {Object} def - The content of component.
 * @return {*}
 */
export function registerCustomComponent(app: Page, name: string, def: object): any {
  const { customComponentMap } = app;
  if (customComponentMap[name]) {
    Log.error(`Define a component(${name}) that already exists.`);
    return;
  }
  customComponentMap[name] = def;
}
