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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SAM_LOG_H_
#define SERVICES_SAMGR_NATIVE_INCLUDE_SAM_LOG_H_

#include "hilog/log.h"

namespace OHOS {
static constexpr OHOS::HiviewDFX::HiLogLabel SYSTEM_ABLILITY_MGR_LABEL = {
    LOG_CORE,
    0xD001800,
    "SAMGR"
};

#ifdef HILOGF
#undef HILOGF
#endif

#ifdef HILOGE
#undef HILOGE
#endif

#ifdef HILOGW
#undef HILOGW
#endif

#ifdef HILOGI
#undef HILOGI
#endif

#ifdef HILOGD
#undef HILOGD
#endif

#define HILOGF(...) (void)OHOS::HiviewDFX::HiLog::Fatal(SYSTEM_ABLILITY_MGR_LABEL, __VA_ARGS__)
#define HILOGE(...) (void)OHOS::HiviewDFX::HiLog::Error(SYSTEM_ABLILITY_MGR_LABEL, __VA_ARGS__)
#define HILOGW(...) (void)OHOS::HiviewDFX::HiLog::Warn(SYSTEM_ABLILITY_MGR_LABEL, __VA_ARGS__)
#define HILOGI(...) (void)OHOS::HiviewDFX::HiLog::Info(SYSTEM_ABLILITY_MGR_LABEL, __VA_ARGS__)
#define HILOGD(...) (void)OHOS::HiviewDFX::HiLog::Debug(SYSTEM_ABLILITY_MGR_LABEL, __VA_ARGS__)
} // namespace OHOS

#endif // #ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SAM_LOG_H_
