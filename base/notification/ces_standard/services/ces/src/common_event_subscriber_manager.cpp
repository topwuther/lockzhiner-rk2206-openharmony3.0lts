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

#include <time.h>
#include "common_event_subscriber_manager.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {

#define FREEZE_TIMEOUT_SECOND 120
const int LENGTH = 80;

CommonEventSubscriberManager::CommonEventSubscriberManager()
    : death_(sptr<IRemoteObject::DeathRecipient>(new SubscriberDeathRecipient())),
      FREEZE_EVENT_TIMEOUT(FREEZE_TIMEOUT_SECOND)
{
    EVENT_LOGI("CommonEventSubscriberManager instance created");
}

CommonEventSubscriberManager::~CommonEventSubscriberManager()
{
    EVENT_LOGI("CommonEventSubscriberManager instance destoryed");
}

int CommonEventSubscriberManager::InsertSubscriber(const SubscribeInfoPtr &eventSubscriberInfo,
    const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
    const std::string &bundleName)
{
    EVENT_LOGI("enter");

    if (eventSubscriberInfo == nullptr) {
        EVENT_LOGE("eventSubscriberInfo is null");
        return ERR_INVALID_VALUE;
    }

    if (commonEventListener == nullptr) {
        EVENT_LOGE("commonEventListener is null");
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> events = eventSubscriberInfo->GetMatchingSkills().GetEvents();
    if (events.size() <= 0) {
        EVENT_LOGE("No subscribed events");
        return ERR_INVALID_VALUE;
    }

    auto record = std::make_shared<EventSubscriberRecord>();
    if (record == nullptr) {
        EVENT_LOGE("Failed to create EventSubscriberRecord");
        return ERR_INVALID_VALUE;
    }

    record->eventSubscriberInfo = eventSubscriberInfo;
    record->commonEventListener = commonEventListener;
    record->recordTime = recordTime;
    record->pid = pid;
    record->uid = uid;
    record->bundleName = bundleName;

    if (death_ != nullptr) {
        commonEventListener->AddDeathRecipient(death_);
    }

    return InsertSubscriberRecordLocked(events, record);
}

int CommonEventSubscriberManager::RemoveSubscriber(const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    if (commonEventListener == nullptr) {
        EVENT_LOGE("commonEventListener is null");
        return ERR_INVALID_VALUE;
    }

    if (death_ != nullptr) {
        commonEventListener->RemoveDeathRecipient(death_);
    }

    return RemoveSubscriberRecordLocked(commonEventListener);
}

std::vector<std::shared_ptr<EventSubscriberRecord>> CommonEventSubscriberManager::GetSubscriberRecords(const Want &want)
{
    EVENT_LOGI("enter");

    auto records = std::vector<SubscriberRecordPtr>();

    GetSubscriberRecordsByWantLocked(want, records);

    return records;
}

void CommonEventSubscriberManager::DumpDetailed(
    const std::string &title, const SubscriberRecordPtr &record, const std::string format, std::string &dumpInfo)
{

    char systime[LENGTH];
    strftime(systime, sizeof(char) * LENGTH, "%Y%m%d %I:%M %p", &record->recordTime);

    std::string recordTime = format + "Time: " + std::string(systime) + "\n";
    std::string pid = format + "PID: " + std::to_string(record->pid) + "\n";
    std::string uid = format + "UID: " + std::to_string(record->uid) + "\n";
    std::string bundleName = format + "BundleName: " + record->bundleName + "\n";
    std::string priority = format + "Priority: " + std::to_string(record->eventSubscriberInfo->GetPriority()) + "\n";
    std::string permission = format + "Permission: " + record->eventSubscriberInfo->GetPermission() + "\n";
    std::string deviceId = format + "DevicedID: " + record->eventSubscriberInfo->GetDeviceId() + "\n";

    std::string events = format + "\tEvent: ";
    std::string separator;
    for (int eventNum = 0; eventNum < record->eventSubscriberInfo->GetMatchingSkills().CountEvent(); ++eventNum) {
        if (eventNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        events = events + separator + record->eventSubscriberInfo->GetMatchingSkills().GetEvent(eventNum);
    }
    events = events + "\n";

    std::string entities = format + "\tEntity: ";
    for (int entityNum = 0; entityNum < record->eventSubscriberInfo->GetMatchingSkills().CountEntities(); ++entityNum) {
        if (entityNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        entities = entities + separator + record->eventSubscriberInfo->GetMatchingSkills().GetEntity(entityNum);
    }
    entities = entities + "\n";

    std::string scheme = format + "\tScheme: ";
    for (int schemeNum = 0; schemeNum < record->eventSubscriberInfo->GetMatchingSkills().CountSchemes(); ++schemeNum) {
        if (schemeNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        scheme = scheme + separator + record->eventSubscriberInfo->GetMatchingSkills().GetScheme(schemeNum);
    }
    scheme = scheme + "\n";

    std::string matchingSkills = format + "MatchingSkills:\n" + events + entities + scheme;

    std::string isFreeze = record->isFreeze ? "true" : "false";
    isFreeze = format + "IsFreeze: " + isFreeze + "\n";

    std::string freezeTime;
    if (record->freezeTime == 0) {
        freezeTime = format + "FreezeTime:  -\n";
    } else {
        freezeTime = format + "FreezeTime: " + std::to_string(record->freezeTime) + "\n";
    }

    dumpInfo = title + recordTime + pid + uid + bundleName + priority + permission + deviceId + matchingSkills +
               isFreeze + freezeTime;
}

void CommonEventSubscriberManager::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    std::vector<SubscriberRecordPtr> records;

    std::lock_guard<std::mutex> lock(mutex_);
    GetSubscriberRecordsByEvent(event, records);

    if (records.size() == 0) {
        state.emplace_back("Subscribers:\tNo information");
        return;
    }

    int num = 0;
    for (auto record : records) {
        num++;
        std::string title = std::to_string(num);
        if (num == 1) {
            title = "Subscribers:\tTotal " + std::to_string(records.size()) + " subscribers\nNO " + title + "\n";
        } else {
            title = "NO " + title + "\n";
        }
        std::string dumpInfo;
        DumpDetailed(title, record, "\t", dumpInfo);
        state.emplace_back(dumpInfo);
    }
}

int CommonEventSubscriberManager::InsertSubscriberRecordLocked(
    const std::vector<std::string> &events, const SubscriberRecordPtr &record)
{
    if (events.size() == 0) {
        EVENT_LOGE("No subscribed events");
        return ERR_INVALID_VALUE;
    }

    if (record == nullptr) {
        EVENT_LOGE("record is null");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (auto event : events) {
        auto infoItem = eventSubscribers_.find(event);
        if (infoItem != eventSubscribers_.end()) {
            infoItem->second.insert(record);
        } else {
            std::multiset<SubscriberRecordPtr> EventSubscribersPtr;
            EventSubscribersPtr.insert(record);
            eventSubscribers_[event] = EventSubscribersPtr;
        }
    }

    subscribers_.emplace_back(record);

    return ERR_OK;
}

int CommonEventSubscriberManager::RemoveSubscriberRecordLocked(const sptr<IRemoteObject> &commonEventListener)
{
    if (commonEventListener == nullptr) {
        EVENT_LOGE("commonEventListener is null");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> events;

    for (auto it = subscribers_.begin(); it != subscribers_.end(); ++it) {
        if (commonEventListener == (*it)->commonEventListener) {
            RemoveFrozenEventsBySubscriber((*it));
            (*it)->commonEventListener = nullptr;
            events = (*it)->eventSubscriberInfo->GetMatchingSkills().GetEvents();
            subscribers_.erase(it);
            break;
        }
    }

    for (auto event : events) {
        for (auto it = eventSubscribers_[event].begin(); it != eventSubscribers_[event].end(); ++it) {
            if ((commonEventListener == (*it)->commonEventListener) || ((*it)->commonEventListener == nullptr)) {
                (*it)->commonEventListener = nullptr;
                eventSubscribers_[event].erase(it);
                break;
            }
        }
        if (eventSubscribers_[event].size() == 0) {
            eventSubscribers_.erase(event);
        }
    }

    return ERR_OK;
}

void CommonEventSubscriberManager::GetSubscriberRecordsByWantLocked(
    const Want &want, std::vector<SubscriberRecordPtr> &records)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (eventSubscribers_.size() <= 0) {
        return;
    }

    auto recordsItem = eventSubscribers_.find(want.GetAction());
    if (recordsItem == eventSubscribers_.end()) {
        return;
    }

    std::multiset<SubscriberRecordPtr> subscriberRecords = recordsItem->second;
    for (auto it = subscriberRecords.begin(); it != subscriberRecords.end(); it++) {
        if ((*it)->eventSubscriberInfo->GetMatchingSkills().Match(want)) {
            records.emplace_back(*it);
        }
    }
}

void CommonEventSubscriberManager::GetSubscriberRecordsByEvent(
    const std::string &event, std::vector<SubscriberRecordPtr> &records)
{

    if (event.empty()) {
        records = subscribers_;
    } else {
        auto infoItem = eventSubscribers_.find(event);
        if (infoItem != eventSubscribers_.end()) {
            for (auto recordPtr : infoItem->second) {
                records.emplace_back(recordPtr);
            }
        }
    }
}

void CommonEventSubscriberManager::UpdateFreezeInfo(
    const uid_t &uid, const bool &freezeState, const int64_t &freezeTime)
{
    EVENT_LOGI("enter");

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> events;
    for (auto recordPtr : subscribers_) {
        if (recordPtr->uid == uid) {
            if (freezeState) {
                recordPtr->freezeTime = freezeTime;
            } else {
                recordPtr->freezeTime = 0;
            }
            recordPtr->isFreeze = freezeState;
            EVENT_LOGI("recordPtr->uid: %{public}d", recordPtr->uid);
            EVENT_LOGI("recordPtr->isFreeze: %{public}d", recordPtr->isFreeze);
        }
    }
}

void CommonEventSubscriberManager::InsertFrozenEvents(
    const SubscriberRecordPtr &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    if (subscriberRecord == nullptr) {
        EVENT_LOGE("subscriberRecord is null");
        return;
    }

    auto record = std::make_shared<CommonEventRecord>(eventRecord);
    std::lock_guard<std::mutex> lock(mutex_);
    auto frozenRecordsItem = frozenEvents_.find(subscriberRecord->uid);
    if (frozenRecordsItem != frozenEvents_.end()) {
        auto eventRecordsItem = frozenRecordsItem->second.find(subscriberRecord);
        if (eventRecordsItem != frozenRecordsItem->second.end()) {
            eventRecordsItem->second.emplace_back(record);
            time_t backRecordTime = mktime(&eventRecordsItem->second.back()->recordTime);
            time_t frontRecordTime = mktime(&eventRecordsItem->second.front()->recordTime);
            EVENT_LOGD("backRecordTime: %{public}ld", backRecordTime);
            EVENT_LOGD("frontRecordTime: %{public}ld", frontRecordTime);
            time_t timeDiff = backRecordTime - frontRecordTime;
            EVENT_LOGD("timeDiff: %{public}ld", timeDiff);
            if (timeDiff > FREEZE_EVENT_TIMEOUT) {
                eventRecordsItem->second.erase(eventRecordsItem->second.begin());
            }
        } else {
            std::vector<EventRecordPtr> EventRecords;
            EventRecords.emplace_back(record);
            frozenRecordsItem->second[subscriberRecord] = EventRecords;
        }
    } else {
        std::map<SubscriberRecordPtr, std::vector<EventRecordPtr>> frozenRecords;
        std::vector<EventRecordPtr> EventRecords;
        EventRecords.emplace_back(record);
        frozenRecords[subscriberRecord] = EventRecords;
        frozenEvents_[subscriberRecord->uid] = frozenRecords;
    }
}

std::map<SubscriberRecordPtr, std::vector<EventRecordPtr>> CommonEventSubscriberManager::GetFrozenEvents(
    const uid_t &uid)
{
    EVENT_LOGI("enter");

    std::map<SubscriberRecordPtr, std::vector<EventRecordPtr>> frozenEvents;
    std::lock_guard<std::mutex> lock(mutex_);
    auto infoItem = frozenEvents_.find(uid);
    if (infoItem != frozenEvents_.end()) {
        frozenEvents = infoItem->second;
    }

    RemoveFrozenEvents(uid);

    return frozenEvents;
}

void CommonEventSubscriberManager::RemoveFrozenEvents(const uid_t &uid)
{
    EVENT_LOGI("enter");
    auto infoItem = frozenEvents_.find(uid);
    if (infoItem != frozenEvents_.end()) {
        frozenEvents_.erase(uid);
    }
}

void CommonEventSubscriberManager::RemoveFrozenEventsBySubscriber(const SubscriberRecordPtr &subscriberRecord)
{
    EVENT_LOGI("enter");

    auto frozenRecordsItem = frozenEvents_.find(subscriberRecord->uid);
    if (frozenRecordsItem != frozenEvents_.end()) {
        auto eventRecordsItems = frozenRecordsItem->second.find(subscriberRecord);
        if (eventRecordsItems != frozenRecordsItem->second.end()) {
            frozenRecordsItem->second.erase(subscriberRecord);
        }
    }
}
}  // namespace EventFwk
}  // namespace OHOS