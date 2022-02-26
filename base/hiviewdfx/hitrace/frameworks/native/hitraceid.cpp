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

#include "hitrace/hitraceid.h"

namespace OHOS {
namespace HiviewDFX {

HiTraceId::HiTraceId()
{
    id_.valid = HITRACE_ID_INVALID;
    id_.ver = 0;
    id_.chainId = 0;
    id_.flags = 0;
    id_.spanId = 0;
    id_.parentSpanId = 0;
}

HiTraceId::HiTraceId(const HiTraceIdStruct& id) : id_(id)
{}

HiTraceId::HiTraceId(const uint8_t* pIdArray, int len)
{
    id_ = HiTraceBytesToId(pIdArray, len);
}

bool HiTraceId::IsValid() const
{
    return HiTraceIsValid(&id_);
}

bool HiTraceId::IsFlagEnabled(HiTraceFlag flag) const
{
    return HiTraceIsFlagEnabled(&id_, flag);
}

void HiTraceId::EnableFlag(HiTraceFlag flag)
{
    HiTraceEnableFlag(&id_, flag);
    return;
}

int HiTraceId::GetFlags() const
{
    return HiTraceGetFlags(&id_);
}

void HiTraceId::SetFlags(int flags)
{
    HiTraceSetFlags(&id_, flags);
    return;
}

uint64_t HiTraceId::GetChainId() const
{
    return HiTraceGetChainId(&id_);
}

void HiTraceId::SetChainId(uint64_t chainId)
{
    HiTraceSetChainId(&id_, chainId);
    return;
}

uint64_t HiTraceId::GetSpanId() const
{
    return HiTraceGetSpanId(&id_);
}

void HiTraceId::SetSpanId(uint64_t spanId)
{
    HiTraceSetSpanId(&id_, spanId);
    return;
}

uint64_t HiTraceId::GetParentSpanId() const
{
    return HiTraceGetParentSpanId(&id_);
}

void HiTraceId::SetParentSpanId(uint64_t parentSpanId)
{
    HiTraceSetParentSpanId(&id_, parentSpanId);
    return;
}

int HiTraceId::ToBytes(uint8_t* pIdArray, int len) const
{
    return HiTraceIdToBytes(&id_, pIdArray, len);
}

} // namespace HiviewDFX
} // namespace OHOS