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

import {GestureType, GestureMask} from "./gesture"
import {Circle} from "./circle";
import {Ellipse} from "./ellipse";
import {Path} from "./path";
import {Rect} from "./rect";

export declare const Component: ClassDecorator;
export declare const Entry: ClassDecorator;
export declare const Observed: ClassDecorator;
export declare const Preview: ClassDecorator;
export declare const State: PropertyDecorator;
export declare const Prop: PropertyDecorator;
export declare const Link: PropertyDecorator;
export declare const ObjectLink: PropertyDecorator;
export declare const Provide: PropertyDecorator & ((value: string) => PropertyDecorator);
export declare const Consume: PropertyDecorator & ((value: string) => PropertyDecorator);
export declare const StorageProp: (value: string) => PropertyDecorator;
export declare const StorageLink: (value: string) => PropertyDecorator;
export declare const Watch: (value: string) => PropertyDecorator;
export declare const Builder: MethodDecorator;
export declare const CustomDialog: ClassDecorator;
declare interface NormalResource {
  readonly id: number;
  readonly type: number;
  readonly params?: any[];
}

declare interface RawfileResource {
  readonly fileName: string;
}

export declare type Resource = RawfileResource | NormalResource;

export declare interface Configuration {
  readonly colorMode: string;
  readonly fontScale: number;
}

declare const isSystemplugin: Function;

// global function
export declare function $s(value: string, params?: object | Array<any> | number): string;

export declare function $m(value: string): string;

export declare function $r(value: string, ...params: any[]): Resource;

export declare function $rawfile(value: string): Resource;

export declare function animateTo(value: {
  duration?: number, tempo?: number, curve?: Curve | string,
  delay?: number, iterations?: number, playMode?: PlayMode, onFinish?: () => void
}, event: () => void): void;

export declare function vp2px(value: number): number;

export declare function px2vp(value: number): number;

export declare function fp2px(value: number): number;

export declare function px2fp(value: number): number;

export declare function lpx2px(value: number): number;

export declare function px2lpx(value: number): number;

export declare function setAppBgColor(params: string): void;

// common enum
export declare enum Color {
  White,
  Black,
  Blue,
  Brown,
  Gray,
  Green,
  Grey,
  Orange,
  Pink,
  Red,
  Yellow
}

export declare enum ImageFit {
  Contain,
  Cover,
  Auto,
  Fill,
  ScaleDown,
  None
}

export declare enum BorderStyle {
  Dotted,
  Dashed,
  Solid
}

export declare enum LineJoinStyle {
  Miter,
  Round,
  Bevel
}

export declare enum TouchType {
  Down,
  Up,
  Move,
  Cancel
}

export declare enum AnimationStatus {
  Initial,
  Running,
  Paused,
  Stopped
}

export declare enum Curve {
  Linear,
  Ease,
  EaseIn,
  EaseOut,
  EaseInOut,
  FastOutSlowIn,
  LinearOutSlowIn,
  FastOutLinearIn,
  ExtremeDeceleration,
  Sharp,
  Rhythm,
  Smooth,
  Friction
}

export declare enum FillMode {
  None,
  Forwards,
  Backwards,
  Both
}

export declare enum PlayMode {
  Normal,
  Reverse,
  Alternate,
  AlternateReverse
}

export declare enum BlurStyle {
  SmallLight,
  MediumLight,
  LargeLight,
  XlargeLight,
  SmallDark,
  MediumDark,
  LargeDark,
  XlargeDark
}

export declare enum KeyType {
  Down,
  Up,
  LongPress
}

export declare enum KeySource {
  Unknown,
  Keyboard,
  Mouse,
  Dpad,
  Gamepad,
  Joystick,
  TouchScreen,
  TouchPad,
  Stylus,
  Trackball,
  Knob,
  Any
}

export declare enum Edge {
  Top,
  Center,
  Bottom,
  Baseline,
  Start,
  Middle,
  End
}

export declare enum Week {
  Mon,
  Tue,
  Wed,
  Thur,
  Fri,
  Sat,
  Sun,
}

export declare enum Direction {
  Ltr,
  Rtl,
  Auto
}

export declare enum BarState {
  Off,
  Auto,
  On
}

export declare enum EdgeEffect {
  Spring,
  Fade,
  None
}

export interface ClickEvent {
  screenX: number;
  screenY: number;
  x: number;
  y: number;
  timestamp: number;
}

export interface TouchObject {
  type: TouchType;
  id: number;
  screenX: number;
  screenY: number;
  x: number;
  y: number;
}

export interface TouchEvent {
  type: TouchType;
  touches: TouchObject[];
  changedTouches: TouchObject[];
  timestamp: number;
  stopPropagation?: () => void;
}

interface PasteData {
  getPlainText(): string;
  setPlainText(value: string);
}

export declare class PixelMap {
  release(): void;
}

interface DragEvent {
  getPasteData(): PasteData;
  getX(): number;
  getY(): number;
  getDescription(): string;
  setDescription(value: string);
  setPixmap(value: PixelMap);
}

export interface KeyEvent {
  type: KeyType;
  keyCode: number;
  keyText: string;
  keySource: KeySource;
  deviceId: number;
  metaKey: number;
  timestamp: number;
  stopPropagation?: () => void;
}

export declare enum Alignment {
  TopStart,
  Top,
  TopEnd,
  Start,
  Center,
  End,
  BottomStart,
  Bottom,
  BottomEnd
}

export declare enum TransitionType {
  All,
  Insert,
  Delete
}

export declare enum RelateType {
  FILL,
  FIT
}

export declare enum Visibility {
  Visible,
  Hidden,
  None
}

export declare enum LineCapStyle {
  Butt,
  Round,
  Square
}

export declare enum Axis {
  Vertical,
  Horizontal
}

export declare enum HorizontalAlign {
  Start,
  Center,
  End
}

export declare enum FlexAlign {
  Start,
  Center,
  End,
  SpaceBetween,
  SpaceAround,
  SpaceEvenly
}

export declare enum ItemAlign {
  Auto,
  Start,
  Center,
  End,
  Baseline,
  Stretch
}

export declare enum FlexDirection {
  Row,
  Column,
  RowReverse,
  ColumnReverse
}

export declare enum FlexWrap {
  NoWrap,
  Wrap,
  WrapReverse
}

export declare enum VerticalAlign {
  Top,
  Center,
  Bottom
}

export declare class VerticalAlignDeclaration {
  constructor(value?: VerticalAlign)
}

export declare class HorizontalAlignDeclaration {
  constructor(value?: HorizontalAlign)
}

export declare enum ImageRepeat {
  NoRepeat,
  X,
  Y,
  XY
}

export declare enum ImageSize {
  Auto,
  Cover,
  Contain
}

export declare enum GradientDirection {
  Left,
  Top,
  Right,
  Bottom,
  LeftTop,
  LeftBottom,
  RightTop,
  RightBottom,
  None
}

export declare enum SharedTransitionEffectType {
  Static,
  Exchange
}

export declare class CommonMethod<T> {
  constructor();

  width(value: number | string | Resource): T;

  height(value: number | string | Resource): T;

  size(value: { width?: number | string | Resource, height?: number | string | Resource}): T;

  constraintSize(value: { minWidth?: number | string | Resource, maxWidth?: number | string | Resource, minHeight?: number | string | Resource, maxHeight?: number | string | Resource}): T;

  layoutPriority(value: number | string): T;

  layoutWeight(value: number | string): T;

  padding(value: { top?: number | string | Resource, right?: number | string | Resource, bottom?: number | string | Resource, left?: number | string  | Resource} | number | string | Resource): T;

  margin(value: { top?: number | string | Resource, right?: number | string | Resource, bottom?: number | string | Resource, left?: number | string  | Resource} | number | string | Resource): T;

  backgroundColor(value: Color | number | string | Resource): T;

  backgroundImage(src: string, repeat?: ImageRepeat): T;

  backgroundImageSize(value: { width?: number | string, height?: number | string } | ImageSize): T;

  backgroundImagePosition(value: { x?: number | string, y?: number | string } | Alignment): T;

  opacity(value: number | Resource): T

  border(value: { width?: number | string | Resource, color?: Color | number | string | Resource, radius?: number | string | Resource, style?: BorderStyle }): T;

  borderStyle(value: BorderStyle): T;

  borderWidth(value: number | string | Resource): T;

  borderColor(value: Color | number | string | Resource): T;

  borderRadius(value: number | string | Resource): T;

  navigationTitle(value: string): T;

  navigationSubTitle(value: string): T;

  hideNavigationBar(value: boolean): T;

  hideNavigationBackButton(value: boolean): T;

  toolBar(value: object): T;

  hideToolBar(value: boolean): T;

  onClick(event: (event?: ClickEvent) => void): T;

  onTouch(event: (event?: TouchEvent) => void): T;

  onHover(event: (isHover?: boolean) => void): T;

  onKeyEvent(event: (event?: KeyEvent) => void): T;

  onPan(event: (event?: any) => void): T;

  animation(value: {
    duration?: number, tempo?: number, curve?: Curve | string, delay?: number, iterations?: number,
    playMode?: PlayMode, onFinish?: () => void
  }): T;

  transition(value: {
    type?: TransitionType, opacity?: number,
    translate?: { x?: number | string, y?: number | string, z?: number | string },
    scale?: { x?: number, y?: number, z?: number, centerX?: number | string, centerY?: number | string },
    rotate?: { x?: number, y?: number, z?: number, centerX?: number | string, centerY?: number | string, angle: number | string }
  }): T;

  gesture(gesture: GestureType, mask?: GestureMask): T;

  priorityGesture(gesture: GestureType, mask?: GestureMask): T;

  parallelGesture(gesture: GestureType, mask?: GestureMask): T;

  blur(value: number): T;

  brightness(value: number): T;

  contrast(value: number): T;

  grayscale(value: number): T;

  colorBlend(value: Color | string | Resource): T;

  saturate(value: number): T;

  sepia(value: number): T;

  invert(value: number): T;

  hueRotate(value:{deg: string | number}): T;

  backdropBlur(value: number): T;

  windowBlur(value: { percent: number, style?: BlurStyle }): T;

  translate(value: { x?: number | string, y?: number | string, z?: number | string }): T;

  scale(value: { x?: number, y?: number, z?: number, centerX?: number | string, centerY?: number | string }): T;

  gridSpan(value: number): T

  gridOffset(value: number): T

  rotate(value: { x?: number, y?: number, z?: number, centerX?: number | string, centerY?: number | string, angle: number | string }): T;

  transform(value: object): T;

  onAppear(event: () => void): T;

  onDisAppear(event: () => void): T;

  visibility(value: Visibility): T;

  flexGrow(value: number): T;

  flexShrink(value: number): T;

  flexBasis(value: number | string): T;

  alignSelf(value: ItemAlign): T;

  displayPriority(value: number): T;

  useAlign(align: HorizontalAlignDeclaration | VerticalAlignDeclaration, value: { side: Edge, offset?: number | string }): T;

  zIndex(value: number): T;

  sharedTransition(id: string, options?: {
    duration?: number,
    curve?: Curve | string,
    delay?: number,
    motionPath?: {
      path: string,
      from?: number,
      to?: number,
      rotatable?: boolean
    },
    zIndex?: number,
    type?: SharedTransitionEffectType,
  }): T;

  direction(value: Direction): T;

  align(value: Alignment): T;

  position(value: { x?: number | string | Resource, y?: number | string | Resource }): T;

  markAnchor(value: { x?: number | string | Resource, y?: number | string | Resource }): T;

  offset(value: { x?: number | string | Resource, y?: number | string | Resource }): T;

  enabled(value: boolean): T;

  useSizeType(value: {
    xs?: number | { span: number, offset: number },
    sm?: number | { span: number, offset: number },
    md?: number | { span: number, offset: number },
    lg?: number | { span: number, offset: number }
  }): T;

  aspectRatio(value: number): T;

  onDrag(event: (event?: DragEvent) => void): T;

  onDragEnter(event: (event?: DragEvent) => void): T;

  onDragMove(event: (event?: DragEvent) => void): T;

  onDragLeave(event: (event?: DragEvent) => void): T;

  onDrop(event: (event?: DragEvent) => void): T;

  overlay(value: string, options?: { align?: Alignment, offset?: { x?: number, y?: number } }): T;

  linearGradient(value: { angle?: number | string, direction?: GradientDirection, colors: Array<any>, repeating?: boolean }): T;

  sweepGradient(value: { center: Array<any>, start?: number | string, end?: number | string, rotation?: number | string, colors: Array<any>, repeating?: boolean }): T;

  radialGradient(value: { center: Array<any>, radius: number | string, colors: Array<any>, repeating?: boolean }): T;

  motionPath(value: { path: string, from?: number, to?: number, rotatable?: boolean }): T;

  shadow(value: { radius: number | Resource, color?: Color | string | Resource, offsetX?: number | Resource, offsetY?: number | Resource }): T;

  clip(value: boolean | Circle | Ellipse | Path | Rect): T;

  mask(value: Circle | Ellipse | Path | Rect): T;

  accessibilityGroup(value: boolean): T;

  accessibilityText(value: string): T;

  accessibilityDescription(value: string): T;

  accessibilityImportance(value: string): T;

  onAccessibility(callback: (event?: { eventType: number }) => void): T;

  geometryTransition(id: string): T;

  bindPopup(show: boolean, popup: {
    message: string,
    placementOnTop: boolean,
    primaryButton: {
      value: string;
      action: () => void;
    };
    secondaryButton: {
      value: string;
      action: () => void;
    };
    onStateChange?: (callback: (event?: { isVisible: boolean }) => void) => void;
  }): T;
}

export declare class CommonShapeMethod<T> extends CommonMethod<T> {
  constructor();

  stroke(value: Color | number | string | Resource): T;

  fill(value: Color | number | string | Resource): T;

  strokeDashOffset(value: number | string): T;

  strokeLineCap(value: LineCapStyle): T;

  strokeLineJoin(value: LineJoinStyle): T;

  strokeMiterLimit(value: number | string): T;

  strokeOpacity(value: number | string | Resource): T;

  fillOpacity(value: number | string | Resource): T;

  strokeWidth(value: number | string | Resource): T;

  antiAlias(value: boolean): T;

  strokeDashArray(value: Array<any>): T
}

export declare class CustomComponent<T> {
  build(): void;

  private aboutToAppear?(): void;

  private aboutToDisappear?(): void;
}
