/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi_preference.h"

#include <cerrno>
#include <cmath>
#include <limits>
#include <linux/limits.h>
#include "securec.h"
#include "common.h"

#include "preferences.h"
#include "preferences_errno.h"
#include "preferences_value.h"
#include "napi_async_proxy.h"

using namespace OHOS::NativePreferences;
using namespace OHOS::JsKit;

namespace OHOS {
namespace PreferencesJsKit {
#define MAX_KEY_LENGTH Preferences::MAX_KEY_LENGTH
#define MAX_VALUE_LENGTH Preferences::MAX_VALUE_LENGTH

struct StorageAysncContext : NapiAsyncProxy<StorageAysncContext>::AysncContext {
    std::string key;
    PreferencesValue defValue = PreferencesValue((int)0);
    bool hasKey;
};

napi_ref PreferencesProxy::constructor;

PreferencesProxy::PreferencesProxy(std::shared_ptr<OHOS::NativePreferences::Preferences> value)
    : value_(value), env_(nullptr), wrapper_(nullptr)
{}

PreferencesProxy::~PreferencesProxy()
{
    napi_delete_reference(env_, wrapper_);
}

void PreferencesProxy::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    LOG_DEBUG("Destructor");
    PreferencesProxy *obj = static_cast<PreferencesProxy *>(nativeObject);
    delete obj;
}

void PreferencesProxy::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("putSync", SetValueSync),  DECLARE_NAPI_FUNCTION("put", SetValue),
        DECLARE_NAPI_FUNCTION("getSync", GetValueSync),  DECLARE_NAPI_FUNCTION("get", GetValue),
        DECLARE_NAPI_FUNCTION("deleteSync", DeleteSync), DECLARE_NAPI_FUNCTION("delete", Delete),
        DECLARE_NAPI_FUNCTION("clearSync", ClearSync),   DECLARE_NAPI_FUNCTION("clear", Clear),
        DECLARE_NAPI_FUNCTION("hasSync", HasKeySync),    DECLARE_NAPI_FUNCTION("has", HasKey),
        DECLARE_NAPI_FUNCTION("flushSync", FlushSync),   DECLARE_NAPI_FUNCTION("flush", Flush),
        DECLARE_NAPI_FUNCTION("on", RegisterObserver),   DECLARE_NAPI_FUNCTION("off", UnRegisterObserver),
    };
    LOG_DEBUG("Init");
    napi_value cons = nullptr;
    napi_define_class(env, "Storage", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(descriptors) / sizeof(napi_property_descriptor), descriptors, &cons);

    napi_create_reference(env, cons, 1, &constructor);
    LOG_DEBUG("Init end");
}

napi_status PreferencesProxy::NewInstance(napi_env env, napi_value arg, napi_value *instance)
{
    napi_status status;

    const int argc = 1;
    napi_value argv[argc] = {arg};

    napi_value cons;
    status = napi_get_reference_value(env, constructor, &cons);
    if (status != napi_ok) {
        return status;
    }

    status = napi_new_instance(env, cons, argc, argv, instance);
    if (status != napi_ok) {
        return status;
    }

    return napi_ok;
}

napi_value PreferencesProxy::New(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("New");
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_value _this = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    if (_this == nullptr) {
        LOG_WARN("get this failed");
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "input type not string");
    char *path = new char[PATH_MAX];
    memset_s(path, PATH_MAX, 0, PATH_MAX);
    size_t pathLen = 0;
    napi_status status = napi_get_value_string_utf8(env, args[0], path, PATH_MAX, &pathLen);
    if (status != napi_ok) {
        LOG_ERROR("get path failed. ");
        delete[] path;
        return nullptr;
    }
    // get native object
    int errCode = 0;
    std::shared_ptr<OHOS::NativePreferences::Preferences> preference =
        OHOS::NativePreferences::PreferencesHelper::GetPreferences(path, errCode);
    delete[] path;
    NAPI_ASSERT(env, preference != nullptr, "failed to call native");
    PreferencesProxy *obj = new PreferencesProxy(preference);
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, _this, obj, PreferencesProxy::Destructor,
        nullptr, // finalize_hint
        &obj->wrapper_));
    LOG_DEBUG("New end");
    return _this;
}

template<typename T>
bool CheckNumberType(double input)
{
    if (input > (std::numeric_limits<T>::max)() || input < (std::numeric_limits<T>::min)()) {
        return false;
    }
    return true;
}

bool IsFloat(double input)
{
    return abs(input - floor(input)) >= 0; // DBL_EPSILON;
}

napi_value PreferencesProxy::GetValueSync(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    size_t argc = 2; // arg count
    napi_value args[2] = {0};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    NAPI_ASSERT(env, argc == 2, "Wrong number of arguments");
    // get value type
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "type mismatch for key");

    // get input key
    char key[MAX_KEY_LENGTH] = {0};
    size_t keySize = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], key, MAX_KEY_LENGTH, &keySize));
    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    NAPI_ASSERT(env, (obj != nullptr && obj->value_ != nullptr), "unwrap null native pointer");

    napi_value output = nullptr;
    NAPI_CALL(env, napi_typeof(env, args[1], &valueType));
    if (valueType == napi_number) {
        double value = 0.0;
        NAPI_CALL(env, napi_get_value_double(env, args[1], &value));
        LOG_DEBUG("get number");
        double result = obj->value_->GetDouble(key, value);
        NAPI_CALL(env, napi_create_double(env, result, &output)); // double
        LOG_DEBUG("get number end");
    } else if (valueType == napi_string) {
        LOG_DEBUG("get string");
        char *value = new char[MAX_VALUE_LENGTH];
        size_t valueSize = 0;
        napi_get_value_string_utf8(env, args[1], value, MAX_VALUE_LENGTH, &valueSize);
        // get value
        std::string result = obj->value_->GetString(key, value);
        delete[] value;
        NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.size(), &output));
        LOG_DEBUG("get string end");
    } else if (valueType == napi_boolean) {
        LOG_DEBUG("get bool");
        bool value = false;
        NAPI_CALL(env, napi_get_value_bool(env, args[1], &value));
        // get value
        bool result = obj->value_->GetBool(key, value);
        NAPI_CALL(env, napi_get_boolean(env, result, &output));
        LOG_DEBUG("get bool end");
    } else {
        NAPI_ASSERT(env, false, "Wrong second parameter type");
    }
    return output;
}

void ParseKey(const napi_env& env, const napi_value& arg, StorageAysncContext* asyncContext)
{
    // get input key
    char key[MAX_KEY_LENGTH] = { 0 };
    size_t keySize = 0;
    napi_get_value_string_utf8(env, arg, key, MAX_KEY_LENGTH, &keySize);
    asyncContext->key = key;
}

void ParseDefValue(const napi_env& env, const napi_value& arg, StorageAysncContext* asyncContext)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, arg, &valueType);
    if (valueType == napi_number) {
        double number = 0.0;
        napi_get_value_double(env, arg, &number);
        PreferencesValue value((double)number);
        asyncContext->defValue = value;
    } else if (valueType == napi_string) {
        char* str = new char[MAX_VALUE_LENGTH];
        size_t valueSize = 0;
        napi_get_value_string_utf8(env, arg, str, MAX_VALUE_LENGTH, &valueSize);
        PreferencesValue value((std::string)(str));
        asyncContext->defValue = value;
        delete[] str;
    } else if (valueType == napi_boolean) {
        bool bValue = false;
        napi_get_value_bool(env, arg, &bValue);
        PreferencesValue value((bool)(bValue));
        asyncContext->defValue = value;
    } else {
        LOG_ERROR("Wrong second parameter type");
    }
}

napi_value PreferencesProxy::GetValue(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<StorageAysncContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<StorageAysncContext>::InputParser> parsers;
    parsers.push_back(ParseKey);
    parsers.push_back(ParseDefValue);
    proxy.ParseInputs(parsers);

    return proxy.DoAsyncWork(
        "GetValue",
        [](StorageAysncContext* asyncContext) {
            int errCode = OK;
            PreferencesProxy* obj = reinterpret_cast<PreferencesProxy*>(asyncContext->boundObj);
            if (asyncContext->defValue.IsBool()) {
                bool tmpValue = (bool)obj->value_->GetBool(asyncContext->key, (bool)asyncContext->defValue);
                asyncContext->defValue = PreferencesValue((bool)tmpValue);
            } else if (asyncContext->defValue.IsString()) {
                std::string tmpValue = obj->value_->GetString(asyncContext->key, (std::string)asyncContext->defValue);
                asyncContext->defValue = PreferencesValue((std::string)tmpValue);
            } else if (asyncContext->defValue.IsDouble()) {
                double tmpValue = obj->value_->GetDouble(asyncContext->key, (double)asyncContext->defValue);
                asyncContext->defValue = PreferencesValue((double)tmpValue);
            } else {
                errCode = ERR;
            }

            return errCode;
        },
        [](StorageAysncContext* asyncContext, napi_value& output) {
            int errCode = OK;
            if (asyncContext->defValue.IsBool()) {
                napi_get_boolean(asyncContext->env, (bool)asyncContext->defValue, &output);
            } else if (asyncContext->defValue.IsString()) {
                std::string tempStr = (std::string)asyncContext->defValue;
                napi_create_string_utf8(asyncContext->env, tempStr.c_str(), tempStr.size(), &output);
            } else if (asyncContext->defValue.IsDouble()) {
                napi_create_double(asyncContext->env, (double)asyncContext->defValue, &output);
            } else {
                errCode = ERR;
            }

            return errCode;
        });
}

napi_value PreferencesProxy::SetValueSync(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};

    LOG_DEBUG("SETVALUE");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    NAPI_ASSERT(env, argc == 2, "Wrong number of arguments");
    // get value type
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "type mismatch for key");

    // get input key
    char key[MAX_KEY_LENGTH] = {0};
    size_t out = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], key, MAX_KEY_LENGTH, &out));

    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    NAPI_ASSERT(env, (obj != nullptr && obj->value_ != nullptr), "unwrap null native pointer");

    NAPI_CALL(env, napi_typeof(env, args[1], &valueType));
    if (valueType == napi_number) {
        double value = 0.0;
        NAPI_CALL(env, napi_get_value_double(env, args[1], &value));
        int result = obj->value_->PutDouble(key, (double) value);
        NAPI_ASSERT(env, result == E_OK, "call PutDouble failed");
    } else if (valueType == napi_string) {
        char *value = new char[MAX_VALUE_LENGTH];
        napi_get_value_string_utf8(env, args[1], value, MAX_VALUE_LENGTH, &out);
        // get value
        int result = obj->value_->PutString(key, value);
        delete[] value;
        NAPI_ASSERT(env, result == E_OK, "call PutString failed");
    } else if (valueType == napi_boolean) {
        bool value = false;
        NAPI_CALL(env, napi_get_value_bool(env, args[1], &value));
        // get value
        int result = obj->value_->PutBool(key, value);
        NAPI_ASSERT(env, result == E_OK, "call PutBool failed");
    } else {
        NAPI_ASSERT(env, false, "Wrong second parameter type");
    }
    return nullptr;
}

napi_value PreferencesProxy::SetValue(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<StorageAysncContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<StorageAysncContext>::InputParser> parsers;
    parsers.push_back(ParseKey);
    parsers.push_back(ParseDefValue);
    proxy.ParseInputs(parsers);

    return proxy.DoAsyncWork(
        "SetValue",
        [](StorageAysncContext* asyncContext) {
            int errCode = OK;
            PreferencesProxy* obj = reinterpret_cast<PreferencesProxy*>(asyncContext->boundObj);
            if (asyncContext->defValue.IsBool()) {
                errCode = obj->value_->PutBool(asyncContext->key, (bool)asyncContext->defValue);
            } else if (asyncContext->defValue.IsString()) {
                errCode = obj->value_->PutString(asyncContext->key, (std::string)asyncContext->defValue);
            } else if (asyncContext->defValue.IsDouble()) {
                errCode = obj->value_->PutDouble(asyncContext->key, (double)asyncContext->defValue);
            } else {
                errCode = ERR;
            }

            return errCode;
        },
        [](StorageAysncContext* asyncContext, napi_value& output) {
            napi_status status = napi_get_undefined(asyncContext->env, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value PreferencesProxy::DeleteSync(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");
    // get value type
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "type mismatch for key");

    char key[MAX_KEY_LENGTH] = {0};
    size_t out = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], key, MAX_KEY_LENGTH, &out));
    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    int result = obj->value_->Delete(key);
    NAPI_ASSERT(env, result == E_OK, "call Delete failed");
    LOG_INFO("Delete");
    return nullptr;
}

napi_value PreferencesProxy::Delete(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<StorageAysncContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<StorageAysncContext>::InputParser> parsers;
    parsers.push_back(ParseKey);
    proxy.ParseInputs(parsers);

    return proxy.DoAsyncWork(
        "Delete",
        [](StorageAysncContext* asyncContext) {
            PreferencesProxy* obj = reinterpret_cast<PreferencesProxy*>(asyncContext->boundObj);
            int errCode = obj->value_->Delete(asyncContext->key);

            return errCode;
        },
        [](StorageAysncContext* asyncContext, napi_value& output) {
            napi_status status = napi_get_undefined(asyncContext->env, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value PreferencesProxy::HasKeySync(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");
    // get value type
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "type mismatch for key");

    char key[MAX_KEY_LENGTH] = {0};
    size_t out = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], key, MAX_KEY_LENGTH, &out));
    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    bool result = obj->value_->HasKey(key);
    napi_value output = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &output));
    LOG_DEBUG("HasKey");
    return output;
}

napi_value PreferencesProxy::HasKey(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<StorageAysncContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<StorageAysncContext>::InputParser> parsers;
    parsers.push_back(ParseKey);
    proxy.ParseInputs(parsers);

    return proxy.DoAsyncWork(
        "HasKey",
        [](StorageAysncContext* asyncContext) {
            PreferencesProxy* obj = reinterpret_cast<PreferencesProxy*>(asyncContext->boundObj);
            asyncContext->hasKey = obj->value_->HasKey(asyncContext->key);

            return OK;
        },
        [](StorageAysncContext* asyncContext, napi_value& output) {
            napi_status status = napi_get_boolean(asyncContext->env, asyncContext->hasKey, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value PreferencesProxy::Flush(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<StorageAysncContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<StorageAysncContext>::InputParser> parsers;
    proxy.ParseInputs(parsers);

    return proxy.DoAsyncWork(
        "Flush",
        [](StorageAysncContext* asyncContext) {
            PreferencesProxy* obj = reinterpret_cast<PreferencesProxy*>(asyncContext->boundObj);
            return obj->value_->FlushSync();
        },
        [](StorageAysncContext* asyncContext, napi_value& output) {
            napi_status status = napi_get_undefined(asyncContext->env, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value PreferencesProxy::FlushSync(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));
    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    int result = obj->value_->FlushSync();
    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &output));
    LOG_DEBUG("FlushSync");
    return output;
}

napi_value PreferencesProxy::ClearSync(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));
    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    int result = obj->value_->Clear();
    NAPI_ASSERT(env, result == E_OK, "call Clear failed");
    LOG_DEBUG("Clear");
    return nullptr;
}

napi_value PreferencesProxy::Clear(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<StorageAysncContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<StorageAysncContext>::InputParser> parsers;
    proxy.ParseInputs(parsers);

    return proxy.DoAsyncWork(
        "Clear",
        [](StorageAysncContext* asyncContext) {
            PreferencesProxy* obj = reinterpret_cast<PreferencesProxy*>(asyncContext->boundObj);
            return obj->value_->Clear();
        },
        [](StorageAysncContext* asyncContext, napi_value& output) {
            napi_status status = napi_get_undefined(asyncContext->env, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value PreferencesProxy::RegisterObserver(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};

    LOG_DEBUG("RegisterObserver");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "key not string type");

    NAPI_CALL(env, napi_typeof(env, args[1], &type));
    NAPI_ASSERT(env, type == napi_function, "observer not function type");

    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    // reference save
    obj->observer_ = std::make_shared<PreferencesObserverImpl>(env, args[1]);
    obj->value_->RegisterObserver(obj->observer_);
    LOG_DEBUG("RegisterObserver end");

    return nullptr;
}

napi_value PreferencesProxy::UnRegisterObserver(napi_env env, napi_callback_info info)
{
    napi_value _this = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};
    LOG_DEBUG("UnRegisterObserver");

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "key not string type");

    NAPI_CALL(env, napi_typeof(env, args[1], &type));
    NAPI_ASSERT(env, type == napi_function, "observer not function type");

    PreferencesProxy *obj = nullptr;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    obj->value_->UnRegisterObserver(obj->observer_);
    obj->observer_.reset();
    obj->observer_ = nullptr;
    LOG_DEBUG("UnRegisterObserver end");
    return nullptr;
}

PreferencesObserverImpl::PreferencesObserverImpl(napi_env env, napi_value callback) : observerRef(nullptr)
{
    this->env_ = env;
    napi_create_reference(env_, callback, 1, &observerRef);
}

PreferencesObserverImpl::~PreferencesObserverImpl()
{
    napi_delete_reference(env_, observerRef);
}

void PreferencesObserverImpl::OnChange(Preferences &preferences, const std::string &key)
{
    LOG_DEBUG("OnChange key:%{public}s", key.c_str());
    napi_value callback = nullptr;
    napi_value args[1] = {0};

    napi_create_string_utf8(env_, key.c_str(), key.size(), &args[0]);
    napi_get_reference_value(env_, observerRef, &callback);

    napi_value global = nullptr;
    napi_get_global(env_, &global);
    napi_call_function(env_, global, callback, 1, args, nullptr);
    LOG_DEBUG("OnChange key end");
}
} // namespace PreferencesJsKit
} // namespace OHOS
