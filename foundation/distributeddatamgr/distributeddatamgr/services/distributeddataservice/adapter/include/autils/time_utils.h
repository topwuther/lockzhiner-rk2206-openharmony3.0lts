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

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <ctime>
#include <sys/time.h>

namespace OHOS {
namespace DistributedKv {
constexpr int64_t SEC_TO_MICROSEC = 1000000;

class TimeUtils final {
public:
    // micro seconds since 1970
    static inline uint64_t CurrentTimeMicros()
    {
        struct timeval tv = { 0, 0 };
        gettimeofday(&tv, nullptr);
        return (tv.tv_sec * SEC_TO_MICROSEC + tv.tv_usec);
    }
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif