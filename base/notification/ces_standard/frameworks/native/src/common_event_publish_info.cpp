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

#include "common_event_publish_info.h"
#include "event_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace EventFwk {
CommonEventPublishInfo::CommonEventPublishInfo() : sticky_(false), ordered_(false)
{
    EVENT_LOGD("constructor used to create a common event publish info instance");
}

CommonEventPublishInfo::CommonEventPublishInfo(const CommonEventPublishInfo &commonEventPublishInfo)
{
    EVENT_LOGD(
        "constructor used to create a common event publish info instance by copying parameters from an existing one.");

    sticky_ = commonEventPublishInfo.sticky_;
    ordered_ = commonEventPublishInfo.ordered_;
    subscriberPermissions_ = commonEventPublishInfo.subscriberPermissions_;
}

CommonEventPublishInfo::~CommonEventPublishInfo()
{
    EVENT_LOGD("deconstructor used to destory a publish info instance");
}

void CommonEventPublishInfo::SetSticky(bool sticky)
{
    sticky_ = sticky;
}

bool CommonEventPublishInfo::IsSticky() const
{
    return sticky_;
}

void CommonEventPublishInfo::SetSubscriberPermissions(const std::vector<std::string> &subscriberPermissions)
{
    subscriberPermissions_ = subscriberPermissions;
}

const std::vector<std::string> &CommonEventPublishInfo::GetSubscriberPermissions() const
{
    return subscriberPermissions_;
}

void CommonEventPublishInfo::SetOrdered(bool ordered)
{
    ordered_ = ordered;
}

bool CommonEventPublishInfo::IsOrdered() const
{
    return ordered_;
}

bool CommonEventPublishInfo::Marshalling(Parcel &parcel) const
{
    EVENT_LOGD("enter");

    // write subscriberpermissions
    std::vector<std::u16string> permissionVec_;
    for (std::vector<std::string>::size_type i = 0; i < subscriberPermissions_.size(); ++i) {
        permissionVec_.emplace_back(Str8ToStr16(subscriberPermissions_[i]));
    }
    if (!parcel.WriteString16Vector(permissionVec_)) {
        EVENT_LOGE("common event Publish Info write permission failed");
        return false;
    }

    // write ordered
    if (!parcel.WriteBool(ordered_)) {
        EVENT_LOGE("common event Publish Info write ordered failed");
        return false;
    }

    // write sticky
    if (!parcel.WriteBool(sticky_)) {
        EVENT_LOGE("common event Publish Info write sticky failed");
        return false;
    }

    return true;
}

bool CommonEventPublishInfo::ReadFromParcel(Parcel &parcel)
{
    EVENT_LOGD("enter");

    // read subscriberpermissions
    std::vector<std::u16string> permissionVec_;
    if (!parcel.ReadString16Vector(&permissionVec_)) {
        EVENT_LOGE("ReadFromParcel read permission error");
        return false;
    }
    subscriberPermissions_.clear();
    for (std::vector<std::u16string>::size_type i = 0; i < permissionVec_.size(); i++) {
        subscriberPermissions_.emplace_back(Str16ToStr8(permissionVec_[i]));
    }

    // read ordered
    ordered_ = parcel.ReadBool();

    // read sticky
    sticky_ = parcel.ReadBool();

    return true;
}

CommonEventPublishInfo *CommonEventPublishInfo::Unmarshalling(Parcel &parcel)
{
    CommonEventPublishInfo *commonEventPublishInfo = new CommonEventPublishInfo();

    if (commonEventPublishInfo && !commonEventPublishInfo->ReadFromParcel(parcel)) {
        delete commonEventPublishInfo;
        commonEventPublishInfo = nullptr;
    }

    return commonEventPublishInfo;
}
}  // namespace EventFwk
}  // namespace OHOS