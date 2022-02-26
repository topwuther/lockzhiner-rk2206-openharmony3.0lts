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
#define LOG_TAG "KVManager"

#include "kv_manager.h"
#include <securec.h>
#include "js_util.h"
#include "distributed_kv_data_manager.h"
#include "async_call.h"
#include "single_kv_store.h"
#include "log_print.h"

using namespace OHOS::DistributedKv;
namespace OHOS::DistributedData {
napi_ref KVManager::ctor_ = nullptr;
napi_value KVManager::CreateKVManager(napi_env env, napi_callback_info info)
{
    ZLOGD("get kv manager!");
    struct ContextInfo {
        napi_ref ref = nullptr;
    };
    auto ctxInfo = std::make_shared<ContextInfo>();
    auto input = [ctxInfo](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        ZLOGD("CreateKVManager parser to native params %{public}d!", static_cast<int>(argc));
        NAPI_ASSERT_BASE(env, (argc > 0) && (argc <= 2), " should 1 or 2 parameters!", napi_invalid_arg);
        napi_value bundle = nullptr;
        napi_status retStatus = napi_get_named_property(env, argv[0], "bundleName", &bundle);
        NAPI_ASSERT_BASE(env,
            ((retStatus == napi_ok) && (bundle != nullptr) && !(JSUtil::Convert2String(env, bundle).empty())),
            " bundleName is null!", napi_invalid_arg);
        napi_value kvManger = nullptr;
        napi_status status = napi_new_instance(env, GetCtor(env), argc, argv, &kvManger);
        if ((kvManger == nullptr) || (status != napi_ok)) {
            return napi_generic_failure;
        }
        napi_create_reference(env, kvManger, 1, &(ctxInfo->ref));
        return napi_ok;
    };
    auto output = [ctxInfo](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_reference_value(env, ctxInfo->ref, result);
        napi_delete_reference(env, ctxInfo->ref);
        return status;
    };
    auto context = std::make_shared<AsyncCall::Context>(input, output);
    // function createKVManager(config: KVManagerConfig, callback: AsyncCallback<KVManager>): void;
    // AsyncCallback at position 1
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env);
}

napi_value KVManager::GetKVStore(napi_env env, napi_callback_info info)
{
    ZLOGD("get kv store!");
    struct ContextInfo {
        std::string storeId;
        Options options;
        KVManager *proxy = nullptr;
        SingleKVStore *kvStore = nullptr;
        napi_ref ref = nullptr;
    };
    auto ctxInfo = std::make_shared<ContextInfo>();
    auto input = [ctxInfo](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        ZLOGD("GetKVStore parser to native params %{public}d!", static_cast<int>(argc));
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&ctxInfo->proxy)), napi_invalid_arg);
        NAPI_ASSERT_BASE(env, ctxInfo->proxy != nullptr, "there is no native kv store", napi_invalid_arg);
        ctxInfo->storeId = JSUtil::Convert2String(env, argv[0]);
        ctxInfo->options = JSUtil::Convert2Options(env, argv[1]);
        napi_value kvStore = nullptr;
        napi_status status = napi_new_instance(env, SingleKVStore::GetCtor(env), argc, argv, &kvStore);
        napi_unwrap(env, kvStore, reinterpret_cast<void**>(&ctxInfo->kvStore));
        if (ctxInfo->kvStore == nullptr) {
            return napi_invalid_arg;
        }
        napi_create_reference(env, kvStore, 1, &(ctxInfo->ref));
        return status;
    };
    auto output = [ctxInfo](napi_env env, napi_value *result) -> napi_status {
        if (*(ctxInfo->kvStore) == nullptr) {
            ZLOGE("get kv store failed!");
            *result = nullptr;
            napi_delete_reference(env, ctxInfo->ref);
            return napi_object_expected;
        }
        ZLOGD("get kv store success!");
        napi_status status = napi_get_reference_value(env, ctxInfo->ref, result);
        napi_delete_reference(env, ctxInfo->ref);
        return status;
    };
    auto exec = [ctxInfo](AsyncCall::Context *ctx) {
        ctxInfo->proxy->kvDataManager_.GetSingleKvStore(
            ctxInfo->options, {ctxInfo->proxy->bundleName_}, {ctxInfo->storeId},
            [ctxInfo](Status, std::unique_ptr<SingleKvStore> kvStore) {
                *(ctxInfo->kvStore) = std::move(kvStore);
            });
    };
    auto context = std::make_shared<AsyncCall::Context>(input, output);
    // getKVStore<T extends KVStore>(storeId: string, options: Options, callback: AsyncCallback<T>): void;
    // AsyncCallback at position 2
    AsyncCall asyncCall(env, info, context, 2);
    return asyncCall.Call(env, exec);
}

napi_value KVManager::GetCtor(napi_env env)
{
    napi_value cons;
    if (ctor_ != nullptr) {
        NAPI_CALL(env, napi_get_reference_value(env, ctor_, &cons));
        return cons;
    }

    napi_property_descriptor clzDes[] = {
        DECLARE_NAPI_METHOD("getKVStore", GetKVStore)
    };
    NAPI_CALL(env, napi_define_class(env, "KVManager", NAPI_AUTO_LENGTH, Initialize, nullptr,
                                     sizeof(clzDes) / sizeof(napi_property_descriptor), clzDes, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &ctor_));
    return cons;
}

napi_value KVManager::Initialize(napi_env env, napi_callback_info info)
{
    ZLOGD("constructor kv manager!");
    napi_value self = nullptr;
    size_t argc = JSUtil::MAX_ARGC;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    napi_value bundle = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, argv[0], "bundleName", &bundle));
    auto *proxy = new KVManager();
    proxy->bundleName_ = JSUtil::Convert2String(env, bundle);
    auto finalize = [](napi_env env, void* data, void* hint) {
        KVManager *proxy = reinterpret_cast<KVManager *>(data);
        delete proxy;
    };
    if (napi_wrap(env, self, proxy, finalize, nullptr, nullptr) != napi_ok) {
        finalize(env, proxy, nullptr);
        return nullptr;
    }
    return self;
}
}
