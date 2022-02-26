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

#ifndef RELIABILITY_XCOLLIE_UTILS_H
#define RELIABILITY_XCOLLIE_UTILS_H

#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
static constexpr OHOS::HiviewDFX::HiLogLabel g_XCOLLIE_LOG_LABEL = {LOG_CORE, 0xD002D06, "XCollie"};
#define XCOLLIE_LOGF(...) (void)OHOS::HiviewDFX::HiLog::Fatal(g_XCOLLIE_LOG_LABEL, ##__VA_ARGS__)
#define XCOLLIE_LOGE(...) (void)OHOS::HiviewDFX::HiLog::Error(g_XCOLLIE_LOG_LABEL, ##__VA_ARGS__)
#define XCOLLIE_LOGW(...) (void)OHOS::HiviewDFX::HiLog::Warn(g_XCOLLIE_LOG_LABEL, ##__VA_ARGS__)
#define XCOLLIE_LOGI(...) (void)OHOS::HiviewDFX::HiLog::Info(g_XCOLLIE_LOG_LABEL, ##__VA_ARGS__)
#define XCOLLIE_LOGD(...) (void)OHOS::HiviewDFX::HiLog::Debug(g_XCOLLIE_LOG_LABEL, ##__VA_ARGS__)
} // end of HiviewDFX
} // end of OHOS
#endif
