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
#include "hiappevent_config.h"

#include <algorithm>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>

#include "hiappevent_base.h"
#include "hiappevent_read.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_config" };
const std::string DISABLE = "disable";
const std::string MAX_STORAGE = "max_storage";
constexpr uint64_t STORAGE_UNIT_KB = 1024;
constexpr uint64_t STORAGE_UNIT_MB = STORAGE_UNIT_KB * 1024;
constexpr uint64_t STORAGE_UNIT_GB = STORAGE_UNIT_MB * 1024;
constexpr uint64_t STORAGE_UNIT_TB = STORAGE_UNIT_GB * 1024;
constexpr int DECIMAL_UNIT = 10;
constexpr int SHORT_STORAGE_UNIT_LEN = 1;
constexpr int LONG_STORAGE_UNIT_LEN = 2;

std::mutex g_mutex;

std::string TransUpperToUnderscoreAndLower(const std::string& str)
{
    if (str.empty()) {
        return "";
    }

    std::stringstream ss;
    for (size_t i = 0; i < str.size(); i++) {
        char tmp = str[i];
        if (tmp < 'A' || tmp > 'Z') {
            ss << tmp;
            continue;
        }
        if (i != 0) { // prevent string from starting with an underscore
            ss << "_";
        }
        tmp += 32; // 32 means upper case to lower case
        ss << tmp;
    }

    return ss.str();
}
}

HiAppEventConfig& HiAppEventConfig::GetInstance()
{
    static HiAppEventConfig instance;
    return instance;
}

bool HiAppEventConfig::SetConfigurationItem(std::string name, std::string value)
{
    // trans uppercase to underscore and lowercase
    name = TransUpperToUnderscoreAndLower(name);
    HiLog::Debug(LABEL, "start to configure, name=%{public}s, value=%{public}s.", name.c_str(), value.c_str());

    if (name == "") {
        HiLog::Error(LABEL, "item name can not be empty.");
        return false;
    }
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if (value == "") {
        HiLog::Error(LABEL, "item value can not be empty.");
        return false;
    }
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    if (name == DISABLE) {
        return SetDisableItem(value);
    } else if (name == MAX_STORAGE) {
        return SetMaxStorageSizeItem(value);
    } else {
        HiLog::Error(LABEL, "unrecognized configuration item name.");
        return false;
    }
}

bool HiAppEventConfig::SetDisableItem(const std::string& value)
{
    if (value == "true") {
        SetDisable(true);
    } else if (value == "false") {
        SetDisable(false);
    } else {
        HiLog::Error(LABEL, "invalid bool value=%{public}s of the application dotting switch.", value.c_str());
        return false;
    }
    return true;
}

bool HiAppEventConfig::SetMaxStorageSizeItem(const std::string& value)
{
    if (!std::regex_match(value, std::regex("[0-9]+[k|m|g|t]?[b]?"))) {
        HiLog::Error(LABEL, "invalid value=%{public}s of the event file dir storage quota size.", value.c_str());
        return false;
    }

    int len = value.length();
    std::string::size_type numEndIndex = 0;
    uint64_t numValue = std::stoull(value, &numEndIndex, DECIMAL_UNIT);
    if ((int)numEndIndex == len) {
        SetMaxStorageSize(numValue);
        return true;
    }

    int unitLen = ((int)numEndIndex == len - 1) ? SHORT_STORAGE_UNIT_LEN : LONG_STORAGE_UNIT_LEN;
    char unitChr = value[len - unitLen];
    uint64_t maxStoSize = 0;
    switch (unitChr) {
        case 'b':
            maxStoSize = numValue;
            break;
        case 'k':
            maxStoSize = numValue * STORAGE_UNIT_KB;
            break;
        case 'm':
            maxStoSize = numValue * STORAGE_UNIT_MB;
            break;
        case 'g':
            maxStoSize = numValue * STORAGE_UNIT_GB;
            break;
        case 't':
            maxStoSize = numValue * STORAGE_UNIT_TB;
            break;
        default:
            HiLog::Error(LABEL, "invalid storage unit value=%{public}c.", unitChr);
            return false;
    }

    SetMaxStorageSize(maxStoSize);
    return true;
}

void HiAppEventConfig::SetDisable(bool disable)
{
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        this->disable = disable;
    }
}

void HiAppEventConfig::SetMaxStorageSize(uint64_t size)
{
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        this->maxStorageSize = size;
    }
}

void HiAppEventConfig::SetStorageDir(const std::string& dir)
{
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        this->storageDir = dir;
        LogAssistant::Instance().UpdateHiAppEventLogDir(dir);
    }
}

bool HiAppEventConfig::GetDisable()
{
    return this->disable;
}

uint64_t HiAppEventConfig::GetMaxStorageSize()
{
    return this->maxStorageSize;
}

std::string HiAppEventConfig::GetStorageDir()
{
    return this->storageDir;
}
}
}