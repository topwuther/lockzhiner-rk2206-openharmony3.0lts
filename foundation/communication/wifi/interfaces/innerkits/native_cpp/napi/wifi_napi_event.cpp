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

#include "wifi_napi_event.h"
#include <shared_mutex>
#include "wifi_napi_utils.h"
#include "wifi_logger.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiNAPIEvent");

const std::string WIFI_EVENT_TYPE_POWER_STATE = "wifiStateChange";
const std::string WIFI_EVENT_TYPE_CONN_STATE = "wifiConnectionChange";

const std::string WIFI_USUAL_EVENT_POWER_STATE = "usual.event.wifi.POWER_STATE";
const std::string WIFI_USUAL_EVENT_CONN_STATE = "usual.event.wifi.CONN_STATE";

std::shared_mutex g_regInfoMutex;
static std::map<std::string, EventRegisterInfo> g_eventRegisterInfo;

static std::map<std::string, std::string> g_mapEventTypeToUsualEvent = {
    { WIFI_EVENT_TYPE_POWER_STATE, WIFI_USUAL_EVENT_POWER_STATE },
    { WIFI_EVENT_TYPE_CONN_STATE, WIFI_USUAL_EVENT_CONN_STATE },
};

static std::map<std::string, UserDefinedEventProcessFunc> g_mapUserDefinedEventProcessFunc = {};

class EventRegisterInfo {
public:
    explicit EventRegisterInfo(EventManager* context) : m_context(context) {
    }

    EventRegisterInfo() {
    }

    virtual ~EventRegisterInfo() {
    }

    std::set<napi_ref>& GetHandlersCb() {
        return m_handlersCb;
    }

    void SetSubscriber(std::shared_ptr<WifiEventSubscriber>& subscriber) {
        m_subscriber = subscriber;
    }

    std::shared_ptr<WifiEventSubscriber> GetSubscriber() {
        return m_subscriber;
    }

    void SetContext(EventManager* context) {
        m_context = context;
    }

    EventManager* GetContext() {
        return m_context;
    }

private:
    std::set<napi_ref> m_handlersCb;
    std::shared_ptr<WifiEventSubscriber> m_subscriber;
    EventManager* m_context;
};

void Event::SetName(std::string& name) {
    m_name = name;
}

std::string Event::GetName() {
    return m_name;
}

napi_env Event::GetEnv() {
    return m_env;
}

napi_value WifiCommonEvent::PackResult() {
    napi_value result;
    napi_create_int32(GetEnv(), m_value, &result);
    return result;
}

static bool GetUsualEventByEventType(const std::string& type, std::string& usual) {
    std::map<std::string, std::string>::const_iterator it = g_mapEventTypeToUsualEvent.find(type);
    if (it == g_mapEventTypeToUsualEvent.end()) {
        return false;
    }
    usual = it->second;
    return true;
}

static bool GetEventTypeByUsualEvent(const std::string& usual, std::string& type) {
    for (auto& each : g_mapEventTypeToUsualEvent) {
        if (each.second == usual) {
            type = each.first;
            return true;
        }
    }
    return false;
}

static bool IsEventTypeExist(const std::string& type) {
    return g_mapEventTypeToUsualEvent.find(type) != g_mapEventTypeToUsualEvent.end();
}

void WifiEventSubscriber::OnReceiveEvent(const CommonEventData& data) {
    std::string event = data.GetWant().GetAction();
    int code = data.GetCode();
    WIFI_LOGI("[Napi Event] Received event: %{public}s, value: %{public}d", event.c_str(), code);

    std::string type;
    if (!GetEventTypeByUsualEvent(event, type)) {
        WIFI_LOGI("[Napi Event] Received event: %{public}s is ignored", event.c_str());
        return;
    }

    EventManager* manager = nullptr;
    {
        std::shared_lock<std::shared_mutex> guard(g_regInfoMutex);
        std::map<std::string, EventRegisterInfo>::iterator it = g_eventRegisterInfo.find(type);
        if (it == g_eventRegisterInfo.end()) {
            WIFI_LOGE("[Napi Event] no register info for event: %{public}s", type.c_str());
            return;
        }
        manager = it->second.GetContext();
        if (manager == nullptr) {
            WIFI_LOGE("[Napi Event] Context is null");
            return;
        }
    }

    std::map<std::string, UserDefinedEventProcessFunc>::iterator iter = g_mapUserDefinedEventProcessFunc.find(type);
    if (iter != g_mapUserDefinedEventProcessFunc.end()) {
        WIFI_LOGI("[Napi Event] Has user-defined func for event: %{public}s", type.c_str());
        iter->second(manager->GetEnv(), type, data);
    } else {
        WIFI_LOGI("[Napi Event] Use default policy to process event: %{public}s", type.c_str());
        WifiCommonEvent commonEvent(manager->GetEnv(), type, code);
        if (!manager->Send(commonEvent)) {
            WIFI_LOGE("[Napi Event] Send event error");
        }
    }
}

EventManager::EventManager(napi_env env, napi_value thisVar) : m_env(env) {
    m_thisVarRef = nullptr;
    napi_create_reference(env, thisVar, 1, &m_thisVarRef);
}

EventManager::~EventManager() {}

bool EventManager::Send(Event& event) {
    WIFI_LOGI("[Napi Event] Report event: %{public}s", event.GetName().c_str());

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(m_env, &scope);

    std::shared_lock<std::shared_mutex> guard(g_regInfoMutex);
    std::map<std::string, EventRegisterInfo>::iterator it = g_eventRegisterInfo.find(event.GetName());
    if (it == g_eventRegisterInfo.end()) {
        WIFI_LOGE("[Napi Event] Event receive owner not exits: %{public}s", event.GetName().c_str());
        return false;
    }

    bool result = true;
    napi_value thisVar = nullptr;
    napi_get_reference_value(m_env, m_thisVarRef, &thisVar);
    for (auto& each : it->second.GetHandlersCb()) {
        napi_value undefine;
        napi_value handler = nullptr;
        napi_get_undefined(m_env, &undefine);
        napi_get_reference_value(m_env, each, &handler);
        napi_value jsEvent = event.PackResult();
        if (napi_call_function(m_env, thisVar, handler, 1, &jsEvent, &undefine) != napi_ok) {
            WIFI_LOGE("[Napi Event] Report event failed");
            result = false;
        }
    }
    napi_close_handle_scope(m_env, scope);
    return result;
}

bool EventManager::SubscribeServiceEvent(const std::string& event) {
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<WifiEventSubscriber> subscriber = std::make_shared<WifiEventSubscriber>(subscriberInfo);
    WIFI_LOGI("[Napi Event] Subscribe event -> %{public}s", event.c_str());
    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);
    if (subscribeResult) {
        g_eventRegisterInfo[m_eventType].SetSubscriber(subscriber);
    } else {
        WIFI_LOGE("[Napi Event] Subscribe service event error: %{public}s", event.c_str());
    }
    return subscribeResult;
}

bool EventManager::UnsubscribeServiceEvent(const std::string& event) {
    bool unsubscribeResult = CommonEventManager::SubscribeCommonEvent(g_eventRegisterInfo[m_eventType].GetSubscriber());
    if (!unsubscribeResult) {
        WIFI_LOGE("[Napi Event] Unsubscribe service event error: %{public}s", event.c_str());
    }
    return unsubscribeResult;
}

bool EventManager::SubscribeEvent(const std::string& name, napi_value handler) {
    WIFI_LOGI("[Napi Event] Subscribe event: %{public}s", name.c_str());

    if (!IsEventTypeExist(name)) {
        WIFI_LOGE("[Napi Event] Subscribe event is not a valid event: %{public}s", name.c_str());
        return false;
    }
    SetEventType(name);
    std::unique_lock<std::shared_mutex> guard(g_regInfoMutex);
    std::map<std::string, EventRegisterInfo>::iterator it = g_eventRegisterInfo.find(name);
    if (it == g_eventRegisterInfo.end()) {
        std::string usualEvent;
        GetUsualEventByEventType(name, usualEvent);
        bool result = SubscribeServiceEvent(usualEvent);
        if (!result) {
            WIFI_LOGE("[Napi Event] Service register event failed: %{public}s", name.c_str());
            return false;
        }

        EventRegisterInfo regInfo(this);
        g_eventRegisterInfo[name] = regInfo;
    }

    if (g_eventRegisterInfo[name].GetContext() != this) {
        WIFI_LOGW("[Napi Event] Subscribe event context changed!");
        g_eventRegisterInfo[name].SetContext(this);
    }

    napi_ref handlerRef = nullptr;
    napi_create_reference(m_env, handler, 1, &handlerRef);
    g_eventRegisterInfo[name].GetHandlersCb().insert(handlerRef);
    return true;
}

void EventManager::DeleteHanderRef(std::set<napi_ref>& setRefs, napi_value handler) {
    for (auto& each : setRefs) {
        napi_value handlerTemp = nullptr;
        napi_get_reference_value(m_env, each, &handlerTemp);
        bool isEqual = false;
        napi_strict_equals(m_env, handlerTemp, handler, &isEqual);
        if (isEqual) {
            napi_delete_reference(m_env, each);
            setRefs.erase(each);
            return;
        }
    }
}

void EventManager::DeleteAllHanderRef(std::set<napi_ref>& setRefs) {
    for (auto& each : setRefs) {
        napi_delete_reference(m_env, each);
    }
    setRefs.clear();
}

bool EventManager::UnsubscribeEvent(const std::string& name, napi_value handler) {
    WIFI_LOGI("[Napi Event] Unsubscribe event: %{public}s", name.c_str());

    if (!IsEventTypeExist(name)) {
        WIFI_LOGE("[Napi Event] Unsubscribe event is not a valid event: %{public}s", name.c_str());
        return false;
    }

    bool isNeedUnsubscribe = false;
    std::unique_lock<std::shared_mutex> guard(g_regInfoMutex);
    std::map<std::string, EventRegisterInfo>::iterator it = g_eventRegisterInfo.find(name);
    if (it == g_eventRegisterInfo.end()) {
        WIFI_LOGE("[Napi Event] Unsubscribe event is not subscribe: %{public}s", name.c_str());
        return false;
    }
    if (handler != nullptr) {
        DeleteHanderRef(it->second.GetHandlersCb(), handler);
    } else {
        WIFI_LOGW("[Napi Event] All callback is unsubscribe for event: %{public}s", name.c_str());
        DeleteAllHanderRef(it->second.GetHandlersCb());
    }
    /* No one subscribes event now */
    if (it->second.GetHandlersCb().empty()) {
        isNeedUnsubscribe = true;
    }

    SetEventType(name);
    if (isNeedUnsubscribe) {
        std::string usualEvent;
        GetUsualEventByEventType(name, usualEvent);
        bool result = UnsubscribeServiceEvent(usualEvent);
        g_eventRegisterInfo.erase(name);
        if (!result) {
            WIFI_LOGE("[Napi Event] Service unregister event failed: %{public}s", name.c_str());
            return false;
        }
    }
    return true;
}

void EventManager::SetEventType(const std::string& type) {
    m_eventType = type;
}

napi_env EventManager::GetEnv() {
    return m_env;
}

napi_value On(napi_env env, napi_callback_info cbinfo) {
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

    napi_valuetype eventName = napi_undefined;
    napi_typeof(env, argv[0], &eventName);
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, argv[1], &handler);
    NAPI_ASSERT(env, handler == napi_function, "type mismatch for parameter 2");

    EventManager* manager = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&manager);
    if (status == napi_ok) {
        char type[64] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);
        manager->SubscribeEvent(type, argv[1]);
    } else {
        WIFI_LOGE("[Napi Event] On unwrap class failed");
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value Off(napi_env env, napi_callback_info cbinfo) {
    size_t requireArgc = 1;
    size_t requireArgcWithCb = 2;
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    NAPI_ASSERT(env, argc >= requireArgc, "requires at least 1 parameter");

    napi_valuetype eventName = napi_undefined;
    napi_typeof(env, argv[0], &eventName);
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");

    if (argc >= requireArgcWithCb) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, argv[1], &handler);
        NAPI_ASSERT(env, handler == napi_function, "type mismatch for parameter 2");
    }

    EventManager* manager = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&manager);
    if (status == napi_ok) {
        char type[64] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);
        manager->UnsubscribeEvent(type, argc >= requireArgcWithCb ? argv[1] : nullptr);
    } else {
        WIFI_LOGE("[Napi Event] Off unwrap class failed");
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value EventListenerConstructor(napi_env env, napi_callback_info cbinfo) {
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    EventManager* eventManager = new EventManager(env, thisVar);
    WIFI_LOGI("[Napi Event] Event listener constructor");
    napi_wrap(
        env, thisVar, eventManager,
        [](napi_env env, void* data, void* hint) {
            WIFI_LOGI("[Napi Event] Event listener destructor");
            EventManager* eventManager = (EventManager *)data;
            delete eventManager;
        },
        nullptr, nullptr);
    return thisVar;
}
}  // namespace Wifi
}  // namespace OHOS
