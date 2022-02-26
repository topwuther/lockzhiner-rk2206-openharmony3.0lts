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

#ifndef FOUNDATION_EVENT_COMMON_LOG_INCLUDE_EVENT_LOG_WRAPPER_H
#define FOUNDATION_EVENT_COMMON_LOG_INCLUDE_EVENT_LOG_WRAPPER_H

#include "hilog/log.h"
#include <string>

namespace OHOS {
namespace EventFwk {
#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD001100
#endif
#ifndef EVENT_LOG_TAG
#define EVENT_LOG_TAG NULL
#endif

enum class EventLogLevel { DEBUG = 0, INFO, WARN, ERROR, FATAL };

static constexpr OHOS::HiviewDFX::HiLogLabel Event_LABEL = {LOG_CORE, LOG_DOMAIN, EVENT_LOG_TAG};

class EventLogWrapper {
public:
    static bool JudgeLevel(const EventLogLevel &level);

    static void SetLogLevel(const EventLogLevel &level)
    {
        level_ = level;
    }

    static const EventLogLevel &GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const char *str);

private:
    static EventLogLevel level_;
};

#define PRINT_LOG(LEVEL, Level, fmt, ...)                    \
    if (EventLogWrapper::JudgeLevel(EventLogLevel::LEVEL))   \
    OHOS::HiviewDFX::HiLog::Level(Event_LABEL,               \
        "[%{public}s:(%{public}s):%{public}d] " fmt,         \
        EventLogWrapper::GetBriefFileName(__FILE__).c_str(), \
        __FUNCTION__,                                        \
        __LINE__,                                            \
        ##__VA_ARGS__)

#define EVENT_LOGD(fmt, ...) PRINT_LOG(DEBUG, Debug, fmt, ##__VA_ARGS__)
#define EVENT_LOGI(fmt, ...) PRINT_LOG(INFO, Info, fmt, ##__VA_ARGS__)
#define EVENT_LOGW(fmt, ...) PRINT_LOG(WARN, Warn, fmt, ##__VA_ARGS__)
#define EVENT_LOGE(fmt, ...) PRINT_LOG(ERROR, Error, fmt, ##__VA_ARGS__)
#define EVENT_LOGF(fmt, ...) PRINT_LOG(FATAL, Fatal, fmt, ##__VA_ARGS__)
}  // namespace EventFwk
}  // namespace OHOS
#endif  // FOUNDATION_EVENT_COMMON_LOG_INCLUDE_EVENT_LOG_WRAPPER_H
