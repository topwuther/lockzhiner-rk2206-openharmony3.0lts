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

#include "slot.h"

namespace OHOS {
namespace NotificationNapi {
const int ADD_SLOT_MAX_PARA = 2;
const int ADD_SLOTS_MAX_PARA = 2;
const int SET_SLOT_AS_BUNDLE_MAX_PARA = 3;
const int GET_SLOT_MAX_PARA = 2;
const int GET_SLOT_NUM_AS_BUNDLE_MAX_PARA = 2;
const int GET_SLOTS_MAX_PARA = 1;
const int GET_SLOTS_AS_BUNDLE_MAX_PARA = 2;
const int REMOVE_SLOT_MAX_PARA = 2;
const int REMOVE_ALL_SLOTS_MAX_PARA = 1;

struct ParametersInfoAddSlot {
    NotificationSlot slot;
    enum NotificationConstant::SlotType inType = NotificationConstant::SlotType::OTHER;
    bool isAddSlotByType = false;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoAddSlot {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    NotificationSlot slot;
    enum NotificationConstant::SlotType inType = NotificationConstant::SlotType::OTHER;
    bool isAddSlotByType = false;
    CallbackPromiseInfo info;
};

struct ParametersInfoAddSlots {
    std::vector<NotificationSlot> slots;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoAddSlots {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::vector<NotificationSlot> slots;
    CallbackPromiseInfo info;
};

struct ParametersInfoSetSlotByBundle {
    BundleOption option;
    std::vector<sptr<NotificationSlot>> slots;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoSetSlotByBundle {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    ParametersInfoSetSlotByBundle params;
    CallbackPromiseInfo info;
};

struct ParametersInfoGetSlot {
    enum NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoGetSlot {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    enum NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
    napi_value result = nullptr;
    CallbackPromiseInfo info;
};

struct ParametersInfoGetSlotNumByBundle {
    BundleOption option;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoGetSlotNumByBundle {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value result = nullptr;
    ParametersInfoGetSlotNumByBundle params;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoGetSlots {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value result = nullptr;
    CallbackPromiseInfo info;
};

struct ParametersInfoGetSlotsByBundle {
    BundleOption option;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoGetSlotsByBundle {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value result = nullptr;
    ParametersInfoGetSlotsByBundle params;
    CallbackPromiseInfo info;
};

struct ParametersInfoRemoveSlot {
    enum NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoRemoveSlot {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    enum NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoRemoveAllSlots {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    CallbackPromiseInfo info;
};

napi_value ParseParametersByAddSlot(const napi_env &env, const napi_callback_info &info, ParametersInfoAddSlot &paras)
{
    ANS_LOGI("enter");
    size_t argc = ADD_SLOT_MAX_PARA;
    napi_value argv[ADD_SLOT_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    // argv[0]: NotificationSlot
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(
        env, (valuetype == napi_object || valuetype == napi_number), "Wrong argument type. Object or number expected.");
    if (valuetype == napi_number) {
        paras.isAddSlotByType = true;
        int slotType = 0;
        napi_get_value_int32(env, argv[0], &slotType);
        if (!Common::SlotTypeJSToC(SlotType(slotType), paras.inType)) {
            return nullptr;
        }
    } else {
        paras.isAddSlotByType = false;
        if (!Common::GetNotificationSlot(env, argv[0], paras.slot)) {
            return nullptr;
        }
    }

    // argv[1]:callback
    if (argc >= ADD_SLOT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &paras.callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ParseParametersByAddSlots(const napi_env &env, const napi_callback_info &info, ParametersInfoAddSlots &paras)
{
    ANS_LOGI("enter");
    size_t argc = ADD_SLOTS_MAX_PARA;
    napi_value argv[ADD_SLOTS_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    // argv[0]: Array<NotificationSlot>
    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");
    uint32_t length = 0;
    napi_get_array_length(env, argv[0], &length);
    NAPI_ASSERT(env, length > 0, "The array is empty.");
    for (size_t i = 0; i < length; i++) {
        napi_value nSlot = nullptr;
        napi_get_element(env, argv[0], i, &nSlot);
        NAPI_CALL(env, napi_typeof(env, nSlot, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        NotificationSlot slot;
        if (!Common::GetNotificationSlot(env, nSlot, slot)) {
            return nullptr;
        }
        paras.slots.emplace_back(slot);
    }

    // argv[1]:callback
    if (argc >= ADD_SLOTS_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &paras.callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ParseParametersSetSlotByBundle(
    const napi_env &env, const napi_callback_info &info, ParametersInfoSetSlotByBundle &params)
{
    ANS_LOGI("enter");

    size_t argc = SET_SLOT_AS_BUNDLE_MAX_PARA;
    napi_value argv[SET_SLOT_AS_BUNDLE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= SET_SLOT_AS_BUNDLE_MAX_PARA - 1, "Wrong number of arguments");

    // argv[0]: bundle
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type.Object expected.");
    auto retValue = Common::GetBundleOption(env, argv[0], params.option);
    if (retValue == nullptr) {
        ANS_LOGE("GetBundleOption failed.");
        return nullptr;
    }

    // argv[1]: slot
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type.Object expected.");
    NotificationSlot slot;
    if (!Common::GetNotificationSlot(env, argv[1], slot)) {
        return nullptr;
    }
    std::vector<NotificationSlot> slots;
    slots.emplace_back(slot);

    for (auto vec : slots) {
        sptr<NotificationSlot> slotPtr = new (std::nothrow) NotificationSlot(vec);
        if (slotPtr == nullptr) {
            ANS_LOGE("Failed to create NotificationSlot ptr");
            return nullptr;
        }
        params.slots.emplace_back(slotPtr);
    }

    // argv[2]:callback
    if (argc >= SET_SLOT_AS_BUNDLE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[SET_SLOT_AS_BUNDLE_MAX_PARA - 1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[SET_SLOT_AS_BUNDLE_MAX_PARA - 1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParametersByGetSlot(const napi_env &env, const napi_callback_info &info, ParametersInfoGetSlot &paras)
{
    ANS_LOGI("enter");
    size_t argc = GET_SLOT_MAX_PARA;
    napi_value argv[GET_SLOT_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    // argv[0]: SlotType
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int slotType = 0;
    napi_get_value_int32(env, argv[0], &slotType);
    if (!Common::SlotTypeJSToC(SlotType(slotType), paras.outType)) {
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= GET_SLOT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &paras.callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ParseParametersGetSlotNumByBundle(
    const napi_env &env, const napi_callback_info &info, ParametersInfoGetSlotNumByBundle &params)
{
    ANS_LOGI("enter");

    size_t argc = GET_SLOT_NUM_AS_BUNDLE_MAX_PARA;
    napi_value argv[GET_SLOT_NUM_AS_BUNDLE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= GET_SLOT_NUM_AS_BUNDLE_MAX_PARA - 1, "Wrong number of arguments");

    // argv[0]: bundle
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type.Object expected.");
    auto retValue = Common::GetBundleOption(env, argv[0], params.option);
    if (retValue == nullptr) {
        ANS_LOGE("GetBundleOption failed.");
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= GET_SLOT_NUM_AS_BUNDLE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParametersByGetSlots(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    ANS_LOGI("enter");
    size_t argc = GET_SLOTS_MAX_PARA;
    napi_value argv[GET_SLOTS_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    // argv[0]:callback
    napi_valuetype valuetype = napi_undefined;
    if (argc >= GET_SLOTS_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ParseParametersGetSlotsByBundle(
    const napi_env &env, const napi_callback_info &info, ParametersInfoGetSlotsByBundle &params)
{
    ANS_LOGI("enter");

    size_t argc = GET_SLOTS_AS_BUNDLE_MAX_PARA;
    napi_value argv[GET_SLOTS_AS_BUNDLE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= GET_SLOTS_AS_BUNDLE_MAX_PARA - 1, "Wrong number of arguments");

    // argv[0]: bundle
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type.Object expected.");
    auto retValue = Common::GetBundleOption(env, argv[0], params.option);
    if (retValue == nullptr) {
        ANS_LOGE("GetBundleOption failed.");
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= GET_SLOTS_AS_BUNDLE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParametersByRemoveSlot(
    const napi_env &env, const napi_callback_info &info, ParametersInfoRemoveSlot &paras)
{
    ANS_LOGI("enter");
    size_t argc = REMOVE_SLOT_MAX_PARA;
    napi_value argv[REMOVE_SLOT_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    // argv[0]: SlotType
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int slotType = 0;
    napi_get_value_int32(env, argv[0], &slotType);
    if (!Common::SlotTypeJSToC(SlotType(slotType), paras.outType)) {
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= REMOVE_SLOT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &paras.callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ParseParametersByRemoveAllSlots(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    ANS_LOGI("enter");
    size_t argc = REMOVE_ALL_SLOTS_MAX_PARA;
    napi_value argv[REMOVE_ALL_SLOTS_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc >= REMOVE_ALL_SLOTS_MAX_PARA) {
        napi_valuetype valuetype = napi_undefined;
        // argv[0]:callback
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }
    return Common::NapiGetNull(env);
}

napi_value AddSlot(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoAddSlot paras;
    if (ParseParametersByAddSlot(env, info, paras) == nullptr) {
        return Common::JSParaError(env, paras.callback);
    }

    AsyncCallbackInfoAddSlot *asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoAddSlot {
        .env = env,
        .asyncWork = nullptr,
        .slot = paras.slot,
        .inType = paras.inType,
        .isAddSlotByType = paras.isAddSlotByType
    };
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "addSlot", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("AddSlot napi_create_async_work start");
            AsyncCallbackInfoAddSlot *asynccallbackinfo = (AsyncCallbackInfoAddSlot *)data;
            if (asynccallbackinfo->isAddSlotByType) {
                asynccallbackinfo->info.errorCode = NotificationHelper::AddSlotByType(asynccallbackinfo->inType);
            } else {
                asynccallbackinfo->info.errorCode = NotificationHelper::AddNotificationSlot(asynccallbackinfo->slot);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("AddSlot napi_create_async_work end");
            AsyncCallbackInfoAddSlot *asynccallbackinfo = (AsyncCallbackInfoAddSlot *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value AddSlots(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoAddSlots paras;
    if (ParseParametersByAddSlots(env, info, paras) == nullptr) {
        return Common::JSParaError(env, paras.callback);
    }

    AsyncCallbackInfoAddSlots *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoAddSlots {.env = env, .asyncWork = nullptr, .slots = paras.slots};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "addSlots", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("AddSlots napi_create_async_work start");
            AsyncCallbackInfoAddSlots *asynccallbackinfo = (AsyncCallbackInfoAddSlots *)data;
            asynccallbackinfo->info.errorCode = NotificationHelper::AddNotificationSlots(asynccallbackinfo->slots);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("AddSlots napi_create_async_work end");
            AsyncCallbackInfoAddSlots *asynccallbackinfo = (AsyncCallbackInfoAddSlots *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value SetSlotByBundle(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoSetSlotByBundle params {};
    if (ParseParametersSetSlotByBundle(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoSetSlotByBundle *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoSetSlotByBundle {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "setSlotByBundle", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("SetSlotByBundle napi_create_async_work start");
            AsyncCallbackInfoSetSlotByBundle *asynccallbackinfo = (AsyncCallbackInfoSetSlotByBundle *)data;

            NotificationBundleOption bundleOption;
            bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
            bundleOption.SetUid(asynccallbackinfo->params.option.uid);
            asynccallbackinfo->info.errorCode =
                NotificationHelper::UpdateNotificationSlots(bundleOption, asynccallbackinfo->params.slots);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("SetSlotByBundle napi_create_async_work end");
            AsyncCallbackInfoSetSlotByBundle *asynccallbackinfo = (AsyncCallbackInfoSetSlotByBundle *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value GetSlot(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoGetSlot paras;
    if (ParseParametersByGetSlot(env, info, paras) == nullptr) {
        return Common::JSParaError(env, paras.callback);
    }

    AsyncCallbackInfoGetSlot *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoGetSlot {.env = env, .asyncWork = nullptr, .outType = paras.outType};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getSlot", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetSlot napi_create_async_work start");
            AsyncCallbackInfoGetSlot *asynccallbackinfo = (AsyncCallbackInfoGetSlot *)data;

            sptr<NotificationSlot> slot = nullptr;
            asynccallbackinfo->info.errorCode =
                NotificationHelper::GetNotificationSlot(asynccallbackinfo->outType, slot);
            asynccallbackinfo->result = Common::NapiGetNull(env);
            if (asynccallbackinfo->info.errorCode != ERR_OK) {
                return;
            }
            if (slot == nullptr) {
                asynccallbackinfo->info.errorCode = ERROR;
                return;
            }
            napi_create_object(env, &asynccallbackinfo->result);
            if (!Common::SetNotificationSlot(env, *slot, asynccallbackinfo->result)) {
                asynccallbackinfo->info.errorCode = ERROR;
                asynccallbackinfo->result = Common::NapiGetNull(env);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetSlot napi_create_async_work end");
            AsyncCallbackInfoGetSlot *asynccallbackinfo = (AsyncCallbackInfoGetSlot *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value GetSlotNumByBundle(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoGetSlotNumByBundle params {};
    if (ParseParametersGetSlotNumByBundle(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoGetSlotNumByBundle *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoGetSlotNumByBundle {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getSlotNumByBundle", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetSlotNumByBundle napi_create_async_work start");
            AsyncCallbackInfoGetSlotNumByBundle *asynccallbackinfo = (AsyncCallbackInfoGetSlotNumByBundle *)data;

            NotificationBundleOption bundleOption;
            bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
            bundleOption.SetUid(asynccallbackinfo->params.option.uid);
            int num = 0;
            asynccallbackinfo->info.errorCode = NotificationHelper::GetNotificationSlotNumAsBundle(bundleOption, num);
            napi_create_int32(env, num, &asynccallbackinfo->result);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetSlotNumByBundle napi_create_async_work end");
            AsyncCallbackInfoGetSlotNumByBundle *asynccallbackinfo = (AsyncCallbackInfoGetSlotNumByBundle *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value GetSlots(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    if (ParseParametersByGetSlots(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    AsyncCallbackInfoGetSlots *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoGetSlots {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getSlots", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetSlots napi_create_async_work start");
            AsyncCallbackInfoGetSlots *asynccallbackinfo = (AsyncCallbackInfoGetSlots *)data;

            std::vector<sptr<NotificationSlot>> slots;
            asynccallbackinfo->info.errorCode = NotificationHelper::GetNotificationSlots(slots);
            if (asynccallbackinfo->info.errorCode != ERR_OK) {
                asynccallbackinfo->result = Common::NapiGetNull(env);
                return;
            }

            napi_value arr = nullptr;
            napi_create_array(env, &arr);
            size_t count = 0;
            for (auto vec : slots) {
                if (!vec) {
                    ANS_LOGW("Invalid NotificationSlot object ptr");
                    continue;
                }
                napi_value nSlot = nullptr;
                napi_create_object(env, &nSlot);
                if (!Common::SetNotificationSlot(env, *vec, nSlot)) {
                    continue;
                }
                napi_set_element(env, arr, count, nSlot);
                count++;
            }
            ANS_LOGI("getSlots count = %{public}zu", count);
            asynccallbackinfo->result = arr;
            if ((count == 0) && (slots.size() > 0)) {
                asynccallbackinfo->info.errorCode = ERROR;
                asynccallbackinfo->result = Common::NapiGetNull(env);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetSlots napi_create_async_work end");
            AsyncCallbackInfoGetSlots *asynccallbackinfo = (AsyncCallbackInfoGetSlots *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value GetSlotsByBundle(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoGetSlotsByBundle params {};
    if (ParseParametersGetSlotsByBundle(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoGetSlotsByBundle *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoGetSlotsByBundle {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getSlotsByBundle", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetSlotsByBundle napi_create_async_work start");
            AsyncCallbackInfoGetSlotsByBundle *asynccallbackinfo = (AsyncCallbackInfoGetSlotsByBundle *)data;

            NotificationBundleOption bundleOption;
            bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
            bundleOption.SetUid(asynccallbackinfo->params.option.uid);

            std::vector<sptr<NotificationSlot>> slots;
            asynccallbackinfo->info.errorCode = NotificationHelper::GetNotificationSlotsForBundle(bundleOption, slots);
            if (asynccallbackinfo->info.errorCode != ERR_OK) {
                asynccallbackinfo->result = Common::NapiGetNull(env);
                return;
            }

            napi_value arr = nullptr;
            napi_create_array(env, &arr);
            size_t count = 0;
            for (auto vec : slots) {
                if (!vec) {
                    ANS_LOGW("Invalid NotificationSlot object ptr");
                    continue;
                }
                napi_value nSlot = nullptr;
                napi_create_object(env, &nSlot);
                if (!Common::SetNotificationSlot(env, *vec, nSlot)) {
                    continue;
                }
                napi_set_element(env, arr, count, nSlot);
                count++;
            }
            ANS_LOGI("getSlots count = %{public}zu", count);
            asynccallbackinfo->result = arr;
            if ((count == 0) && (slots.size() > 0)) {
                asynccallbackinfo->info.errorCode = ERROR;
                asynccallbackinfo->result = Common::NapiGetNull(env);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetSlotsByBundle napi_create_async_work end");
            AsyncCallbackInfoGetSlotsByBundle *asynccallbackinfo = (AsyncCallbackInfoGetSlotsByBundle *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value RemoveSlot(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoRemoveSlot paras;
    if (ParseParametersByRemoveSlot(env, info, paras) == nullptr) {
        return Common::JSParaError(env, paras.callback);
    }

    AsyncCallbackInfoRemoveSlot *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoRemoveSlot {.env = env, .asyncWork = nullptr, .outType = paras.outType};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "removeSlot", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("removeSlot napi_create_async_work start");
            AsyncCallbackInfoRemoveSlot *asynccallbackinfo = (AsyncCallbackInfoRemoveSlot *)data;
            asynccallbackinfo->info.errorCode = NotificationHelper::RemoveNotificationSlot(asynccallbackinfo->outType);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("removeSlot napi_create_async_work end");
            AsyncCallbackInfoRemoveSlot *asynccallbackinfo = (AsyncCallbackInfoRemoveSlot *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value RemoveAllSlots(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    if (ParseParametersByRemoveAllSlots(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    AsyncCallbackInfoRemoveAllSlots *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoRemoveAllSlots {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "removeAll", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("RemoveAllSlots napi_create_async_work start");
            AsyncCallbackInfoRemoveAllSlots *asynccallbackinfo = (AsyncCallbackInfoRemoveAllSlots *)data;
            asynccallbackinfo->info.errorCode = NotificationHelper::RemoveAllSlots();
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("RemoveAllSlots napi_create_async_work end");
            AsyncCallbackInfoRemoveAllSlots *asynccallbackinfo = (AsyncCallbackInfoRemoveAllSlots *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

}  // namespace NotificationNapi
}  // namespace OHOS