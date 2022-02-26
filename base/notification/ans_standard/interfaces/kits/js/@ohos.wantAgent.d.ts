/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { AsyncCallback , Callback} from './basic';
import { Want } from './ability/want';
import { WantAgentInfo } from './wantAgent/wantAgentInfo';

/**
 * Provide the method obtain trigger, cancel, and compare and to obtain
 * the bundle name, UID of an {@link WantAgent} object.
 *
 * @name wantAgent
 * @since 7
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace wantAgent {
  /**
   * Obtains an {@link WantAgent} object.
   *
   * @param info Indicates the {@link WantAgentInfo} object that contains parameters of the
   * {@link WantAgent} object to create.
   * @return Returns the created {@link WantAgent} object.
   *
   * @since 7
   */
  function getWantAgent(info: WantAgentInfo, callback: AsyncCallback<WantAgent>): void;
  function getWantAgent(info: WantAgentInfo): Promise<WantAgent>;

  /**
   * Enumerates flags for using an {@link WantAgent}.
   *
   * @since 7
   */
  export enum WantAgentFlags {
    ONE_TIME_FLAG = 0,
    NO_BUILD_FLAG,
    CANCEL_PRESENT_FLAG,
    UPDATE_PRESENT_FLAG,
    CONSTANT_FLAG,
    REPLACE_ELEMENT,
    REPLACE_ACTION,
    REPLACE_URI,
    REPLACE_ENTITIES,
    REPLACE_BUNDLE
  }

  /**
   * Identifies the operation for using an {@link WantAgent}, such as starting an ability or sending a common event.
   *
   * @since 7
   */
  export enum OperationType {
    UNKNOWN_TYPE = 0,
    START_ABILITY,
    START_ABILITIES,
    START_SERVICE,
    SEND_COMMON_EVENT
  }
}

export type WantAgent = object;

export default wantAgent;