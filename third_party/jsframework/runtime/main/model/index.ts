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
 * 2021.01.08 - Reconstruct the class 'Vm' and make it more adaptable to framework.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

/**
 * @fileOverview
 * ViewModel Constructor & definition
 */

import {
  extend,
  Log,
  removeItem
} from '../../utils/index';
import {
  initState,
  initBases
} from '../reactivity/state';
import {
  build, FragBlockInterface
} from './compiler';
import {
  set,
  del
} from '../reactivity/observer';
import {
  watch,
  initPropsToData
} from './pageLife';
import {
  initEvents,
  ExternalEvent,
  Evt
} from './events';
import {
  selector
} from './selector';
import Page from '../page/index';
import Element from '../../vdom/Element';
import {
  ComputedInterface,
  cssType,
  Props,
  VmOptions,
  MediaStatusInterface,
  ExternalBindingInterface
} from './vmOptions';
import {
  hasOwn
} from '../util/shared';

/**
 * VM constructor.
 * @param {string} type - Type.
 * @param {null | VmOptions} options - Component options.
 * @param {Vm} parentVm   which contains _app.
 * @param {Element | FragBlockInterface} parentEl - root element or frag block.
 * @param {Object} mergedData - External data.
 * @param {ExternalEvent} externalEvents - External events.
 */
export default class Vm {
  private _parent: Vm;
  private _app: Page;
  private _computed: ComputedInterface;
  private _css: cssType;
  private _type: string;
  private readonly _descriptor: string;
  private _props: Props;
  private _vmOptions: VmOptions;
  private _selector: object;
  private _ids: Record<string, {vm: Vm, el: Element}>;
  private _init: boolean;
  private _ready: boolean;
  private _valid: boolean;
  private _vmEvents: object;
  private _childrenVms: Vm[];
  private _visible: boolean;
  private _data: any;
  private _shareData: any;
  private _realParent: Vm
  private _parentEl: Element | FragBlockInterface;
  private _rootEl: Element;
  private _$refs: Record<string, Element>;
  private _externalBinding: ExternalBindingInterface;
  private _isHide: boolean;
  private _mediaStatus: Partial<MediaStatusInterface<string, boolean>>;
  private _methods: Record<string, (...args: unknown[]) => any>;
  private _slotContext: { content: Record<string, any>, parentVm: Vm };
  private _$app: any;

  constructor(
    type: string,
    options: null | VmOptions,
    parentVm: Vm | any,
    parentEl: Element | FragBlockInterface,
    mergedData: object,
    externalEvents: ExternalEvent
  ) {
    this._$app = global.aceapp;
    this._parent = parentVm._realParent ? parentVm._realParent : parentVm;
    this._app = parentVm._app;
    parentVm._childrenVms && parentVm._childrenVms.push(this);

    if (!options && this._app.customComponentMap) {
      options = this._app.customComponentMap[type];
    }
    const data = options.data || {};
    const shareData = options.shareData || {};
    this._vmOptions = options;
    this._computed = options.computed;
    this._css = options.style;
    this._selector = selector(this._css);
    this._ids = {};
    this._$refs = {};
    this._vmEvents = {};
    this._childrenVms = [];
    this._type = type;
    this._valid = true;
    this._props = [];
    this._methods = {};

    // Bind events and lifecycles.
    initEvents(this, externalEvents);

    Log.debug(
      `'_innerInit' lifecycle in Vm(${this._type}) and mergedData = ${JSON.stringify(mergedData)}.`
    );
    this.$emit('hook:_innerInit');
    this._data = (typeof data === 'function' ? data.apply(this) : data) || {};
    this._shareData = (typeof shareData === 'function' ? shareData.apply(this) : shareData) || {};
    this._descriptor = options._descriptor;
    if (global.aceapp && global.aceapp.i18n && global.aceapp.i18n.extend) {
      global.aceapp.i18n.extend(this);
    }
    if (global.aceapp && global.aceapp.dpi && global.aceapp.dpi.extend) {
      global.aceapp.dpi.extend(this);
    }

    // MergedData means extras params.
    if (mergedData) {
      if (hasOwn(mergedData, 'paramsData') && hasOwn(mergedData, 'dontOverwrite') && mergedData['dontOverwrite'] === false) {
        dataAccessControl(this, mergedData['paramsData'], this._app.options && this._app.options.appCreate);
        extend(this._data, mergedData['paramsData']);
      } else {
        dataAccessControl(this, mergedData, this._app.options && this._app.options.appCreate);
        extend(this._data, mergedData);
      }
    }

    initPropsToData(this);
    initState(this);
    initBases(this);
    Log.debug(`"onInit" lifecycle in Vm(${this._type})`);

    if (mergedData && hasOwn(mergedData, 'paramsData') && hasOwn(mergedData, 'dontOverwrite')) {
      if (mergedData['dontOverwrite'] === false) {
        this.$emit('hook:onInit');
      } else {
        this.$emitDirect('hook:onInit', mergedData['paramsData']);
      }
    } else {
      this.$emit('hook:onInit');
    }

    if (!this._app.doc) {
      return;
    }
    this.mediaStatus = {};
    this.mediaStatus.orientation = this._app.options.orientation;
    this.mediaStatus.width = this._app.options.width;
    this.mediaStatus.height = this._app.options.height;
    this.mediaStatus.resolution = this._app.options.resolution;
    this.mediaStatus['device-type'] = this._app.options.deviceType;
    this.mediaStatus['aspect-ratio'] = this._app.options.aspectRatio;
    this.mediaStatus['device-width'] = this._app.options.deviceWidth;
    this.mediaStatus['device-height'] = this._app.options.deviceHeight;
    this.mediaStatus['round-screen'] = this._app.options.roundScreen;
    this.mediaStatus['dark-mode'] = this._app.options.darkMode;

    // If there is no parentElement, specify the documentElement.
    this._parentEl = parentEl || this._app.doc.documentElement;
    build(this);
  }

  /**
   * Get the element by id.
   * @param {string | number} [id] - Element id.
   * @return {Element} Element object. if get null, return root element.
   */
  public $element(id?: string | number): Element {
    if (id) {
      if (typeof id !== 'string' && typeof id !== 'number') {
        Log.warn(`Invalid parameter type: The type of 'id' should be string or number, not ${typeof id}.`);
        return;
      }
      const info: any = this.ids[id];
      if (info) {
        return info.el;
      }
    } else {
      return this.rootEl;
    }
  }

  /**
   * Get the vm by id.
   * @param {string} id - Vm id.
   * @return {Vm} Vm object.
   */
  public $vm(id: string): Vm {
    const info = this.ids[id];
    if (info) {
      return info.vm;
    }
  }

  /**
   * Get parent Vm of current.
   */
  public $parent(): Vm {
    return this.parent;
  }

  /**
   * Get child Vm of current.
   */
  public $child(id: string): Vm {
    if (typeof id !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'id' should be string, not ${typeof id}.`);
      return;
    }
    return this.$vm(id);
  }

  /**
   * Get root element of current.
   */
  public $rootElement(): Element {
    return this.rootEl;
  }

  /**
   * Get root Vm of current.
   */
  public $root(): Vm {
    return getRoot(this);
  }

  /**
   * Execution Method.
   * @param {string} type - Type.
   * @param {Object} [detail] - May needed for Evt.
   * @param {*} args - Arg list.
   * @return {*}
   */
  public $emit(type: string, detail?: object, ...args: any[]): any[] {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    const events = this._vmEvents;
    const handlerList = events[type];
    if (handlerList) {
      const results = [];
      const evt = new Evt(type, detail);
      handlerList.forEach((handler) => {
        results.push(handler.call(this, evt, ...args));
      });
      return results;
    }
  }

  /**
   * Execution Method directly.
   * @param {string} type - Type.
   * @param {*} args - Arg list.
   * @return {*}
   */
  public $emitDirect(type: string, ...args: any[]): any[] {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    const events = this._vmEvents;
    const handlerList = events[type];
    if (handlerList) {
      const results = [];
      handlerList.forEach((handler) => {
        results.push(handler.call(this, ...args));
      });
      return results;
    }
  }

  /**
   * Dispatch events, passing upwards along the parent.
   * @param {string} type - Type.
   * @param {Object} [detail] - May needed for Evt.
   */
  public $dispatch(type: string, detail?: object): void {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    const evt = new Evt(type, detail);
    this.$emit(type, evt);
    if (!evt.hasStopped() && this._parent && this._parent.$dispatch) {
      this._parent.$dispatch(type, evt);
    }
  }

  /**
   * Broadcast event, which is passed down the subclass.
   * @param {string} type - Type.
   * @param {Object} [detail] - May be needed for Evt.
   */
  public $broadcast(type: string, detail?: object): void {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    const evt = new Evt(type, detail);
    this.$emit(type, evt);
    if (!evt.hasStopped() && this._childrenVms) {
      this._childrenVms.forEach((subVm) => {
        subVm.$broadcast(type, evt);
      });
    }
  }

  /**
   * Add the event listener.
   * @param {string} type - Type.
   * @param {Function} handler - To add.
   */
  public $on(type: string, handler: Function): void {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    if (typeof handler !== 'function') {
      Log.warn(`Invalid parameter type: The type of 'handler' should be function, not ${typeof handler}.`);
      return;
    }
    const events = this._vmEvents;
    const handlerList = events[type] || [];
    handlerList.push(handler);
    events[type] = handlerList;
    if (type === 'hook:onReady' && this._ready) {
      this.$emit('hook:onReady');
    }
  }

  /**
   * Remove the event listener.
   * @param {string} type - Type.
   * @param {Function} handler - To remove.
   */
  public $off(type: string, handler: Function): void {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    if (typeof handler !== 'function') {
      Log.warn(`Invalid parameter type: The type of 'handler' should be function, not ${typeof handler}.`);
      return;
    }
    const events = this._vmEvents;
    if (!handler) {
      delete events[type];
      return;
    }
    const handlerList = events[type];
    if (!handlerList) {
      return;
    }
    removeItem(handlerList, handler);
  }

  /**
   * Execution element.fireEvent Method.
   * @param {string} type - Type.
   * @param {Object} data - needed for Evt.
   * @param {string} id - Element id.
   */
  public $emitElement(type: string, data: object, id: string): void {
    if (typeof type !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'type' should be string, not ${typeof type}.`);
      return;
    }
    if (typeof id !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'id' should be string, not ${typeof id}.`);
      return;
    }
    const info = this.ids[id];
    if (info) {
      const element = info.el;
      const evt = new Evt(type, data);
      element.fireEvent(type, evt, false);
    } else {
      Log.warn('The id is invalid, id = ' + id);
    }
  }

  /**
   * Watch a calc function and callback if the calc value changes.
   * @param {string} data - Data that needed.
   * @param {Function | string} callback - Callback function.
   */
  public $watch(data: string, callback: ((...args: any) => any) | string): void {
    if (typeof data !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'data' should be string, not ${typeof data}.`);
      return;
    }
    if (typeof callback !== 'function' && typeof callback !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'callback' should be function or string, not ${typeof callback}.`);
      return;
    }
    watch(this, data, callback);
  }

  /**
   * Set a property on an object.
   * @param {string} key - Get value by key.
   * @param {*} value - Property
   */
  public $set(key: string, value: any): void {
    if (typeof key !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'key' should be string with '.', not ${typeof key}.`);
      return;
    }
    if (key.indexOf('.') !== -1) {
      _proxySet(this.data, key, value);
    }
    set(this.data, key, value);
  }

  /**
   * Delete a property and trigger change.
   * @param {string} key - Get by key.
   */
  public $delete(key: string): void {
    if (typeof key !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'key' should be string, not ${typeof key}.`);
      return;
    }
    del(this.data, key);
  }

  /**
   * Delete Vm object.
   */
  public destroy(): void {
    Log.debug(`[JS Framework] "onDestroy" lifecycle in Vm(${this.type})`);
    this.$emit('hook:onDestroy');
    this.$emit('hook:onDetached');
    fireNodeDetached(this._rootEl);
    this._valid = false;

    delete this._app;
    delete this._computed;
    delete this._css;
    delete this._data;
    delete this._ids;
    delete this._vmOptions;
    delete this._parent;
    delete this._parentEl;
    delete this._rootEl;
    delete this._$refs;

    // Destroy child vms recursively.
    if (this._childrenVms) {
      let vmCount: number = this._childrenVms.length;
      while (vmCount--) {
        this.destroy.call(this._childrenVms[vmCount], this._childrenVms[vmCount]);
      }
      delete this._childrenVms;
    }
    delete this._type;
    delete this._vmEvents;
  }

  /**
   * $t function.
   * @param {string} key - Key.
   * @return {string} - Key.
   */
  public $t(key: string): string {
    return key;
  }

  /**
   * $tc function.
   * @param {string} key - Key.
   * @return {string} - Key.
   */
  public $tc(key: string): string {
    return key;
  }

  /**
   * $r function.
   * @param {string} key - Key.
   * @return {string} - Key.
   */
  public $r(key: string): string {
    return key;
  }

  /**
   * $app function.
   * @return {*} - aceapp.
   */
  public get $app(): any {
    return this._$app;
  }

  /**
   * Methods of this Vm.
   * @type {Object}
   * @readonly
   */
  public get methods() {
    return this._methods;
  }

  /**
   * Type of this Vm.
   * @type {string}
   */
  public get type() {
    return this._type;
  }

  public set type(newType) {
    this._type = newType;
  }

  /**
   * Css of this Vm.
   * @type {[key: string]: any}
   * @readonly
   */
  public get css() {
    return this._css;
  }

  /**
   * Options of this Vm.
   * @type {VmOptions}
   */
  public get vmOptions() {
    return this._vmOptions;
  }

  public set vmOptions(newOptions: VmOptions) {
    this._vmOptions = newOptions;
  }

  /**
   * Parent of this Vm.
   * @type {Vm}
   * @readonly
   */
  public get parent() {
    return this._parent;
  }

  /**
   * RealParent of this Vm.
   * @type {Vm}
   */
  public get realParent() {
    return this._realParent;
  }

  public set realParent(realParent: Vm) {
    this._realParent = realParent;
  }

  /**
   * Computed of this Vm.
   * @type {ComputedInterface}
   */
  public get computed() {
    return this._computed;
  }

  public set computed(newComputed: ComputedInterface) {
    this._computed = newComputed;
  }

  /**
   * Selector of this Vm.
   * @type {Object}
   * @readonly
   */
  public get selector() {
    return this._selector;
  }

  /**
   * ParentEl of this Vm.
   * @type {FragBlockInterface | Element}
   */
  public get parentEl() {
    return this._parentEl;
  }

  public set parentEl(newParentEl: FragBlockInterface | Element) {
    this._parentEl = newParentEl;
  }

  /**
   * App of this Vm.
   * @type {Page}
   */
  public get app() {
    return this._app;
  }

  public set app(newApp: Page) {
    this._app = newApp;
  }

  /**
   * ShareData of this Vm.
   * @type {*}
   */
  public get shareData() {
    return this._shareData;
  }

  public set shareData(newShareData: object) {
    this._shareData = newShareData;
  }

  /**
   * Data of this Vm.
   * @type {*}
   */
  public get data() {
    return this._data;
  }

  public set data(newData: any) {
    this._data = newData;
  }

  /**
   * Props of this Vm.
   * @type {Props}
   * @readonly
   */
  public get props() {
    return this._props;
  }

  /**
   * Init of this Vm.
   * @type {boolean}
   */
  public get init() {
    return this._init;
  }

  public set init(newInit: boolean) {
    this._init = newInit;
  }

  /**
   * Valid of this Vm.
   * @type {boolean}
   * @readonly
   */
  public get valid() {
    return this._valid;
  }

  /**
   * Visible of this Vm.
   * @type {boolean}
   */
  public get visible() {
    return this._visible;
  }

  public set visible(newVisible) {
    this._visible = newVisible;
  }

  /**
   * Ready of this Vm.
   * @type {boolean}
   */
  public get ready() {
    return this._ready;
  }

  public set ready(newReady: boolean) {
    this._ready = newReady;
  }

  /**
   * RootEl of this Vm.
   * @type {Element}
   */
  public get rootEl() {
    return this._rootEl;
  }

  public set rootEl(newRootEl: Element) {
    this._rootEl = newRootEl;
  }

  /**
   * Ids of this Vm.
   * @type {{[key: string]: { vm: Vm, el: Element}}}
   * @readonly
   */
  public get ids() {
    return this._ids;
  }

  /**
   * VmEvents of this Vm.
   * @type {Object}
   * @readonly
   */
  public get vmEvents() {
    return this._vmEvents;
  }

  /**
   * children of vm.
   * @return {Array} - children of Vm.
   */
  public get childrenVms() {
    return this._childrenVms;
  }

  /**
   * ExternalBinding of this Vm.
   * @type {ExternalBinding}
   */
  public get externalBinding() {
    return this._externalBinding;
  }

  public set externalBinding(newExternalBinding: ExternalBindingInterface) {
    this._externalBinding = newExternalBinding;
  }

  /**
   * Descriptor of this Vm.
   * @type {string}
   * @readonly
   */
  public get descriptor() {
    return this._descriptor;
  }

  /**
   * IsHide of this Vm.
   * @type {boolean}
   */
  public get isHide() {
    return this._isHide;
  }

  public set isHide(newIsHide: boolean) {
    this._isHide = newIsHide;
  }

  /**
   * MediaStatus of this Vm.
   * @type {MediaStatusInterface<string, boolean>}
   */
  public get mediaStatus() {
    return this._mediaStatus;
  }

  public set mediaStatus(newMediaStatus: Partial<MediaStatusInterface<string, boolean>>) {
    this._mediaStatus = newMediaStatus;
  }

  /**
   * $refs of this Vm.
   * @type {[key: string]: Element}
   * @readonly
   */
  public get $refs() {
    return this._$refs;
  }

  /**
   * slotContext of this Vm.
   * @type { content: Record<string, any>, parentVm: Vm }
   */
  public get slotContext() {
    return this._slotContext;
  }

  public set slotContext(newMSoltContext: { content: Record<string, any>, parentVm: Vm }) {
    this._slotContext = newMSoltContext;
  }
}

/**
 * Set proxy.
 * @param {Object} data - Data that needed.
 * @param {string} key - Get prop by key.
 * @param {*} value - Property.
 */
function _proxySet(data: object, key: string, value: any): void {
  let tempObj = data;
  const keys = key.split('.');
  const len = keys.length;
  for (let i = 0; i < len; i++) {
    const prop = keys[i];
    if (i === len - 1) {
      set(tempObj, prop, value);
      tempObj = null;
      break;
    }
    if (tempObj[prop] === null || typeof tempObj[prop] !== 'object' && typeof tempObj[prop] !== 'function') {
      Log.warn(`Force define property '${prop}' of '${JSON.stringify(tempObj)}' with value '{}', `
        + `old value is '${tempObj[prop]}'.`);
      set(tempObj, prop, {});
    }
    tempObj = tempObj[prop];
  }
}

/**
 * Control data access.
 * @param {Vm} vm - Vm object.
 * @param {Object} mergedData - Merged data.
 * @param {boolean} external - If has external data.
 */
function dataAccessControl(vm: any, mergedData: object, external: boolean): void {
  if (vm._descriptor && Object.keys(vm._descriptor).length !== 0) {
    const keys = Object.keys(mergedData);
    keys.forEach(key => {
      const desc = vm._descriptor[key];
      if (!desc || desc.access === 'private' || external && desc.access === 'protected') {
        Log.error(`(${key}) can not modify`);
        delete mergedData[key];
      }
    });
  }
}

/**
 * Get root Vm.
 * @param {Vm} vm - Vm object.
 * @return {Vm} Root vm.
 */
function getRoot(vm: any): Vm {
  const parent = vm.parent;
  if (!parent) {
    return vm;
  }
  if (parent._rootVm) {
    return vm;
  }
  return getRoot(parent);
}

/**
 * order node and fire detached event.
 * @param {Element} el - Element object.
 */
function fireNodeDetached(el: Element) {
  if (!el) {
    return;
  }
  if (el.event && el.event['detached']) {
    el.fireEvent('detached', {});
  }
  if (el.children && el.children.length !== 0) {
    for (const child of el.children) {
      fireNodeDetached(child as Element);
    }
  }
}
