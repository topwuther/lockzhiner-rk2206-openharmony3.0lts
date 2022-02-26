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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_ORDERED_EVENT_RECORD_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_ORDERED_EVENT_RECORD_H

#include "common_event_record.h"
#include "common_event_subscriber_manager.h"

namespace OHOS {
namespace EventFwk {
struct OrderedEventRecord : public CommonEventRecord {
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

    std::vector<std::shared_ptr<EventSubscriberRecord>> receivers;
    sptr<IRemoteObject> resultTo;
    sptr<IRemoteObject> curReceiver;
    std::vector<int> deliveryState;
    int enqueueClockTime;
    int64_t dispatchTime;
    int64_t receiverTime;
    int64_t finishTime;
    int nextReceiver;
    int state;
    bool resultAbort;

    OrderedEventRecord()
        : resultTo(nullptr),
          curReceiver(nullptr),
          enqueueClockTime(0),
          dispatchTime(0),
          receiverTime(0),
          finishTime(0),
          nextReceiver(0),
          state(0),
          resultAbort(false)
    {}

    inline void FillCommonEventRecord(const CommonEventRecord &commonEventRecord)
    {
        commonEventData = commonEventRecord.commonEventData;
        publishInfo = commonEventRecord.publishInfo;
        recordTime = commonEventRecord.recordTime;
        pid = commonEventRecord.pid;
        uid = commonEventRecord.uid;
        bundleName = commonEventRecord.bundleName;
        isSystemEvent = commonEventRecord.isSystemEvent;
    }
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_ORDERED_EVENT_RECORD_H