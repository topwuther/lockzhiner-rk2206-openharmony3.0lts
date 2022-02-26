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
import { TriggerInfo } from './wantAgent/triggerInfo';

/**
 * Provide the method obtain trigger, cancel, and compare and to obtain
 * the bundle name, UID of an {@link WantAgent} object.
 *
 * @name wantAgent
 * @since 7
 * @devices phone, tablet, tv, wearable, car
 * @import wantAgent from '@ohos.wantAgent';
 * @permission N/A
 */
declare namespace wantAgent {
  /**
   * Obtains the bundle name of an {@link WantAgent}.
   *
   * @param agent Indicates the {@link WantAgent} whose bundle name is to be obtained.
   * @return Returns the bundle name of the {@link WantAgent} if any.
   * @since 7
   */
  function getBundleName(agent: WantAgent, callback: AsyncCallback<string>): void;
  function getBundleName(agent: WantAgent): Promise<string>;

  /**
   * Obtains the bundle name of an {@link WantAgent}.
   *
   * @param agent Indicates the {@link WantAgent} whose UID is to be obtained.
   * @return Returns the UID of the {@link WantAgent} if any; returns {@code -1} otherwise.
   * @since 7
   */
  function getUid(agent: WantAgent, callback: AsyncCallback<number>): void;
  function getUid(agent: WantAgent): Promise<number>;

  /**
   * Obtains the {@link Want} of an {@link WantAgent}.
   *
   * @param agent Indicates the {@link WantAgent} whose UID is to be obtained.
   * @return Returns the {@link Want} of the {@link WantAgent}.
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function getWant(agent: WantAgent, callback: AsyncCallback<Want>): void;
  function getWant(agent: WantAgent): Promise<Want>;

  /**
   * Cancels an {@link WantAgent}. Only the application that creates the {@link IntentAgent} can cancel it.
   *
   * @param agent Indicates the {@link WantAgent} to cancel.
   * @since 7
   */
  function cancel(agent: WantAgent, callback: AsyncCallback<void>): void;
  function cancel(agent: WantAgent): Promise<void>;

  /**
   * Obtains the bundle name of an {@link WantAgent}.
   *
   * @param agent Indicates the {@link WantAgent} whose bundle name is to be obtained.
   * @param triggerInfo Indicates the {@link TriggerInfo} object that contains triggering parameters.
   * @param callback Indicates the callback method to be called after the {@link WantAgent} is triggered.
   * @since 7
   */
  function trigger(agent: WantAgent, triggerInfo: TriggerInfo, callback?: Callback<CompleteData>): void;

  /**
   * Checks whether two {@link WantAgent} objects are the same.
   *
   * @param agent Indicates one of the {@link WantAgent} object to compare.
   * @param otherAgent Indicates the other {@link WantAgent} object to compare.
   * @return Returns {@code true} If the two objects are the same; returns {@code false} otherwise.
   * @since 7
   */
  function equal(agent: WantAgent, otherAgent: WantAgent, callback: AsyncCallback<boolean>): void;
  function equal(agent: WantAgent, otherAgent: WantAgent): Promise<boolean>;

  /**
   * Obtains an {@link WantAgent} object.
   *
   * @param info Indicates the {@link WantAgentInfo} object that contains parameters of the
   * {@link WantAgent} object to create.
   * @return Returns the created {@link WantAgent} object.
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

  /**
   * A callback for the {@link trigger()} method. After the method execution is complete,
   * the callback process will start.
   *
   * @since 7
   */
  export interface CompleteData {
    info: WantAgent;
    want: Want;
    finalCode: number;
    finalData: string;
    extraInfo?: {[key: string]: any};
  }
}

export type WantAgent = object;

export default wantAgent;