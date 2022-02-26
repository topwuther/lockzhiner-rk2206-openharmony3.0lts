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

#ifndef TIME_HELPER_H
#define TIME_HELPER_H

#include <mutex>

#include "meta_data.h"
#include "runtime_context.h"
#include "ref_object.h"

namespace DistributedDB {
class TimeHelper {
public:
    constexpr static int64_t BASE_OFFSET = 10000LL * 365LL * 24LL * 3600LL * 1000LL * 1000LL * 10L; // 10000 year 100ns

    constexpr static int64_t MAX_VALID_TIME = BASE_OFFSET * 2; // 20000 year 100ns

    static const uint64_t TO_100_NS = 10; // 1us to 100ns

    static const uint64_t MS_TO_100_NS = 10000; // 1ms to 100ns

    static const TimeStamp INVALID_TIMESTAMP = 0;

    // Get current system time
    static TimeStamp GetSysCurrentTime();

    TimeHelper();
    ~TimeHelper();

    // Init the TimeHelper
    int Initialize(const IKvDBSyncInterface *inStorage, std::shared_ptr<Metadata> &inMetadata);

    // Get TimeStamp when write data into db, export interface;
    TimeStamp GetTime();

    // Get max data time from db
    TimeStamp GetMaxDataItemTime();

    // Get local time offset
    TimeOffset GetLocalTimeOffset() const;

    // Get local time
    int SaveLocalTimeOffset(TimeOffset offset);

private:
    static std::mutex systemTimeLock_;
    static TimeStamp lastSystemTimeUs_;
    static TimeStamp currentIncCount_;
    static const uint64_t MAX_INC_COUNT = 9; // last bit from 0-9
    const IKvDBSyncInterface *storage_;
    std::shared_ptr<Metadata> metadata_;
};
} // namespace DistributedDB

#endif // TIME_HELPER_H
