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

#include "reporter.h"
#include "communication_fault_impl.h"
#include "database_fault_impl.h"
#include "runtime_fault_impl.h"
#include "service_fault_impl.h"

#include "traffic_statistic_impl.h"
#include "visit_statistic_impl.h"
#include "database_statistic_impl.h"
#include "api_performance_statistic_impl.h"

namespace OHOS {
namespace DistributedKv {
Reporter* Reporter::GetInstance()
{
    static Reporter reporter;
    return &reporter;
}

FaultReporter* Reporter::CommunicationFault()
{
    static CommunicationFaultImpl communicationFault;
    return &communicationFault;
}

FaultReporter* Reporter::DatabaseFault()
{
    static DatabaseFaultImpl databaseFault;
    return &databaseFault;
}

FaultReporter* Reporter::RuntimeFault()
{
    static RuntimeFaultImpl runtimeFault;
    return &runtimeFault;
}

FaultReporter* Reporter::ServiceFault()
{
    static ServiceFaultImpl serviceFault;
    return &serviceFault;
}

StatisticReporter<TrafficStat>* Reporter::TrafficStatistic()
{
    static TrafficStatisticImpl trafficStatistic;
    return &trafficStatistic;
}

StatisticReporter<struct VisitStat>* Reporter::VisitStatistic()
{
    static VisitStatisticImpl visitStatistic;
    return &visitStatistic;
}

StatisticReporter<struct DbStat>* Reporter::DatabaseStatistic()
{
    static DatabaseStatisticImpl databaseStatistic;
    return &databaseStatistic;
}

StatisticReporter<ApiPerformanceStat>* Reporter::ApiPerformanceStatistic()
{
    static ApiPerformanceStatisticImpl apiPerformanceStat;
    return &apiPerformanceStat;
}
} // namespace DistributedKv
} // namespace OHOS