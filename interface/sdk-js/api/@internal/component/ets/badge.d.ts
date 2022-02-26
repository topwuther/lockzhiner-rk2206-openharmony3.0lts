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

import {CommonMethod, Color} from "./common"

declare enum BadgePosition {
  Right,
  RightTop,
  Left
}

interface BadgeStyle {
  color?: Color;
  fontSize?: number | string;
  badgeSize?: number | string;
  badgeColor: Color;
}

export declare class BadgeExtend<T> extends BadgeAttribute<T> {
}

interface Badge extends BadgeAttribute<Badge> {
  (value: {count: number, position?: BadgePosition, maxCount?: number, style?: BadgeStyle}): Badge;
  (value: {value: string, position?: BadgePosition, maxCount?: number, style?: BadgeStyle}): Badge;
}

declare class BadgeAttribute<T> extends CommonMethod<T> {
}

export declare const BadgeInterface: Badge