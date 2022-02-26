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

#include "event_target.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
    napi_value JS_Constructor(napi_env env, napi_callback_info cbInfo)
    {
        IMSA_HILOGI("JS_Constructor() is called!");
        napi_value thisVar = nullptr;
        void *data = nullptr;
        napi_get_cb_info(env, cbInfo, nullptr, nullptr, &thisVar, &data);

        OHOS::MiscServices::EventTarget *eventTarget = new OHOS::MiscServices::EventTarget(env, thisVar);
        napi_wrap(env, thisVar, eventTarget,
            [](napi_env env, void *data, void *hint) {
                EventTarget *eventTarget = (EventTarget*)data;
                delete eventTarget;
                eventTarget = nullptr;
            },
            nullptr, nullptr);
        OHOS::sptr<EventTarget> eventTarget_ = eventTarget;
        InputMethodAbility::GetInstance()->setEventTarget(eventTarget_);
        return thisVar;
    }

    napi_value JS_InsertText(napi_env env, napi_callback_info cbInfo)
    {
        IMSA_HILOGI("JS_InsertText() is called!");
        size_t argc = 1;
        napi_value argv[2] = { 0 };
        napi_value thisVar = nullptr;
        void *data = nullptr;
        napi_get_cb_info(env, cbInfo, &argc, argv, &thisVar, &data);

        EventTarget *eventTarget = nullptr;
        napi_unwrap(env, thisVar, (void **)&eventTarget);

        char type[64] = { 0 };
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);
        std::string text = type;
        InputMethodAbility::GetInstance()->InsertText(text);

        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        IMSA_HILOGI("JS_InsertText() is end!");
        return result;
    }

    napi_value JS_DeleteBackward(napi_env env, napi_callback_info cbInfo)
    {
        IMSA_HILOGI("JS_DeleteBackward() is called!");
        size_t argc = 1;
        napi_value argv[2] = { 0 };
        napi_value thisVar = nullptr;
        void *data = nullptr;
        napi_get_cb_info(env, cbInfo, &argc, argv, &thisVar, &data);

        EventTarget *eventTarget = nullptr;
        napi_unwrap(env, thisVar, (void **)&eventTarget);

        int32_t value32 = 0;
        napi_get_value_int32(env, argv[0], &value32);
        InputMethodAbility::GetInstance()->DeleteBackward(value32);

        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    }

    napi_value JS_HideKeyboardSelf(napi_env env, napi_callback_info cbInfo)
    {
        IMSA_HILOGI("JS_HideKeyboardSelf() is called!");
        size_t argc = 1;
        napi_value argv[2] = { 0 };
        napi_value thisVar = nullptr;
        void *data = nullptr;
        napi_get_cb_info(env, cbInfo, &argc, argv, &thisVar, &data);

        EventTarget *eventTarget = nullptr;
        napi_unwrap(env, thisVar, (void **)&eventTarget);

        InputMethodAbility::GetInstance()->HideKeyboardSelf();

        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    }

    napi_value JS_On(napi_env env, napi_callback_info cbInfo)
    {
        IMSA_HILOGI("JS_On() is called!");
        size_t requireArgc = 2;
        size_t argc = 2;
        napi_value argv[2] = { 0 };
        napi_value thisVar = 0;
        void *data = nullptr;
        napi_get_cb_info(env, cbInfo, &argc, argv, &thisVar, &data);

        EventTarget *eventTarget = nullptr;
        napi_unwrap(env, thisVar, (void **)&eventTarget);

        NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

        napi_valuetype eventValueType;
        napi_typeof(env, argv[0], &eventValueType);
        NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

        napi_valuetype eventHandleType;
        napi_typeof(env, argv[1], &eventHandleType);
        NAPI_ASSERT(env, eventHandleType == napi_function, "type mismatch for parameter 2");

        char type[64] = { 0 };
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);

        IMSA_HILOGI("call ima on function");
        eventTarget->On((const char*)type, argv[1]);

        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        IMSA_HILOGI("JS_On() is end!");
        return result;
    }

    napi_value JS_Off(napi_env env, napi_callback_info cbInfo)
    {
        size_t requireArgc = 1;
        size_t argc = 2;
        napi_value argv[2] = { 0 };
        napi_value thisVar = 0;
        void *data = nullptr;
        napi_get_cb_info(env, cbInfo, &argc, argv, &thisVar, &data);

        EventTarget *eventTarget = nullptr;
        napi_unwrap(env, thisVar, (void **)&eventTarget);

        NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

        napi_valuetype eventValueType;
        napi_typeof(env, argv[0], &eventValueType);
        NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

        char *type = nullptr;
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);
        type = new char[typeLen + 1];
        napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

        if (argc > requireArgc) {
            NAPI_ASSERT(env, eventValueType == napi_function, "type mismatch for parameter 2");
            eventTarget->Off(type, argv[1]);
        } else {
            eventTarget->Off(type);
        }

        delete type;
        type = nullptr;
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    }

    napi_value InputMethodEngineInit(napi_env env, napi_value exports)
    {
        IMSA_HILOGI("InputMethodEngineInit() is called!");
        const char className[] = "EventTarget";
        napi_value constructor = nullptr;
        napi_property_descriptor desc[] = {
            DECLARE_NAPI_FUNCTION("insertText", JS_InsertText),
            DECLARE_NAPI_FUNCTION("DeleteBackward", JS_DeleteBackward),
            DECLARE_NAPI_FUNCTION("HideKeyboardSelf", JS_HideKeyboardSelf),
            DECLARE_NAPI_FUNCTION("on", JS_On),
            DECLARE_NAPI_FUNCTION("off", JS_Off),
        };
        napi_define_class(env, className, sizeof(className), JS_Constructor, nullptr,
                          sizeof(desc) / sizeof(desc[0]), desc, &constructor);
        napi_set_named_property(env, exports, "InputMethodEngine", constructor);
        return exports;
    }

    /*
     * module define
     */
    static napi_module inputMethodEngineModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = InputMethodEngineInit,
        .nm_modname = "inputMethodEngine",
        .nm_priv = ((void*)0),
        .reserved = {0},
    };
    /*
     * module register
     */
    extern "C" __attribute__((constructor)) void RegisterModule()
    {
    IMSA_HILOGI("RegisterModule() is called!");
    napi_module_register(&inputMethodEngineModule);
    }
}
}