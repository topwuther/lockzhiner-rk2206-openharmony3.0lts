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

#ifndef FREEZE_RESOLVER_H
#define FREEZE_RESOLVER_H

#include <list>
#include <memory>
#include <set>
#include <vector>

#include "fault_detect_event.h"
#include "pipeline.h"
#include "rule_cluster.h"
#include "singleton.h"
#include "watch_point.h"

namespace OHOS {
namespace HiviewDFX {
class FreezeResolver : public Singleton<FreezeResolver> {
    DECLARE_SINGLETON(FreezeResolver);

public:
    bool Init();
    std::string GetTimeZone() const;
    std::string GetTimeString(unsigned long timestamp) const;
    std::shared_ptr<PipelineEvent> ProcessHardwareEvent() const;
    std::shared_ptr<PipelineEvent> ProcessLongPressEvent() const;
    bool ResolveLongPressEvent() const;
    std::shared_ptr<PipelineEvent> ProcessSystemEvent() const;
    std::shared_ptr<PipelineEvent> ProcessEvent(WatchPoint &watchPoint) const;

private:
    static const inline std::string HEADER = "*******************************************";
    static const inline std::string DOMAIN_LONGPRESS = "KEY_PRESS";
    static const inline std::string STRINGID_LONGPRESS = "LONG_PRESS";
    static const inline std::string HARDWARE_SCOPE = "hw";
    static const inline std::string PATTERN_WITH_SPACE = "\\s*[^\\n\\r]*";
    static const inline std::string PATTERN_WITHOUT_SPACE = "\\s*=\\s*([^ \\n]*)";
    static const inline std::string HYPHEN = "-";
    static const inline std::string APPFREEZE = "appfreeze";
    static const inline std::string SYSFREEZE = "sysfreeze";
    static const inline std::string SP_APPFREEZE = "AppFreeze";
    static const inline std::string SP_ENDSTACK = "END_STACK";
    static const inline std::string FAULTLOGGER_PATH = "/data/log/faultlog/faultlogger/";
    static const inline std::string DEFAULT_PATH = "/system/etc/hiview/reliability";
    static const int DEFAULT_TIME_WINDOW = 30;
    static const int REBOOT_TIME_WINDOW = 300;
    static const int TIME_STRING_LEN = 16;
    static const int MILLISECOND = 1000;
    static const int MINUTES_IN_HOUR = 60;

    void GetConfigValue(int id, std::vector<std::string>& values);
    bool ResolveEvent(WatchPoint& watchPoint, WatchPoint& matchedWatchPoint,
        std::list<WatchPoint>& list, FreezeResult& result) const;
    void UpdateEventLog(WatchPoint& watchPoint) const;
    bool MergeEventLog(const WatchPoint &watchPoint, const std::list<WatchPoint>& list) const;

    unsigned long startTime_;
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif // FREEZE_DETECTOR_RESOLVER_H
