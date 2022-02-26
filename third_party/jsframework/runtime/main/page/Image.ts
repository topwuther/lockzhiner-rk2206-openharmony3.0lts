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

import Page from './index';

let image;

/**
 * This class provide a Image object.
 */
export class Image {
  private _src: string;
  private _height: number;
  private _width: number;
  private _onload: (...args: any | null) => void;
  private _onerror: (...args: any | null) => void;

  constructor(page: Page) {
    image = page.requireModule('system.image');
    this._src = '';
    this._height = 0;
    this._width = 0;
    this._onload = null;
    this._onerror = null;
  }

  /**
   * Src of this Image.
   * @type {string}
   */
  public get src() {
    return this._src;
  }

  public set src(src) {
    this._src = src;
    image.getImage({
      src: this._src,
      width: this._width,
      height: this._height,
      success: data => {
        if (this.onload && typeof this.onload === 'function') {
          this.onload(data);
        }
      },
      fail: data => {
        if (this.onerror && typeof this.onerror === 'function') {
          this.onerror(data);
        }
      }
    });
  }

  /**
   * Width of this Image.
   * @type {number}
   */
  public get width() {
    return this._width;
  }

  public set width(width) {
    this._width = width;
  }

  /**
   * Height of this Image.
   * @type {number}
   */
  public get height() {
    return this._height;
  }

  public set height(height) {
    this._height = height;
  }

  /**
   * Triggered when the image is successfully loaded.
   * @type {Function}
   */
  public get onload() {
    return this._onload;
  }

  public set onload(onload: (...args: any | null) => void) {
    this._onload = onload;
  }

  /**
   * Triggered when the image fails to be loaded.
   * @type {Function}
   */
  public get onerror() {
    return this._onerror;
  }

  public set onerror(onerror: (...args: any | null) => void) {
    this._onerror = onerror;
  }
}
