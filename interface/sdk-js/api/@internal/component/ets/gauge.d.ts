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

import {CommonMethod, Color} from "./common";

export declare class GaugeExtend<T> extends GaugeAttribute<T> {
}

interface Gauge extends GaugeAttribute<Gauge> {
  (options: {value: number, min?: number, max?: number}): Gauge;
}

declare class GaugeAttribute<T> extends CommonMethod<T> {
  value(value: number): T;
  startAngle(angle: number): T;
  endAngle(angle: number): T;
  colors(colors: Array<any>): T;
  strokeWidth(length: number): T;
  labelTextConfig(markedLabelText: string): T;
  labelColorConfig(markedLabelColor: Color): T;
}

export declare const GaugeInterface: Gauge;