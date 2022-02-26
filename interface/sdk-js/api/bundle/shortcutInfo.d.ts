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
  * @name  Provides methods for obtaining information about the ability that a shortcut will start, including the target
  *        bundle name and ability class name.
  * @since 7
  * @SysCap SystemCapability.Appexecfwk
  * @permission NA
  * @devices phone, tablet, tv, wearable, car
  */
export interface ShortcutWant{
  /**
    * @default Indicates the target bundle of the shortcut want
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly targetBundle: string;
  /**
    * @default Indicates the target class of the shortcut want
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly targetClass: string;
}

 /**
  * @name  Provides information about a shortcut, including the shortcut ID and label.
  * @since 7
  * @SysCap SystemCapability.Appexecfwk
  * @permission NA
  * @devices phone, tablet, tv, wearable, car
  */
export interface ShortcutInfo {
  /**
    * @default Indicates the ID of the application to which this shortcut belongs
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly id: string;
  /**
    * @default Indicates the name of the bundle containing the shortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly bundleName: string;
  /**
    * @default Indicates the host ability of the shortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly hostAbility: string;
  /**
    * @default Indicates the icon of the shortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly icon: string;
  /**
    * @default Indicates the label of the shortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly label: string;
  /**
    * @default Indicates the disableMessage of the shortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly disableMessage: string;
  /**
    * @default Indicates the wants of the shortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly wants: Array<ShortcutWant>;
  /**
    * @default Indicates whether the shortcut is static
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly isStatic?: boolean
  /**
    * @default Indicates whether the shortcut is homeshortcut
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly isHomeShortcut?: boolean;
  /**
    * @default Indicates whether the shortcut is enabled
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
  readonly isEnabled?: boolean;
}