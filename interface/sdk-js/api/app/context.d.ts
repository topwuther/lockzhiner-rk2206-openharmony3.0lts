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

import { AsyncCallback } from '../basic';
import { ApplicationInfo } from '../bundle/applicationInfo';
import { ProcessInfo } from './processInfo';
import { ElementName } from '../bundle/elementName';

/**
 * The context of an ability or an application.  It allows access to
 * application-specific resources, request and verification permissions.
 * Can only be obtained through the ability.
 *
 * @since 6
 * @SysCap SystemCapability.Appexecfwk
 * @devices phone, tablet, tv, wearable, car
 * @import import abilityManager from 'app/context'
 * @permission N/A
 */
export interface Context {

    /**
    * Get the local root dir of an app. If it is the first call, the dir
    * will be created.
    * @note If in the context of the ability, return the root dir of
    * the ability; if in the context of the application, return the
    * root dir of the application.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable, car
    * @return the root dir
    */
    getOrCreateLocalDir(): Promise<string>;
    getOrCreateLocalDir(callback: AsyncCallback<string>): void;
    /**
    * Verify whether the specified permission is allowed for a particular
    * pid and uid running in the system.
    * @param permission The name of the specified permission
    * @param pid process id
    * @param uid user id
    * @note Pid and uid are optional. If you do not pass in pid and uid,
    * it will check your own permission.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable, car
    * @return asynchronous callback with {@code 0} if the PID
    *         and UID have the permission; callback with {@code -1} otherwise.
    */
    verifyPermission(permission: string, options?: PermissionOptions): Promise<number>;
    verifyPermission(permission: string, options: PermissionOptions, callback: AsyncCallback<number>): void;
    verifyPermission(permission: string, callback: AsyncCallback<number>): void;

    /**
    * Requests certain permissions from the system.
    * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
    * @param requestCode Indicates the request code to be passed to the PermissionRequestResult
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable, car
    */
    requestPermissionsFromUser(permissions: Array<string>, requestCode: number, resultCallback: AsyncCallback<PermissionRequestResult>): void;

    /**
    * Obtains information about the current application.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable
    */
    getApplicationInfo(callback: AsyncCallback<ApplicationInfo>): void
    getApplicationInfo(): Promise<ApplicationInfo>;

    /**
    * Obtains the bundle name of the current ability.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable
    */
    getBundleName(callback: AsyncCallback<string>): void
    getBundleName(): Promise<string>;

    /**
    * Obtains information about the current process, including the process ID and name.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable
    */
    getProcessInfo(callback: AsyncCallback<ProcessInfo>): void
    getProcessInfo(): Promise<ProcessInfo>;

    /**
    * Obtains the ohos.bundle.ElementName object of the current ability. This method is available only to Page abilities.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable
    */
    getElementName(callback: AsyncCallback<ElementName>): void
    getElementName(): Promise<ElementName>;

    /**
    * Obtains the name of the current process.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable
    */
    getProcessName(callback: AsyncCallback<string>): void
    getProcessName(): Promise<string>;

    /**
    * Obtains the bundle name of the ability that called the current ability.
    * @since 7
    * @sysCap SystemCapability.Appexecfwk
    * @devices phone, tablet, tv, wearable
    */
    getCallingBundle(callback: AsyncCallback<string>): void
    getCallingBundle(): Promise<string>;
}

/**
 * @name the result of requestPermissionsFromUser with asynchronous callback
 * @since 7
 * @SysCap SystemCapability.Appexecfwk
 * @permission N/A
 * @devices phone, tablet, tv, wearable, car
 */
interface PermissionRequestResult {
    /**
    * @default The request code passed in by the user
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
    requestCode: number;

    /**
    * @default The permissions passed in by the user
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
    permissions: Array<string>;

    /**
    * @default The results for the corresponding request permissions
    * @since 7
    * @SysCap SystemCapability.Appexecfwk
    */
    authResults: Array<number>;
}

interface PermissionOptions {
 /**
  * @default The process id
  * @since 7
  * @SysCap SystemCapability.Appexecfwk
  */
 pid?: number;

 /**
  * @default The user id
  * @since 7
  * @SysCap SystemCapability.Appexecfwk
  */
 uid?: number;
}