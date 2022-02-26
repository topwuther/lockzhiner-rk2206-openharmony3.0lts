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

import {
  getValue,
  Log
} from '../../../utils/index';

interface dpiConstructor {
  new(options: object): Dpi;
}

export interface DPIInterface {
  instance?: { dpi: dpiConstructor };
}

interface VMInterface {
  $r: Function;
  _dpi: Function;
}

const instances = {};

/**
 * This class provides multi-resolution display support.
 */
class Dpi {
  public image: object;

  constructor(options) {
    this.image = options.images;
  }

  /**
   * Check the value of the key in images.
   * @param {string} path - Source path.
   * @return {Object | string} The value of the key if found.
   */
  public $r(path: string): object | string {
    if (typeof path !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'path' should be string, not ${typeof path}.`);
      return;
    }
    const images = this.image;
    let res;
    for (const index in images) {
      res = getValue(path, images[index]);
      if (res) {
        return res;
      }
    }
    return path;
  }

  /**
  * Extend _dpi to Vm.
  * @param {VMInterface} Vm - The Vm.
  */
  public extend(Vm: VMInterface): void {
    Object.defineProperty(Vm, '_dpi', {
      configurable: true,
      enumerable: true,
      get: function proxyGetter() {
        return this.dpi ? this.dpi : global.aceapp.dpi;
      }
    });
    Vm.$r = function(key: string): string {
      const dpi = this._dpi;
      return dpi.$r(key);
    };
  }
}

/**
 * Init the dpi object.
 */
export default {
  create: (id: number): DPIInterface => {
    instances[id] = [];
    if (typeof global.dpi === 'function') {
      return {};
    }
    const dpiObject = {
      dpi: class extends Dpi {
        constructor(options) {
          super(options);
          instances[id].push(this);
        }
      }
    };
    return {
      instance: dpiObject
    };
  },
  destroy: (id: number): void => {
    delete instances[id];
  }
};
