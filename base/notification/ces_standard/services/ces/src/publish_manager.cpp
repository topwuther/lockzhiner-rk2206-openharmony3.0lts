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

#include "publish_manager.h"

#include "event_log_wrapper.h"
#include "system_time.h"

namespace OHOS {
namespace EventFwk {

#define FLOOD_ATTACH_MAX 20
#define FLOOD_ATTACH_INTERVAL_MAX 5

PublishManager::PublishManager() : floodAttackMax_(FLOOD_ATTACH_MAX), floodAttackIntervalMax_(FLOOD_ATTACH_INTERVAL_MAX)
{}

PublishManager::~PublishManager()
{}

bool PublishManager::CheckIsFloodAttack(pid_t appUid)
{
    EVENT_LOGI("enter");

    bool isValid = false;

    floodAttackAppStatistics_[appUid].publishNum++;

    if (floodAttackAppStatistics_[appUid].startPublishTime == 0) {
        floodAttackAppStatistics_[appUid].startPublishTime = SystemTime::GetNowSysTime();
    }

    if (floodAttackAppStatistics_[appUid].publishNum > floodAttackMax_) {
        int64_t innternal = SystemTime::GetNowSysTime() - floodAttackAppStatistics_[appUid].startPublishTime;

        if (innternal >= floodAttackIntervalMax_) {
            floodAttackAppStatistics_[appUid].publishNum = 1;
            floodAttackAppStatistics_[appUid].startPublishTime = SystemTime::GetNowSysTime();
            isValid = true;
        }
    } else {
        isValid = true;
    }

    return isValid;
}
}  // namespace EventFwk
}  // namespace OHOS