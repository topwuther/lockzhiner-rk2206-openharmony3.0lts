/*
 * Copyright (c) 2021 Huawei Device Co., Ltd. 2021-2021.
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
#ifndef STORAGE_SERVICE_FILESYSTEM_LOG_H
#define STORAGE_SERVICE_FILESYSTEM_LOG_H

#include "hilog/log.h"
namespace OHOS {
static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = { LOG_CORE, 0, "fs_manager" };

#define SSLOG_F(...) (void)OHOS::HiviewDFX::HiLog::Fatal(LOG_LABEL, __VA_ARGS__)
#define SSLOG_E(...) (void)OHOS::HiviewDFX::HiLog::Error(LOG_LABEL, __VA_ARGS__)
#define SSLOG_W(...) (void)OHOS::HiviewDFX::HiLog::Warn(LOG_LABEL, __VA_ARGS__)
#define SSLOG_I(...) (void)OHOS::HiviewDFX::HiLog::Info(LOG_LABEL, __VA_ARGS__)
#define SSLOG_D(...) (void)OHOS::HiviewDFX::HiLog::Debug(LOG_LABEL, __VA_ARGS__)

#define _SS_DFUNCTION HiviewDFX::HiLog::Info
#define _SS_IFUNCTION HiviewDFX::HiLog::Info
#define _SS_WFUNCTION HiviewDFX::HiLog::Warn
#define _SS_EFUNCTION HiviewDFX::HiLog::Error

#define _SS_LOGCPRINTF(func, fmt, ...) func(LABEL, "<%{public}d>" fmt, __LINE__, ##__VA_ARGS__)

#define SSLOGD(fmt, ...) _SS_LOGCPRINTF(_SS_DFUNCTION, " DEBUG " fmt, ##__VA_ARGS__)
#define SSLOGI(fmt, ...) _SS_LOGCPRINTF(_SS_IFUNCTION, fmt, ##__VA_ARGS__)
#define SSLOGW(fmt, ...) _SS_LOGCPRINTF(_SS_WFUNCTION, fmt, ##__VA_ARGS__)
#define SSLOGE(fmt, ...) _SS_LOGCPRINTF(_SS_EFUNCTION, fmt, ##__VA_ARGS__)

#define _SS_FUNCTION __func__

#define SSLOGFD(fmt, ...) SSLOGD("%{public}s: " fmt, _SS_FUNCTION, ##__VA_ARGS__)
#define SSLOGFI(fmt, ...) SSLOGI("%{public}s: " fmt, _SS_FUNCTION, ##__VA_ARGS__)
#define SSLOGFW(fmt, ...) SSLOGW("%{public}s: " fmt, _SS_FUNCTION, ##__VA_ARGS__)
#define SSLOGFE(fmt, ...) SSLOGE("%{public}s: " fmt, _SS_FUNCTION, ##__VA_ARGS__)
} // namespace OHOS

#endif // STORAGE_SERVICE_FILESYSTEM_LOG_H