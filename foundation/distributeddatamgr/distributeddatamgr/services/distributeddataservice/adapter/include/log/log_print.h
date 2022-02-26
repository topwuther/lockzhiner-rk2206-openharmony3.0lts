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

#ifndef DISTRIBUTEDDATA_LOG_PRINT_H
#define DISTRIBUTEDDATA_LOG_PRINT_H

#ifndef KVSTORE_API
#define KVSTORE_API __attribute__ ((visibility ("default")))
#endif
#define OS_OHOS
#if defined OS_OHOS // log for OHOS

#include "hilog/log.h"
namespace OHOS {
namespace DistributedKv {

KVSTORE_API static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = { LOG_CORE, 0xD001610, "ZDDS" };

} // end namespace DistributesdKv

namespace AppDistributedKv {

KVSTORE_API static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = { LOG_CORE, 0xD001620, "ZDDC" };

} // end namespace AppDistributesdKv
} // end namespace OHOS

#define ZLOGD(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Debug(LOG_LABEL, LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGI(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Info(LOG_LABEL, LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGW(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Warn(LOG_LABEL, LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGE(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Error(LOG_LABEL, LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#else
    #error // unknown system
#endif

#endif // DISTRIBUTEDDATA_LOG_PRINT_H
