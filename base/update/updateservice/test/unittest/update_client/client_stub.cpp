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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "client_stub.h"
#include "gtest/gtest.h"
#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "node_api.h"
#include "node_api_types.h"
#include "securec.h"
#include "update_client.h"
#include "update_helper.h"
#include "update_session.h"

using namespace updateClient;
using namespace std;
using namespace OHOS::update_engine;
static constexpr int NUMBER_2 = 2;

TestNApiValue g_callback(napi_function, nullptr);

uint32_t g_testSessionId = 0;
UpdateClient* g_testClient = nullptr;
static int32_t g_undefined = 100;
bool g_callbackFuncationCalled = false;

static UpdatePolicy g_updatePolicy = {
    true,
    true,
    INSTALLMODE_AUTO,
    AUTOUPGRADECONDITION_IDLE,
    {1000, 1000}
};

TestNApiEnv g_testEnv;

static napi_value CreateNapiValue(int type, const void *value)
{
    auto testValue = new TestNApiValue(type, value);
    g_testEnv.testValueList.push_back(testValue);
    return (napi_value)testValue;
}

void FreeAllNapiValues()
{
    auto iter = g_testEnv.testValueList.begin();
    while (iter != g_testEnv.testValueList.end()) {
        auto value = *iter;
        g_testEnv.testValueList.erase(iter);
        delete value;
        iter = g_testEnv.testValueList.begin();
    }
}

TestNApiValue::TestNApiValue(int type, const void *value)
{
    type_ = type;
    switch (type) {
        case napi_boolean:
            bValue = *static_cast<bool*>(const_cast<void*>(value));
            break;
        case napi_number:
            intValue = *static_cast<int*>(const_cast<void*>(value));
            break;
        case napi_string:
            strValue = std::string(static_cast<char*>(const_cast<void*>(value)));
            break;
        case napi_bigint:
            int64Value = *static_cast<int64_t*>(const_cast<void*>(value));
            break;
        default:
            break;
    }
}

UpdateSession *TestGetUpdateSession()
{
    printf(" TestGetUpdateSession g_testSessionId %u\n", g_testSessionId);
    if (g_testClient != nullptr) {
        return g_testClient->GetUpdateSession(g_testSessionId);
    }
    return nullptr;
}

#ifdef __cplusplus
extern "C" {
#endif

napi_status napi_create_string_utf8(napi_env env, const char *str, size_t length, napi_value *result)
{
    *result = CreateNapiValue(napi_string, str);
    return napi_status::napi_ok;
}

napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype *result)
{
    if (((TestNApiValue*)value)->GetType() == TEST_NVALUE_TYPE_CONTEXT) {
        *result = napi_object;
    } else if (((TestNApiValue*)value)->GetType() == TEST_NVALUE_TYPE_UPGRADE) {
        *result = napi_object;
    } else if (((TestNApiValue*)value)->GetType() == TEST_NVALUE_TYPE_UPDATE_POLICY) {
        *result = napi_object;
    } else {
        *result = (napi_valuetype)((TestNApiValue*)value)->GetType();
    }
    return napi_status::napi_ok;
}

napi_status napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount,
    napi_ref* result)
{
    return napi_status::napi_ok;
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result)
{
    return napi_status::napi_ok;
}

napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    return napi_status::napi_ok;
}

napi_status napi_call_function(napi_env env, napi_value recv,
    napi_value func, size_t argc, const napi_value *argv, napi_value *result)
{
    g_callbackFuncationCalled = true;
    return napi_status::napi_ok;
}

napi_status napi_delete_async_work(napi_env env, napi_async_work work)
{
    return napi_status::napi_ok;
}

napi_status napi_queue_async_work(napi_env env, napi_async_work work)
{
    return napi_status::napi_ok;
}

napi_status napi_cancel_async_work(napi_env env, napi_async_work work)
{
    return napi_status::napi_ok;
}

napi_status napi_create_object(napi_env env, napi_value* result)
{
    *result = CreateNapiValue(napi_object, nullptr);
    return napi_status::napi_ok;
}

napi_status napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value)
{
    return napi_status::napi_ok;
}

napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    *result = CreateNapiValue(napi_object, nullptr);
    return napi_status::napi_ok;
}

napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result)
{
    *result = CreateNapiValue(napi_number, &value);
    return napi_status::napi_ok;
}

napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    *result = CreateNapiValue(napi_number, &value);
    return napi_status::napi_ok;
}

napi_status napi_get_undefined(napi_env env, napi_value* result)
{
    *result = (napi_value)&g_undefined;
    return napi_status::napi_ok;
}

napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result)
{
    *result = CreateNapiValue(napi_bigint, &value);
    return napi_status::napi_ok;
}

napi_status napi_throw_error(napi_env env, const char* code, const char* msg)
{
    return napi_status::napi_ok;
}
napi_status napi_is_exception_pending(napi_env env, bool* result)
{
    return napi_status::napi_ok;
}

napi_status napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
{
    return napi_status::napi_ok;
}

napi_status napi_define_properties(napi_env env, napi_value object,
    size_t property_count, const napi_property_descriptor *properties)
{
    return napi_status::napi_ok;
}

void napi_module_register(napi_module* mod) {}

napi_status napi_has_named_property(napi_env env, napi_value object, const char *utf8name,
    bool *result)
{
    *result = false;
    TestNApiValue* testValue = (TestNApiValue*)object;
    if (testValue->GetType() == TEST_NVALUE_TYPE_CONTEXT) {
        *result = true;
    } else if (testValue->GetType() == TEST_NVALUE_TYPE_UPGRADE ||
        testValue->GetType() == TEST_NVALUE_TYPE_UPDATE_POLICY) {
            *result = true;
    }
    return napi_status::napi_ok;
}

napi_status napi_get_named_property(napi_env env, napi_value object, const char* utf8name,
    napi_value* result)
{
    TestNApiEnv* testEnv = (TestNApiEnv*)env;
    TestNApiValue* testValue = (TestNApiValue*)object;

    if (testValue->GetType() == TEST_NVALUE_TYPE_CONTEXT) {
        if (strcmp(utf8name, "serverAddr") == 0) {
            *result = CreateNapiValue(napi_string, "10.50.40.92");
        } else if (strcmp(utf8name, "upgradeDevId") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->clientContext.upgradeDevId.c_str());
        } else if (strcmp(utf8name, "controlDevId") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->clientContext.controlDevId.c_str());
        } else if (strcmp(utf8name, "upgradeApp") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->clientContext.upgradeApp.c_str());
        } else if (strcmp(utf8name, "type") == 0) {
            *result = CreateNapiValue(napi_number, &testEnv->clientContext.type);
        }
    } else if (testValue->GetType() == TEST_NVALUE_TYPE_UPGRADE) {
        if (strcmp(utf8name, "versionName") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->pakcageInfo.result[0].versionName.c_str());
        } else if (strcmp(utf8name, "versionCode") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->pakcageInfo.result[0].versionCode.c_str());
        } else if (strcmp(utf8name, "verifyInfo") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->pakcageInfo.result[0].verifyInfo.c_str());
        } else if (strcmp(utf8name, "descriptPackageId") == 0) {
            *result = CreateNapiValue(napi_string, testEnv->pakcageInfo.result[0].descriptPackageId.c_str());
        } else if (strcmp(utf8name, "packageType") == 0) {
            *result = CreateNapiValue(napi_number, &testEnv->pakcageInfo.result[0].packageType);
        } else if (strcmp(utf8name, "size") == 0) {
            *result = CreateNapiValue(napi_bigint, &testEnv->pakcageInfo.result[0].size);
        }
    } else if (testValue->GetType() == TEST_NVALUE_TYPE_UPDATE_POLICY) {
        if (strcmp(utf8name, "autoDownload") == 0) {
            *result = CreateNapiValue(napi_number, (void*)&g_updatePolicy.autoDownload);
        } else if (strcmp(utf8name, "autoDownloadNet") == 0) {
            *result = CreateNapiValue(napi_string, (void*)&g_updatePolicy.autoDownloadNet);
        } else if (strcmp(utf8name, "mode") == 0) {
            *result = CreateNapiValue(napi_string, (void*)&g_updatePolicy.mode);
        } else if (strcmp(utf8name, "autoUpgradeCondition") == 0) {
            *result = CreateNapiValue(napi_string, (void*)&g_updatePolicy.autoUpgradeCondition);
        }
    }
    return napi_status::napi_ok;
}

napi_status napi_get_value_string_utf8(napi_env env, napi_value value, char* buf,
    size_t bufsize, size_t* result)
{
    TestNApiValue* testValue = (TestNApiValue*)value;
    if (buf != nullptr) {
        memcpy_s(buf, bufsize, testValue->strValue.data(), testValue->strValue.size());
    }
    printf("napi_get_value_string_utf8 %s bufsize %zu  %zu \n",
        testValue->strValue.c_str(), bufsize, testValue->strValue.size());
    *result = testValue->strValue.size();
    return napi_status::napi_ok;
}

napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result)
{
    TestNApiValue* testValue = (TestNApiValue*)value;
    *result = testValue->intValue;
    return napi_status::napi_ok;
}

napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result)
{
    TestNApiValue* testValue = (TestNApiValue*)value;
    *result = static_cast<uint32_t>(testValue->intValue);
    return napi_status::napi_ok;
}

napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result)
{
    TestNApiValue* testValue = (TestNApiValue*)value;
    *result = testValue->int64Value;
    return napi_status::napi_ok;
}

napi_status napi_set_named_property(napi_env env, napi_value object, const char *utf8name,
    napi_value value)
{
    TestNApiEnv* testEnv = (TestNApiEnv*)env;
    TestNApiValue* testValue = (TestNApiValue*)value;
    if (testValue == nullptr) {
        return napi_status::napi_ok;
    }
    if (testValue->GetType() == napi_string) {
        if (testValue->strValue.empty()) {
            return napi_status::napi_ok;
        }
        if (strcmp(utf8name, "versionName") == 0) {
            testEnv->pakcageInfo.result[0].versionName = testValue->strValue;
        } else if (strcmp(utf8name, "versionCode") == 0) {
            testEnv->pakcageInfo.result[0].versionCode = testValue->strValue;
        } else if (strcmp(utf8name, "verifyInfo") == 0) {
            testEnv->pakcageInfo.result[0].verifyInfo = testValue->strValue;
        } else if (strcmp(utf8name, "descriptPackageId") == 0) {
            if (testValue->strValue.size() > 0) {
                testEnv->pakcageInfo.result[0].descriptPackageId = testValue->strValue;
            }
        } else if (strcmp(utf8name, "errMsg") == 0) {
            testEnv->errorMsg = testValue->strValue;
        }
    } else if (testValue->GetType() == napi_number) {
        if (strcmp(utf8name, "packageType") == 0) {
            testEnv->pakcageInfo.result[0].packageType = (PackageType)testValue->intValue;
        } else if (strcmp(utf8name, "status") == 0) {
            testEnv->progress.status = (UpgradeStatus)testValue->intValue;
        } else if (strcmp(utf8name, "percent") == 0) {
            testEnv->progress.percent = testValue->intValue;
        }
    } else if (testValue->GetType() == napi_bigint) {
        if (strcmp(utf8name, "size") == 0 && testValue->int64Value != 0) {
            testEnv->pakcageInfo.result[0].size = (size_t)testValue->int64Value;
        }
    }
    return napi_status::napi_ok;
}

// Asynchronous execution
napi_status napi_create_async_work(napi_env env, napi_value async_resource,
    napi_value async_resource_name, napi_async_execute_callback execute,
    napi_async_complete_callback complete, void *data, napi_async_work *result)
{
    UpdateSession *sess = static_cast<UpdateSession *>(data);
    g_testSessionId = sess->GetSessionId();
    printf(" g_testSessionId %u\n", g_testSessionId);
    *result = (napi_async_work)CreateNapiValue(0, nullptr);
    return napi_status::napi_ok;
}

napi_status napi_get_cb_info(
    napi_env env,              // [in] NAPI environment handle
    napi_callback_info cbinfo, // [in] Opaque callback-info handle
    size_t *argc,              // [in-out] Specifies the size of the provided argv array
    // and receives the actual count of args.
    napi_value *argv,          // [out] Array of values
    napi_value *this_arg,      // [out] Receives the JS 'this' arg for the call
    void **data)               // [out] Receives the data pointer for the callback.
{
    TestNApiEnv* testEnv = (TestNApiEnv*)env;
    printf("++++++++++++++ napi_get_cb_info argc %zu testStage %d \n", *argc, testEnv->testStage);
    switch (testEnv->testStage) {
        case SESSION_GET_UPDATER: {
            if (*argc > 1) {
                *argc = (testEnv->engineType != 0) ? NUMBER_2 : 1;
                argv[0] = CreateNapiValue(napi_string, testEnv->eventType.c_str());
                argv[1] = CreateNapiValue(napi_string, "OTA");
            } else {
                argv[0] = CreateNapiValue(napi_string, "OTA");
                *argc = 1;
            }
            break;
        }
        case SESSION_VERIFY_PACKAGE:{
            if (*argc > 1) {
                argv[0] = CreateNapiValue(napi_string, "/data/updater/updater/updater_success.zip");
                argv[1] = CreateNapiValue(napi_string, "/data/updater/src/signing_cert.crt");
            }
            *argc = NUMBER_2;
            break;
        }
        case SESSION_SET_POLICY: {
            argv[0] = CreateNapiValue(TEST_NVALUE_TYPE_UPDATE_POLICY, nullptr);
            if (*argc > 1) {
                argv[1] = CreateNapiValue(napi_function, nullptr);
            }
            *argc = testEnv->testAsyncorPermose ? 1 : NUMBER_2;
            break;
        }
        case SESSION_SUBSCRIBE:
        case SESSION_UNSUBSCRIBE: {
            argv[0] = CreateNapiValue(napi_string, testEnv->eventType.c_str());
            if (*argc > 1) {
                argv[1] = CreateNapiValue(napi_function, nullptr);
            }
            *argc = testEnv->testAsyncorPermose ? 1 : NUMBER_2;
            break;
        }
        default:{
            *argc = testEnv->testAsyncorPermose ? 0 : 1;
            *argv = CreateNapiValue(napi_function, nullptr);
            break;
        }
    }
    return napi_status::napi_ok;
}

napi_status napi_is_array(napi_env env, napi_value value, bool *result)
{
    *result = true;
    return napi_status::napi_ok;
}

napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t *result)
{
    *result = sizeof(g_updatePolicy.autoUpgradeInterval) / sizeof(g_updatePolicy.autoUpgradeInterval[0]);
    return napi_status::napi_ok;
}

napi_status napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value *result)
{
    if (index >= sizeof(g_updatePolicy.autoUpgradeInterval) / sizeof(g_updatePolicy.autoUpgradeInterval[0])) {
        *result = nullptr;
        return napi_status::napi_ok;
    }
    *result = CreateNapiValue(napi_number, &g_updatePolicy.autoUpgradeInterval[index]);
    return napi_status::napi_ok;
}

napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result)
{
    return napi_status::napi_ok;
}

napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope)
{
    return napi_status::napi_ok;
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    *result = true;
    return napi_status::napi_ok;
}

napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result)
{
    *result = true;
    return napi_status::napi_ok;
}

napi_status napi_create_promise(napi_env env, napi_deferred* deferred, napi_value* promise)
{
    return napi_status::napi_ok;
}

napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution)
{
    return napi_status::napi_ok;
}

napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection)
{
    return napi_status::napi_ok;
}

// Methods to work with external data objects
napi_status napi_wrap(napi_env env, napi_value jsObject,
    void* native_object, napi_finalize finalize_cb, void* finalize_hint, napi_ref* result)
{
    return napi_status::napi_ok;
}

napi_status napi_unwrap(napi_env env, napi_value jsObject, void** result)
{
    TestNApiEnv* testEnv = (TestNApiEnv*)env;
    if (testEnv->testStage && testEnv->noneClient) {
        *result = nullptr;
        testEnv->noneClient = false;
        return napi_status::napi_ok;
    }
    if (g_testClient == nullptr) {
        g_testClient = new UpdateClient(env, jsObject);
    }
    *result = g_testClient;
    return napi_status::napi_ok;
}

napi_status napi_define_class(napi_env env, const char* utf8name, size_t length, napi_callback constructor,
    void* data, size_t property_count, const napi_property_descriptor* properties, napi_value* result)
{
    std::function<napi_value(napi_env env, napi_callback_info info)> func = constructor;
    func(env, nullptr);
    return napi_status::napi_ok;
}

napi_status napi_new_instance(napi_env env, napi_value constructor, size_t argc, const napi_value* argv,
    napi_value* result)
{
    return napi_status::napi_ok;
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object, void** result)
{
    return napi_status::napi_ok;
}

#ifdef __cplusplus
}
#endif