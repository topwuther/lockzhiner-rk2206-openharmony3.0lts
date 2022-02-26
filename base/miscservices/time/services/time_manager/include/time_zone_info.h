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
#ifndef SERVICES_INCLUDE_TIME_ZONE_INFO_H
#define SERVICES_INCLUDE_TIME_ZONE_INFO_H

#include <singleton.h>
#include <map>
#include <mutex>
#include <sys/time.h>
#include <vector>
#include <fstream>

#include "refbase.h"
#include "time.h"
#include "time_common.h"
#include "json/json.h"

namespace OHOS {
namespace MiscServices {
struct zoneInfoEntry {
    std::string ID;
    std::string alias;
    float utcOffsetHours;
};

class TimeZoneInfo {
    DECLARE_DELAYED_SINGLETON(TimeZoneInfo)
public:
    bool GetTimezone(std::string &timezoneId);
    bool SetTimezone(std::string timezoneId);
    void Init();
private:
    bool InitStorage();
    bool SetOffsetToKernel(float offset);
    bool GetOffsetById(const std::string timezoneId, float &offset);
    bool GetTimezoneFromFile(std::string &timezoneId);
    bool SaveTimezoneToFile(std::string timezoneId);
    std::string curTimezoneId_;
    std::map<std::string, struct zoneInfoEntry> timezoneInfoMap_;
};
} // MiscServices
} // OHOS

#endif // SERVICES_INCLUDE_TIME_ZONE_INFO_H