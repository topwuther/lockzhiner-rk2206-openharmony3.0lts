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

#ifndef HIVIEWDFX_HITRACE_ID_H
#define HIVIEWDFX_HITRACE_ID_H

#include "hitrace/hitracec.h"

#ifdef __cplusplus

namespace OHOS {
namespace HiviewDFX {
class HiTraceId final {
public:
    HiTraceId();
    HiTraceId(const HiTraceIdStruct& id);
    HiTraceId(const uint8_t* pIdArray, int len);
    ~HiTraceId() = default;

    bool IsValid() const;
    bool IsFlagEnabled(HiTraceFlag flag) const;
    void EnableFlag(HiTraceFlag flag);
    int GetFlags() const;
    void SetFlags(int flags);
    uint64_t GetChainId() const;
    void SetChainId(uint64_t chainId);
    uint64_t GetSpanId() const;
    void SetSpanId(uint64_t spanId);
    uint64_t GetParentSpanId() const;
    void SetParentSpanId(uint64_t parentSpanId);
    int ToBytes(uint8_t* pIdArray, int len) const;

private:
    HiTraceIdStruct id_;
    friend class HiTrace;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // __cplusplus

#endif // HIVIEWDFX_HITRACE_ID_H
