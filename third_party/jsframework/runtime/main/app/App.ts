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

import Page from '../page/index';

/**
 * This class defines the information of a application.
 */
export class App {
  public static pageMap: Map<string, Page> = new Map();
  private _packageName: string;
  private _appInstanceId: string;
  private _events: object;
  private _globalKeys: any[];
  private _appGlobal: ProxyConstructor;

  constructor(packageName: string, appInstanceId: string) {
    this._packageName = packageName || 'notset';
    this._appInstanceId = appInstanceId;
    this._events = {};
    this._globalKeys = [];
    bindGlobal(this);
  }

  /**
   * PackageName of this App.
   * @type {string}
   */
  public get packageName() {
    return this._packageName;
  }

  public set packageName(packageName: string) {
    this._packageName = packageName;
  }

  /**
   * AppInstanceId of this App.
   * @type {string}
   */
  public get appInstanceId() {
    return this._appInstanceId;
  }

  public set appInstanceId(appInstanceId: string) {
    this._appInstanceId = appInstanceId;
  }

  /**
   * GlobalKeys of this App.
   * @type {*}
   */
  public get globalKeys() {
    return this._globalKeys;
  }

  public set globalKeys(key: any) {
    this._globalKeys.push(key);
  }

  /**
   * AppGlobal of this App.
   * @type {ProxyConstructor}
   */
  public get appGlobal() {
    return this._appGlobal;
  }

  public set appGlobal(appGlobal: ProxyConstructor) {
    this._appGlobal = appGlobal;
  }

  /**
    * Bind life cycle of App.
    * @param {string} type - Function name of life cycle.
    * @param {Function} handler - Function of life cycle.
  */
  public onEvent(type: string, handler: Function): void {
    if (!type || typeof handler !== 'function') {
      return;
    }
    const events: object = this._events;
    const handlerList: Function[] = events[type] || [];
    handlerList.push(handler);
    events[type] = handlerList;
  }

  /**
   * Emit event.
   * @param {string} type - Event of type.
   * @param {*} errors
   */
  public emitEvent(type: string, errors?: any): void {
    const events: object = this._events;
    const handlerList: Function[] = events[type];

    if (handlerList) {
      handlerList.forEach((handler) => {
        handler.call(global.aceapp, errors);
      });
    }
  }

  /**
   * Delete globalKeys of App.
   */
  public deleteGlobalKeys(): void {
    if (this._globalKeys) {
      let i: number = this._globalKeys.length;
      while (i--) {
        const key: any = this._globalKeys.splice(i, 1)[0];
        if (key === 'setTimeout') {
          global[key] = undefined;
        } else {
          delete global[key];
        }
      }
    }
  }

  /**
   * Assign timerAPIs to appGlobal.
   * @param {Object} timerAPIs - TimerAPI.
   */
  public setTimer(timerAPIs: object): void {
    const that = this;
    Object.keys(timerAPIs).forEach((api) => {
      that._appGlobal[api] = timerAPIs[api];
    });
  }

  /**
   * Get page.
   * @return {Page} Page.
   */
  public getAppInstance(): Page {
    return App.pageMap.get(this._appInstanceId);
  }
}

/**
 * Assign appGlobal of App.
 * @param {App} app - App instance.
 */
function bindGlobal(app: App): void {
  app.appGlobal = new Proxy(Object.create(global), {
    set(target, key, value, receiver) {
      const ret: boolean = Reflect.set(target, key, value, receiver);
      if (receiver[key] === target[key]) {
        // set in app Global
        global[key] = value;
        app.globalKeys = key;
      }
      return ret;
    }
  });
}
