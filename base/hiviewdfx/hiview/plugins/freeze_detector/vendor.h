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

#ifndef FREEZE_VENDOR_H
#define FREEZE_VENDOR_H

#include <set>
#include <string>
#include <vector>

#include "fault_detect_event.h"
#include "pipeline.h"
#include "resolver.h"
#include "singleton.h"
#include "watch_point.h"

namespace OHOS {
namespace HiviewDFX {
class Vendor : public Singleton<Vendor> {
    DECLARE_SINGLETON(Vendor);

public:
    bool Init();
    bool GetInitFlag();
    std::string GetRebootReason();
    bool IsFreezeEvent(const std::string& domain, const std::string& stringId) const;
    bool IsApplicationEvent(const std::string& domain, const std::string& stringId) const;
    bool IsHardwareEvent(const std::string& domain, const std::string& stringId) const;
    bool IsBetaVersion() const;
    std::set<std::string> GetFreezeStringIds() const;
    std::shared_ptr<PipelineEvent> ProcessHardwareEvent(unsigned long timestamp);
    std::shared_ptr<PipelineEvent> MakeEvent(
        const WatchPoint &watchPoint, const WatchPoint& matchedWatchPoint,
        const std::list<WatchPoint>& list, const FreezeResult& result) const;
    bool ReduceRelevanceEvents(std::list<WatchPoint>& list, const FreezeResult& result) const;
    FreezeResult MakeSystemResult() const;
    void UpdateHardwareEventLog(WatchPoint& watchPoint) const;
    void SetCmdlinePath(const std::string& path)
    {
        cmdlinePath_ = path;
    };

private:
    static const int MAX_LINE_NUM = 100;
    static const int SYSTEM_RESULT_ID = 1;
    static const int APPLICATION_RESULT_ID = 0;
    static const inline std::string AP_S_PRESS6S = "AP_S_PRESS6S";
    static const inline std::string BR_PRESS10S = "BR_PRESS_10S";
    static const inline std::string LONG_PRESS = "LONG_PRESS";
    static const inline std::string PRESS10S = "press10s";
    static const inline std::string REBOOT_REASON = "reboot_reason";
    static const inline std::string NORMAL_RESET_TYPE = "normal_reset_type";
    static const inline std::string PATTERN_WITH_SPACE = "\\s*[^\\n\\r]*";
    static const inline std::string PATTERN_WITHOUT_SPACE = "\\s*=\\s*([^ \\n]*)";
    static const inline std::string SYSTEM_SCOPE = "sys";

    void GetCmdlineContent();
    void GetRebootReasonConfig();
    void GetRelevanceConfig(std::vector<std::string>& values) const;
    bool GetMatchString(const std::string& src, std::string& dst, const std::string& key) const;
    bool IsSystemEvent(const std::string& domain, const std::string& stringId) const;
    bool IsSystemResult(const FreezeResult& result) const;
    bool IsApplicationResult(const FreezeResult& result) const;

    static const std::vector<std::pair<std::string, std::string>> applicationPairs_;
    static const std::vector<std::pair<std::string, std::string>> systemPairs_;
    static const std::vector<std::pair<std::string, std::string>> hardwarePairs_;

    std::string cmdlinePath_;
    std::string cmdlineContent_;
    std::vector<std::string> rebootReasons_;
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif // FREEZE_VENDOR_H
