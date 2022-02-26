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

#include "vendor.h"

#include "db_helper.h"
#include "plugin.h"
#include "resolver.h"
#include "string_util.h"
#include "tbox.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_RELIABILITY_LOG_TAG("FreezeDetector");

const std::vector<std::pair<std::string, std::string>> Vendor::applicationPairs_ = {
    {"APPEXECFWK", "UI_BLOCK_3S"},
    {"APPEXECFWK", "UI_BLOCK_6S"},
};

const std::vector<std::pair<std::string, std::string>> Vendor::systemPairs_ = {
    {"HUNGTASK", "HUNGTASK"},
};

const std::vector<std::pair<std::string, std::string>> Vendor::hardwarePairs_ = {
    {"TP", "TP_I2C"},
};

bool Vendor::IsFreezeEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : applicationPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    for (auto const pair : systemPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsApplicationEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : applicationPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsSystemEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : systemPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsHardwareEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : hardwarePairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsSystemResult(const FreezeResult& result) const
{
    return result.GetId() == SYSTEM_RESULT_ID;
}

bool Vendor::IsApplicationResult(const FreezeResult& result) const
{
    return result.GetId() == APPLICATION_RESULT_ID;
}

bool Vendor::IsBetaVersion() const
{
    return true;
}

std::set<std::string> Vendor::GetFreezeStringIds() const
{
    std::set<std::string> set;

    for (auto const pair : applicationPairs_) {
        set.insert(pair.second);
    }
    for (auto const pair : systemPairs_) {
        set.insert(pair.second);
    }

    return set;
}

bool Vendor::GetMatchString(const std::string& src, std::string& dst, const std::string& pattern) const
{
    std::regex reg(pattern);
    std::smatch result;
    if (std::regex_search(src, result, reg)) {
        dst = StringUtil::TrimStr(result[1], '\n');
        return true;
    }
    return false;
}

std::string Vendor::GetRebootReason()
{
    std::string reboot = "";
    std::string reset = "";
    if (GetMatchString(cmdlineContent_, reboot, REBOOT_REASON + PATTERN_WITHOUT_SPACE) &&
        GetMatchString(cmdlineContent_, reset, NORMAL_RESET_TYPE + PATTERN_WITHOUT_SPACE)) {
        if (reboot == BR_PRESS10S || reset == BR_PRESS10S) {
            HIVIEW_LOGI("reboot reason:LONG_PRESS.");
            return BR_PRESS10S;
        }
        for (auto reason : rebootReasons_) {
            if (reason == reboot || reason == reset) {
                HIVIEW_LOGI("reboot reason:LONG_PRESS.");
                return LONG_PRESS;
            }
        }
    }
    HIVIEW_LOGI("no reboot reason.");
    return "";
}

std::shared_ptr<PipelineEvent> Vendor::ProcessHardwareEvent(unsigned long timestamp)
{
    return nullptr;
}

bool Vendor::ReduceRelevanceEvents(std::list<WatchPoint>& list, const FreezeResult& result) const
{
    HIVIEW_LOGI("before size=%{public}zu", list.size());
    if (IsSystemResult(result) == false && IsApplicationResult(result) == false) {
        list.clear();
        return false;
    }

    // erase if not system event
    if (IsSystemResult(result)) {
        std::list<WatchPoint>::iterator watchPoint;
        for (watchPoint = list.begin(); watchPoint != list.end();) {
            if (IsSystemEvent(watchPoint->GetDomain(), watchPoint->GetStringId())) {
                watchPoint++;
            } else {
                watchPoint = list.erase(watchPoint);
            }
        }
    }

    std::vector<std::string> relevances = result.GetRelevanceStringIds(); // without domains

    // erase if not application event or not relevance event
    if (IsApplicationResult(result)) {
        std::list<WatchPoint>::iterator watchPoint;
        for (watchPoint = list.begin(); watchPoint != list.end();) {
            if (IsApplicationEvent(watchPoint->GetDomain(), watchPoint->GetStringId())) {
                watchPoint++;
            }
            else if (IsSystemEvent(watchPoint->GetDomain(), watchPoint->GetStringId()) &&
                std::find(relevances.begin(), relevances.end(), watchPoint->GetStringId()) != relevances.end()) {
                    watchPoint++;
            } else {
                watchPoint = list.erase(watchPoint);
            }
        }
    }

    list.sort();
    list.unique();
    HIVIEW_LOGI("after size=%{public}zu", list.size());
    return list.size() != 0;
}

FreezeResult Vendor::MakeSystemResult() const
{
    return FreezeResult(0, SYSTEM_SCOPE);
}

std::shared_ptr<PipelineEvent> Vendor::MakeEvent(
    const WatchPoint &watchPoint, const WatchPoint& matchedWatchPoint,
    const std::list<WatchPoint>& list, const FreezeResult& result) const
{
    for (auto node : list) {
        DBHelper::UpdateEventIntoDB(node, result.GetId());
    }

    return nullptr;
}

void Vendor::UpdateHardwareEventLog(WatchPoint& watchPoint) const
{
    return;
}

Vendor::Vendor() : cmdlinePath_("/proc/cmdline"), cmdlineContent_("")
{
    rebootReasons_.clear();
}

Vendor::~Vendor()
{
    rebootReasons_.clear();
}

bool Vendor::Init()
{
    GetCmdlineContent();
    return true;
}

bool Vendor::GetInitFlag()
{
    return true;
}

void Vendor::GetRebootReasonConfig()
{
    rebootReasons_.clear();
    rebootReasons_.push_back(PRESS10S);
    rebootReasons_.push_back(AP_S_PRESS6S);
    rebootReasons_.push_back(BR_PRESS10S);
}

void Vendor::GetCmdlineContent()
{
    if (FileUtil::LoadStringFromFile(cmdlinePath_, cmdlineContent_) == false) {
        HIVIEW_LOGE("failed to read cmdline:%{public}s.", cmdlinePath_.c_str());
    }
}
} // namespace HiviewDFX
} // namespace OHOS
