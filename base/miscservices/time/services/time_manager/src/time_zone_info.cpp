/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "time_zone_info.h"
#include "time_file_utils.h"

namespace OHOS {
namespace MiscServices {
namespace {
const std::string TIMEZONE_FILE_PATH = "/data/misc/zoneinfo/timezone.json";
static const int HOURS_TO_MINUTES = 60;
}

TimeZoneInfo::TimeZoneInfo()
{
    std::vector<struct zoneInfoEntry> timezoneList = { 
        {"Antarctica/McMurdo", "AQ", 12},
        {"America/Argentina/Buenos_Aires", "AR", -3},
        {"Australia/Sydney", "AU", 10},
        {"America/Noronha", "BR", -2},
        {"America/St_Johns", "CA", -2.5},
        {"Africa/Kinshasa", "CD", 1},
        {"America/Santiago", "CL", -3},
        {"Asia/Shanghai", "CN", 8},
        {"Asia/Nicosia", "CY", 3},
        {"Europe/Berlin", "DE", 2},
        {"America/Guayaquil", "CEST", -5},
        {"Europe/Madrid", "ES", 2},
        {"Pacific/Pohnpei", "FM", 11},
        {"America/Godthab", "GL", -2},
        {"Asia/Jakarta", "ID", 7},
        {"Pacific/Tarawa", "KI", 12},
        {"Asia/Almaty", "KZ", 6},
        {"Pacific/Majuro", "MH", 12},
        {"Asia/Ulaanbaatar", "MN", 8},
        {"America/Mexico_City", "MX", -5},
        {"Asia/Kuala_Lumpur", "MY", 8},
        {"Pacific/Auckland", "NZ", 12},
        {"Pacific/Tahiti", "PF", -10},
        {"Pacific/Port_Moresby", "PG", 10},
        {"Asia/Gaza", "PS", 3},
        {"Europe/Lisbon", "PT", 1},
        {"Europe/Moscow", "RU", 3},
        {"Europe/Kiev", "UA", 3},
        {"Pacific/Wake", "UM", 12},
        {"America/New_York", "US", -4},
        {"Asia/Tashkent", "UZ", 5}
    };

    for (auto tz : timezoneList) {
        timezoneInfoMap_[tz.ID] = tz;
    }
}

TimeZoneInfo::~TimeZoneInfo()
{
    timezoneInfoMap_.clear();
}

void TimeZoneInfo::Init()
{
    TIME_HILOGD(TIME_MODULE_SERVICE, "start.");
    std::string timezoneId;
    float gmtOffset;
    if (!InitStorage()) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "end, InitStorage failed.");
        return;
    }
    if (!GetTimezoneFromFile(timezoneId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "end, GetTimezoneFromFile failed.");
        return;
    }
    if (!GetOffsetById(timezoneId, gmtOffset)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "end, GetOffsetById failed.");
        return;
    }
    if (!SetOffsetToKernel(gmtOffset)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "end, SetOffsetToKernel failed.");
        return;
    }
    curTimezoneId_ = timezoneId;
    TIME_HILOGD(TIME_MODULE_SERVICE, "end.");
}

bool TimeZoneInfo::InitStorage()
{
    auto filePath = TIMEZONE_FILE_PATH.c_str();
    if (!TimeFileUtils::IsExistFile(filePath)) {
        TIME_HILOGD(TIME_MODULE_SERVICE, "Timezone file not existed :%{public}s.", filePath);
        const std::string dir = TimeFileUtils::GetPathDir(filePath);
        if (dir.empty()) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "Filepath invalid.");
            return false;
        }
        if (!TimeFileUtils::IsExistDir(dir.c_str())) {
            if (!TimeFileUtils::MkRecursiveDir(dir.c_str(), true)) {
                TIME_HILOGE(TIME_MODULE_SERVICE, "Create filepath failed :%{public}s.", filePath);
                return false;
            }
            TIME_HILOGD(TIME_MODULE_SERVICE, "Create filepath success :%{public}s.", filePath);
        }
    }
    return true;
}

bool TimeZoneInfo::SetTimezone(std::string timezoneId)
{
    float gmtOffset;
    if (!GetOffsetById(timezoneId, gmtOffset)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Timezone unsupport %{public}s.", timezoneId.c_str());
        return false;
    }
    TIME_HILOGD(TIME_MODULE_SERVICE, "timezone :%{public}s , GMT Offset :%{public}f",  timezoneId.c_str(), gmtOffset);
    if (!SetOffsetToKernel(gmtOffset)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Set kernel failed.");
        return false;
    }
    if (!SaveTimezoneToFile(timezoneId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Save file failed.");
        return false;
    }
    curTimezoneId_ = timezoneId;
    return true;
}

bool TimeZoneInfo::GetTimezone(std::string &timezoneId)
{
    timezoneId = curTimezoneId_;
    return true;
}

bool TimeZoneInfo::SetOffsetToKernel(float offsetHour)
{
    struct timezone tz{};
    tz.tz_minuteswest = static_cast<int>(offsetHour * HOURS_TO_MINUTES);
    tz.tz_dsttime = 0;
    TIME_HILOGD(TIME_MODULE_SERVICE, "settimeofday, Offset hours :%{public}f , Offset minutes :%{public}d",  offsetHour, tz.tz_minuteswest);
    int result = settimeofday(NULL, &tz);
    if (result < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "settimeofday fail: %{public}d.", result);
        return false;
    }
    return true;
}

bool TimeZoneInfo::GetTimezoneFromFile(std::string &timezoneId)
{
    Json::Value root;
    std::ifstream ifs;
    ifs.open(TIMEZONE_FILE_PATH);
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs)) {
        ifs.close();
        TIME_HILOGE(TIME_MODULE_SERVICE, "Read file failed %{public}s.", errs.c_str());
        return false;
    }
    timezoneId = root["TimezoneId"].asString();
    TIME_HILOGE(TIME_MODULE_SERVICE, "Read file %{public}s.", timezoneId.c_str());
    ifs.close();
    return true;
}

bool TimeZoneInfo::SaveTimezoneToFile(std::string timezoneId)
{
    std::ofstream ofs;
    ofs.open(TIMEZONE_FILE_PATH);
    Json::Value root;
    root["TimezoneId"] = timezoneId;
    Json::StreamWriterBuilder builder;
    const std::string json_file = Json::writeString(builder, root);
    ofs << json_file;
    ofs.close();
    TIME_HILOGD(TIME_MODULE_SERVICE, "Write file %{public}s.", timezoneId.c_str());
    return true;
}

bool TimeZoneInfo::GetOffsetById(const std::string timezoneId, float &offset)
{
    auto itEntry = timezoneInfoMap_.find(timezoneId);
    if (itEntry != timezoneInfoMap_.end()) {
        auto zoneInfo = itEntry->second;
        offset = zoneInfo.utcOffsetHours;
        return true;
    }
    TIME_HILOGE(TIME_MODULE_SERVICE, "TimezoneId not found.");
    return false;
}
}
}