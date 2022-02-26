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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_CONTROL_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_CONTROL_MANAGER_H

#include "common_event_data.h"
#include "common_event_permission_manager.h"
#include "common_event_publish_info.h"
#include "common_event_record.h"
#include "common_event_subscriber_manager.h"
#include "history_event_record.h"
#include "ordered_event_handler.h"
#include "ordered_event_record.h"

namespace OHOS {
namespace EventFwk {
class CommonEventControlManager : public std::enable_shared_from_this<CommonEventControlManager> {
public:
    CommonEventControlManager();

    ~CommonEventControlManager();

    bool PublishCommonEvent(const CommonEventRecord &eventRecord, const sptr<IRemoteObject> &commonEventListener);

    std::shared_ptr<OrderedEventRecord> GetMatchingOrderedReceiver(sptr<IRemoteObject> proxy);

    bool FinishReceiverAction(std::shared_ptr<OrderedEventRecord> recordPtr, const int &code,
        const std::string &receiverData, const bool &abortEvent);

    void CurrentOrderedEventTimeout(bool isFromMsg);

    void ProcessNextOrderedEvent(bool isSendMsg);

    bool PublishFreezeCommonEvent(const uid_t &uid);

    void DumpState(const std::string &event, std::vector<std::string> &state);

    void DumpHistoryState(const std::string &event, std::vector<std::string> &state);

private:
    bool ProcessUnorderedEvent(const CommonEventRecord &eventRecord);

    bool GetUnorderedEventHandler();

    bool NotifyUnorderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecord);

    bool ProcessOrderedEvent(
        const CommonEventRecord &commonEventRecord, const sptr<IRemoteObject> &commonEventListener);

    bool GetOrderedEventHandler();

    bool EnqueueOrderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr);

    bool EnqueueUnorderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr);

    bool ScheduleOrderedCommonEvent();

    bool NotifyOrderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecordPtr, int index);

    bool SetTimeout(int64_t timeoutTime);

    bool CancelTimeout();

    bool FinishReceiver(std::shared_ptr<OrderedEventRecord> recordPtr, const int &code, const std::string &receiverData,
        const bool &abortEvent);

    int CheckPermission(const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord);

    bool CheckSubcriberPermission(const Permission &permission, const EventSubscriberRecord &subscriberRecord);

    void SetTime(int recIdx, std::shared_ptr<OrderedEventRecord> &sp, bool timeoutMessage);

    void CheckSubscriberRequiredPermission(const std::string &subscriberRequiredPermission,
        const CommonEventRecord &eventRecord, const EventSubscriberRecord &subscriberRecord, bool &skip);

    void CheckPublisherRequiredPermissions(const std::vector<std::string> &publisherRequiredPermissions,
        const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord, bool &skip);

    bool NotifyFreezeEvents(const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord);

    void GetOrderedEventRecords(const std::string &event, std::vector<std::shared_ptr<OrderedEventRecord>> &records);

    void GetUnorderedEventRecords(const std::string &event, std::vector<std::shared_ptr<OrderedEventRecord>> &records);

    void GetHistoryEventRecords(const std::string &event, std::vector<History_event_record> &records);

    void DumpStateByCommonEventRecord(const std::shared_ptr<OrderedEventRecord> &record, std::string &dumpInfo);

    void DumpStateBySubscriberRecord(const std::shared_ptr<OrderedEventRecord> &record, std::string &dumpInfo);

    void DumpHistoryStateBySubscriberRecord(const History_event_record &record, std::string &dumpInfo);

    void DumpHistoryStateByCommonEventRecord(const History_event_record &record, std::string &dumpInfo);

    void EnqueueHistoryEventRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr);

private:
    std::shared_ptr<EventHandler> handler_;
    std::shared_ptr<OrderedEventHandler> handlerOrdered_;
    std::vector<std::shared_ptr<OrderedEventRecord>> orderedEventQueue_;
    std::vector<std::shared_ptr<OrderedEventRecord>> unorderedEventQueue_;
    std::vector<History_event_record> historyEventRecords_;
    bool pendingTimeoutMessage_;
    bool scheduled_;
    const int64_t TIMEOUT;  // How long we allow a receiver to run before giving up on it. Unit: ms
    std::mutex orderedMutex_;
    std::mutex unorderedMutex_;
    std::mutex historyMutex_;
    const unsigned int HISTORY_MAX_SIZE = 100;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_CONTROL_MANAGER_H