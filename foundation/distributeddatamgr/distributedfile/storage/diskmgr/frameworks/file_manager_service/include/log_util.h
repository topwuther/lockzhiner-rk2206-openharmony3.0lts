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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_LOG_UTIL_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_LOG_UTIL_H
#pragma once

#include "hilog/log.h"

namespace OHOS {
namespace FileManager {
static constexpr int FILEMANAGER_DOMAIN_ID = 0;
static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = { LOG_CORE, FILEMANAGER_DOMAIN_ID, "file_manager" };

#ifdef HILOGD
#    undef HILOGD
#endif

#ifdef HILOGF
#    undef HILOGF
#endif

#ifdef HILOGE
#    undef HILOGE
#endif

#ifdef HILOGW
#    undef HILOGW
#endif

#ifdef HILOGI
#    undef HILOGI
#endif

#define HILOGD(fmt, ...)                                                                                     \
    (void)OHOS::HiviewDFX::HiLog::Debug(LOG_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGI(fmt, ...)                                                                                     \
    (void)OHOS::HiviewDFX::HiLog::Info(LOG_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGW(fmt, ...)                                                                                     \
    (void)OHOS::HiviewDFX::HiLog::Warn(LOG_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGE(fmt, ...)                                                                                     \
    (void)OHOS::HiviewDFX::HiLog::Error(LOG_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGF(fmt, ...)                                                                                     \
    (void)OHOS::HiviewDFX::HiLog::Fatal(LOG_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
} // namespace FileManager
} // namespace OHOS
#endif