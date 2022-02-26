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

#include "hitrace/hitrace.h"
#include "hitrace_inner.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {

HiTraceId HiTrace::Begin(const string& name, int flags)
{
    return HiTraceId(::HiTraceBegin(name.c_str(), flags));
}

void HiTrace::End(const HiTraceId& id)
{
    ::HiTraceEnd(&(id.id_));
    return;
}

HiTraceId HiTrace::GetId()
{
    return HiTraceId(::HiTraceGetId());
}

void HiTrace::SetId(const HiTraceId& id)
{
    ::HiTraceSetId(&(id.id_));
    return;
}

void HiTrace::ClearId()
{
    ::HiTraceClearId();
    return;
}

HiTraceId HiTrace::CreateSpan()
{
    return HiTraceId(::HiTraceCreateSpan());
}

void HiTrace::Tracepoint(HiTraceTracepointType type, const HiTraceId& id, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ::HiTraceTracepointInner(HITRACE_CM_DEFAULT, type, &(id.id_), fmt, args);
    va_end(args);

    return;
}

void HiTrace::Tracepoint(HiTraceCommunicationMode mode, HiTraceTracepointType type, const HiTraceId& id,
    const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ::HiTraceTracepointInner(mode, type, &(id.id_), fmt, args);
    va_end(args);

    return;
}
} // namespace HiviewDFX
} // namespace OHOS