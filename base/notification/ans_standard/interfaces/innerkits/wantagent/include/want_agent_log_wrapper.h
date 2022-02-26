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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_LOG_WRAPPER_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_LOG_WRAPPER_H

#include "hilog/log.h"
#include <string>

namespace OHOS::Notification::WantAgent {
#ifndef WANT_AGENT_LOG_DOMAIN
#define WANT_AGENT_LOG_DOMAIN 0xD001000
#endif
#ifndef WANT_AGENT_LOG_TAG
#define WANT_AGENT_LOG_TAG "WantAgent"
#endif

enum class WantAgentLogLevel { DEBUG = 0, INFO, WARN, ERROR, FATAL };

static constexpr OHOS::HiviewDFX::HiLogLabel Want_Agent_LABEL = {LOG_CORE, WANT_AGENT_LOG_DOMAIN, WANT_AGENT_LOG_TAG};

class WantAgentLogWrapper {
public:
    static bool JudgeLevel(const WantAgentLogLevel &level);

    static void SetLogLevel(const WantAgentLogLevel &level)
    {
        level_ = level;
    }

    static const WantAgentLogLevel &GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const char *str);

private:
    static WantAgentLogLevel level_;
};

#define PRINT_LOG(LEVEL, Level, fmt, ...)                          \
    if (WantAgentLogWrapper::JudgeLevel(WantAgentLogLevel::LEVEL)) \
    OHOS::HiviewDFX::HiLog::Level(Want_Agent_LABEL,                \
        "[%{public}s(%{public}s)] " fmt,                           \
        WantAgentLogWrapper::GetBriefFileName(__FILE__).c_str(),   \
        __FUNCTION__,                                              \
        ##__VA_ARGS__)

#define WANT_AGENT_LOGD(fmt, ...) PRINT_LOG(DEBUG, Debug, fmt, ##__VA_ARGS__)
#define WANT_AGENT_LOGI(fmt, ...) PRINT_LOG(INFO, Info, fmt, ##__VA_ARGS__)
#define WANT_AGENT_LOGW(fmt, ...) PRINT_LOG(WARN, Warn, fmt, ##__VA_ARGS__)
#define WANT_AGENT_LOGE(fmt, ...) PRINT_LOG(ERROR, Error, fmt, ##__VA_ARGS__)
#define WANT_AGENT_LOGF(fmt, ...) PRINT_LOG(FATAL, Fatal, fmt, ##__VA_ARGS__)
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_LOG_WRAPPER_H
