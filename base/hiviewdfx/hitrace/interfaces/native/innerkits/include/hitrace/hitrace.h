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

#ifndef HIVIEWDFX_HITRACE_CPP_H
#define HIVIEWDFX_HITRACE_CPP_H

#include "hitrace/hitrace.h"
#include "hitrace/hitraceid.h"

#ifdef __cplusplus

#include <string>

namespace OHOS {
namespace HiviewDFX {
class HiTrace final {
public:
    static HiTraceId Begin(const std::string& name, int flags);
    static void End(const HiTraceId& id);
    static HiTraceId GetId();
    static void SetId(const HiTraceId& id);
    static void ClearId();
    static HiTraceId CreateSpan();
    static void Tracepoint(HiTraceTracepointType type, const HiTraceId& id, const char* fmt, ...)
        __attribute__((__format__(os_log, 3, 4)));
    static void Tracepoint(HiTraceCommunicationMode mode, HiTraceTracepointType type, const HiTraceId& id,
        const char* fmt, ...) __attribute__((__format__(os_log, 4, 5)));
private:
    HiTrace() = default;
    ~HiTrace() = default;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // __cplusplus

#endif // HIVIEWDFX_HITRACE_CPP_H
