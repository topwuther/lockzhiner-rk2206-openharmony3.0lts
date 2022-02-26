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

#include "want_agent_info.h"
#include "want.h"
#include "want_params.h"

using namespace OHOS::AAFwk;

namespace OHOS::Notification::WantAgent {
WantAgentInfo::WantAgentInfo() : WantAgentInfo(nullptr)
{}

WantAgentInfo::WantAgentInfo(int requestCode, const WantAgentConstant::OperationType &operationType,
    WantAgentConstant::Flags flag, std::vector<std::shared_ptr<Want>> &wants,
    const std::shared_ptr<WantParams> &extraInfo)
{
    requestCode_ = requestCode;
    operationType_ = operationType;
    flags_.push_back(flag);
    if (!wants.empty()) {
        for (auto want : wants) {
            if (want != nullptr) {
                wants_.push_back(std::make_shared<Want>(*want));
            }
        }
    }
    if (extraInfo != nullptr) {
        extraInfo_ = std::make_shared<WantParams>(*extraInfo);
    }
}

WantAgentInfo::WantAgentInfo(int requestCode, const WantAgentConstant::OperationType &operationType,
    const std::vector<WantAgentConstant::Flags> &flags, std::vector<std::shared_ptr<Want>> &wants,
    const std::shared_ptr<WantParams> &extraInfo)
{
    requestCode_ = requestCode;
    operationType_ = operationType;
    if (!flags.empty()) {
        flags_.insert(flags_.end(), flags.begin(), flags.end());
    }
    if (!wants.empty()) {
        for (auto want : wants) {
            if (want != nullptr) {
                wants_.push_back(std::make_shared<Want>(*want));
            }
        }
    }
    if (extraInfo != nullptr) {
        extraInfo_ = std::make_shared<WantParams>(*extraInfo);
    }
}

WantAgentInfo::WantAgentInfo(const std::shared_ptr<WantAgentInfo> &paramInfo)
{
    if (paramInfo != nullptr) {
        requestCode_ = paramInfo->GetRequestCode();
        operationType_ = paramInfo->GetOperationType();
        flags_.insert(flags_.end(), paramInfo->GetFlags().begin(), paramInfo->GetFlags().end());
        if (!paramInfo->GetWants().empty()) {
            for (auto want : paramInfo->GetWants()) {
                wants_.push_back(std::make_shared<Want>(*want));
            }
        }
        if (paramInfo->GetExtraInfo() != nullptr) {
            extraInfo_ = std::make_shared<WantParams>(*paramInfo->GetExtraInfo());
        }
    }
}

int WantAgentInfo::GetRequestCode() const
{
    return requestCode_;
}

WantAgentConstant::OperationType WantAgentInfo::GetOperationType() const
{
    return operationType_;
}

std::vector<WantAgentConstant::Flags> WantAgentInfo::GetFlags() const
{
    return flags_;
}

std::vector<std::shared_ptr<Want>> WantAgentInfo::GetWants() const
{
    return wants_;
}

std::shared_ptr<WantParams> WantAgentInfo::GetExtraInfo() const
{
    return extraInfo_;
}
}  // namespace OHOS::Notification::WantAgent