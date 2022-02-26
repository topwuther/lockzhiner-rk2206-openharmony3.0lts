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

#pragma once

#include <cstdio>
#include <string>
#include <vector>

#ifndef FILE_SUBSYSTEM_DEV_ON_PC
#include "hilog/log.h"
#endif

namespace OHOS {
namespace DistributedFS {
#ifndef FILE_SUBSYSTEM_DEV_ON_PC
static constexpr int FILEIO_DOMAIN_ID = 0;
static constexpr OHOS::HiviewDFX::HiLogLabel FILEIO_LABEL = { LOG_CORE, FILEIO_DOMAIN_ID, "distributedfilejs" };

#ifdef HILOGD
#undef HILOGD
#endif

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

#define HILOGD(fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Debug(OHOS::DistributedFS::FILEIO_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGI(fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Info(OHOS::DistributedFS::FILEIO_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGW(fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Warn(OHOS::DistributedFS::FILEIO_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGE(fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Error(OHOS::DistributedFS::FILEIO_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGF(fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Fatal(OHOS::DistributedFS::FILEIO_LABEL, "%{public}s: " fmt, __func__, ##__VA_ARGS__)

#else

#define PCLOG(fmt, ...)                                                  \
    do {                                                                 \
        const std::vector<std::string> filter = {                        \
            "{public}",                                                  \
            "{private}",                                                 \
        };                                                               \
        std::string str____(fmt);                                        \
        for (auto &&pattern : filter) {                                  \
            size_t pos = 0;                                              \
            while (std::string::npos != (pos = str____.find(pattern))) { \
                str____.erase(pos, pattern.length());                    \
            }                                                            \
        }                                                                \
        str____ += "\n";                                                 \
        printf(str____.c_str(), ##__VA_ARGS__);                          \
    } while (0);

#define HILOGD(fmt, ...) PCLOG("%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGI(fmt, ...) PCLOG("%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGW(fmt, ...) PCLOG("%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGE(fmt, ...) PCLOG("%{public}s: " fmt, __func__, ##__VA_ARGS__)
#define HILOGF(fmt, ...) PCLOG("%{public}s: " fmt, __func__, ##__VA_ARGS__)

#endif
} // namespace DistributedFS
} // namespace OHOS