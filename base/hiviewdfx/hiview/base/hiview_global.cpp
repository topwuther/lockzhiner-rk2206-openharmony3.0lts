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
#include "hiview_global.h"
#include <mutex>
namespace OHOS {
namespace HiviewDFX {
namespace {
std::unique_ptr<HiviewGlobal>& GetOrSetGlobalReference(std::unique_ptr<HiviewGlobal> ref)
{
    static std::unique_ptr<HiviewGlobal> globalRef;
    if ((globalRef == nullptr) && (ref != nullptr)) {
        globalRef = std::move(ref);
    }
    return globalRef;
}
}

void HiviewGlobal::CreateInstance(HiviewContext& context)
{
    static std::once_flag flag;
    std::call_once(flag, [&] {
        auto globalRef = std::make_unique<HiviewGlobal>(context);
        GetOrSetGlobalReference(std::move(globalRef));
    });
}

// maybe null reference, check before use
std::unique_ptr<HiviewGlobal>& HiviewGlobal::GetInstance()
{
    return GetOrSetGlobalReference(nullptr);
}

std::string HiviewGlobal::GetHiViewDirectory(HiviewContext::DirectoryType type) const
{
    return context_.GetHiViewDirectory(type);
}

std::string HiviewGlobal::GetHiviewProperty(const std::string& key, const std::string& defaultValue)
{
    return context_.GetHiviewProperty(key, defaultValue);
}

bool HiviewGlobal::SetHiviewProperty(const std::string& key, const std::string& value, bool forceUpdate)
{
    return context_.SetHiviewProperty(key, value, forceUpdate);
}

void HiviewGlobal::PostAsyncEventToTarget(const std::string &targetPlugin, std::shared_ptr<Event> event)
{
    context_.PostAsyncEventToTarget(nullptr, targetPlugin, event);
}

bool HiviewGlobal::PostSyncEventToTarget(const std::string &targetPlugin, std::shared_ptr<Event> event)
{
    return context_.PostSyncEventToTarget(nullptr, targetPlugin, event);
}

void HiviewGlobal::PostUnorderedEvent(std::shared_ptr<Event> event)
{
    context_.PostUnorderedEvent(nullptr, event);
}
} // namespace HiviewDFX
} // namespace OHOS