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

#include "notification_bundle_option.h"
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
NotificationBundleOption::NotificationBundleOption(const std::string &bundleName, const int32_t uid) : bundleName_(bundleName), uid_(uid)
{}

NotificationBundleOption::~NotificationBundleOption()
{}

void NotificationBundleOption::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

std::string NotificationBundleOption::GetBundleName() const
{
    return bundleName_;
}

void NotificationBundleOption::SetUid(const int32_t uid)
{
    uid_ = uid;
}

int32_t NotificationBundleOption::GetUid() const
{
    return uid_;
}

std::string NotificationBundleOption::Dump()
{
    return "NotificationBundleOption[ bundleName = " + bundleName_ + ", uid = " + std::to_string(uid_) + " ]";
}

bool NotificationBundleOption::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName_)) {
        ANS_LOGE("Failed to write bundle name");
        return false;
    }

    if (!parcel.WriteInt32(uid_)) {
        ANS_LOGE("Failed to write uid");
        return false;
    }

    return true;
}

NotificationBundleOption *NotificationBundleOption::Unmarshalling(Parcel &parcel)
{
    auto pbundleOption = new NotificationBundleOption();
    if ((nullptr != pbundleOption) && !pbundleOption->ReadFromParcel(parcel)) {
        delete pbundleOption;
        pbundleOption = nullptr;
    }

    return pbundleOption;
}

bool NotificationBundleOption::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(bundleName_)) {
        ANS_LOGE("Failed to read bundle name");
        return false;
    }

    uid_ = parcel.ReadInt32();

    return true;
}
}  // namespace Notification
}  // namespace OHOS
