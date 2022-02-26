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
import { AsyncCallback } from './basic';
import { Want } from './ability/want';
import { StartAbilityParameter } from './ability/startAbilityParameter';
import { AbilityResult } from './ability/abilityResult';
import { Context } from './app/context';
import { DataAbilityHelper } from './ability/dataAbilityHelper';
import { ConnectOptions } from './ability/connectOptions';

/**
 * A Feature Ability represents an ability with a UI and is designed to interact with users.
 * @name featureAbility
 * @since 6
 * @sysCap AAFwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace featureAbility {
  /**
   * Obtain the want sended from the source ability.
   *
   * @devices phone, tablet
   * @since 6
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function getWant(callback: AsyncCallback<Want>): void;
  function getWant(): Promise<Want>;

  /**
   * Starts a new ability.
   *
   * @devices phone, tablet
   * @since 6
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function startAbility(parameter: StartAbilityParameter, callback: AsyncCallback<number>): void;
  function startAbility(parameter: StartAbilityParameter): Promise<number>;

  /**
   * Obtains the application context.
   *
   * @return Returns the application context.
   * @since 6
   */
  function getContext(): Context;

  /**
   * Starts an ability and returns the execution result when the ability is destroyed.
   *
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return Returns the {@link AbilityResult}.
   */
  function startAbilityForResult(parameter: StartAbilityParameter, callback: AsyncCallback<AbilityResult>): void;
  function startAbilityForResult(parameter: StartAbilityParameter): Promise<AbilityResult>;

  /**
   * Sets the result code and data to be returned by this Page ability to the caller
   * and destroys this Page ability.
   *
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param parameter Indicates the result to return.
   * @return -
   */
  function terminateSelfWithResult(parameter: AbilityResult, callback: AsyncCallback<void>): void;
  function terminateSelfWithResult(parameter: AbilityResult): Promise<void>;

  /**
   * Destroys this Page ability.
   *
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @return -
   */
  function terminateSelf(callback: AsyncCallback<void>): void;
  function terminateSelf(): Promise<void>;

  /**
   * Obtains the dataAbilityHelper.
   *
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param uri Indicates the path of the file to open.
   * @return Returns the dataAbilityHelper.
   */
  function acquireDataAbilityHelper(uri: string): DataAbilityHelper;

   /**
   * Checks whether the main window of this ability has window focus.
   *
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @return Returns {@code true} if this ability currently has window focus; returns {@code false} otherwise.
   */
  function hasWindowFocus(callback: AsyncCallback<boolean>): void;
  function hasWindowFocus(): Promise<boolean>;

  /**
   * Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
   * @default -
   * @devices phone, tablet
   * @since 7
   * @SysCap aafwk
   * @param request The element name of the service ability
   * @param options The remote object instance
   * @return Returns the number code of the ability connected
   */
  function connectAbility(request: Want, options:ConnectOptions ): number;

  /**
  * The callback interface was connect successfully.
  * @default -
  * @devices phone, tablet
  * @since 7
  * @SysCap aafwk
  * @param connection The number code of the ability connected
  */
  function disconnectAbility(connection: number, callback:AsyncCallback<void>): void;
  function disconnectAbility(connection: number): Promise<void>;

  export enum AbilityWindowConfiguration {
    WINDOW_MODE_UNDEFINED = 0,
    WINDOW_MODE_FULLSCREEN = 1,
    WINDOW_MODE_SPLIT_PRIMARY = 100,
    WINDOW_MODE_SPLIT_SECONDARY = 101,
    WINDOW_MODE_FLOATING = 102
  }

  export enum AbilityStartSetting {
    BOUNDS_KEY = "abilityBounds",
    WINDOW_MODE_KEY = "windowMode",
    DISPLAY_ID_KEY = "displayId"
  }

  export enum ErrorCode {
    NO_ERROR = 0,
    INVALID_PARAMETER = -1,
    ABILITY_NOT_FOUND = -2,
    PERMISSION_DENY = -3
  }

  export enum ContinuationState {
    LOCAL_RUNNING = 0,
    REMOTE_RUNNING = 1,
    REPLICA_RUNNING = 2
  }

  export enum DataAbilityOperationType {
    TYPE_INSERT = 1,
    TYPE_UPDATE = 2,
    TYPE_DELETE = 3,
    TYPE_ASSERT = 4,
  }
}
export default featureAbility;