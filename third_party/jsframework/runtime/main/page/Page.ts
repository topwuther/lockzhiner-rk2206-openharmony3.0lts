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
 * Page constructor & definition
 */

import Differ from './api/Differ';
import { requireModule } from './register';
import { updateActions, callTasks } from './api/index';
import { Options } from '../app/index';
import Vm from '../model/index';
import Document from '../../vdom/Document';

/**
 * Directive description to native.
 */
export interface Task {
  module: string;
  method: string;
  args: number[];
}

/**
 * Page constructor for framework.
 * @param {string} id
 * @param {Options} options
 * @param {string} packageName
 */
export default class Page {
  private _packageName: string;
  private _id: string;
  private _options: Options;
  private _vm: Vm;
  private _customComponentMap: object;
  private _commonModules: object;
  private _doc: Document;
  private _differ: Differ;
  private _i18nService: object;
  private _dpiService: object;
  private _lastSignal: number;
  private _destroyed: boolean;
  private readonly _routerParams: any;

  constructor(id: string, options: Options, packageName: string, data: object) {
    this._packageName = packageName || 'notset';
    this._id = id;
    this._options = options || null;
    this._vm = null;
    this._customComponentMap = {};
    this._commonModules = {};
    this._doc = new Document(id, this._options.bundleUrl);
    this._differ = new Differ(id);
    this._routerParams = data;
  }

  /**
   * LastSignal of this Page.
   * @type {number}
   */
  get lastSignal() {
    return this._lastSignal;
  }

  set lastSignal(newLastSignal: number) {
    this._lastSignal = newLastSignal;
  }

  /**
   * PackageName of this Page.
   * @type {string}
   */
  get packageName() {
    return this._packageName;
  }

  /**
   * Id of this Page.
   * @type {string}
   */
  get id() {
    return this._id;
  }

  set id(id: string) {
    this._id = id;
  }

  /**
   * Options of this Page.
   * @type {Options}
   */
  get options() {
    return this._options;
  }

  set options(options: Options) {
    this._options = options;
  }

  /**
   * Vm of this Page.
   * @type {Vm}
   */
  get vm() {
    return this._vm;
  }

  set vm(vm: Vm) {
    this._vm = vm;
  }

  /**
   * CustomComponentMap of this Page.
   * @type {Object}
   */
  get customComponentMap() {
    return this._customComponentMap;
  }

  set customComponentMap(customComponentMap: object) {
    this._customComponentMap = customComponentMap;
  }

  /**
   * CommonModules of this Page.
   * @type {Object}
   */
  get commonModules() {
    return this._commonModules;
  }

  set commonModules(commonModules: object) {
    this._commonModules = commonModules;
  }

  /**
   * Doc of this Page.
   * @type {Document}
   */
  get doc() {
    return this._doc;
  }

  set doc(doc: Document) {
    this._doc = doc;
  }

  /**
   * Differ of this Page.
   * @type {Differ}
   */
  get differ() {
    return this._differ;
  }

  set differ(differ: Differ) {
    this._differ = differ;
  }

  /**
   * I18nService of this Page.
   * @type {Object}
   */
  get i18nService() {
    return this._i18nService;
  }

  set i18nService(i18nService: object) {
    this._i18nService = i18nService;
  }

  /**
   * DpiService of this page.
   * @type {Object}
   */
  get dpiService() {
    return this._dpiService;
  }

  set dpiService(dpiService: object) {
    this._dpiService = dpiService;
  }

  /**
   * Destroyed of this page.
   * @type {boolean}
   */
  get destroyed() {
    return this._destroyed;
  }

  set destroyed(destroyed: boolean) {
    this._destroyed = destroyed;
  }

  /**
   * Obtain methods of a module.
   * @param {string} name
   * @return {*}
   */
  public requireModule(name: string): any {
    return requireModule(this, name);
  }

  /**
   * Collect all virtual-DOM mutations together and send them to renderer.
   * @return {*}
   */
  public updateActions(): any {
    return updateActions(this);
  }

  /**
   * Call all tasks from a page to renderer (native).
   * @param {Task[] | Task} tasks
   * @return {*}
   */
  public callTasks(tasks: Task[] | Task): any {
    return callTasks(this, tasks);
  }

  /**
   * get params from other page.
   * @type {Object}
   */
  get routerParams() {
    return this._routerParams;
  }
}
