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

#include "common_event_subscribe_info.h"
#include "event_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace EventFwk {
CommonEventSubscribeInfo::CommonEventSubscribeInfo(const MatchingSkills &matchingSkills)
    : matchingSkills_(matchingSkills), priority_(0), threadMode_(CommonEventSubscribeInfo::ASYNC)
{
    EVENT_LOGD("constructor used to create a common event subscriber info instance by matcingSkills");
}

CommonEventSubscribeInfo::CommonEventSubscribeInfo() : priority_(0), threadMode_(CommonEventSubscribeInfo::ASYNC)
{
    EVENT_LOGD("constructor used to create a common event subscriber info instance");
}

CommonEventSubscribeInfo::CommonEventSubscribeInfo(const CommonEventSubscribeInfo &commonEventSubscribeInfo)
{
    matchingSkills_ = commonEventSubscribeInfo.matchingSkills_;
    priority_ = commonEventSubscribeInfo.priority_;
    permission_ = commonEventSubscribeInfo.permission_;
    deviceId_ = commonEventSubscribeInfo.deviceId_;
    threadMode_ = commonEventSubscribeInfo.threadMode_;
}

CommonEventSubscribeInfo::~CommonEventSubscribeInfo()
{
    EVENT_LOGD("deconstructor used to destroy a common event subscriber info instance");
}

void CommonEventSubscribeInfo::SetPriority(const int32_t &priority)
{
    priority_ = priority;
}

int32_t CommonEventSubscribeInfo::GetPriority() const
{
    return priority_;
}

void CommonEventSubscribeInfo::SetPermission(const std::string &permission)
{
    permission_ = permission;
}

std::string CommonEventSubscribeInfo::GetPermission() const
{
    return permission_;
}

CommonEventSubscribeInfo::ThreadMode CommonEventSubscribeInfo::GetThreadMode() const
{
    return threadMode_;
}

void CommonEventSubscribeInfo::SetThreadMode(CommonEventSubscribeInfo::ThreadMode threadMode)
{
    threadMode_ = threadMode;
}

void CommonEventSubscribeInfo::SetDeviceId(const std::string &deviceId)
{
    deviceId_ = deviceId;
}

std::string CommonEventSubscribeInfo::GetDeviceId() const
{
    return deviceId_;
}

const MatchingSkills &CommonEventSubscribeInfo::GetMatchingSkills() const
{
    return matchingSkills_;
}

bool CommonEventSubscribeInfo::Marshalling(Parcel &parcel) const
{
    // write permission
    if (!parcel.WriteString16(Str8ToStr16(permission_))) {
        EVENT_LOGE("Failed to write permission");
        return false;
    }

    // write priority
    if (!parcel.WriteUint32(priority_)) {
        EVENT_LOGE("Failed to write priority");
        return false;
    }

    // write threadMode
    if (!parcel.WriteUint32(threadMode_)) {
        EVENT_LOGE("Failed to write threadMode");
        return false;
    }

    // write deviceId
    if (!parcel.WriteString16(Str8ToStr16(deviceId_))) {
        EVENT_LOGE("Failed to write deviceId");
        return false;
    }

    // write matchingSkills
    if (!parcel.WriteParcelable(&matchingSkills_)) {
        EVENT_LOGE("Failed to write matchingSkills");
        return false;
    }

    return true;
}

bool CommonEventSubscribeInfo::ReadFromParcel(Parcel &parcel)
{
    // read permission
    permission_ = Str16ToStr8(parcel.ReadString16());

    // read priority
    priority_ = parcel.ReadUint32();

    // read threadMode
    threadMode_ = (CommonEventSubscribeInfo::ThreadMode)parcel.ReadUint32();

    // read deviceId
    deviceId_ = Str16ToStr8(parcel.ReadString16());

    // read MatchingSkills
    auto skills = parcel.ReadParcelable<MatchingSkills>();
    if (skills != nullptr) {
        matchingSkills_ = *skills;
        delete skills;
    } else {
        EVENT_LOGE("Failed to read matchingSkills");
        return false;
    }

    return true;
}

CommonEventSubscribeInfo *CommonEventSubscribeInfo::Unmarshalling(Parcel &parcel)
{
    CommonEventSubscribeInfo *commonEventSubscribeInfo = new CommonEventSubscribeInfo();

    if (commonEventSubscribeInfo && !commonEventSubscribeInfo->ReadFromParcel(parcel)) {
        delete commonEventSubscribeInfo;
        commonEventSubscribeInfo = nullptr;
    }

    return commonEventSubscribeInfo;
}
}  // namespace EventFwk
}  // namespace OHOS