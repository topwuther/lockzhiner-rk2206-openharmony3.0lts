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

import { Log } from '../../utils/index';
import Watcher from '../reactivity/watcher';
import Element from '../../vdom/Element';
import Vm from './index';

/**
 * Enum for Page lifecycle hooks.
 * @enum {string}
 * @readonly
 */
/* eslint-disable no-unused-vars */
export const enum PageLifecycleHooks {
  /**
   * ONINIT Type
   */
  ONINIT = 'onInit',
  /**
   * ONREADY Type
   */
  ONREADY = 'onReady',
  /**
   * ONSHOW Type
   */
  ONSHOW = 'onShow',
  /**
   * ONHIDE Type
   */
  ONHIDE = 'onHide',
  /**
   * ONBACKPRESS Type
   */
  ONBACKPRESS = 'onBackPress',
  /**
   * ONMENUPRESS Type
   */
  ONMENUPRESS = 'onMenuPress',
  /**
   * ONMENUBUTTONPRESS Type
   */
  ONMENUBUTTONPRESS = 'onMenuButtonPress',
  /**
   * ONSUSPENDED Type
   */
  ONSUSPENDED = 'onSuspended',
  /**
   * ONSTARTCONTINUATUIN Type
   */
  ONSTARTCONTINUATUIN = 'onStartContinuation',
  /**
   * ONCOMPLETECONTINUATION Type
   */
  ONCOMPLETECONTINUATION = 'onCompleteContinuation',
  /**
   * ONSAVEDATA Type
   */
  ONSAVEDATA = 'onSaveData',
  /**
   * ONRESTOREDATA Type
   */
  ONRESTOREDATA = 'onRestoreData',
  /**
   * ONNEWREQUEST Type
   */
  ONNEWREQUEST = 'onNewRequest',
  /**
   * ONCONFIGURATIONUPDATED Type
   */
  ONCONFIGURATIONUPDATED = 'onConfigurationUpdated',
  /**
   * ONLAYOUTREADY Type
   */
  ONLAYOUTREADY = 'onLayoutReady',
  /**
   * ONACTIVE Type
   */
  ONACTIVE = 'onActive',
  /**
   * ONLAYOUTREADY Type
   */
  ONINACTIVE = 'onInactive'
}
/* eslint-enable no-unused-vars */

const PAGE_LIFE_CYCLE_TYPES: Array<PageLifecycleHooks> = [
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
  PageLifecycleHooks.ONLAYOUTREADY,
  PageLifecycleHooks.ONACTIVE,
  PageLifecycleHooks.ONINACTIVE
];

/**
 * Bind page lifeCycle.
 * @param {Vm} vm - Vm object.
 * @param {Element} element - Element object.
 */
export function bindPageLifeCycle(vm: Vm, element: Element): void {
  const options = vm.vmOptions || {};
  PAGE_LIFE_CYCLE_TYPES.forEach(type => {
    let eventType;
    if (type === PageLifecycleHooks.ONSHOW) {
      eventType = 'viewappear';
    } else if (type === PageLifecycleHooks.ONHIDE) {
      eventType = 'viewdisappear';
    } else if (type === PageLifecycleHooks.ONBACKPRESS) {
      eventType = 'clickbackitem';
    } else if (type === PageLifecycleHooks.ONSUSPENDED) {
      eventType = 'viewsuspended';
    } else if (type === 'onConfigurationUpdated') {
      eventType = 'onConfigurationUpdated';
    } else if (type === 'onLayoutReady') {
      eventType = 'layoutReady';
    } else if (type === 'onActive') {
      eventType = 'viewactive';
    } else if (type === 'onInactive') {
      eventType = 'viewinactive';
    } else {
      eventType = type;
    }

    const handle = options[type];
    let isEmitEvent = false;
    if (handle) {
      isEmitEvent = true;
      element.addEvent(eventType, eventHandle);
    } else {
      if (type === PageLifecycleHooks.ONSHOW || type === PageLifecycleHooks.ONHIDE) {
        element.addEvent(eventType, eventHandle);
      }
    }

    /**
     * Hadle event methods.
     * @param {*} event - Event methods.
     * @param {*} args - Arg list.
     * @return {*}
     */
    function eventHandle(event, ...args: any[]): any {
      if (type === PageLifecycleHooks.ONSHOW) {
        emitSubVmLife(vm, 'onPageShow');
        vm.visible = true;
      } else if (type === PageLifecycleHooks.ONHIDE) {
        emitSubVmLife(vm, 'onPageHide');
        vm.visible = false;
      } else if (type === PageLifecycleHooks.ONCONFIGURATIONUPDATED) {
        return vm.$emitDirect(`hook:${type}`, ...args);
      }

      Log.debug(`EventHandle: isEmitEvent = ${isEmitEvent}, event = ${event}, args = ${JSON.stringify(args)}.`);
      if (isEmitEvent) {
        if (type === PageLifecycleHooks.ONNEWREQUEST) {
          return handleNewRequest(args[0]);
        } else if (type === PageLifecycleHooks.ONSAVEDATA) {
          return handleSaveData();
        } else if (type === PageLifecycleHooks.ONRESTOREDATA) {
          return handleRestoreData(args[0]);
        } else if (type === PageLifecycleHooks.ONCOMPLETECONTINUATION) {
          return vm.$emitDirect(`hook:${type}`, ...args);
        } else {
          return vm.$emit(`hook:${type}`, {}, ...args);
        }
      }
    }

    /**
     * Handle saveData.
     * @return {string | boolean} If no hook, return false. Otherwise return vm.shareData.
     */
    function handleSaveData(): string | boolean {
      const allData = {
        saveData: {},
        shareData: {}
      };
      const result = vm.$emitDirect(`hook:${type}`, allData.saveData);
      if (!result) {
        return false;
      }
      const shareResult = vm.shareData || {};
      if (shareResult instanceof Object && !(shareResult instanceof Array)) {
        allData.shareData = shareResult;
      }
      return JSON.stringify(allData);
    }

    /**
     * Handle restore Data.
     * @param {*} restoreData - Restore data.
     * @return {*}
     */
    function handleRestoreData(restoreData: any) {
      const saveData = restoreData.saveData || {};
      const shareData = restoreData.shareData || {};

      Object.assign(vm.shareData, shareData);
      return vm.$emitDirect(`hook:${type}`, saveData);
    }
    function handleNewRequest(data: any) {
      Object.assign(vm.data, data);
      return vm.$emitDirect(`hook:${type}`);
    }
  });
}

/**
 * Watch a calc function and callback if the calc value changes.
 * @param {Vm} vm - Vm object.
 * @param {string} data - Data that needed.
 * @param {Function | string} callback - Callback function.
 * @return {*}
 */
export function watch(vm: Vm, data: string, callback: ((...args: any) => any) | string): any {
  function calc() {
    let arr = [];
    arr = data.split('.');
    let retData = this;
    arr.forEach(type => {
      if (retData) {
        retData = retData[type];
      }
    });
    return retData;
  }
  const watcher = new Watcher(vm, calc, function(value, oldValue) {
    if (typeof value !== 'object' && value === oldValue) {
      return;
    }
    if (typeof callback === 'function') {
      callback(value, oldValue);
    } else {
      if (vm.methods[callback] && typeof vm.methods[callback] === 'function') {
        vm.methods[callback](value, oldValue);
      }
    }
  }, null);
  return watcher.value;
}

/**
 * Prop is assigned to data to observe prop.
 * @param {Vm} vm - Vm object.
 */
export function initPropsToData(vm: Vm): void {
  vm.props.forEach(prop => {
    if (vm.data) {
      vm.data[prop] = vm[prop];
    }
  });
}

/**
 * Emit subVm lifecycle
 * @param {Vm} vm - Vm object.
 * @param {String} type - event type
 */
export function emitSubVmLife(vm: Vm, type:string) {
  if (vm.childrenVms) {
    vm.childrenVms.forEach((subVm) => {
      subVm.$emit(`hook:${type}`);
      emitSubVmLife(subVm, type);
    });
  }
}
