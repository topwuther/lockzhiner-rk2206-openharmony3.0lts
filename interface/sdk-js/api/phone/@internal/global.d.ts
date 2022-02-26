/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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
 * Sets the interval for repeatedly calling a function.
 * @param handler Indicates the function to be called repeatedly at the interval.
 * @param delay Indicates the interval between each two calls, in milliseconds. The function will be called after this delay.
 * @param arguments Indicates additional arguments to pass to "handler" when the timer goes off.
 * @returns Returns the timer ID.
 */
export declare function setInterval(handler: TimerHandler, delay: number, ...arguments: any[]): number;

/**
 * Sets a timer after which a function will be executed.
 * @param handler Indicates the function to be called after the timer goes off.
 * @param delay Indicates the delay (in milliseconds) after which the function will be called. If this parameter is left empty, default value "0" will be used, which means that the function will be called immediately or as soon as possible.
 * @param arguments Indicates additional arguments to pass to "handler" when the timer goes off.
 * @returns Returns the timer ID.
 */
export declare function setTimeout(handler: TimerHandler, delay?: number, ...arguments: any[]): number;

/**
 * Cancels the interval set by " setInterval()".
 * @param intervalID Indicates the timer ID returned by "setInterval()".
 */
export declare function clearInterval(intervalID?: number): void;

/**
 * Cancels the timer set by "setTimeout()".
 * @param timeoutID Indicates the timer ID returned by "setTimeout()".
 */
export declare function clearTimeout(timeoutID?: number): void;

/**
 * You can create an Image object by calling new Image().
 */
export declare class Image {
  /**
   * Network address or local resource. The internal URI is supported.
   */
  src: string;
  /**
   * Image width.
   */
  width?: number;
  /**
   * Image height.
   */
  height?: number;
  /**
   * Called when an image is successfully loaded. This function has no parameter.
   */
  onload?: () => void;
  /**
   * Called when an image fails to be loaded. This function has no parameter.
   */
  onerror?: () => void;
}

/**
 * An ImageData object is a common object that stores the actual pixel data of a Canvas object.
 */
export declare class ImageData {
  /**
   * Actual width of the ImageData object, in pixels.
   */
  width: number;
  /**
   * Actual height of the ImageData object, in pixels.
   */
  height: number;
  /**
   * A one-dimensional array of color values. The color values are sorted in the RGBA order and represented by integers from 0 to 255.
   */
  data: Uint8ClampedArray;
}
