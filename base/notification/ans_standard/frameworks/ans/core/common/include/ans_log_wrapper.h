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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_LOG_HELPER_H
#define BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_LOG_HELPER_H

#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace Notification {

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD001800
#endif

#ifndef ANS_LOG_TAG
#define ANS_LOG_TAG NULL
#endif

enum class AnsLogLevel : uint8_t { DEBUG = 0, INFO, WARN, ERROR, FATAL };

static constexpr OHOS::HiviewDFX::HiLogLabel ANS_LABEL = {LOG_CORE, LOG_DOMAIN, "ANS_STANDARD"};

class AnsLogWrapper {
public:
    AnsLogWrapper() = delete;
    ~AnsLogWrapper() = delete;

    static bool JudgeLevel(const AnsLogLevel &level);

    static void SetLogLevel(const AnsLogLevel &level)
    {
        level_ = level;
    }

    static const AnsLogLevel &GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const char *str);

private:
    static AnsLogLevel level_;
};

#define PRINT_LOG(LEVEL, Level, fmt, ...)                  \
    if (AnsLogWrapper::JudgeLevel(AnsLogLevel::LEVEL))     \
    OHOS::HiviewDFX::HiLog::Level(ANS_LABEL,               \
        "[%{public}s(%{public}s):%{public}d] " fmt,        \
        AnsLogWrapper::GetBriefFileName(__FILE__).c_str(), \
        __FUNCTION__,                                      \
        __LINE__,                                          \
        ##__VA_ARGS__)

#define ANS_LOGD(fmt, ...) PRINT_LOG(DEBUG, Debug, fmt, ##__VA_ARGS__)
#define ANS_LOGI(fmt, ...) PRINT_LOG(INFO, Info, fmt, ##__VA_ARGS__)
#define ANS_LOGW(fmt, ...) PRINT_LOG(WARN, Warn, fmt, ##__VA_ARGS__)
#define ANS_LOGE(fmt, ...) PRINT_LOG(ERROR, Error, fmt, ##__VA_ARGS__)
#define ANS_LOGF(fmt, ...) PRINT_LOG(FATAL, Fatal, fmt, ##__VA_ARGS__)
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_LOG_HELPER_H