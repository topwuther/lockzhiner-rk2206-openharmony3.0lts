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

import {CommonMethod} from "./common";

export declare enum BarMode {
  Scrollable,
  Fixed
}

export declare enum BarPosition {
  Start,
  End
}

export declare class TabsController {
  constructor();

  changeIndex(value: number): void;
}

export declare class TabsExtend<T> extends TabsAttribute<T> {
}

interface Tabs extends TabsAttribute<Tabs> {
  (value?: { barPosition?: BarPosition, index?: number, controller?: TabsController }): Tabs;
}

declare class TabsAttribute<T> extends CommonMethod<T> {
  vertical(value: boolean): T;

  scrollable(value: boolean): T;

  barMode(value: BarMode): T;

  barWidth(value: number): T;

  barHeight(value: number): T;

  animationDuration(value: number): T;

  onChange(event: (index: number) => void): T;
}

export declare const TabsInterface: Tabs;
