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

#ifndef DISTRIBUTEDDATAMGR_DFX_TYPES_H
#define DISTRIBUTEDDATAMGR_DFX_TYPES_H

#include <string>
#include <memory>
#include "db_meta_callback_delegate.h"

namespace OHOS {
namespace DistributedKv {
struct ModuleName {
    static const inline std::string DEVICE = "DEVICE";
    static const inline std::string USER = "USER";
};

enum class Fault {
    // Service Fault
    SF_SERVICE_PUBLISH = 0,
    SF_GET_SERVICE = 1,
    SF_CREATE_DIR = 2,
    SF_DATABASE_CONFIG = 3,
    SF_PROCESS_LABEL = 4,
    SF_INIT_DELEGATE = 5,

    // Runtime Fault
    RF_OPEN_DB = 20,
    RF_CLOSE_DB = 21,
    RF_DELETE_DB = 22,
    RF_DATA_SUBSCRIBE = 23,
    RF_DATA_UNSUBSCRIBE = 24,
    RF_CREATE_SNAPSHOT = 25,
    RF_RELEASE_SNAPSHOT = 26,
    RF_KEY_STORE_DAMAGE = 27,
    RF_KEY_STORE_LOST = 28,
    RF_DB_ROM = 29,
    RF_GET_DB = 30,
    RF_SUBCRIBE =  31,
    RF_UNSUBCRIBE =  32,
    RF_REPORT_THERAD_ERROR = 33,

    // Communication Fault
    CF_CREATE_SESSION = 40,
    CF_CREATE_PIPE = 41,
    CF_SEND_PIPE_BROKEN = 42,
    CF_REV_PIPE_BROKEN = 43,
    CF_SERVICE_DIED = 44,
    CF_CREATE_ZDN = 45,
    CF_QUERY_ZDN = 46,

    // Database Fault
    DF_DB_DAMAGE = 60,
};

enum class FaultType {
    SERVICE_FAULT = 0,
    RUNTIME_FAULT = 1,
    DATABASE_FAULT = 2,
    COMM_FAULT = 3,
};

struct FaultMsg {
    FaultType faultType;
    std::string moduleName;
    std::string interfaceName;
    Fault errorType;
};

struct VisitStat {
    std::string appId;
    std::string interfaceName;
    KVSTORE_API std::string GetKey() const
    {
        return appId + interfaceName;
    }
};

struct TrafficStat {
    std::string appId;
    std::string deviceId;
    int sendSize;
    int receivedSize;
    KVSTORE_API std::string GetKey() const
    {
        return appId + deviceId;
    }
};

struct DbStat {
    std::string userId;
    std::string appId;
    std::string storeId;
    int dbSize;
    std::shared_ptr<DbMetaCallbackDelegate> delegate;

    KVSTORE_API std::string GetKey() const
    {
        return userId + appId + storeId;
    }
};

struct DbPerformanceStat {
    std::string appId;
    std::string storeId;
    uint64_t currentTime;
    uint64_t completeTime;
    int size;
    std::string srcDevId;
    std::string dstDevId;
};

struct ApiPerformanceStat {
    std::string interfaceName;
    uint64_t costTime;
    uint64_t averageTime;
    uint64_t worstTime;
    KVSTORE_API std::string GetKey() const
    {
        return interfaceName;
    }
};

enum class ReportStatus {
    SUCCESS = 0,
    ERROR = 1,
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_DFX_TYPES_H
