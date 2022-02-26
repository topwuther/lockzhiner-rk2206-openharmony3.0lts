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

#include "common_event_control_manager.h"
#include <cinttypes>
#include "bundle_manager_helper.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
#include "ievent_receive.h"
#include "system_time.h"

namespace OHOS {
namespace EventFwk {

#define TENSECONDS 10000
const int LENGTH = 80;
using frozenRecords = std::map<std::shared_ptr<EventSubscriberRecord>, std::vector<std::shared_ptr<CommonEventRecord>>>;

CommonEventControlManager::CommonEventControlManager()
    : handler_(nullptr), handlerOrdered_(nullptr), pendingTimeoutMessage_(false), scheduled_(false), TIMEOUT(TENSECONDS)
{
    EVENT_LOGD("enter");
}

CommonEventControlManager::~CommonEventControlManager()
{
    EVENT_LOGD("enter");
}

bool CommonEventControlManager::PublishCommonEvent(
    const CommonEventRecord &eventRecord, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    bool ret = false;

    if (!eventRecord.publishInfo->IsOrdered()) {
        ret = ProcessUnorderedEvent(eventRecord);
    } else {
        ret = ProcessOrderedEvent(eventRecord, commonEventListener);
    }

    return ret;
}

bool CommonEventControlManager::PublishFreezeCommonEvent(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!GetUnorderedEventHandler()) {
        return false;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    frozenRecords frozenEventRecords = spinstance->GetFrozenEvents(uid);

    EVENT_LOGD("frozenEventRecords size: %{public}zu", frozenEventRecords.size());
    for (auto record : frozenEventRecords) {
        EVENT_LOGD("CommonEventRecord size: %{public}zu", record.second.size());
        for (auto vec : record.second) {
            EVENT_LOGD("subscriber proxy: %{public}p", &(record.first->commonEventListener));
            std::function<void()> innerCallback =
                std::bind(&CommonEventControlManager::NotifyFreezeEvents, this, *(record.first), *vec);
            handler_->PostImmediateTask(innerCallback);
        }
    }

    return true;
}

bool CommonEventControlManager::NotifyFreezeEvents(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");
    EVENT_LOGD("subscriber proxy: %{public}p", &subscriberRecord.commonEventListener);
    EVENT_LOGD("subscriber uid: %{public}d", subscriberRecord.uid);
    EVENT_LOGD("subscriber isFreeze: %{public}d", subscriberRecord.isFreeze);
    EVENT_LOGD("subscriber freezeTime: %{public}" PRId64, subscriberRecord.freezeTime);
    EVENT_LOGD("CommonEvent Action: %{public}s", eventRecord.commonEventData->GetWant().GetAction().c_str());
    EVENT_LOGD("CommonEvent Type: %{public}s", eventRecord.commonEventData->GetWant().GetType().c_str());

    sptr<IEventReceive> commonEventListenerProxy = iface_cast<IEventReceive>(subscriberRecord.commonEventListener);

    if (!commonEventListenerProxy) {
        EVENT_LOGE("Fail to get IEventReceive proxy");
        return false;
    }

    int ret = CheckPermission(subscriberRecord, eventRecord);
    if (ret != OrderedEventRecord::DELIVERED) {
        EVENT_LOGE("check permission is failed");
        return false;
    }
    commonEventListenerProxy->NotifyEvent(*(eventRecord.commonEventData), false, eventRecord.publishInfo->IsSticky());
    return true;
}

bool CommonEventControlManager::GetUnorderedEventHandler()
{
    if (!handler_) {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
        if (!handler_) {
            EVENT_LOGE("Failed to create UnorderedEventHandler");
            return false;
        }
    }
    return true;
}

bool CommonEventControlManager::NotifyUnorderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecord)
{
    EVENT_LOGI("enter");
    std::lock_guard<std::mutex> lock(unorderedMutex_);
    for (auto vec : eventRecord->receivers) {
        int index = eventRecord->nextReceiver++;
        EVENT_LOGD("vec->uid: %{public}d", vec->uid);
        EVENT_LOGD("vec->isFreeze: %{public}d", vec->isFreeze);
        eventRecord->curReceiver = vec->commonEventListener;
        if (vec->isFreeze) {
            eventRecord->deliveryState[index] = OrderedEventRecord::SKIPPED;
            DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertFrozenEvents(vec, *eventRecord);
        } else {
            sptr<IEventReceive> commonEventListenerProxy = iface_cast<IEventReceive>(vec->commonEventListener);
            if (!commonEventListenerProxy) {
                eventRecord->deliveryState[index] = OrderedEventRecord::SKIPPED;
                EVENT_LOGE("Failed to get IEventReceive proxy");
                continue;
            }
            int ret = CheckPermission(*vec, *eventRecord);
            eventRecord->deliveryState[index] = ret;
            if (ret == OrderedEventRecord::DELIVERED) {
                eventRecord->state = OrderedEventRecord::RECEIVEING;
                commonEventListenerProxy->NotifyEvent(
                    *(eventRecord->commonEventData), false, eventRecord->publishInfo->IsSticky());
                eventRecord->state = OrderedEventRecord::RECEIVED;
            }
        }
    }

    EnqueueHistoryEventRecord(eventRecord);

    auto it = std::find(unorderedEventQueue_.begin(), unorderedEventQueue_.end(), eventRecord);
    if (it != unorderedEventQueue_.end()) {
        unorderedEventQueue_.erase(it);
    }
    return true;
}

bool CommonEventControlManager::ProcessUnorderedEvent(const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    bool ret = false;

    if (!GetUnorderedEventHandler()) {
        return ret;
    }

    std::shared_ptr<OrderedEventRecord> eventRecordPtr = std::make_shared<OrderedEventRecord>();
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return ret;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    eventRecordPtr->FillCommonEventRecord(eventRecord);
    eventRecordPtr->receivers = spinstance->GetSubscriberRecords(eventRecord.commonEventData->GetWant());

    for (auto vec : eventRecordPtr->receivers) {
        eventRecordPtr->deliveryState.emplace_back(OrderedEventRecord::PENDING);
    }

    EnqueueUnorderedRecord(eventRecordPtr);

    std::function<void()> innerCallback =
        std::bind(&CommonEventControlManager::NotifyUnorderedEvent, this, eventRecordPtr);

    if (eventRecord.isSystemEvent) {
        ret = handler_->PostImmediateTask(innerCallback);
    } else {
        ret = handler_->PostTask(innerCallback);
    }

    return ret;
}

std::shared_ptr<OrderedEventRecord> CommonEventControlManager::GetMatchingOrderedReceiver(sptr<IRemoteObject> proxy)
{
    EVENT_LOGI("enter");

    std::lock_guard<std::mutex> lock(orderedMutex_);

    if (orderedEventQueue_.size() > 0) {
        std::shared_ptr<OrderedEventRecord> firstRecord = orderedEventQueue_.front();
        if ((firstRecord != nullptr) && (firstRecord->curReceiver == proxy)) {
            return firstRecord;
        }
    }

    return nullptr;
}

bool CommonEventControlManager::GetOrderedEventHandler()
{
    if (!handlerOrdered_) {
        handlerOrdered_ = std::make_shared<OrderedEventHandler>(EventRunner::Create(), shared_from_this());
        if (!handlerOrdered_) {
            EVENT_LOGE("Failed to create OrderedEventHandler");
            return false;
        }
    }
    return true;
}

bool CommonEventControlManager::ProcessOrderedEvent(
    const CommonEventRecord &commonEventRecord, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    bool ret = false;

    if (!GetOrderedEventHandler()) {
        return ret;
    }

    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    if (eventRecord == nullptr) {
        EVENT_LOGE("eventRecord is null");
        return ret;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    eventRecord->FillCommonEventRecord(commonEventRecord);
    eventRecord->resultTo = commonEventListener;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;
    eventRecord->receivers = spinstance->GetSubscriberRecords(commonEventRecord.commonEventData->GetWant());
    for (auto vec : eventRecord->receivers) {
        eventRecord->deliveryState.emplace_back(OrderedEventRecord::PENDING);
    }

    EnqueueOrderedRecord(eventRecord);

    ret = ScheduleOrderedCommonEvent();

    return ret;
}

bool CommonEventControlManager::EnqueueUnorderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr)
{
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return false;
    }

    std::lock_guard<std::mutex> lock(unorderedMutex_);

    unorderedEventQueue_.emplace_back(eventRecordPtr);

    return true;
}

bool CommonEventControlManager::EnqueueOrderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr)
{
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return false;
    }

    std::lock_guard<std::mutex> lock(orderedMutex_);

    orderedEventQueue_.emplace_back(eventRecordPtr);

    return true;
}

void CommonEventControlManager::EnqueueHistoryEventRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr)
{
    EVENT_LOGI("enter");

    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is nullptr");
        return;
    }

    History_event_record record;
    record.want = eventRecordPtr->commonEventData->GetWant();
    record.code = eventRecordPtr->commonEventData->GetCode();
    record.data = eventRecordPtr->commonEventData->GetData();

    record.sticky = eventRecordPtr->publishInfo->IsSticky();
    record.ordered = eventRecordPtr->publishInfo->IsOrdered();
    record.subscriberPermissions = eventRecordPtr->publishInfo->GetSubscriberPermissions();

    record.recordTime = eventRecordPtr->recordTime;
    record.pid = eventRecordPtr->pid;
    record.uid = eventRecordPtr->uid;
    record.bundleName = eventRecordPtr->bundleName;
    record.isSystemEvent = eventRecordPtr->isSystemEvent;

    for (auto vec : eventRecordPtr->receivers) {
        HistorySubscriberRecord receiver;
        receiver.recordTime = vec->recordTime;
        receiver.bundleName = vec->bundleName;
        receiver.priority = vec->eventSubscriberInfo->GetPriority();
        receiver.permission = vec->eventSubscriberInfo->GetPermission();
        receiver.deviceId = vec->eventSubscriberInfo->GetDeviceId();
        receiver.isFreeze = vec->isFreeze;
        receiver.freezeTime = vec->freezeTime;
        record.receivers.emplace_back(receiver);
    }

    record.resultTo = eventRecordPtr->resultTo;
    record.deliveryState = eventRecordPtr->deliveryState;
    record.dispatchTime = eventRecordPtr->dispatchTime;
    record.receiverTime = eventRecordPtr->receiverTime;
    record.state = eventRecordPtr->state;
    record.resultAbort = eventRecordPtr->resultAbort;

    std::lock_guard<std::mutex> lock(historyMutex_);
    if (historyEventRecords_.size() == HISTORY_MAX_SIZE) {
        historyEventRecords_.erase(historyEventRecords_.begin());
    }
    historyEventRecords_.emplace_back(record);
}

bool CommonEventControlManager::ScheduleOrderedCommonEvent()
{
    EVENT_LOGI("enter");

    if (scheduled_) {
        return true;
    }

    scheduled_ = true;

    return handlerOrdered_->SendEvent(InnerEvent::Get(OrderedEventHandler::ORDERED_EVENT_START));
}

bool CommonEventControlManager::NotifyOrderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecordPtr, int index)
{
    EVENT_LOGI("enter with index %{public}d", index);

    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr = nullptr");
        return false;
    }

    int receiverNum = eventRecordPtr->receivers.size();
    if ((index < 0) || (index >= receiverNum)) {
        EVENT_LOGE("Invalid index (= %{public}d)", index);
        return false;
    }

    int ret = CheckPermission(*(eventRecordPtr->receivers[index]), *eventRecordPtr);
    if (ret == OrderedEventRecord::SKIPPED) {
        eventRecordPtr->deliveryState[index] = ret;
    } else if (ret == OrderedEventRecord::DELIVERED) {
        if (eventRecordPtr->receivers[index]->isFreeze) {
            EVENT_LOGD("vec->uid: %{public}d", eventRecordPtr->receivers[index]->uid);
            EVENT_LOGD("vec->isFreeze: %{public}d", eventRecordPtr->receivers[index]->isFreeze);
            DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertFrozenEvents(
                eventRecordPtr->receivers[index], *eventRecordPtr);
            eventRecordPtr->deliveryState[index] = OrderedEventRecord::SKIPPED;
            eventRecordPtr->curReceiver = nullptr;
            return true;
        }

        eventRecordPtr->deliveryState[index] = ret;

        eventRecordPtr->curReceiver = eventRecordPtr->receivers[index]->commonEventListener;

        eventRecordPtr->state = OrderedEventRecord::RECEIVEING;

        sptr<IEventReceive> receiver = iface_cast<IEventReceive>(eventRecordPtr->curReceiver);
        if (!receiver) {
            EVENT_LOGE("Failed to get IEventReceive proxy");
            eventRecordPtr->curReceiver = nullptr;
            return false;
        }

        eventRecordPtr->state = OrderedEventRecord::RECEIVED;

        receiver->NotifyEvent(*(eventRecordPtr->commonEventData), true, eventRecordPtr->publishInfo->IsSticky());
    }

    return true;
}

void CommonEventControlManager::ProcessNextOrderedEvent(bool isSendMsg)
{
    EVENT_LOGI("enter");

    if (isSendMsg) {
        scheduled_ = false;
    }

    std::shared_ptr<OrderedEventRecord> sp = nullptr;

    std::lock_guard<std::mutex> lock(orderedMutex_);

    do {
        if (orderedEventQueue_.size() == 0) {
            return;
        }

        sp = orderedEventQueue_.front();
        bool forceReceive = false;
        int numReceivers = sp->receivers.size();
        int64_t nowSysTime = SystemTime::GetNowSysTime();

        if (sp->dispatchTime > 0) {
            if ((numReceivers > 0) && (nowSysTime > sp->dispatchTime + (2 * TIMEOUT * numReceivers))) {
                CurrentOrderedEventTimeout(false);
                forceReceive = true;
                sp->state = OrderedEventRecord::IDLE;
            }
        }

        if (sp->state != OrderedEventRecord::IDLE) {
            return;
        }

        if ((sp->receivers.size() == 0) || (sp->nextReceiver >= numReceivers) || sp->resultAbort || forceReceive) {
            // No more receivers for this ordered common event, then process the final result receiver
            if (sp->resultTo != nullptr) {
                EVENT_LOGI("Process the final subscriber");
                sptr<IEventReceive> receiver = iface_cast<IEventReceive>(sp->resultTo);
                if (!receiver) {
                    EVENT_LOGE("Failed to get IEventReceive proxy");
                    return;
                }
                receiver->NotifyEvent(*(sp->commonEventData), true, sp->publishInfo->IsSticky());

            }

            CancelTimeout();

            EnqueueHistoryEventRecord(sp);

            orderedEventQueue_.erase(orderedEventQueue_.begin());

            sp = nullptr;
        }
    } while (sp == nullptr);

    int recIdx = sp->nextReceiver++;
    SetTime(recIdx, sp, pendingTimeoutMessage_);

    NotifyOrderedEvent(sp, recIdx);
    if (sp->curReceiver == nullptr) {
        sp->state = OrderedEventRecord::IDLE;
        ScheduleOrderedCommonEvent();
    }

    return;
}

bool CommonEventControlManager::SetTimeout(int64_t timeoutTime)
{
    EVENT_LOGI("enter");

    bool ret = true;

    if (!pendingTimeoutMessage_) {
        pendingTimeoutMessage_ = true;
        ret = handlerOrdered_->SendTimingEvent(OrderedEventHandler::ORDERED_EVENT_TIMEOUT, timeoutTime);
    }

    return ret;
}

bool CommonEventControlManager::CancelTimeout()
{
    EVENT_LOGI("enter");

    if (pendingTimeoutMessage_) {
        pendingTimeoutMessage_ = false;
        handlerOrdered_->RemoveEvent(OrderedEventHandler::ORDERED_EVENT_TIMEOUT);
    }

    return true;
}

void CommonEventControlManager::CurrentOrderedEventTimeout(bool isFromMsg)
{
    EVENT_LOGI("enter");

    if (isFromMsg) {
        pendingTimeoutMessage_ = false;
    }

    if (orderedEventQueue_.size() == 0) {
        return;
    }

    int64_t nowSysTime = SystemTime::GetNowSysTime();
    std::shared_ptr<OrderedEventRecord> sp = orderedEventQueue_.front();

    if (isFromMsg) {
        int64_t timeoutTime = sp->receiverTime + TIMEOUT;
        if (timeoutTime > nowSysTime) {
            SetTimeout(timeoutTime);
            return;
        }
    }

    // The processing of current receiver has timeout
    sp->receiverTime = nowSysTime;

    if (sp->nextReceiver > 0) {
        std::shared_ptr<EventSubscriberRecord> subscriberRecord = sp->receivers[sp->nextReceiver - 1];
        EVENT_LOGW("Timeout: When %{public}s (pid = %{public}d, uid = %{public}d) process common event %{public}s",
            subscriberRecord->bundleName.c_str(),
            subscriberRecord->pid,
            subscriberRecord->uid,
            sp->commonEventData->GetWant().GetAction().c_str());

        sp->deliveryState[sp->nextReceiver - 1] = OrderedEventRecord::TIMEOUT;
    }

    // Forced to finish the current receiver to process the next receiver
    int code = sp->commonEventData->GetCode();
    const std::string &strRef = sp->commonEventData->GetData();
    bool abort = sp->resultAbort;
    FinishReceiver(sp, code, strRef, abort);

    ScheduleOrderedCommonEvent();

    return;
}

bool CommonEventControlManager::FinishReceiver(std::shared_ptr<OrderedEventRecord> recordPtr, const int &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (recordPtr == nullptr) {
        EVENT_LOGE("recordPtr is null");
        return false;
    }

    EVENT_LOGI("enter recordPtr->state=%{public}d", recordPtr->state);

    int state = recordPtr->state;
    recordPtr->state = OrderedEventRecord::IDLE;
    recordPtr->curReceiver = nullptr;
    recordPtr->commonEventData->SetCode(code);
    recordPtr->commonEventData->SetData(receiverData);
    recordPtr->resultAbort = abortEvent;

    return state == OrderedEventRecord::RECEIVED;
}

bool CommonEventControlManager::FinishReceiverAction(std::shared_ptr<OrderedEventRecord> recordPtr, const int &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (recordPtr == nullptr) {
        EVENT_LOGE("recordPtr is nullptr");
        return false;
    }

    bool doNext = false;
    doNext = FinishReceiver(recordPtr, code, receiverData, abortEvent);
    if (doNext) {
        ProcessNextOrderedEvent(false);
    }

    return true;
}

void CommonEventControlManager::CheckSubscriberRequiredPermission(const std::string &subscriberRequiredPermission,
    const CommonEventRecord &eventRecord, const EventSubscriberRecord &subscriberRecord, bool &skip)
{
    bool ret = false;

    if (!subscriberRequiredPermission.empty()) {
        ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
            eventRecord.bundleName, subscriberRequiredPermission);
        if (!ret) {
            EVENT_LOGW("No permission to send common event %{public}s "
                       "from %{public}s (pid = %{public}d, uid = %{public}d) "
                       "to %{public}s (pid = %{public}d, uid = %{public}d) "
                       "due to registered subscriber require %{public}s",
                eventRecord.commonEventData->GetWant().GetAction().c_str(),
                eventRecord.bundleName.c_str(),
                eventRecord.pid,
                eventRecord.uid,
                subscriberRecord.bundleName.c_str(),
                subscriberRecord.pid,
                subscriberRecord.uid,
                subscriberRequiredPermission.c_str());
            skip = true;
        }
    }
}

void CommonEventControlManager::CheckPublisherRequiredPermissions(
    const std::vector<std::string> &publisherRequiredPermissions, const EventSubscriberRecord &subscriberRecord,
    const CommonEventRecord &eventRecord, bool &skip)
{
    bool ret = false;

    if ((!skip) && (publisherRequiredPermissions.size() > 0)) {
        for (auto publisherRequiredPermission : publisherRequiredPermissions) {
            ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
                subscriberRecord.bundleName, publisherRequiredPermission);
            if (!ret) {
                skip = true;
                EVENT_LOGW("No permission to receive common event %{public}s "
                           "to %{public}s (pid = %{public}d, uid = %{public}d) "
                           "due to publisher %{public}s (pid = %{public}d, uid = %{public}d) requires %{public}s",
                    eventRecord.commonEventData->GetWant().GetAction().c_str(),
                    subscriberRecord.bundleName.c_str(),
                    subscriberRecord.pid,
                    subscriberRecord.uid,
                    eventRecord.bundleName.c_str(),
                    eventRecord.pid,
                    eventRecord.uid,
                    publisherRequiredPermission.c_str());
                break;
            }
        }
    }
}

int CommonEventControlManager::CheckPermission(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    bool skip = false;
    bool ret = false;
    std::string subscriberRequiredPermission = subscriberRecord.eventSubscriberInfo->GetPermission();
    std::vector<std::string> publisherRequiredPermissions = eventRecord.publishInfo->GetSubscriberPermissions();

    Permission per = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(
        eventRecord.commonEventData->GetWant().GetAction());

    ret = CheckSubcriberPermission(per, subscriberRecord);
    EVENT_LOGI("after CheckSubcriberPermission and ret = %{public}d", ret);
    if (!ret) {
        return OrderedEventRecord::SKIPPED;
    }

    CheckSubscriberRequiredPermission(subscriberRequiredPermission, eventRecord, subscriberRecord, skip);

    CheckPublisherRequiredPermissions(publisherRequiredPermissions, subscriberRecord, eventRecord, skip);

    if (skip) {
        return OrderedEventRecord::SKIPPED;
    }

    return OrderedEventRecord::DELIVERED;
}

bool CommonEventControlManager::CheckSubcriberPermission(
    const Permission &permission, const EventSubscriberRecord &subscriberRecord)
{
    EVENT_LOGI("enter");
    EVENT_LOGI("size = %{public}zu", permission.names.size());
    bool ret = false;

    if (permission.names.size() < 1) {
        return true;
    }
    if (permission.names.size() == 1) {
        ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
            subscriberRecord.bundleName, permission.names[0]);
    } else {
        if (permission.state == Permission::AND) {
            for (auto vec : permission.names) {
                ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
                    subscriberRecord.bundleName, vec);
                if (!ret) {
                    break;
                }
            }
        } else if (permission.state == Permission::OR) {
            for (auto vec : permission.names) {
                ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
                    subscriberRecord.bundleName, vec);
                if (ret) {
                    break;
                }
            }
        }
    }

    return ret;
}

void CommonEventControlManager::SetTime(int recIdx, std::shared_ptr<OrderedEventRecord> &sp, bool timeoutMessage)
{
    EVENT_LOGI("enter");

    sp->receiverTime = SystemTime::GetNowSysTime();

    if (recIdx == 0) {
        sp->dispatchTime = sp->receiverTime;
    }

    if (!timeoutMessage) {
        int64_t timeoutTime = sp->receiverTime + TIMEOUT;
        SetTimeout(timeoutTime);
    }

    return;
}

void CommonEventControlManager::GetUnorderedEventRecords(
    const std::string &event, std::vector<std::shared_ptr<OrderedEventRecord>> &records)
{
    EVENT_LOGI("enter");
    if (event.empty()) {
        records = unorderedEventQueue_;
    } else {
        for (auto vec : unorderedEventQueue_) {
            if (vec->commonEventData->GetWant().GetAction() == event) {
                records.emplace_back(vec);
            }
        }
    }
}

void CommonEventControlManager::GetOrderedEventRecords(
    const std::string &event, std::vector<std::shared_ptr<OrderedEventRecord>> &records)
{
    EVENT_LOGI("enter");
    if (event.empty()) {
        records = orderedEventQueue_;
    } else {
        for (auto vec : orderedEventQueue_) {
            if (vec->commonEventData->GetWant().GetAction() == event) {
                records.emplace_back(vec);
            }
        }
    }
}

void CommonEventControlManager::GetHistoryEventRecords(
    const std::string &event, std::vector<History_event_record> &records)
{
    EVENT_LOGI("enter");
    if (event.empty()) {
        records = historyEventRecords_;
    } else {
        for (auto vec : historyEventRecords_) {
            if (vec.want.GetAction() == event) {
                records.emplace_back(vec);
            }
        }
    }
}

void CommonEventControlManager::DumpStateByCommonEventRecord(
    const std::shared_ptr<OrderedEventRecord> &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    char systime[LENGTH];
    strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &record->recordTime);

    std::string recordTime = "\tTime: " + std::string(systime) + "\n";
    std::string pid = "\tPID: " + std::to_string(record->pid) + "\n";
    std::string uid = "\tUID: " + std::to_string(record->uid) + "\n";
    std::string bundleName = "\tBundleName: " + record->bundleName + "\n";

    std::string permission = "\tRequiredPermission: ";
    std::string separator;
    int permissionNum = 0;
    for (auto permissionVec : record->publishInfo->GetSubscriberPermissions()) {
        if (permissionNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        permission = permission + separator + permissionVec;
        permissionNum++;
    }
    permission = permission + "\n";

    std::string isSticky;
    if (record->publishInfo->IsSticky()) {
        isSticky = "\tIsSticky: true\n";
    } else {
        isSticky = "\tIsSticky: false\n";
    }

    std::string isOrdered;
    if (record->publishInfo->IsOrdered()) {
        isOrdered = "\tIsOrdered: true\n";
    } else {
        isOrdered = "\tIsOrdered: false\n";
    }
    std::string isSystemEvent = record->isSystemEvent ? "true" : "false";
    isSystemEvent = "\tIsSystemEvent: " + isSystemEvent + "\n";

    std::string action = "\t\tAction: " + record->commonEventData->GetWant().GetAction() + "\n";

    std::string entities = "\t\tEntity: ";
    int entityNum = 0;
    for (auto entitiesVec : record->commonEventData->GetWant().GetEntities()) {
        if (entityNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        entities = entities + separator + entitiesVec;
        entityNum++;
    }
    entities = entities + "\n";

    std::string scheme = "\t\tScheme: " + record->commonEventData->GetWant().GetScheme() + "\n";
    std::string uri = "\t\tUri: " + record->commonEventData->GetWant().GetUriString() + "\n";
    std::string flags = "\t\tFlags: " + std::to_string(record->commonEventData->GetWant().GetFlags()) + "\n";
    std::string type = "\t\tType: " + record->commonEventData->GetWant().GetType() + "\n";
    std::string bundle = "\t\tBundleName: " + record->commonEventData->GetWant().GetBundle() + "\n";
    std::string ability = "\t\tAbilityName: " + record->commonEventData->GetWant().GetElement().GetAbilityName() + "\n";
    std::string deviced = "\t\tDevicedID: " + record->commonEventData->GetWant().GetElement().GetDeviceID() + "\n";

    std::string want = "\tWant:\n" + action + entities + scheme + uri + flags + type + bundle + ability + deviced;
    std::string code = "\tCode: " + std::to_string(record->commonEventData->GetCode()) + "\n";
    std::string data = "\tData: " + record->commonEventData->GetData() + "\n";

    std::string lastSubscriber;
    if (record->resultTo) {
        lastSubscriber = "\tHasLastSubscriber: true\n";
    } else {
        lastSubscriber = "\tHasLastSubscriber: false\n";
    }

    std::string state;
    switch (record->state) {
        case OrderedEventRecord::IDLE:
            state = "\tEventState: IDLE\n";
            break;
        case OrderedEventRecord::RECEIVEING:
            state = "\tEventState: RECEIVEING\n";
            break;
        case OrderedEventRecord::RECEIVED:
            state = "\tEventState: RECEIVED\n";
            break;
    }

    std::string dispatchTime = "\tDispatchTime: " + std::to_string(record->dispatchTime) + "\n";
    std::string receiverTime = "\tReceiverTime: " + std::to_string(record->receiverTime) + "\n";
    std::string resultAbort = record->resultAbort ? "true" : "false";
    resultAbort = "\tResultAbort: " + resultAbort + "\n";

    dumpInfo = recordTime + pid + uid + bundleName + permission + isSticky + isOrdered + isSystemEvent + want + code +
               data + lastSubscriber + state + receiverTime + dispatchTime + resultAbort;
}

void CommonEventControlManager::DumpHistoryStateByCommonEventRecord(
    const History_event_record &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    char systime[LENGTH];
    strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &record.recordTime);

    std::string recordTime = "\tTime: " + std::string(systime) + "\n";
    std::string pid = "\tPID: " + std::to_string(record.pid) + "\n";
    std::string uid = "\tUID: " + std::to_string(record.uid) + "\n";
    std::string bundleName = "\tBundleName: " + record.bundleName + "\n";

    std::string permission = "\tRequiredPermission: ";
    std::string separator;
    int permissionNum = 0;
    for (auto permissionVec : record.subscriberPermissions) {
        if (permissionNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        permission = permission + separator + permissionVec;
        permissionNum++;
    }
    permission = permission + "\n";

    std::string isSticky;
    if (record.sticky) {
        isSticky = "\tIsSticky: true\n";
    } else {
        isSticky = "\tIsSticky: false\n";
    }

    std::string isOrdered;
    if (record.ordered) {
        isOrdered = "\tIsOrdered: true\n";
    } else {
        isOrdered = "\tIsOrdered: false\n";
    }
    std::string isSystemEvent = record.isSystemEvent ? "true" : "false";
    isSystemEvent = "\tIsSystemEvent: " + isSystemEvent + "\n";

    std::string action = "\t\tAction: " + record.want.GetAction() + "\n";

    std::string entities = "\t\tEntity: ";
    int entityNum = 0;
    for (auto entitiesVec : record.want.GetEntities()) {
        if (entityNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        entities = entities + separator + entitiesVec;
        entityNum++;
    }
    entities = entities + "\n";

    std::string scheme = "\t\tScheme: " + record.want.GetScheme() + "\n";
    std::string uri = "\t\tUri: " + record.want.GetUriString() + "\n";
    std::string flags = "\t\tFlags: " + std::to_string(record.want.GetFlags()) + "\n";
    std::string type = "\t\tType: " + record.want.GetType() + "\n";
    std::string bundle = "\t\tBundleName: " + record.want.GetBundle() + "\n";
    std::string ability = "\t\tAbilityName: " + record.want.GetElement().GetAbilityName() + "\n";
    std::string deviced = "\t\tDevicedID: " + record.want.GetElement().GetDeviceID() + "\n";

    std::string want = "\tWant:\n" + action + entities + scheme + uri + flags + type + bundle + ability + deviced;
    std::string code = "\tCode: " + std::to_string(record.code) + "\n";
    std::string data = "\tData: " + record.data + "\n";

    std::string lastSubscriber;
    if (record.resultTo) {
        lastSubscriber = "\tHasLastSubscriber: true\n";
    } else {
        lastSubscriber = "\tHasLastSubscriber: false\n";
    }

    std::string state;
    switch (record.state) {
        case OrderedEventRecord::IDLE:
            state = "\tEventState: IDLE\n";
            break;
        case OrderedEventRecord::RECEIVEING:
            state = "\tEventState: RECEIVEING\n";
            break;
        case OrderedEventRecord::RECEIVED:
            state = "\tEventState: RECEIVED\n";
            break;
    }

    std::string dispatchTime = "\tDispatchTime: " + std::to_string(record.dispatchTime) + "\n";
    std::string receiverTime = "\tReceiverTime: " + std::to_string(record.receiverTime) + "\n";
    std::string resultAbort = record.resultAbort ? "true" : "false";
    resultAbort = "\tResultAbort: " + resultAbort + "\n";

    dumpInfo = recordTime + pid + uid + bundleName + permission + isSticky + isOrdered + isSystemEvent + want + code +
               data + lastSubscriber + state + receiverTime + dispatchTime + resultAbort;
}

void CommonEventControlManager::DumpStateBySubscriberRecord(
    const std::shared_ptr<OrderedEventRecord> &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    if (record->receivers.size() == 0) {
        dumpInfo = "\tSubscribers:\tNo information";
        return;
    }

    int num = 0;
    for (auto receiver : record->receivers) {
        num++;

        std::string title = std::to_string(num);
        if (num == 1) {
            title = "\tSubscribers:\tTotal " + std::to_string(record->receivers.size()) + " subscribers\n\tNO " +
                    title + "\n";
        } else {
            title = "\tNO " + title + "\n";
        }

        std::string dumpInfoBySubscriber;
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->DumpDetailed(
            title, receiver, "\t\t", dumpInfoBySubscriber);

        std::string deliveryState;
        switch (record->deliveryState[num - 1]) {
            case OrderedEventRecord::PENDING:
                deliveryState = "\t\tEventState: PENDING\n";
                break;
            case OrderedEventRecord::DELIVERED:
                deliveryState = "\t\tEventState: DELIVERED\n";
                break;
            case OrderedEventRecord::SKIPPED:
                deliveryState = "\t\tEventState: SKIPPED\n";
                break;
            case OrderedEventRecord::TIMEOUT:
                deliveryState = "\t\tEventState: TIMEOUT\n";
                break;
        }
        dumpInfo = dumpInfo + dumpInfoBySubscriber + deliveryState;
    }
}

void CommonEventControlManager::DumpHistoryStateBySubscriberRecord(
    const History_event_record &record, std::string &dumpInfo)
{
    EVENT_LOGI("enter");

    if (record.receivers.size() == 0) {
        dumpInfo = "\tSubscribers:\tNo information";
        return;
    }

    int num = 0;
    for (auto receiver : record.receivers) {
        num++;

        std::string title = std::to_string(num);
        if (num == 1) {
            title = "\tSubscribers:\tTotal " + std::to_string(record.receivers.size()) + " subscribers\n\tNO " + title +
                    "\n";
        } else {
            title = "\tNO " + title + "\n";
        }

        char systime[LENGTH];
        strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &receiver.recordTime);
        std::string format = "\t\t";
        std::string recordTime = format + "Time: " + std::string(systime) + "\n";

        std::string bundleName = format + "BundleName: " + receiver.bundleName + "\n";
        std::string priority = format + "Priority: " + std::to_string(receiver.priority) + "\n";
        std::string permission = format + "Permission: " + receiver.permission + "\n";
        std::string deviceId = format + "DevicedID: " + receiver.deviceId + "\n";

        std::string isFreeze = receiver.isFreeze ? "true" : "false";
        isFreeze = format + "IsFreeze: " + isFreeze + "\n";

        std::string freezeTime;
        if (receiver.freezeTime == 0) {
            freezeTime = format + "FreezeTime:  -\n";
        } else {
            freezeTime = format + "FreezeTime: " + std::to_string(receiver.freezeTime) + "\n";
        }

        std::string deliveryState;
        switch (record.deliveryState[num - 1]) {
            case OrderedEventRecord::PENDING:
                deliveryState = "\t\tEventState: PENDING\n";
                break;
            case OrderedEventRecord::DELIVERED:
                deliveryState = "\t\tEventState: DELIVERED\n";
                break;
            case OrderedEventRecord::SKIPPED:
                deliveryState = "\t\tEventState: SKIPPED\n";
                break;
            case OrderedEventRecord::TIMEOUT:
                deliveryState = "\t\tEventState: TIMEOUT\n";
                break;
        }
        dumpInfo = dumpInfo + title + recordTime + bundleName + priority + permission + deviceId + isFreeze +
                   freezeTime + deliveryState;
    }
}

void CommonEventControlManager::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    std::vector<std::shared_ptr<OrderedEventRecord>> records;
    std::vector<std::shared_ptr<OrderedEventRecord>> unorderedRecords;
    std::vector<std::shared_ptr<OrderedEventRecord>> orderedRecords;
    std::lock_guard<std::mutex> orderedLock(orderedMutex_);
    std::lock_guard<std::mutex> unorderedLock(unorderedMutex_);
    GetUnorderedEventRecords(event, unorderedRecords);
    GetOrderedEventRecords(event, orderedRecords);
    records.insert(records.end(), unorderedRecords.begin(), unorderedRecords.end());
    records.insert(records.end(), orderedRecords.begin(), orderedRecords.end());

    if (records.size() == 0) {
        state.emplace_back("Pending Events:\tNo information");
        return;
    }

    int num = 0;
    for (auto record : records) {
        num++;

        std::string no = std::to_string(num);
        if (num == 1) {
            no = "Pending Events:\tTotal " + std::to_string(records.size()) + " information\nNO " + no + "\n";
        } else {
            no = "NO " + no + "\n";
        }
        std::string commonEventRecord;
        DumpStateByCommonEventRecord(record, commonEventRecord);
        std::string subscriberRecord;
        DumpStateBySubscriberRecord(record, subscriberRecord);
        std::string stateInfo = no + commonEventRecord + subscriberRecord;
        state.emplace_back(stateInfo);
    }
}

void CommonEventControlManager::DumpHistoryState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    std::vector<History_event_record> records;
    std::lock_guard<std::mutex> lock(historyMutex_);
    GetHistoryEventRecords(event, records);

    if (records.size() == 0) {
        state.emplace_back("History Events:\tNo information");
        return;
    }

    int num = 0;
    for (auto record : records) {
        num++;

        std::string no = std::to_string(num);
        if (num == 1) {
            no = "History Events:\tTotal " + std::to_string(records.size()) + " information\nNO " + no + "\n";
        } else {
            no = "NO " + no + "\n";
        }
        std::string commonEventRecord;
        DumpHistoryStateByCommonEventRecord(record, commonEventRecord);
        std::string subscriberRecord;
        DumpHistoryStateBySubscriberRecord(record, subscriberRecord);
        std::string stateInfo = no + commonEventRecord + subscriberRecord;
        state.emplace_back(stateInfo);
    }
}

}  // namespace EventFwk
}  // namespace OHOS