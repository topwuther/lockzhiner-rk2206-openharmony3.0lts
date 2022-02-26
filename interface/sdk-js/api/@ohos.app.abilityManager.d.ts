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
import { AbilityMissionInfo } from './app/abilityMissionInfo';
import { ActiveProcessInfo } from './app/activeProcessInfo';
import { MissionSnapshot } from './app/missionSnapshot';

/**
 * This module provides the capability to manage abilities and obtaining system task information.
 *
 * @since 7
 * @SysCap SystemCapability.Appexecfwk
 * @devices phone, tablet, tv, wearable, car
 * @import import abilityManager from '@ohos.app.abilityManager'
 * @permission N/A
 */
declare namespace abilityManager {

    enum ProcessErrCode {
        NO_ERROR = 0,
        CRASHED,
        NO_RESPONSE,
    }


    /**
     * Get information about running processes
     * @since 7
     * @SysCap SystemCapability.Appexecfwk
     * @devices phone, tablet, tv, wearable, car
     * @return a list of ActiveProcessInfo records describing each process.
     * @permission ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS_EXTENSION
     * @systemapi hide this for inner system use
     */
    function getActiveProcessInfos(): Promise<Array<ActiveProcessInfo>>;
    function getActiveProcessInfos(callback: AsyncCallback<Array<ActiveProcessInfo>>): void;

    /**
     * Get information about the running ability missions
     * @since 7
     * @SysCap SystemCapability.Appexecfwk
     * @devices phone, tablet, tv, wearable, car
     * @param upperLimit The maximum number of mission infos to return in the array.
     * @return an array of AbilityMissionInfo records describing each active mission.
     * @permission ohos.permission.ACCESS_MISSIONS
     * @systemapi hide this for inner system use
     */
     function getActiveAbilityMissionInfos(upperLimit: number): Promise<Array<AbilityMissionInfo>>;
     function getActiveAbilityMissionInfos(upperLimit: number, callback: AsyncCallback<Array<AbilityMissionInfo>>): void;

    /**
     * Get information about recently run missions
     * @since 7
     * @SysCap SystemCapability.Appexecfwk
     * @devices phone, tablet, tv, wearable, car
     * @param upperLimit The maximum number of previous mission infos to return in the array.
     * @return an array of AbilityMissionInfo records describing each of the previous mission.
     * @permission ohos.permission.ACCESS_MISSIONS_EXTRA
     * @systemapi hide this for inner system use
     */
    function getPreviousAbilityMissionInfos(upperLimit: number): Promise<Array<AbilityMissionInfo>>;
    function getPreviousAbilityMissionInfos(upperLimit: number, callback: AsyncCallback<Array<AbilityMissionInfo>>): void;

    /**
     * Delete the specified missions
     * @since 7
     * @SysCap SystemCapability.Appexecfwk
     * @devices phone, tablet, tv, wearable, car
     * @param missionIds An array of missions, representing the missions that need to be deleted.
     * @permission ohos.permission.DELETE_MISSIONS
     * @systemapi hide this for inner system use
     */
    function deleteMissions(missionIds: Array<number>): Promise<void>;
    function deleteMissions(missionIds: Array<number>, callback: AsyncCallback<void>): void;
}

export default abilityManager;
