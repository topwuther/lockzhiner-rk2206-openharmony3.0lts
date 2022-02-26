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

#ifndef DISTRIBUTEDDATAMGR_HI_VIEW_ADAPTER_H
#define DISTRIBUTEDDATAMGR_HI_VIEW_ADAPTER_H

#include <map>
#include <mutex>
#include "dfx_types.h"
#include "dfx_code_constant.h"
#include "hisysevent.h"
#include "kv_store_thread_pool.h"
#include "kv_store_task.h"
#include "value_hash.h"

namespace OHOS {
namespace DistributedKv {
template<typename T>
struct StatisticWrap {
    T val;
    int times;
    int code;
};

class HiViewAdapter {
public:
    ~HiViewAdapter();
    static void ReportFault(int dfxCode, const FaultMsg &msg);
    static void ReportVisitStatistic(int dfxCode, const VisitStat &stat);
    static void ReportTrafficStatistic(int dfxCode, const TrafficStat &stat);
    static void ReportDatabaseStatistic(int dfxCode, const DbStat &stat);
    static void ReportApiPerformanceStatistic(int dfxCode, const ApiPerformanceStat &stat);
    static void StartTimerThread();

private:
    static constexpr int POOL_SIZE = 3;
    static std::shared_ptr<KvStoreThreadPool> pool_;

    static std::mutex visitMutex_;
    static std::map<std::string, StatisticWrap<VisitStat>> visitStat_;
    static void InvokeVisit();

    static std::mutex trafficMutex_;
    static std::map<std::string, StatisticWrap<TrafficStat>> trafficStat_;
    static void InvokeTraffic();

    static std::mutex dbMutex_;
    static std::map<std::string, StatisticWrap<DbStat>> dbStat_;
    static void InvokeDbSize();
    static void ReportDbSize(const StatisticWrap<DbStat> &stat);

    static std::mutex apiPerformanceMutex_;
    static std::map<std::string, StatisticWrap<ApiPerformanceStat>> apiPerformanceStat_;
    static void InvokeApiPerformance();

private:
    // fault key
    static const inline std::string FAULT_TYPE = "FAULT_TYPE";
    static const inline std::string MODULE_NAME = "MODULE_NAME";
    static const inline std::string INTERFACE_NAME = "INTERFACE_NAME";
    static const inline std::string ERROR_TYPE = "ERROR_TYPE";

    // Database statistic
    static const inline std::string USER_ID = "ANONYMOUS_UID";
    static const inline std::string APP_ID = "APP_ID";
    static const inline std::string STORE_ID = "STORE_ID";
    static const inline std::string DB_SIZE = "DB_SIZE";

    // interface visit statistic
    static const inline std::string TIMES = "TIMES";
    static const inline std::string DEVICE_ID = "ANONYMOUS_DID";
    static const inline std::string SEND_SIZE = "SEND_SIZE";
    static const inline std::string RECEIVED_SIZE = "RECEIVED_SIZE";
    static const inline std::string COMPLETE_TIME = "COMPLETE_TIME";
    static const inline std::string SIZE = "SIZE";
    static const inline std::string SRC_DEVICE_ID = "ANONYMOUS_SRC_DID";
    static const inline std::string DST_DEVICE_ID = "ANONYMOUS_DST_DID";
    static const inline std::string AVERAGE_TIMES = "AVERAGE_TIME";
    static const inline std::string WORST_TIMES = "WORST_TIME";
    static const inline std::string INTERFACES = "INTERFACES";
private:
    static std::mutex runMutex_;
    static bool running_;
    static const inline int EXEC_TIME = 23;
    static const inline int SIXTY_SEC = 60;

    static const inline int WAIT_TIME = 2 * 60 * 60; // 2 hours
    static const inline int PERIOD_TIME_US = 1 * 1000 * 1000; // 1 s
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_HI_VIEW_ADAPTER_H
