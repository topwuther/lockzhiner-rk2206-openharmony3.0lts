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

#ifndef SERVICES_SAFWK_NATIVE_INCLUDE_SAM_LOG_H_
#define SERVICES_SAFWK_NATIVE_INCLUDE_SAM_LOG_H_

#include "hilog/log.h"

namespace OHOS {
static constexpr OHOS::HiviewDFX::HiLogLabel SAFWK_LABEL = {
    LOG_CORE,
    0xD001800,
    "SAFWK"
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

#define SAFWK_LOG(level, TAG, fmt, ...) \
    HiviewDFX::HiLog::level(SAFWK_LABEL, "%{public}s::%{public}s " fmt, TAG.c_str(), __FUNCTION__, ##__VA_ARGS__)

#define HILOGF(TAG, fmt, ...) SAFWK_LOG(Fatal, TAG, fmt, ##__VA_ARGS__)
#define HILOGE(TAG, fmt, ...) SAFWK_LOG(Error, TAG, fmt, ##__VA_ARGS__)
#define HILOGW(TAG, fmt, ...) SAFWK_LOG(Warn,  TAG, fmt, ##__VA_ARGS__)
#define HILOGI(TAG, fmt, ...) SAFWK_LOG(Info,  TAG, fmt, ##__VA_ARGS__)
#define HILOGD(TAG, fmt, ...) SAFWK_LOG(Debug, TAG, fmt, ##__VA_ARGS__)
}

#endif
