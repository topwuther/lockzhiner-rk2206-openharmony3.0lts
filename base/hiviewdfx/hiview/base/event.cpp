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

#include "event.h"
#include "audit.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
void Event::SetValue(const std::string &name, const std::string &value)
{
    if (name.length() > 0) {
        // force update original value
        bundle_[name] = value;
    }
}

const std::string Event::GetValue(const std::string &name) const
{
    auto it = bundle_.find(name);
    if (it != bundle_.end()) {
        return it->second;
    }
    return std::string("");
}

void Event::SetValue(const std::string &name, int32_t value)
{
    std::string str = std::to_string(value);
    if (name.length() > 0) {
        // force update original value
        bundle_[name] = str;
    }
}

int32_t Event::GetIntValue(const std::string &name) const
{
    auto it = bundle_.find(name);
    if (it != bundle_.end()) {
        const int decimal = 10;
        int32_t ret = static_cast<int32_t>(std::strtol(it->second.c_str(), nullptr, decimal));
        return (errno == ERANGE) ? -1 : ret;
    }
    return -1;
}

void Event::ResetTimestamp()
{
    createTime_ = TimeUtil::GenerateTimestamp();
}

void Event::SetKeyValuePairs(std::map<std::string, std::string> keyValuePairs)
{
    bundle_.insert(keyValuePairs.begin(), keyValuePairs.end());
}

std::map<std::string, std::string> Event::GetKeyValuePairs() const
{
    return bundle_;
}

void EventListener::AddListenerInfo(uint32_t type, const EventIdRange& range)
{
    auto it = listenerInfo_.find(type);
    if (it != listenerInfo_.end()) {
        it->second.insert(range);
        return;
    }

    std::set<EventIdRange> listenerSet;
    listenerSet.insert(range);
    listenerInfo_[type] = std::move(listenerSet);
}

void EventListener::AddListenerInfo(uint32_t type, const std::set<EventIdRange> &listenerInfo)
{
    auto it = listenerInfo_.find(type);
    if (it != listenerInfo_.end()) {
        it->second.insert(listenerInfo.begin(), listenerInfo.end());
        return;
    }
    listenerInfo_[type] = listenerInfo;
}

bool EventListener::GetListenerInfo(uint32_t type, std::set<EventIdRange> &listenerInfo)
{
    auto it = listenerInfo_.find(type);
    if (it != listenerInfo_.end()) {
        listenerInfo.clear();
        listenerInfo.insert(it->second.begin(), it->second.end());
        return true;
    }
    return false;
}

void EventListener::AddListenerInfo(uint32_t type, const std::string& eventName)
{
    auto it = strListenerInfo_.find(type);
    if (it != strListenerInfo_.end()) {
        it->second.insert(eventName);
        return;
    }

    std::set<std::string> listenNamesSet;
    listenNamesSet.insert(eventName);
    strListenerInfo_[type] = std::move(listenNamesSet);
}

void EventListener::AddListenerInfo(uint32_t type, const std::set<std::string> &eventNames)
{
    auto it = strListenerInfo_.find(type);
    if (it != strListenerInfo_.end()) {
        it->second.insert(eventNames.begin(), eventNames.end());
        return;
    }
    strListenerInfo_[type] = eventNames;
}

bool EventListener::GetListenerInfo(uint32_t type, std::set<std::string> &eventNames)
{
    auto it = strListenerInfo_.find(type);
    if (it != strListenerInfo_.end()) {
        eventNames.clear();
        eventNames.insert(it->second.begin(), it->second.end());
        return true;
    }
    return false;
}
}
}
