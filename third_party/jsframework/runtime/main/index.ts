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
 * @fileOverview framework entry.
 */

import Vm from './model';
export { createInstance, destroyInstance } from './manage/instance/life';
import { registerModules } from './manage/instance/register';
export { receiveTasks } from './manage/event/bridge';
export { getRoot } from './manage/instance/misc';
import {
  appDestroy,
  appError,
  appShow,
  appHide
} from './app/index';

/**
 * Prevent modification of Vm and Vm.prototype.
 */
Object.freeze(Vm);

export { registerModules, appDestroy, appError, appShow, appHide };
