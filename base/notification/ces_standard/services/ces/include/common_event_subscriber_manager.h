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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUBSCRIBER_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUBSCRIBER_MANAGER_H

#include <string>
#include <map>
#include <set>
#include <vector>

#include "singleton.h"
#include "refbase.h"
#include "iremote_object.h"
#include "want.h"
#include "common_event_record.h"
#include "common_event_subscribe_info.h"
#include "subscriber_death_recipient.h"

namespace OHOS {
namespace EventFwk {
struct EventSubscriberRecord {
    std::shared_ptr<CommonEventSubscribeInfo> eventSubscriberInfo;
    sptr<IRemoteObject> commonEventListener;
    struct tm recordTime;
    pid_t pid;
    uid_t uid;
    std::string bundleName;
    bool isFreeze;
    int64_t freezeTime;

    EventSubscriberRecord()
        : eventSubscriberInfo(nullptr), commonEventListener(nullptr), pid(0), uid(0), isFreeze(false), freezeTime(0)
    {}
};

struct FrozenEventRecord {
    std::shared_ptr<EventSubscriberRecord> subscriberRecordPtr;
    std::vector<std::shared_ptr<CommonEventRecord>> eventRecordPtrs;

    FrozenEventRecord() : subscriberRecordPtr(nullptr)
    {}
};

inline bool operator<(const std::shared_ptr<EventSubscriberRecord> &a, const std::shared_ptr<EventSubscriberRecord> &b)
{
    return a->eventSubscriberInfo->GetPriority() > b->eventSubscriberInfo->GetPriority();
}

using SubscriberRecordPtr = std::shared_ptr<EventSubscriberRecord>;
using SubscribeInfoPtr = std::shared_ptr<CommonEventSubscribeInfo>;
using EventRecordPtr = std::shared_ptr<CommonEventRecord>;

class CommonEventSubscriberManager : public DelayedSingleton<CommonEventSubscriberManager> {
public:
    CommonEventSubscriberManager();

    virtual ~CommonEventSubscriberManager() override;

    int InsertSubscriber(const SubscribeInfoPtr &eventSubscriberInfo, const sptr<IRemoteObject> &commonEventListener,
        const struct tm &recordTime, const pid_t &pid, const uid_t &uid, const std::string &bundleName);

    int RemoveSubscriber(const sptr<IRemoteObject> &commonEventListener);

    std::vector<SubscriberRecordPtr> GetSubscriberRecords(const Want &want);

    void UpdateFreezeInfo(const uid_t &uid, const bool &freezeState, const int64_t &freezeTime = 0);

    void InsertFrozenEvents(const SubscriberRecordPtr &eventListener, const CommonEventRecord &eventRecord);

    std::map<SubscriberRecordPtr, std::vector<EventRecordPtr>> GetFrozenEvents(const uid_t &uid);

    void DumpDetailed(
        const std::string &title, const SubscriberRecordPtr &record, const std::string format, std::string &dumpInfo);

    void DumpState(const std::string &event, std::vector<std::string> &state);

private:
    int InsertSubscriberRecordLocked(const std::vector<std::string> &events, const SubscriberRecordPtr &record);

    int RemoveSubscriberRecordLocked(const sptr<IRemoteObject> &commonEventListener);

    void GetSubscriberRecordsByWantLocked(const Want &want, std::vector<SubscriberRecordPtr> &records);

    void GetSubscriberRecordsByEvent(const std::string &event, std::vector<SubscriberRecordPtr> &records);

    void RemoveFrozenEventsBySubscriber(const SubscriberRecordPtr &subscriberRecord);

    void RemoveFrozenEvents(const uid_t &uid);

private:
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> death_;
    std::map<std::string, std::multiset<SubscriberRecordPtr>> eventSubscribers_;
    std::vector<SubscriberRecordPtr> subscribers_;
    std::map<uid_t, std::map<SubscriberRecordPtr, std::vector<EventRecordPtr>>> frozenEvents_;
    const time_t FREEZE_EVENT_TIMEOUT;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_SUBSCRIBER_MANAGER_H