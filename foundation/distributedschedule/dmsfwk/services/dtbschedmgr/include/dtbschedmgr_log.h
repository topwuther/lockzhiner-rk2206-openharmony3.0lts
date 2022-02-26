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

#ifndef SERVICES_DTBSCHEDMGR_INCLUDE_DTBSCHEDMGR_LOG_H_
#define SERVICES_DTBSCHEDMGR_INCLUDE_DTBSCHEDMGR_LOG_H_

#include "errors.h"
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"

namespace OHOS {
namespace DistributedSchedule {
static constexpr OHOS::HiviewDFX::HiLogLabel DSCHED_SERVICE_LABEL = {LOG_CORE, 0xD001800, "DSched_Service"};

#ifdef HILOGD
#undef HILOGD
#endif

#ifdef HILOGF
#undef HILOGF
#endif

#ifdef HILOGE
#undef HILOGE
#endif

#ifdef HILOGW
#undef HILOGW
#endif

#ifdef HILOGI
#undef HILOGI
#endif

#define HILOGD(...) (void)OHOS::HiviewDFX::HiLog::Debug(DSCHED_SERVICE_LABEL, __VA_ARGS__)
#define HILOGE(...) (void)OHOS::HiviewDFX::HiLog::Error(DSCHED_SERVICE_LABEL, __VA_ARGS__)
#define HILOGF(...) (void)OHOS::HiviewDFX::HiLog::Fatal(DSCHED_SERVICE_LABEL, __VA_ARGS__)
#define HILOGI(...) (void)OHOS::HiviewDFX::HiLog::Info(DSCHED_SERVICE_LABEL, __VA_ARGS__)
#define HILOGW(...) (void)OHOS::HiviewDFX::HiLog::Warn(DSCHED_SERVICE_LABEL, __VA_ARGS__)

enum {
    /**
     * Module type: Distributed schedule Service side
     */
    DMS_MODULE_TYPE_SERVICE = 0,
};

// offset of dms error, only be used in this file.
constexpr ErrCode DMS_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_DISTRIBUTEDSCHEDULE, DMS_MODULE_TYPE_SERVICE);

enum {
    /**
     * Result(29360128) for invalid parameters.
     */
    INVALID_PARAMETERS_ERR = DMS_SERVICE_ERR_OFFSET,
     /**
     * Result(29360129) for no response when RPC call.
     */
    RPC_CALL_NORESPONSE_ERR = 29360129,
    /**
     * Result(29360130) for not find connect callback.
     */
    NO_CONNECT_CALLBACK_ERR = 29360130,
    /**
     * Result(29360131) for remote invalid parameters.
     */
    INVALID_REMOTE_PARAMETERS_ERR = 29360131,
    /**
     * Result(29360132) for remote device fail to bind ability.
     */
    REMOTE_DEVICE_BIND_ABILITY_ERR = 29360132,
    /**
     * Result(29360133) for remote device fail to unbind ability.
     */
    REMOTE_DEVICE_UNBIND_ABILITY_ERR = 29360133,
    /**
     * Result(29360134) for remote invalid parameters.
     */
    REMOTE_FAIL_GET_SHELLINFO_ERR = 29360134,
    /**
     * Result(29360135) for remote device fail to start ability.
     */
    REMOTE_DEVICE_START_ABILITY_ERR = 29360135,
    /**
     * Result(29360136) for remote device fail to stop ability.
     */
    REMOTE_DEVICE_STOP_ABILITY_ERR = 29360136,
    /**
     * Result(29360137) for reg repeat ability token.
     */
    REG_REPEAT_ABILITY_TOKEN_ERR = 29360137,
    /**
     * Result(29360138) for no ability token.
     */
    NO_ABILITY_TOKEN_ERR = 29360138,
    /**
     * Result(29360139) for no appthread.
     */
    NO_APP_THREAD_ERR = 29360139,
    /**
     * Result(29360140) for select device continue error.
     */
    SELECT_DEVICE_CONTINUE_ERR = 29360140,
    /**
     * Result(29360141) for dump write file error.
     */
    DMS_WRITE_FILE_FAILED_ERR = 29360141,
    /**
     * Result(29360142) for notify app the bind remote service's device is offline.
     */
    DEVICE_OFFLINE_ERR = 29360142,
    /**
     * Result(29360143) for not find hitrace id.
     */
    NO_HITRACE_RECORD_ERR = 29360143,
    /**
     * Result(29360144) for get local deviceid fail.
     */
    GET_LOCAL_DEVICE_ERR = 29360144,
    /**
     * Result(29360145) for get local deviceid fail.
     */
    GET_DATAABILITY_ERR = 29360145,
    /**
     * Result(29360146) for DistributedSched Service Publish Exception.
     */
    DMS_PUBLISH_EXCEPTION = 29360146,
    /**
     * Result(29360147) for DistributedSched Storage Exception.
     */
    DMS_GET_STORAGE_EXCEPTION = 29360147,
    /**
     * Result(29360148) for DistributedSched Service Get System Ability Manager Exception.
     */
    DMS_GET_SAMGR_EXCEPTION = 29360148,
    /**
     * Result(29360149) for DistributedSched Service IPC call noresponse error.
     */
    IPC_CALL_NORESPONSE_ERR = 29360149,
    /**
     * Result(29360150) for DistributedSched Service call bundle manger service error.
     */
    BUNDLE_MANAGER_SERVICE_ERR = 29360150,
    /**
     * Result(29360151) for DistributedSched Service call distributed manger error.
     */
    DISTRIBUTED_MANAGER_ERR = 29360151,
    /**
     * Result(29360152) for DistributedSched Service NotifyCompleteContinuation failed.
     */
    NOTIFYCOMPLETECONTINUATION_FAILED = 29360152,
    /**
     * Result(29360153) for DistributedSched Service Continue Ability Timeout Error.
     */
    CONTINUE_ABILITY_TIMEOUT_ERR = 29360153,
    /**
     * Result(29360154) for DistributedSched Service Notify Complete Continuation abilityToken null.
     */
    ABILITYTOKEN_NULL = 29360154,
    /**
     * Result(29360155) for get device list fail.
     */
    GET_DEVICE_LIST_ERR = 29360155,
    /**
     * Result(29360156) for notify ability install error.
     */
    NOTIFY_ABILITY_INSTALL_ERR = 29360156,
    /**
     * Result(29360157) for permission denied.
     */
    DMS_PERMISSION_DENIED = 29360157,
    /**
     * Result(29360158) for notify ability free install.
     */
    NOTIFY_ABILITY_FREE_INSTALL = 29360158,
    /**
     * Result(29360159) for notify app invalid uid.
     */
    BIND_ABILITY_UID_INVALID_ERR = 29360159,
    /**
     * Result(29360160) for notify app happen connect leak.
     */
    BIND_ABILITY_LEAK_ERR = 29360160,
    /**
     * Result(29360161) for can not service registry.
     */
    NOT_FIND_SERVICE_REGISTRY = 29360161,
    /**
     * Result(29360162) for can not get service.
     */
    NOT_FIND_SERVICE_PROXY = 29360162,
    /**
     * Result(29360163) for IPC Send Request to DEF fail.
     */
    SEND_REQUEST_DEF_FAIL = 29360163,
    /**
     * Result(29360164) for BMS can not find the specified AA.
     */
    CAN_NOT_FOUND_AA_ERR = 29360164,
    /**
     * Result(29360165) for DMS can not register device state callback.
     */
    REGISTER_DEVICE_STATE_CALLBACK_ERR = 29360165,
    /**
     * Result(29360166) for DMS can not unregister device state callback.
     */
    UNREGISTER_DEVICE_STATE_CALLBACK_ERR = 29360166,
    /**
     * Result(29360167) for DMS can not start slave device FA.
     */
    START_SLAVE_FA_ERR = 29360167,
    /**
     * Result(29360168) for DMS can not get dmsproxy handle.
     */
    GET_DMSPROXY_HANDLE_FAIL = 29360168,
    /**
     * Result(29360169) for dmsproxy service exception.
     */
    DMSPROXY_SERVICE_ERR = 29360169,
    /**
     * Result(29360170) for softbus service unavailable.
     */
    SOFTBUS_SERVICE_ERR = 29360170,
    /**
     * Result(29360171) for softbus register callback fail.
     */
    SOFTBUS_REGISTER_CALLBACK_ERR = 29360171,
    /**
     * Result(29360172) for get DPermission sa fail.
     */
    GET_DISTRIBUTED_PERMISSION_SA_ERR = 29360172,
    /**
     * Result(29360173) for get BMS sa fail.
     */
    GET_BUNDLE_MANAGER_ERR = 29360173,
    /**
     * Result(29360174) for get remote dms fail.
     */
    GET_REMOTE_DMS_FAIL = 29360174,
    /**
     * Result(29360175) for account access permission check failed.
     */
    DMS_ACCOUNT_ACCESS_PERMISSION_DENIED = 29360175,
    /**
     * Result(29360176) for component access permission check failed.
     */
    DMS_COMPONENT_ACCESS_PERMISSION_DENIED = 29360176,
    /**
     * Result(29360177) for distributed communication permission check failed.
     */
    DMS_DISTRIBUTED_COMMUNICATION_PERMISSION_DENIED = 29360177,
    /**
     * Result(29360178) for osd function not support.
     */
    TASK_OSD_NOT_SUPPORTED = 29360178,
    /**
     * Result(29360179) for osd switch is off.
     */
    TASK_OSD_NOT_ENABLED = 29360179,
    /**
     * Result(29360180) for osd switch is on.
     */
    TASK_OSD_ENABLED = 29360180,
    /**
     * Result(29360181) for front app check failed.
     */
    CHECK_FRONT_APP_FAIL = 29360181,
    /**
     * Result(29360182) for ble connected.
     */
    BLE_CONNECTED = 29360182,
    /**
     * Result(29360183) for open p2p session failed.
     */
    OPENP2PSESSION_FAILED = 29360183,
    /**
     * Result(29360184) for notify task change listener failed.
     */
    TASK_NOTIFY_CHANGE_LISTENER_ERR = 29360184,
    /**
     * Result(29360185) for subscribe data storage change failed.
     */
    TASK_SUBSCRIBE_DATA_STORAGE_ERR = 29360185,
    /**
     * Result(29360186) for sync snapshot failed.
     */
    TASK_SYNC_SNAPSHOT_ERR = 29360186,
    /**
     * Result(29360187) for sync task info list failed.
     */
    TASK_SYNC_TASK_INFO_LIST_ERR = 29360187,
    /**
     * Result(29360188) for distributed  communication permission check failed.
     */
    REMOTE_DISTRIBUTED_COMMUNICATION_PERMISSION_DENIED = 29360188
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // SERVICES_DTBSCHEDMGR_INCLUDE_DTBSCHEDMGR_LOG_H_
