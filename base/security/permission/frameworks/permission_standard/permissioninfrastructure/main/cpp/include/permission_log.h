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

#ifndef PERMISSION_LOG_H
#define PERMISSION_LOG_H

#ifdef HILOG_ENABLE

#include "hilog/log.h"

#ifndef __cplusplus

#define PERMISSION_LOG_DEBUG(fmt, ...) HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_INFO(fmt, ...) HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_WARN(fmt, ...) HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_ERROR(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_FATAL(fmt, ...) HILOG_FATAL(LOG_CORE, fmt, ##__VA_ARGS__)

#else

#define PERMISSION_LOG_DEBUG(label, fmt, ...) OHOS::HiviewDFX::HiLog::Debug(label, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_INFO(label, fmt, ...) OHOS::HiviewDFX::HiLog::Info(label, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_WARN(label, fmt, ...) OHOS::HiviewDFX::HiLog::Warn(label, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_ERROR(label, fmt, ...) OHOS::HiviewDFX::HiLog::Error(label, fmt, ##__VA_ARGS__)
#define PERMISSION_LOG_FATAL(label, fmt, ...) OHOS::HiviewDFX::HiLog::Fatal(label, fmt, ##__VA_ARGS__)

#endif // __cplusplus

/* define LOG_TAG as "security_*" at your submodule, * means your submodule name such as "security_dac" */
#undef LOG_TAG
#undef LOG_DOMAIN

static constexpr unsigned int SECURITY_DOMAIN_PERMISSION = 0xD002F01;

#else

#include <stdarg.h>
#include <stdio.h>

/* define LOG_TAG as "security_*" at your submodule, * means your submodule name such as "security_dac" */
#undef LOG_TAG

#define PERMISSION_LOG_DEBUG(fmt, ...) printf("[%s] debug: %s: " fmt "\n", LOG_TAG, __func__, ##__VA_ARGS__)
#define PERMISSION_LOG_INFO(fmt, ...) printf("[%s] info: %s: " fmt "\n", LOG_TAG, __func__, ##__VA_ARGS__)
#define PERMISSION_LOG_WARN(fmt, ...) printf("[%s] warn: %s: " fmt "\n", LOG_TAG, __func__, ##__VA_ARGS__)
#define PERMISSION_LOG_ERROR(fmt, ...) printf("[%s] error: %s: " fmt "\n", LOG_TAG, __func__, ##__VA_ARGS__)
#define PERMISSION_LOG_FATAL(fmt, ...) printf("[%s] fatal: %s: " fmt "\n", LOG_TAG, __func__, ##__VA_ARGS__)

#endif // HILOG_ENABLE

#endif // PERMISSION_LOG_H
