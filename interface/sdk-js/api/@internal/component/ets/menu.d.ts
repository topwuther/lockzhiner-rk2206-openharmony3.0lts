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

import { CommonMethod, Resource } from "./common";

export declare enum MenuType {
  Click,
  LongPress
}

export declare class MenuExtend<T> extends MenuAttribute<T> {
}

interface Menu extends MenuAttribute<Menu> {
  (options?: {type?: MenuType, title?: string | Resource}): Menu;
}

declare class MenuAttribute<T> extends CommonMethod<T> {
  show(value: boolean): T;
  showPosition(options: {x: number, y: number}): T;
}

export declare const MenuInterface: Menu;