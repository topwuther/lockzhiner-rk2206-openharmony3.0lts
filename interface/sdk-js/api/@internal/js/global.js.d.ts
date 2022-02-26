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

export declare class Image {
    src: string;
    width?: number;
    height?: number;
    onload?: () => void;
    onerror?: () => void;
}
export declare class ImageData {
    width: number;
    height: number;
    data: Uint8ClampedArray;
}
export interface FocusParamObj {
  focus: boolean;
}
export interface AnimateStyle {
  width: number;
  height: number;
  backgroundColor: string;
  opacity: number;
  backgroundPosition: string;
  transformOrigin: string;
  transform: object;
  offset?: number;
}
export interface AnimateOptions {
  duration: number;
  easing: string;
  delay: number;
  iterations: number | string;
  fill: "none" | "forwards";
}
export interface AnimationResult {
  finished: boolean;
  pending: boolean;
  playstate: string;
  startTime: number;
  play(): void;
  finish(): void;
  pause(): void;
  cancel(): void;
  reverse(): void;
  onfinish: () => void;
  oncancel: () => void;
}
export interface Element {
  focus(obj?: FocusParamObj): void;
  rotation(obj?: FocusParamObj): void;
  animate(
    keyframes: Array<AnimateStyle>,
    options: AnimateOptions
  ): AnimationResult;
}
export interface AnimationElement extends Element {
  play(): void;
  finish(): void;
  pause(): void;
  cancel(): void;
  reverse(): void;
}
export interface ListScrollByOptions {
  dx?: number;
  dy?: number;
  smooth?: boolean;
}
export interface CurrentOffsetResultValue {
  x: number;
  y: number;
}
export interface ListScrollToOptions {
  index: number;
}
export interface ListElement extends Element {
  scrollTo(position: ListScrollToOptions): void;
  scrollBy(data: ListScrollByOptions): void;
  scrollTop(param: { smooth: boolean }): void;
  scrollBottom(param: { smooth: boolean }): void;
  scrollPage(params: { reverse: boolean; smooth: boolean }): void;
  scrollArrow(params: { reverse: boolean; smooth: boolean }): void;
  collapseGroup(param: {
    groupid: string;
  }): void;
  expandGroup(param: {
    groupid: string;
  }): void;
  currentOffset(): CurrentOffsetResultValue;
}
export interface SwiperElement extends Element {
  swipeTo(position: {
    index: number;
  }): void;
  showNext(): void;
  showPrevious(): void;
}
export interface DialogElement extends Element {
  show(): void;
  close(): void;
}
export interface ImageAnimatorElement extends Element {
  start(): void;
  pause(): void;
  stop(): void;
  resume(): void;
  getState(): "Playing" | "Paused" | "Stopped";
}
export interface MarqueeElement extends Element {
  start(): void;
  stop(): void;
}
export interface MenuElement extends Element {
  show(position: { x: number; y: number }): void;
}
export interface ChartElement extends Element {
  append(params: {
    serial: number;
    data: Array<number>;
  }): void;
}
export interface InputElement extends Element {
  focus(param: { focus: boolean }): void;
  showError(param: { error: string }): void;
}
export interface ButtonElement extends Element {
  setProgress(param: { progress: number }): void;
}
export interface TextAreaElement extends Element {
  focus(param: { focus: boolean }): void;
}
export interface PickerElement extends Element {
  show(): void;
}
export interface VideoElement extends Element {
  start(): void;
  pause(): void;
  setCurrentTime(param: { currenttime: number }): void;
  requestFullscreen(param: { screenOrientation: "default" }): void;
  exitFullscreen(): void;
}
export interface TextMetrics {
  width: number;
}
export interface CanvasRenderingContext2D {
  fillRect(x: number, y: number, width: number, height: number): void;
  fillStyle?: string | CanvasGradient | CanvasPattern;
  clearRect(x: number, y: number, width: number, height: number): void;
  strokeRect(x: number, y: number, width: number, height: number): void;
  fillText(text: string, x: number, y: number): void;
  strokeText(text: string, x: number, y: number): void;
  measureText(text: string): TextMetrics;
  lineWidth?: number;
  strokeStyle?: string | CanvasGradient | CanvasPattern;
  stroke(): void;
  beginPath(): void;
  moveTo(x: number, y: number): void;
  lineTo(x: number, y: number): void;
  closePath(): void;
  lineCap: string;
  lineJoin: string;
  miterLimit: number;
  font: string;
  textAlign: "left" | "right" | "center" | "start" | "end";
  textBaseline: string;
  createLinearGradient(
    x0: number,
    y0: number,
    x1: number,
    y1: number
  ): CanvasGradient;
  createPattern(image: Image, repetition: string): object;
  bezierCurveTo(
    cp1x: number,
    cp1y: number,
    cp2x: number,
    cp2y: number,
    x: number,
    y: number
  ): void;
  quadraticCurveTo(cpx: number, cpy: number, x: number, y: number): void;
  arc(
    x: number,
    y: number,
    radius: number,
    startAngle: number,
    endAngel: number,
    anticlockwise?: boolean
  ): void;
  arcTo(x1: number, y1: number, x2: number, y2: number, radius: number): void;
  rect(x: number, y: number, width: number, height: number): void;
  fill(): void;
  clip(): void;
  rotate(rotate: number): void;
  scale(x: number, y: number): void;
  transform(
    scaleX: number,
    skewX: number,
    skewY: number,
    scaleY: number,
    translateX: number,
    translateY: number
  ): void;
  setTransform(
    scaleX: number,
    skewX: number,
    skewY: number,
    scaleY: number,
    translateX: number,
    translateY: number
  ): void;
  translate(x: number, y: number): void;
  globalAlpha: number;
  drawImage(
    image: Image,
    dx: number,
    dy: number,
    dWidth: number,
    dHeight: number,
    sx: number,
    sy: number,
    sWidth: number,
    sHeight: number
  ): void;
  restore: () => void;
  save: () => void;
  createImageData(width: number, height: number): ImageData;
  createImageData(imagedata: ImageData): ImageData;
  getImageData(sx: number, sy: number, sw: number, sh: number): ImageData;
  putImageData(imageData: ImageData, dx: number, dy: number): void;
  putImageData(
    imageData: ImageData,
    dx: number,
    dy: number,
    dirtyX: number,
    dirtyY: number,
    dirtyWidth: number,
    dirtyHeight: number
  ): void;
  setLineDash(segments: Array<number>): void;
  getLineDash(): Array<number>;
  lineDashOffset: number;
  globalCompositeOperation: string;
  shadowBlur: number;
  shadowColor: string;
  shadowOffsetX: number;
  shadowOffsetY: number;
}
export interface CanvasGradient {
  addColorStop(offset: number, color: string): void;
}
export interface CanvasElement extends Element {
  getContext(param: string): CanvasRenderingContext2D;
}
export interface Application {
  $def: object;
}
export interface ViewModel {
  $app: Application;
  $t(path: string, params?: object | Array<any>): string;
  $tc(path: string, count: number): string;
  $r(path: string): string;
  $set(key: string, value: any): void;
  $delete(key: string): void;
  $element(
    id?: string
  ): AnimationElement &
    CanvasElement &
    object &
    ListElement &
    SwiperElement &
    DialogElement &
    ImageAnimatorElement &
    MarqueeElement &
    MenuElement &
    ChartElement &
    InputElement &
    ButtonElement &
    TextAreaElement &
    PickerElement &
    VideoElement;
  $root(): ViewModel & object;
  $parent(): ViewModel & object;
  $child(id: string): ViewModel & object;
  $watch(data: string, callback: string): void;
  $refs: ElementReferences;
  $emit(event: string, params?: object): void;
}
export interface ElementReferences {
  [k: string]: AnimationElement &
    CanvasElement &
    object &
    ListElement &
    SwiperElement &
    DialogElement &
    ImageAnimatorElement &
    MarqueeElement &
    MenuElement &
    ChartElement &
    InputElement &
    ButtonElement &
    TextAreaElement &
    PickerElement &
    VideoElement;
}
export interface Options<T extends ViewModel, Data = DefaultData<T>> {
  data?: Data;
  onInit?(): void;
  onReady?(): void;
  onShow?(): void;
  onHide?(): void;
  onDestroy?(): void;
  onBackPress?(): boolean;
  onCreate?(): void;
}
type DefaultData<T> = object;
type CombinedOptions<T extends ViewModel, Data> = object &
  Options<T, Data> &
  ThisType<T & ViewModel & Data>;
export declare function extendViewModel<T extends ViewModel, Data>(
  options: CombinedOptions<T, Data>
): ViewModel & Data;
