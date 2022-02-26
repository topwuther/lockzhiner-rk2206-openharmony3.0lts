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

#include "notification_subscribe_info.h"

#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {

NotificationSubscribeInfo::NotificationSubscribeInfo()
{}

NotificationSubscribeInfo::~NotificationSubscribeInfo()
{}

NotificationSubscribeInfo::NotificationSubscribeInfo(const NotificationSubscribeInfo &subscribeInfo)
{
    appNames_ = subscribeInfo.GetAppNames();
}

void NotificationSubscribeInfo::AddAppName(const std::string appName)
{
    appNames_.push_back(appName);
}

void NotificationSubscribeInfo::AddAppNames(const std::vector<std::string> &appNames)
{
    appNames_.insert(appNames_.end(), appNames.begin(), appNames.end());
}

std::vector<std::string> NotificationSubscribeInfo::GetAppNames() const
{
    return appNames_;
}

bool NotificationSubscribeInfo::Marshalling(Parcel &parcel) const
{
    // write appNames_
    if (!parcel.WriteStringVector(appNames_)) {
        ANS_LOGE("Can't write appNames_");
        return false;
    }
    return true;
}

NotificationSubscribeInfo *NotificationSubscribeInfo::Unmarshalling(Parcel &parcel)
{
    NotificationSubscribeInfo *info = new NotificationSubscribeInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }

    return info;
}

bool NotificationSubscribeInfo::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadStringVector(&appNames_);
    return true;
}

std::string NotificationSubscribeInfo::Dump()
{
    return "Dump";
}

}  // namespace Notification
}  // namespace OHOS
