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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "update_client.h"

namespace updateClient {
const std::string CLASS_NAME  = "UpdateClient";
constexpr int32_t REF_COUNT = 20;
napi_ref g_reference = nullptr;

napi_value UpdateClientJSConstructor(napi_env env, napi_callback_info info)
{
    // Count of argument is 1
    size_t argc = 1;
    // Set second argument
    napi_value args[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, args, &thisVar, &data);
    UpdateClient* client = new UpdateClient(env, thisVar);
    napi_wrap(env, thisVar, client,
        [](napi_env env, void* data, void* hint) {
            CLIENT_LOGI("UpdateClient Destructor");
            UpdateClient* client = (UpdateClient*)data;
            delete client;
            client = nullptr;
        },
        nullptr, nullptr);
    return thisVar;
}

UpdateClient *GetAndCreateJsUpdateClient(napi_env env, napi_callback_info info, napi_value &obj)
{
    napi_value constructor = nullptr;
    UpdateClient *client = nullptr;
    napi_status status = napi_get_reference_value(env, g_reference, &constructor);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get client");
    status = napi_new_instance(env, constructor, 0, nullptr, &obj);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Error get client");

    napi_unwrap(env, obj, (void**)&client);
    return client;
}

UpdateClient *GetUpdateClient(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    UpdateClient *client = nullptr;
    napi_unwrap(env, thisVar, (void**)&client);
    return client;
}

napi_value GetUpdater(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    if (client != nullptr) {
        return client->GetUpdater(env, info, 1);
    }
    napi_value obj = nullptr;
    client = GetAndCreateJsUpdateClient(env, info, obj);
    if (client != nullptr) {
        napi_value result = client->GetUpdater(env, info, 1);
        if (result != nullptr) {
            return obj;
        }
    }
    napi_remove_wrap(env, obj, (void**)&client);
    delete client;
    return nullptr;
}

napi_value GetUpdaterForOther(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    if (client != nullptr) {
        return client->GetUpdaterForOther(env, info);
    }
    napi_value obj = nullptr;
    client = GetAndCreateJsUpdateClient(env, info, obj);
    if (client != nullptr) {
        client->GetUpdaterForOther(env, info);
    }
    return obj;
}

napi_value GetUpdaterFromOther(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    if (client != nullptr) {
        return client->GetUpdaterFromOther(env, info);
    }
    napi_value obj = nullptr;
    client = GetAndCreateJsUpdateClient(env, info, obj);
    if (client != nullptr) {
        client->GetUpdaterFromOther(env, info);
    }
    return obj;
}

napi_value CheckNewVersion(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->CheckNewVersion(env, info);
}

napi_value SetUpdatePolicy(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->SetUpdatePolicy(env, info);
}

napi_value GetUpdatePolicy(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->GetUpdatePolicy(env, info);
}

napi_value DownloadVersion(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->DownloadVersion(env, info);
}

napi_value CancelUpgrade(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->CancelUpgrade(env, info);
}

napi_value UpgradeVersion(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->UpgradeVersion(env, info);
}

napi_value GetNewVersionInfo(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->GetNewVersionInfo(env, info);
}

napi_value GetUpgradeStatus(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->GetUpgradeStatus(env, info);
}

napi_value UnsubscribeEvent(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->UnsubscribeEvent(env, info);
}

napi_value SubscribeEvent(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->SubscribeEvent(env, info);
}

napi_value ApplyNewVersion(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->ApplyNewVersion(env, info);
}

napi_value RebootAndClean(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->RebootAndClean(env, info);
}

napi_value VerifyUpdatePackage(napi_env env, napi_callback_info info)
{
    UpdateClient* client = GetUpdateClient(env, info);
    CLIENT_CHECK_NAPI_CALL(env, client != nullptr, return nullptr, "Error get client");
    return client->VerifyUpdatePackage(env, info);
}

#ifdef UPDATER_UT
napi_value UpdateClientInit(napi_env env, napi_value exports)
#else
static napi_value UpdateClientInit(napi_env env, napi_value exports)
#endif
{
    CLIENT_LOGI("UpdateClientInit");
    // Registration function
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getUpdater", GetUpdater),
        DECLARE_NAPI_FUNCTION("getUpdaterForOther", GetUpdaterForOther),
        DECLARE_NAPI_FUNCTION("getUpdaterFromOther", GetUpdaterFromOther),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

    // Registration object
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("getUpdater", GetUpdater),
        DECLARE_NAPI_FUNCTION("getUpdaterForOther", GetUpdaterForOther),
        DECLARE_NAPI_FUNCTION("getUpdaterFromOther", GetUpdaterFromOther),

        DECLARE_NAPI_FUNCTION("checkNewVersion", CheckNewVersion),
        DECLARE_NAPI_FUNCTION("getNewVersionInfo", GetNewVersionInfo),

        DECLARE_NAPI_FUNCTION("setUpdatePolicy", SetUpdatePolicy),
        DECLARE_NAPI_FUNCTION("getUpdatePolicy", GetUpdatePolicy),
        DECLARE_NAPI_FUNCTION("getUpgradeStatus", GetUpgradeStatus),

        DECLARE_NAPI_FUNCTION("cancel", CancelUpgrade),
        DECLARE_NAPI_FUNCTION("download", DownloadVersion),
        DECLARE_NAPI_FUNCTION("upgrade", UpgradeVersion),

        DECLARE_NAPI_FUNCTION("applyNewVersion", ApplyNewVersion),
        DECLARE_NAPI_FUNCTION("rebootAndCleanUserData", RebootAndClean),
        DECLARE_NAPI_FUNCTION("verifyUpdatePackage", VerifyUpdatePackage),

        DECLARE_NAPI_FUNCTION("on", SubscribeEvent),
        DECLARE_NAPI_FUNCTION("off", UnsubscribeEvent)
    };

    napi_value result = nullptr;
    napi_define_class(env, CLASS_NAME.c_str(), CLASS_NAME.size(), UpdateClientJSConstructor,
        nullptr, sizeof(descriptors) / sizeof(*descriptors), descriptors, &result);
    napi_set_named_property(env, exports, CLASS_NAME.c_str(), result);
    napi_status status = napi_create_reference(env, result, REF_COUNT, &g_reference);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to create_reference");
    CLIENT_LOGI("UpdateClient g_reference %p", g_reference);
    return exports;
}

/*
 * Module definition
 */
static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = UpdateClientInit,
    .nm_modname = "libupdate.z.so",
    .nm_priv = ((void*)0),
    .reserved = { 0 }
};

/*
 * Module registration function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
}
