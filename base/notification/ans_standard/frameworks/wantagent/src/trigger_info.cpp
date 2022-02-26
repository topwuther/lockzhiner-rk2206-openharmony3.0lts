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

#include "trigger_info.h"

using namespace OHOS::AAFwk;

namespace OHOS::Notification::WantAgent {
TriggerInfo::TriggerInfo() : permission_(""), extraInfo_(nullptr), want_(nullptr), resultCode_(0)
{}

TriggerInfo::TriggerInfo(const std::string &permission, const std::shared_ptr<WantParams> &extraInfo,
    const std::shared_ptr<Want> &want, int resultCode)
{
    permission_ = permission;
    if (extraInfo != nullptr) {
        extraInfo_ = std::make_shared<WantParams>(*extraInfo);
    }
    if (want != nullptr) {
        want_ = std::make_shared<Want>(*want);
    }
    resultCode_ = resultCode;
}

TriggerInfo::TriggerInfo(const TriggerInfo &paramInfo)
{
    permission_ = paramInfo.GetPermission();
    if (paramInfo.GetExtraInfo() != nullptr) {
        extraInfo_ = std::make_shared<WantParams>(*paramInfo.GetExtraInfo());
    }
    if (paramInfo.GetWant() != nullptr) {
        want_ = std::make_shared<Want>(*paramInfo.GetWant());
    }
    resultCode_ = paramInfo.GetResultCode();
}

const TriggerInfo &TriggerInfo::operator=(const TriggerInfo &paramInfo)
{
    permission_ = paramInfo.GetPermission();
    if (paramInfo.GetExtraInfo() != nullptr) {
        extraInfo_ = std::make_shared<WantParams>(*paramInfo.GetExtraInfo());
    }
    if (paramInfo.GetWant() != nullptr) {
        want_ = std::make_shared<Want>(*paramInfo.GetWant());
    }
    resultCode_ = paramInfo.GetResultCode();

    return *this;
}

std::string TriggerInfo::GetPermission() const
{
    return permission_;
}

std::shared_ptr<WantParams> TriggerInfo::GetExtraInfo() const
{
    return extraInfo_;
}

std::shared_ptr<Want> TriggerInfo::GetWant() const
{
    return want_;
}

int TriggerInfo::GetResultCode() const
{
    return resultCode_;
}

TriggerInfo::Builder::Builder()
{}

std::shared_ptr<TriggerInfo::Builder> TriggerInfo::Builder::SetPermission(const std::string &permission)
{
    permission_ = permission;
    return shared_from_this();
}

std::shared_ptr<TriggerInfo::Builder> TriggerInfo::Builder::SetWantParams(const std::shared_ptr<WantParams> &params)
{
    params_ = params;
    return shared_from_this();
}

std::shared_ptr<TriggerInfo::Builder> TriggerInfo::Builder::SetWant(const std::shared_ptr<Want> &want)
{
    want_ = want;
    return shared_from_this();
}

std::shared_ptr<TriggerInfo::Builder> TriggerInfo::Builder::SetResultCode(int resultCode)
{
    resultCode_ = resultCode;
    return shared_from_this();
}

std::shared_ptr<TriggerInfo> TriggerInfo::Builder::Build()
{
    return std::make_shared<TriggerInfo>(permission_, params_, want_, resultCode_);
}
}  // namespace OHOS::Notification::WantAgent