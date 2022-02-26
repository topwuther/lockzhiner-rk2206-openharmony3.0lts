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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_PUBLIC_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_PUBLIC_MANAGER_H

#include <map>
#include <stdint.h>
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
class PublishManager : public DelayedSingleton<PublishManager> {
public:
    struct FloodAttackAttribute {
        int publishNum;
        int64_t startPublishTime;
        FloodAttackAttribute()
        {
            publishNum = 0;
            startPublishTime = 0;
        }
    };

public:
    PublishManager();

    ~PublishManager();

    bool CheckIsFloodAttack(pid_t appUid);

private:
    std::map<pid_t, FloodAttackAttribute> floodAttackAppStatistics_;
    const int floodAttackMax_;
    const int floodAttackIntervalMax_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_PUBLIC_MANAGER_H