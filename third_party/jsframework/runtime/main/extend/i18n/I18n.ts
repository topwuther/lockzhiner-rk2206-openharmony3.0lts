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
  isNull,
  getValue,
  Log
} from '../../../utils/index';

interface I18nConstructor {
  new(options: object): I18n;
}

export interface I18nInterface {
  instance?: { I18n: I18nConstructor };
}

interface VMInterface {
  $t: Function;
  $tc: Function;
  _i18n: Function;
}

const instances = {};

/**
 * This class provide internationalization support.
 */
class I18n {
  public locale: string;
  public messages: any;

  constructor(options) {
    this.locale = options.locale || language;
    this.messages = options.messages;
  }

  /**
   * Provide the '$t' method to import simple resources.
   * @param {string} path - The path of language resources which to be translated.
   * @param {*} [params] - The values of placeholder.
   * @return {*} The translated result.
   */
  public $t(path: string, params?: any): any {
    if (typeof path !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'path' should be string, not ${typeof path}.`);
      return;
    }
    if (!this._hasMessage(this.messages)) {
      return path;
    }
    let value = this._getMessage(this.messages, path);
    if (isNull(value)) {
      return path;
    }
    if (Object.prototype.toString.call(value) === '[object Object]' ||
      Object.prototype.toString.call(value) === '[object Array]') {
      return value;
    }
    value = this._translate(path, value, params);
    return value;
  }

  /**
   * Provide the '$tc' method to import singular and plural resources.
   * @param {string} path - The path of language resources which to be translated.
   * @param {number} [count] - The number which to be translated.
   * @return {*} The translated result.
   */
  public $tc(path: string, count?: number): any {
    if (typeof path !== 'string') {
      Log.warn(`Invalid parameter type: The type of 'path' should be string, not ${typeof path}.`);
      return;
    }
    if (typeof count !== 'number' && !isNull(count)) {
      Log.warn(`Invalid parameter type: The type of 'count' should be number, not ${typeof count}.`);
      return;
    }
    if (!this._hasMessage(this.messages)) {
      return path;
    }
    let value = this._getMessage(this.messages, path);
    if (isNull(value)) {
      return path;
    }
    if (isNull(count)) {
      count = 1;
    }
    value = this._getChoice(count, path, value);
    value = this._translate(path, value, count);
    return value;
  }

  /**
  * Extend _i18n to Vm.
  * @param {VMInterface} Vm - The Vm.
  */
  public extend(Vm: VMInterface): void {
    Object.defineProperty(Vm, '_i18n', {
      configurable: true,
      enumerable: true,
      get: function proxyGetter() {
        return this.i18n ? this.i18n : global.aceapp.i18n;
      }
    });
    Vm.$t = function(path, params) {
      const i18n = this._i18n;
      return i18n.$t(path, params);
    };
    Vm.$tc = function(path, count) {
      const i18n = this._i18n;
      return i18n.$tc(path, count);
    };
  }

  private _hasMessage(message: object[]): boolean {
    if (!message || message.length === 0) {
      Log.debug('I18n message is null.');
      return false;
    }
    return true;
  }

  private _getMessage(messages: any[], path: string): any {
    for (const i in messages) {
      const value = getValue(path, messages[i]);
      if (!isNull(value)) {
        return value;
      }
    }
    return null;
  }

  private _getChoice(count: number, path: string, message: any): any {
    const pluralChoice = i18nPluralRules.select(count);
    if (!pluralChoice) {
      Log.debug('PluralChoice is null.');
      return path;
    }
    return getValue(pluralChoice, message);
  }

  private _translate(path: string, value: any, params: any): any {
    if (isNull(value)) {
      return path;
    }
    if (Object.prototype.toString.call(params) === '[object Array]') {
      value = value.replace(/\{(\d+)\}/g, (_, index) => {
        if (index > params.length - 1 || index < 0) {
          return '';
        }
        return params[index];
      });
    } else if (Object.prototype.toString.call(params) === '[object Object]') {
      value = value.replace(/\{(\w+)\}/g, (_, name) => {
        if (name in params) {
          return params[name];
        }
        return '';
      });
    } else if (Object.prototype.toString.call(params) === '[object Number]') {
      value = value.replace(/\{count\}/g, params);
    } else {
      return value;
    }
    return value;
  }
}

/**
 * Init the i18n object.
 */
export default {
  create: (id: number): I18nInterface | null => {
    instances[id] = [];
    if (typeof global.I18n === 'function') {
      return {};
    }
    const i18nObject = {
      I18n: class extends I18n {
        constructor(options) {
          super(options);
          instances[id].push(this);
        }
      }
    };
    return {
      instance: i18nObject
    };
  },
  destroy: (id: number): void => {
    delete instances[id];
  }
};
