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

#include "common_event_sticky_manager.h"
#include "errors.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
int CommonEventStickyManager::FindStickyEvents(
    const SubscribeInfoPtr &subscriberInfo, std::vector<CommonEventRecordPtr> &commonEventRecords)
{
    EVENT_LOGI("enter");

    if (subscriberInfo == nullptr) {
        EVENT_LOGE("subscriberInfo is null");
        return ERR_INVALID_VALUE;
    }

    auto events = subscriberInfo->GetMatchingSkills().GetEvents();
    if (events.size() == 0) {
        EVENT_LOGW("No subscribed events");
        return ERR_INVALID_VALUE;
    }

    FindStickyEventsLocked(events, commonEventRecords);

    return ERR_OK;
}

bool CommonEventStickyManager::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGI("enter");

    if (event.empty()) {
        EVENT_LOGE("Invalid event name");
        return false;
    }

    return GetStickyCommonEventLocked(event, eventData);
}

int CommonEventStickyManager::UpdateStickyEvent(const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    auto commonEventRecordPtr = std::make_shared<CommonEventRecord>(eventRecord);
    if (commonEventRecordPtr == nullptr) {
        EVENT_LOGE("Failed to create CommonEventRecord");
        return ERR_INVALID_VALUE;
    }

    std::string event = commonEventRecordPtr->commonEventData->GetWant().GetAction();

    return UpdateStickyEventLocked(event, commonEventRecordPtr);
}

void CommonEventStickyManager::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    std::vector<CommonEventRecordPtr> records;

    std::lock_guard<std::mutex> lock(mutex_);

    GetStickyCommonEventRecords(event, records);

    if (records.size() == 0) {
        state.emplace_back("Sticky Events:\tNo information");
        return;
    }

    int num = 0;
    for (auto record : records) {
        num++;

        std::string no = std::to_string(num);
        if (num == 1) {
            no = "Sticky Events:\tTotal " + std::to_string(records.size()) + " information\nNO " + no + "\n";
        } else {
            no = "NO " + no + "\n";
        }

        char systime[80];
        strftime(systime, sizeof(char) * 80, "%Y%m%d %I:%M %p", &record->recordTime);

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
        std::string ability =
            "\t\tAbilityName: " + record->commonEventData->GetWant().GetElement().GetAbilityName() + "\n";
        std::string deviced = "\t\tDevicedID: " + record->commonEventData->GetWant().GetElement().GetDeviceID() + "\n";

        std::string want = "\tWant:\n" + action + entities + scheme + uri + flags + type + bundle + ability + deviced;
        std::string code = "\tCode: " + std::to_string(record->commonEventData->GetCode()) + "\n";
        std::string data = "\tData: " + record->commonEventData->GetData() + "\n";

        std::string dumpInfo = no + recordTime + pid + uid + bundleName + permission + isSticky + isOrdered +
                               isSystemEvent + want + code + data;

        state.emplace_back(dumpInfo);
    }
}

void CommonEventStickyManager::FindStickyEventsLocked(
    const std::vector<std::string> &events, std::vector<CommonEventRecordPtr> &commonEventRecords)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto event : events) {
        auto it = commonEventRecords_.find(event);
        if (it != commonEventRecords_.end()) {
            commonEventRecords.emplace_back(it->second);
        }
    }
}

bool CommonEventStickyManager::GetStickyCommonEventLocked(const std::string &event, CommonEventData &eventData)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = commonEventRecords_.find(event);
    if (it != commonEventRecords_.end()) {
        eventData = *(it->second->commonEventData);
        return true;
    }

    return false;
}

int CommonEventStickyManager::UpdateStickyEventLocked(const std::string &event, const CommonEventRecordPtr &record)
{
    if (event.empty()) {
        EVENT_LOGE("Invalid event name");
        return ERR_INVALID_VALUE;
    }

    if (record == nullptr) {
        EVENT_LOGE("Invalid common event record");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    commonEventRecords_[event] = record;

    return ERR_OK;
}

void CommonEventStickyManager::GetStickyCommonEventRecords(
    const std::string &event, std::vector<CommonEventRecordPtr> &records)
{
    if (event.empty()) {
        for (auto record : commonEventRecords_) {
            records.emplace_back(record.second);
        }
    } else {
        auto recordItem = commonEventRecords_.find(event);
        if (recordItem != commonEventRecords_.end()) {
            records.emplace_back(recordItem->second);
        }
    }
}
}  // namespace EventFwk
}  // namespace OHOS