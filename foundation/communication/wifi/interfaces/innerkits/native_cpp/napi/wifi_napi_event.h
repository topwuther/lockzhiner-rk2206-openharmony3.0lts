/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef WIFI_NAPI_EVENT_H_
#define WIFI_NAPI_EVENT_H_

#include <string>
#include <set>
#include <map>
#include "napi/native_api.h"
#include "common_event_manager.h"
#include "common_event.h"

namespace OHOS {
namespace Wifi {

typedef void (*UserDefinedEventProcessFunc)(const napi_env& env, const std::string& type,
    const OHOS::EventFwk::CommonEventData& data);

class Event {
public:
    Event(napi_env env, std::string& name) : m_env(env), m_name(name) {
    }

    virtual ~Event() {
    }

    virtual napi_value PackResult() = 0;

    void SetName(std::string& name);

    std::string GetName();

    napi_env GetEnv();

private:
    napi_env m_env;
    std::string m_name;
};

class WifiCommonEvent: public Event {
public:
    WifiCommonEvent(napi_env env, std::string& name, int value) : Event(env, name), m_value(value) {
    }

    virtual napi_value PackResult();

private:
    int m_value;
};

class WifiEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit WifiEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo) :
        CommonEventSubscriber(subscribeInfo) {
    }

    virtual ~WifiEventSubscriber() {
    }

    virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class EventRegisterInfo;
class EventManager {
public:
    EventManager(napi_env env, napi_value thisVar);
    virtual ~EventManager();

    bool Send(Event& event);
    bool SubscribeEvent(const std::string& name, napi_value handler);
    bool UnsubscribeEvent(const std::string& name, napi_value handler);
    napi_env GetEnv();

private:
    bool SubscribeServiceEvent(const std::string& event);
    bool UnsubscribeServiceEvent(const std::string& event);
    void DeleteHanderRef(std::set<napi_ref>& setRefs, napi_value handler);
    void DeleteAllHanderRef(std::set<napi_ref>& setRefs);
    void SetEventType(const std::string& type);

private:
    napi_env m_env;
    napi_ref m_thisVarRef;
    std::string m_eventType;
};

napi_value On(napi_env env, napi_callback_info cbinfo);
napi_value Off(napi_env env, napi_callback_info cbinfo);
napi_value EventListenerConstructor(napi_env env, napi_callback_info cbinfo);
}  // namespace Wifi
}  // namespace OHOS

#endif
