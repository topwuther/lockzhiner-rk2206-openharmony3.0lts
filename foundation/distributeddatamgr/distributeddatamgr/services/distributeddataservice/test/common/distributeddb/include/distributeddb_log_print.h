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
#ifndef DISTRIBUTED_DB_LOG_PRINT_H
#define DISTRIBUTED_DB_LOG_PRINT_H

#if defined USING_PRINTF_LOGGER
    #include <cstdio>
#elif defined USING_HILOG_LOGGER
    #include "hilog/log.h"
#endif

#if defined USING_PRINTF_LOGGER
    #define MST_LOG(fmt, ...) \
        (void)(std::printf(fmt"\n", ##__VA_ARGS__))
#elif defined USING_HILOG_LOGGER
    static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = { LOG_CORE, 0xD001630, "DistributedDB[TEST]" };
    #define MST_LOG(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Info(LOG_LABEL, "%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#endif
#endif
