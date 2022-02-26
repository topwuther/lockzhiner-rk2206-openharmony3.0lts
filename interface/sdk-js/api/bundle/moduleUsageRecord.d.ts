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
 * @name Stores FA usage information.
 * @since 7
 * @SysCap SystemCapability.Appexecfwk
 * @permission NA
 * @devices phone, tablet, tv, wearable, car
 * @systemapi hide this for inner system use
 */
export interface ModuleUsageRecord {
  /**
    * @default Indicates the name of the bundle containing the module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly bundleName: string;
  /**
    * @default Indicates the app label id of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly appLabelId: number;
  /**
    * @default Indicates the name of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly name: string;
  /**
    * @default Indicates the label id of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly labelId: number;
  /**
    * @default Indicates the description id of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly descriptionId: number;
  /**
    * @default Indicates the ability name of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly abilityName: string;
  /**
    * @default Indicates the ability label id of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly abilityLabelId: number;
  /**
    * @default Indicates the ability description id of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly abilityDescriptionId: number;
  /**
    * @default Indicates the ability icon id of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly abilityIconId: number;
  /**
    * @default Indicates the launched count of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly launchedCount: number;
  /**
    * @default Indicates the last launch time of this module
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly lastLaunchTime: number;
  /**
    * @default Indicates whether the module is removed
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly isRemoved: boolean;
  /**
    * @default Indicates whether free installation of the module is supported
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly installationFreeSupported: boolean;
}