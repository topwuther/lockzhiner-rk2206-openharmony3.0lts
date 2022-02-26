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

#ifndef DDS_TRACE_H
#define DDS_TRACE_H

#include <string>
#include <atomic>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class DdsTrace {
public:
    KVSTORE_API DdsTrace(const std::string &value, bool isSend = false);
    KVSTORE_API ~DdsTrace();
    KVSTORE_API void SetMiddleTrace(const std::string &beforeValue, const std::string &afterValue);

private:
    void Start(const std::string &value);
    void Middle(const std::string &beforeValue, const std::string &afterValue);
    void Finish(const std::string &value);
    bool SetBytraceEnable();

    enum SwitchOption {
        DEBUG_CLOSE = 0x00,
        BYTRACE_ON = 0x01,
        API_PERFORMANCE_TRACE_ON = 0x02,
    };
    static std::atomic_uint switchOption;
    static std::atomic_uint indexCount;
    static std::atomic_bool isSetBytraceEnabled;
    std::string traceValue{ };
    uint64_t lastTime{ 0 };
    uint32_t traceCount{ 0 };
    bool isSendToHiview{ false };
};
} // namespace OHOS
} // namespace DistributedKv
#endif
