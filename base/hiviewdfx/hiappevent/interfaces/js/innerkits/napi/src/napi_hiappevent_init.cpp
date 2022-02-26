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
#include "napi_hiappevent_init.h"

#include <map>
#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int FAULT_EVENT_TYPE = 1;
constexpr int STATISTIC_EVENT_TYPE = 2;
constexpr int SECURITY_EVENT_TYPE = 3;
constexpr int BEHAVIOR_EVENT_TYPE = 4;

const std::string EVENT_CLASS_NAME = "Event";
const std::string PARAM_CLASS_NAME = "Param";
const std::string EVENT_TYPE_CLASS_NAME = "EventType";

napi_value ClassConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value argv = nullptr;
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, &argv, &thisArg, &data);

    napi_value global = 0;
    napi_get_global(env, &global);

    return thisArg;
}

void InitEventTypeMap(napi_env env, std::map<const char*, napi_value>& eventTypeMap)
{
    napi_value faultEvent = nullptr;
    napi_create_int32(env, FAULT_EVENT_TYPE, &faultEvent);
    napi_value statisticEvent = nullptr;
    napi_create_int32(env, STATISTIC_EVENT_TYPE, &statisticEvent);
    napi_value securityEvent = nullptr;
    napi_create_int32(env, SECURITY_EVENT_TYPE, &securityEvent);
    napi_value behaviorEvent = nullptr;
    napi_create_int32(env, BEHAVIOR_EVENT_TYPE, &behaviorEvent);

    eventTypeMap["FAULT"] = faultEvent;
    eventTypeMap["STATISTIC"] = statisticEvent;
    eventTypeMap["SECURITY"] = securityEvent;
    eventTypeMap["BEHAVIOR"] = behaviorEvent;
}

void InitEventMap(napi_env env, std::map<const char*, napi_value>& eventMap)
{
    napi_value userLoginEvent = nullptr;
    napi_create_string_utf8(env, "hiappevent.user_login", NAPI_AUTO_LENGTH, &userLoginEvent);
    napi_value userLogoutEvent = nullptr;
    napi_create_string_utf8(env, "hiappevent.user_logout", NAPI_AUTO_LENGTH, &userLogoutEvent);
    napi_value dsStartEvent = nullptr;
    napi_create_string_utf8(env, "hiappevent.distributed_service_start", NAPI_AUTO_LENGTH, &dsStartEvent);

    eventMap["USER_LOGIN"] = userLoginEvent;
    eventMap["USER_LOGOUT"] = userLogoutEvent;
    eventMap["DISTRIBUTED_SERVICE_START"] = dsStartEvent;
}

void InitParamMap(napi_env env, std::map<const char*, napi_value>& paramMap)
{
    napi_value userIdParam = nullptr;
    napi_create_string_utf8(env, "user_id", NAPI_AUTO_LENGTH, &userIdParam);
    napi_value dsNameParam = nullptr;
    napi_create_string_utf8(env, "ds_name", NAPI_AUTO_LENGTH, &dsNameParam);
    napi_value dsInstanceIdParam = nullptr;
    napi_create_string_utf8(env, "ds_instance_id", NAPI_AUTO_LENGTH, &dsInstanceIdParam);

    paramMap["USER_ID"] = userIdParam;
    paramMap["DISTRIBUTED_SERVICE_NAME"] = dsNameParam;
    paramMap["DISTRIBUTED_SERVICE_INSTANCE_ID"] = dsInstanceIdParam;
}

void InitConstClassByName(napi_env env, napi_value exports, std::string name)
{
    std::map<const char*, napi_value> propertyMap;
    if (name == EVENT_CLASS_NAME) {
        InitEventMap(env, propertyMap);
    } else if (name == PARAM_CLASS_NAME) {
        InitParamMap(env, propertyMap);
    } else if (name == EVENT_TYPE_CLASS_NAME) {
        InitEventTypeMap(env, propertyMap);
    } else {
        return;
    }

    int i = 0;
    napi_property_descriptor descriptors[propertyMap.size()];
    for (auto it : propertyMap) {
        descriptors[i++] = DECLARE_NAPI_STATIC_PROPERTY(it.first, it.second);
    }

    napi_value result = nullptr;
    napi_define_class(env, name.c_str(), NAPI_AUTO_LENGTH, ClassConstructor, nullptr,
        sizeof(descriptors) / sizeof(*descriptors), descriptors, &result);
    napi_set_named_property(env, exports, name.c_str(), result);
}
}

napi_value InitNapiClass(napi_env env, napi_value exports)
{
    InitConstClassByName(env, exports, EVENT_CLASS_NAME);
    InitConstClassByName(env, exports, PARAM_CLASS_NAME);
    InitConstClassByName(env, exports, EVENT_TYPE_CLASS_NAME);
    return exports;
}
} // HiviewDFX
} // OHOS