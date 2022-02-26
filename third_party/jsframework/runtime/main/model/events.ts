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
 * 2021.01.08 - Reconstruct the class 'Evt' and make it more adaptable to framework.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

/**
 * @fileOverview
 * Everything about component event which includes event object, event listener,
 * event emitter and lifecycle hooks.
 */

import Vm from './index';
import { PageLifecycleHooks } from './pageLife';

export type ExternalEvent = {'hook:_innerInit': () => void} | null;

/**
 * <p>Event object definition. An event object has `type`, `timestamp` and `detail`</p>
 * <p>from which a component emit. The event object could be dispatched to</p>
 * <p>parents or broadcasted to children except `this.stop()` is called.</p>
 */
export class Evt {
  private _timestamp: number;
  private _detail: any;
  private _type: string;
  private _shouldStop: boolean;

  constructor(type: string, detail: any) {
    this._timestamp = Date.now();
    this._detail = detail;
    this._type = type;
    if (detail instanceof Evt) {
      return detail;
    }
  }

  /**
   * Stop dispatch and broadcast.
   */
  public stop() {
    this.shouldStop = true;
  }

  /**
   * Check if it can't be dispatched or broadcasted
   */
  public hasStopped() {
    return this.shouldStop;
  }

  /**
   * ShouldStop of this Evt.
   * @type {boolean}
   */
  public get shouldStop() {
    return this._shouldStop;
  }

  public set shouldStop(newStop: boolean) {
    this._shouldStop = newStop;
  }

  /**
   * Detail of this Evt.
   * @type {*}
   * @readonly
   */
  public get detail() {
    return this._detail;
  }

  /**
   * Timestamp of this Evt.
   * @type {number}
   * @readonly
   */
  public get timestamp() {
    return this._timestamp;
  }

  /**
   * Type of this Evt.
   * @type {string}
   * @readonly
   */
  public get type() {
    return this._type;
  }
}

export const LIFE_CYCLE_TYPES: Array<PageLifecycleHooks | string> = [
  PageLifecycleHooks.ONINIT,
  PageLifecycleHooks.ONREADY,
  PageLifecycleHooks.ONSHOW,
  PageLifecycleHooks.ONHIDE,
  PageLifecycleHooks.ONBACKPRESS,
  PageLifecycleHooks.ONMENUPRESS,
  PageLifecycleHooks.ONMENUBUTTONPRESS,
  PageLifecycleHooks.ONSTARTCONTINUATUIN,
  PageLifecycleHooks.ONCOMPLETECONTINUATION,
  PageLifecycleHooks.ONSAVEDATA,
  PageLifecycleHooks.ONRESTOREDATA,
  PageLifecycleHooks.ONNEWREQUEST,
  PageLifecycleHooks.ONCONFIGURATIONUPDATED,
  PageLifecycleHooks.ONACTIVE,
  PageLifecycleHooks.ONINACTIVE,
  PageLifecycleHooks.ONLAYOUTREADY,
  'onAttached',
  'onDetached',
  'onPageShow',
  'onPageHide',
  'onDestroy'
];

/**
 * Init events.
 * @param {Vm} vm - Vm object.
 * @param {ExternalEvent} externalEvents - External events.
 */
export function initEvents(vm: Vm, externalEvents: ExternalEvent): void {
  const options = vm.vmOptions || {};
  for (const externalEvent in externalEvents) {
    vm.$on(externalEvent, externalEvents[externalEvent]);
  }
  LIFE_CYCLE_TYPES.forEach((type) => {
    vm.$on(`hook:${type}`, options[type]);
  });
}
