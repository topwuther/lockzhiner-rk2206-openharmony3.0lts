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

#ifndef CONSTANT_H
#define CONSTANT_H

#include <algorithm>
#include <string>
#include <map>
#include <vector>

#include "parameter.h"

namespace OHOS {
namespace Security {
namespace Permission {
class Constant {
public:
    enum ServiceId {
        SUBSYS_AAFWK_SYS_ABILITY_ID_BEGIN = 100,
        ABILITY_TOOLS_SERVICE_ID = 116,
        ABILITY_TEST_SERVICE_ID = 179,
        ABILITY_MGR_SERVICE_ID = 180,
        ABILITY_MST_SERVICE_ID = 181,
        SUBSYS_AAFWK_SYS_ABILITY_ID_END = 199,
        SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN = 200,
        SUBSYS_AI_SYS_ABILITY_ID_BEGIN = 300,
        SUBSYS_APPEXECFWK_SYS_ABILITY_ID_BEGIN = 400,
        BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401,
        SUBSYS_APPLICATIONS_SYS_ABILITY_ID_BEGIN = 500,
        APP_MGR_SERVICE_ID = 501,
        INSTALLD_SERVICE_ID = 511,
        SUBSYS_ARVR_SYS_ABILITY_ID_BEGIN = 600,
        SUBSYS_ARVRHARDWARE_SYS_ABILITY_ID_BEGIN = 700,
        SUBSYS_BARRIERFREE_SYS_ABILITY_ID_BEGIN = 800,
        SUBSYS_BIOMETRICS_SYS_ABILITY_ID_BEGIN = 900,
        SUBSYS_BIOMETRICS_SYS_ABILITY_FACERECOGNIZE = 901,
        SUBSYS_BIOMETRICS_SYS_ABILITY_FINGERPRINT = 902,
        SUBSYS_BIOMETRICS_SYS_ABILITY_VOICEID = 903,
        ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID = 3203,
        SUBSYS_SECURITY_PERMISSION_SYS_SERVICE_ID = 3501,
        DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID = 4802,
    };

    /**
     * Indicates message format version, should be compatible.
     */
    const static int32_t DISTRIBUTED_PERMISSION_SERVICE_VERSION = 1;

    /**
     * keep the strategy as the old to DPermission app
     */
    const static int32_t DPERMISSION_DEFAULT = 0;

    /**
     * can grant the permission to DPermission app
     */
    const static int32_t DPERMISSION_GRANT = 1;

    /**
     * cant grant the permission to DPermission app
     */
    const static int32_t DPERMISSION_DENY = 2;

    /**
     * Per user range.
     */
    const static int32_t PER_USER_RANGE = 100000;

    /**
     * First of application-specific UIDs starting.
     */
    const static int32_t FIRST_APPLICATION_UID = 10000;

    /**
     * Last of application-specific UIDs starting.
     */
    const static int32_t LAST_APPLICATION_UID = 19999;

    /**
     * Indicates subject application is ALLOWED to access target ability on object application,
     * associated with target permission.
     */
    const static int32_t PERMISSION_GRANTED = 0;

    /**
     * Indicates subject application is NOT ALLOWED to access target ability on object application,
     * associated with target permission.
     */
    const static int32_t PERMISSION_DENIED = -1;

    /**
     * Indicates the operation of RequestPermissionFromRemote can not be executed,
     * one request is executing now.
     */
    const static int32_t PERMISSION_REQUEST_NOT_FINISH = -11;

    /**
     * Cached status of permission wait for re granting.
     */
    const static int32_t PERMISSION_DEFAULT_STATUS = 0;

    /**
     * Cached status of permission granted.
     */
    const static int32_t PERMISSION_GRANTED_STATUS = 1;

    /**
     * Cached status of permission denied.
     */
    const static int32_t PERMISSION_DENIED_STATUS = 2;

    /**
     * Privilege uid, root.
     */
    const static int32_t ROOT_UID = 0;

    /**
     * Privilege uid, system.
     */
    const static int32_t SYSTEM_UID = 1000;

    /**
     * Max device id length limit.
     */
    const static int32_t MAX_DEVICE_ID_LENGTH = 64;

    /**
     * Max permission count per uid that we can handle.
     */
    const static int32_t MAX_UID_PERMISSIONS_COUNT = 1024;

    /**
     * Distributed user id support currently.
     */
    const static int32_t DISTRIBUTED_USER_ID = 126;

    /**
     * Max distributed uid capacity that we can allocated.
     */
    const static int32_t MAX_DISTRIBUTED_UID_NUMBERS = (LAST_APPLICATION_UID - FIRST_APPLICATION_UID) + 1;

    /**
     * Defines the first distributed uid.
     */
    const static int32_t FIRST_DISTRIBUTED_UID = (DISTRIBUTED_USER_ID * PER_USER_RANGE) + FIRST_APPLICATION_UID;

    /**
     * Defines the last distributed uid starting at {@link #FIRST_DISTRIBUTED_UID}.
     */
    const static int32_t LAST_DISTRIBUTED_UID = (DISTRIBUTED_USER_ID * PER_USER_RANGE) + LAST_APPLICATION_UID;

    /**
     * Status code, indicates general success.
     */
    const static int32_t SUCCESS = 0;

    /**
     * Status code, indicates general failure.
     */
    const static int32_t FAILURE = -1;

    /**
     * Status code, DPMS general failure.
     */
    const static int32_t FAILURE_DPMS = -2;

    /**
     * Status code, indicates failure but can retry.
     */
    const static int32_t FAILURE_BUT_CAN_RETRY = -2;

    /**
     * The initial value for app attribute.
     */
    const static int32_t INITIAL_APP_ATTRIBUTE = -2;

    /**
     * Status code, indicates target distributed uid not exist.
     */
    const static int32_t DISTRIBUTED_UID_NOT_EXIST = -2;

    /**
     * Status code, indicates device id invalid.
     */
    const static int32_t INVALID_DEVICE_ID = -3;

    /**
     * Status code, indicates rUid invalid.
     */
    const static int32_t INVALID_RUID = -4;

    /**
     * Status code, indicates not a privilege uid.
     */
    const static int32_t NOT_PRIVILEGE_UID = -5;

    /**
     * Status code, indicates wait duid ready time out.
     */
    const static int32_t WAIT_DISTRIBUTED_UID_TIME_OUT = -5;

    /**
     * Status code, indicates duid permission invalid.
     */
    const static int32_t INVALID_DISTRIBUTED_UID = -8;

    /**
     * Status code, indicates failed to execute remote command.
     */
    const static int32_t REMOTE_EXECUTE_FAILED = -9;

    /**
     * Status code, indicates uid permission invalid.
     */
    const static int32_t INVALID_UID_PERMISSION = -10;

    /**
     * Status code, indicates failed to get packages for uid.
     */
    const static int32_t CANNOT_GET_PACKAGE_FOR_UID = -12;

    /**
     * Exceed max uid limit. Defined by MAX_UID_PERMISSIONS_COUNT.
     */
    const static int32_t EXCEED_MAX_UID_PERMISSIONS_COUNT = -13;

    /**
     * Status Code, indicates invalid command.
     */
    const static int32_t INVALID_COMMAND = -14;

    /**
     * Session Id, indicates invalid session.
     */
    const static int32_t INVALID_SESSION = -1;

    /**
     * Command status code, indicate a status of command before RPC call.
     */
    const static int32_t STATUS_CODE_BEFORE_RPC = 100001;

    /**
     * The default container size for List, Set, Map and so on.
     */
    const static int32_t DEFAULT_CONTAINER_SIZE = 16;

    /**
     * The max waiting time.
     */
    const static int32_t MAX_WAITING_TIME = 2000;

    static constexpr int32_t INVALID_ID = -1;
    static constexpr int32_t DEFAULT_USERID = 0;

    static const std::string LOCATION;
    static const std::string LOCATION_IN_BACKGROUND;
    static const std::string CAMERA;
    static const std::string MICROPHONE;
    static const std::string READ_CALENDAR;
    static const std::string WRITE_CALENDAR;
    static const std::string ACTIVITY_MOTION;
    static const std::string READ_HEALTH_DATA;
    static const std::string DISTRIBUTED_DATASYNC;
    static const std::string DISTRIBUTED_DATA;
    static const std::string MEDIA_LOCATION;
    static const std::string READ_MEDIA;
    static const std::string WRITE_MEDIA;

    /**
     * Command result string, indicates success.
     */
    static const std::string COMMAND_RESULT_SUCCESS;

    /**
     * Command result string, indicates failed.
     */
    static const std::string COMMAND_RESULT_FAILED;

    /**
     * The maximum number of detailed reocrds in search results.
     */
    const static int32_t MAX_DETAIL_RECORDS = 10;

    /**
     * Database auto delete data time.
     */
    const static int32_t RECORD_DELETE_TIME = 30 * 86400;

    /**
     * Drop precision of timestamp, merge data which are inserted in specified time.
     */
    const static int32_t PRECISION = 60;

    /**
     * Get permission used record code, means success.
     */
    const static int32_t SUCCESS_GET_RECORD = 200;

    /**
     * Get permission used record code, means database operation failed.
     */
    const static int32_t DATABASE_FAILED = 500;

    /**
     * Get permission used record code, means permission name or opCode not defined in NAME_TO_OP_CODE_MAP.
     */
    const static int32_t NOT_DEFINED = 400;

    /**
     * Local device flag.
     */
    const static int32_t LOCAL_DEVICE = 1;

    /**
     * Distributed device flag.
     */
    const static int32_t DISTRIBUTED_DEVICE = 2;

    /**
     * All record flag(for JS).
     */
    const static int32_t ALL_RECORD_FLAG = 1;

    /**
     * Local device without detailed record flag(for JS).
     */
    const static int32_t LOCAL_FLAG = 2;

    /**
     * Local device with detailed record flag(for JS).
     */
    const static int32_t LOCAL_DETAIL_FLAG = 3;

    /**
     * Distributed device without detailed record flag(for JS).
     */
    const static int32_t DISTRIBUTED_FLAG = 4;

    /**
     * Distributed device with detailed record flag(for JS).
     */
    const static int32_t DISTRIBUTED_DETAIL_FLAG = 5;

    /**
     * Device id length.
     */
    const static int32_t DEVICE_UUID_LENGTH = 65;

    /**
     * The time that view the records of the last few days.
     */
    const static int32_t RECORD_TIME = 7 * 86400;

    static const std::string COMMAND_PERMISSIONS_COUNT_FAILED;
    static const std::string COMMAND_GET_PERMISSIONS_FAILED;
    static const std::string COMMAND_GET_REGRANTED_PERMISSIONS_FAILED;

    /**
     * PermissionName translate into opCode map.
     */
    const static std::map<std::string, int32_t> NAME_TO_OP_CODE_MAP;

    /**
     * PermissionName translate into opCode map.
     */
    static bool PermissionNameToOrFromOpCode(std::string &permissionName, int32_t &opCode);

    /**
     * PermissionName translate into opCode map.
     */
    static const std::vector<std::string> USE_BY_LOCAL_APP_LIST;

    /**
     * PermissionName translate into opCode map.
     */
    static bool UseByLocalApp(std::string &permissionName);

    static constexpr int32_t ENCRYPTLEN = 4;
    static constexpr int32_t ENCRYPTBEGIN = 0;
    static constexpr int32_t ENCRYPTEND = 3;
    static std::string EncryptDevId(std::string deviceId);

    /**
     * key info used in ResourceSwitchRemote.
     */
    static const std::string PROFILE_KEY_CAMERA;
    static const std::string PROFILE_KEY_MICROPHONE;
    static const std::string PROFILE_KEY_LOCATION;
    static const std::string PROFILE_KEY_HEALTH_SENSOR;

    static const int32_t RESOURCE_SWITCH_STATUS_DENIED;
    static const int32_t RESOURCE_SWITCH_STATUS_ALLOWED;

    /**
     * permission list for UI preference.
     */
    static const std::vector<std::string> PERMISSION_RECORDS_GETTING_LIST;

    /**
     * permission type
     */
    static const std::string SYSTEM_PERMISSION_TYPE;
    static const std::string APP_PERMISSION_TYPE;

    /**
     * GetLocalDeviceId
     */
    static std::string GetLocalDeviceId();
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_BASE_SERVICE_H