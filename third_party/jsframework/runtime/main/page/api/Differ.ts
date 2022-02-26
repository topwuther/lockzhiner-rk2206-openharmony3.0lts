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

import { hasOwn } from '../../../utils/index';

/**
 * This class provides action for page refresh.
 */
export default class Differ {
  private _id: string;
  private _map: object[];
  private _hooks: Function[];
  private _hasTimer: boolean;

  constructor(id: string) {
    this._id = id;
    this._map = [];
    this._hooks = [];
  }

  /**
   * Check whether the map is empty.
   * @return {boolean}
   */
  public isEmpty(): boolean {
    return this._map.length === 0;
  }

  /**
   * Id of the page.
   * @type {string}
   */
  public get id() {
    return this._id;
  }

  /**
   * Append action.
   * @param {string} type
   * @param {string} ref
   * @param {Function} handler
   */
  public append(type: string, ref: string, handler: Function): void {
    // Ignore depth to speed up render.
    const defaultDepth: number = 1;
    if (!this._hasTimer) {
      this._hasTimer = true;

      // Use setTimeout instead of setTimeoutDiffer
      setTimeout(() => {
        this._hasTimer = false;
        this.flush();
      }, 0);
    }
    const map: object[] = this._map;
    if (!map[defaultDepth]) {
      map[defaultDepth] = {};
    }
    const group: object = map[defaultDepth];
    if (!group[type]) {
      group[type] = {};
    }
    if (type === 'element') {
      if (!group[type][ref]) {
        group[type][ref] = [];
      }
      group[type][ref].push(handler);
    } else {
      group[type][ref] = handler;
    }
  }

  /**
   * Execute actions of differ.
   */
  public flush(): void {
    const map: object[] = this._map.slice();
    this._map.length = 0;
    map.forEach((group) => {
      callTypeList(group, 'element');
      callTypeMap(group, 'repeat');
      callTypeMap(group, 'shown');
    });

    const hooks: Function[] = this._hooks.slice();
    this._hooks.length = 0;
    hooks.forEach((fn) => {
      fn();
    });

    if (!this.isEmpty()) {
      this.flush();
    }
  }
  then(fn) {
    this._hooks.push(fn);
  }
}

function callTypeMap(group: any, type: string): void {
  const map: Function[] = group[type];
  for (const ref in map) {
    map[ref]();
  }
}

function callTypeList(group: any, type: string): void {
  const map: any = group[type];
  for (const ref in map) {
    if (hasOwn(map, ref)) {
      const list: Function[] = map[ref];
      list.forEach((handler) => {
        handler();
      });
    }
  }
}
