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

/**
 * Intercept callback from native and forward to user-defined callback.
 * @param {*} args - Args.
 * @param {boolean} [needPromise] - If asynchronous operations are needed.
 * @return {Object} If promise are needed, return { args, promise }. Otherwise return args.
 */
export function interceptCallback(args: any, needPromise?: boolean): object {
  if (args.length === 0 && !needPromise) {
    return args;
  }
  const first: object = args[0];
  const callbacks: object = {};
  let hasProperty: boolean = false;
  if (typeof first === 'object' &&
    Object.prototype.toString.call(first).toLowerCase() === '[object object]' &&
    args.length === 1
  ) {
    for (const key in first) {
      const value: Function = first[key];
      if (typeof value === 'function') {
        callbacks[key] = value;
      } else {
        hasProperty = true;
      }
    }
  } else {
    hasProperty = true;
  }

  let promise: any;
  const callbLength: number = Object.keys(callbacks).length;
  if (needPromise) {
    if (callbLength <= 0) {
      promise = new PromiseRef();
    }
  }
  if (callbLength > 0 || promise) {
    const callb = (msg: { method: string; arguments: any; }) => {
      let func = callbacks[msg.method];
      const callbArgs: any = msg.arguments;

      if (func !== undefined) {
        func(...callbArgs);
      }

      // Always call complete().
      func = callbacks['complete'];
      if (func !== undefined) {
        func(...callbArgs);
      }
      if (promise) {
        const data: any = callbArgs &&
          callbArgs.length > 0
          ? callbArgs[0]
          : undefined;
        if ('success' === msg.method || 'callback' === msg.method) {
          promise.resolve({ data });
        } else {
          // 200 means common error ,100 :cancel.
          const code: any =
            'cancel' === msg.method
              ? 100
              : callbArgs && callbArgs.length > 1
                ? callbArgs[1]
                : 200;
          promise.reject({ data, code });
        }
      }
    };
    callb.__onlyPromise = callbLength <= 0;

    if (hasProperty) {
      args.push(callb);
    } else {
      args = [callb];
    }
  }
  return needPromise ? { args, promise } : args;
}

/**
 * This class provide a Promise object for asynchronous operation processing.
 */
class PromiseRef {
  private _promise: Promise<object>;
  private _reject: Function;
  private _resolve: Function;

  constructor() {
    this._promise = new Promise((resolve, reject) => {
      this._reject = reject;
      this._resolve = resolve;
    });
  }

  /**
   * Promise of this PromiseRef.
   * @type {Promise}
   */
  public get promise() {
    return this._promise;
  }

  public set promise(newPromise) {
    this._promise = newPromise;
  }

  /**
   * Reject function using the Promise object.
   * @type {Promise}
   */
  public get reject() {
    return this._reject;
  }

  public set reject(data) {
    this._reject = data;
  }

  /**
   * Resolve function using the Promise object.
   * @type {Promise}
   */
  public get resolve() {
    return this._resolve;
  }

  public set resolve(data) {
    this._resolve = data;
  }
}
