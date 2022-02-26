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
import {Want} from "../api/common/ability/want";

export declare class AbilityController {
    constructor();
    startAbility(value: Want);
    performBackPress();
    getStackCount();
}

export declare class AbilityComponentExtend<T> extends AbilityComponentAttribute<T> {
}

interface AbilityComponent extends AbilityComponentAttribute<AbilityComponent> {
  (value: { want: Want, controller?: AbilityController }): AbilityComponent;
}

declare class AbilityComponentAttribute<T> extends CommonMethod<T> {
  onReady(event: () => void): T;
  onDestroy(event: () => void): T;
  onAbilityCreated(event: (name: string) => void): T;
  onAbilityMoveToFront(event: () => void): T;
  onAbilityWillRemove(event: () => void): T;
}

export declare const AbilityComponentInterface: AbilityComponent;
