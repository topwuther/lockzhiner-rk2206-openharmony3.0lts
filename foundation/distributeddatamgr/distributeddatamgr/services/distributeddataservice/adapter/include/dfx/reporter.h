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

#ifndef DISTRIBUTEDDATAMGR_REPORTER_H
#define DISTRIBUTEDDATAMGR_REPORTER_H

#include <memory>
#include <mutex>
#include "dfx_types.h"
#include "statistic_reporter.h"
#include "fault_reporter.h"

namespace OHOS {
namespace DistributedKv {
class Reporter {
public:
    KVSTORE_API static Reporter* GetInstance();
    KVSTORE_API FaultReporter* ServiceFault();
    KVSTORE_API FaultReporter* RuntimeFault();
    KVSTORE_API FaultReporter* DatabaseFault();
    KVSTORE_API FaultReporter* CommunicationFault();

    KVSTORE_API StatisticReporter<DbStat>* DatabaseStatistic();
    KVSTORE_API StatisticReporter<VisitStat>* VisitStatistic();
    KVSTORE_API StatisticReporter<TrafficStat>* TrafficStatistic();
    KVSTORE_API StatisticReporter<ApiPerformanceStat>* ApiPerformanceStatistic();
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_REPORTER_H
