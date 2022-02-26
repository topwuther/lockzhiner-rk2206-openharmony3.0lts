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

#ifndef TIME_HILOG_WRAPPER_H
#define TIME_HILOG_WRAPPER_H

#include "hilog/log.h"

namespace OHOS {
namespace MiscServices {
// param of log interface, such as TIME_HILOGF.
enum TimeSubModule {
    TIME_MODULE_INNERKIT = 0,
    TIME_MODULE_CLIENT,
    TIME_MODULE_SERVICE,
    TIME_MODULE_JAVAKIT, // java kit, defined to avoid repeated use of domain.
    TIME_MODULE_JNI,
    TIME_MODULE_COMMON,  
    TIME_MODULE_JS_NAPI,
    TIME_MODULE_BUTT,
};

// 0xD001C00: subsystem:TIME module:TimeManager, 8 bits reserved.
static constexpr unsigned int BASE_TIME_DOMAIN_ID = 0xD001C00;

enum TimeDomainId {
    TIME_INNERKIT_DOMAIN = BASE_TIME_DOMAIN_ID + TIME_MODULE_INNERKIT,
    TIME_CLIENT_DOMAIN,
    TIME_SERVICE_DOMAIN,
    TIME_JAVAKIT_DOMAIN,
    TIME_JNI_DOMAIN,
    TIME_COMMON_DOMAIN,
    TIME_JS_NAPI,
    TIME_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel TIME_MODULE_LABEL[TIME_MODULE_BUTT] = {
    {LOG_CORE, TIME_INNERKIT_DOMAIN, "TimeInnerKit"},
    {LOG_CORE, TIME_CLIENT_DOMAIN, "TimeClient"},
    {LOG_CORE, TIME_SERVICE_DOMAIN, "TimeService"},
    {LOG_CORE, TIME_JAVAKIT_DOMAIN, "TimeJavaKit"},
    {LOG_CORE, TIME_JNI_DOMAIN, "TimeJni"},
    {LOG_CORE, TIME_COMMON_DOMAIN, "TimeCommon"},
    {LOG_CORE, TIME_JS_NAPI, "TimeJSNAPI"},
};

#define __FILENAME__            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than TIME_MODULE_BUTT.
#define TIME_HILOGF(module, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(TIME_MODULE_LABEL[module], __FORMATED(__VA_ARGS__))
#define TIME_HILOGE(module, ...) (void)OHOS::HiviewDFX::HiLog::Error(TIME_MODULE_LABEL[module], __FORMATED(__VA_ARGS__))
#define TIME_HILOGW(module, ...) (void)OHOS::HiviewDFX::HiLog::Warn(TIME_MODULE_LABEL[module], __FORMATED(__VA_ARGS__))
#define TIME_HILOGI(module, ...) (void)OHOS::HiviewDFX::HiLog::Info(TIME_MODULE_LABEL[module], __FORMATED(__VA_ARGS__))
#define TIME_HILOGD(module, ...) (void)OHOS::HiviewDFX::HiLog::Debug(TIME_MODULE_LABEL[module], __FORMATED(__VA_ARGS__))
} // namespace MiscServices
} // namespace OHOS
#endif // TIME_HILOG_WRAPPER_H
