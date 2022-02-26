/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PREFERENCES_JSKIT_COMMON_H
#define PREFERENCES_JSKIT_COMMON_H

#include <string>
#include <vector>

#include "hilog/log.h"
#include "preferences_errno.h"

namespace OHOS {
namespace PreferencesJsKit {
static const OHOS::HiviewDFX::HiLogLabel PREFIX_LABEL = { LOG_CORE, 0xD001650, "PreferencesJsKit" };

#define LOG_DEBUG(...) ((void)OHOS::HiviewDFX::HiLog::Debug(PREFIX_LABEL, __VA_ARGS__))
#define LOG_INFO(...) ((void)OHOS::HiviewDFX::HiLog::Info(PREFIX_LABEL, __VA_ARGS__))
#define LOG_WARN(...) ((void)OHOS::HiviewDFX::HiLog::Warn(PREFIX_LABEL, __VA_ARGS__))
#define LOG_ERROR(...) ((void)OHOS::HiviewDFX::HiLog::Error(PREFIX_LABEL, __VA_ARGS__))
#define LOG_FATAL(...) ((void)OHOS::HiviewDFX::HiLog::Fatal(PREFIX_LABEL, __VA_ARGS__))

static const std::vector<std::string> ERR_INFO {
    "E_ERROR",
    "E_STALE",
    "E_INVALID_ARGS",
    "E_OUT_OF_MEMORY",
    "E_NOT_PERMIT",
    "E_KEY_EMPTY",
    "E_KEY_EXCEED_MAX_LENGTH",
    "E_PTR_EXIST_ANOTHER_HOLD",
    "E_DELETE_FILE_FAIL",
    "E_EMPTY_FILE_PATH",
    "E_RELATIVE_PATH",
    "E_EMPTY_FILE_NAME",
    "E_INVALID_FILE_PATH",
    "E_PATH_EXCEED_MAX_LENGTH"
};

static inline const char *GetErrStr(int err)
{
    if (err == OHOS::NativePreferences::E_OK) {
        return nullptr;
    }
    size_t index = err - OHOS::NativePreferences::E_BASE - 1;
    if (index >= ERR_INFO.size() || index < 0) {
        return "Unknown error";
    }
    return ERR_INFO.at(index).c_str();
}
} // namespace PreferencesJsKit
} // namespace OHOS

#endif
