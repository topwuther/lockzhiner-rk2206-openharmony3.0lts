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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_HISTORY_EVENT_RECORD_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_HISTORY_EVENT_RECORD_H

#include <string>
#include "want.h"

namespace OHOS {
namespace EventFwk {

struct HistorySubscriberRecord {
    struct tm recordTime {};
    std::string bundleName;
    int32_t priority;
    std::string permission;
    std::string deviceId;
    bool isFreeze;
    int64_t freezeTime;
    HistorySubscriberRecord() : priority(0), isFreeze(false), freezeTime(0)
    {}
};

struct History_event_record {
    enum EventState_ {
        IDLE = 0,
        RECEIVEING,
        RECEIVED,
    };
    enum DeliveryState_ {
        PENDING = 0,
        DELIVERED,
        SKIPPED,
        TIMEOUT,
    };

    Want want;
    int code;
    std::string data;

    bool sticky;
    bool ordered;
    std::vector<std::string> subscriberPermissions;

    struct tm recordTime {};
    pid_t pid;
    uid_t uid;
    std::string bundleName;
    bool isSystemEvent;

    std::vector<HistorySubscriberRecord> receivers;
    sptr<IRemoteObject> resultTo;
    std::vector<int> deliveryState;
    int64_t dispatchTime;
    int64_t receiverTime;
    int state;
    bool resultAbort;

    History_event_record()
        : code(0),
          sticky(false),
          ordered(false),
          pid(0),
          uid(0),
          isSystemEvent(false),
          resultTo(nullptr),
          dispatchTime(0),
          receiverTime(0),
          state(0),
          resultAbort(false)
    {}
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_HISTORY_EVENT_RECORD_H