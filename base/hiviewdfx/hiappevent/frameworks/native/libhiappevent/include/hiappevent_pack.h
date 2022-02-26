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

#ifndef HI_APP_EVENT_PACK_H
#define HI_APP_EVENT_PACK_H

#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
std::shared_ptr<AppEventPack> CreateEventPack(const std::string& eventName, int eventType);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, bool b);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, char c);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, short s);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, int i);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, long l);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, long long l);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, float f);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, double d);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const char *s);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key, const std::string& s);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<bool>& bs);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<char>& cs);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<short>& shs);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<int>& is);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<long>& ls);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<long long>& lls);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<float>& fs);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<double>& ds);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<const char*>& cps);
void AddEventParam(std::shared_ptr<AppEventPack> appEventPack, const std::string& key,
    const std::vector<const std::string>& strs);
} // HiviewDFX
} // OHOS
#endif // HI_APP_EVENT_PACK_H