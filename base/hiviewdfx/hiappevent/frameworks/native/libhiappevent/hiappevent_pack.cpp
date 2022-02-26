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

#include "hiappevent_pack.h"

#include <memory>
#include <vector>

#include "hiappevent_base.h"
#include "hiappevent_write.h"

namespace OHOS {
namespace HiviewDFX {
std::shared_ptr<AppEventPack> CreateEventPack(const std::string& eventName, int eventType)
{
    return std::make_shared<AppEventPack>(eventName, eventType);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, bool b)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, b);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, char c)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, c);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, short s)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, s);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, int i)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, i);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, long l)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, l);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, long long l)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, l);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, float f)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, f);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, double d)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, d);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const char *s)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, s);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::string& s)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, s);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<bool>& bs)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, bs);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<char>& cs)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, cs);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<short>& shs)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, shs);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<int>& is)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, is);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<long>& ls)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, ls);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<long long>& lls)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, lls);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<float>& fs)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, fs);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::vector<double>& ds)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, ds);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<const char*>& cps)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, cps);
}

void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<const std::string>& strs)
{
    if (appEventPack == nullptr) {
        return;
    }
    appEventPack->AddParam(key, strs);
}
} // HiviewDFX
} // OHOS