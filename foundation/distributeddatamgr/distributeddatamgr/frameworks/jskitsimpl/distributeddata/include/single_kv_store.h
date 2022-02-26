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
#ifndef OHOS_SINGLE_KV_STORE_H
#define OHOS_SINGLE_KV_STORE_H
#include <memory>
#include "napi/native_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "kv_manager.h"
#include "async_call.h"
#include "single_kvstore.h"

namespace OHOS::DistributedData {
class SingleKVStore final {
public:
    SingleKVStore() = default;
    ~SingleKVStore();
    SingleKVStore &operator=(std::unique_ptr<DistributedKv::SingleKvStore> &&singleKvStore);
    bool operator==(const std::unique_ptr<DistributedKv::SingleKvStore> &singleKvStore);
    static napi_value GetCtor(napi_env env);
    static napi_value OnEvent(napi_env env, napi_callback_info info);
    static napi_value Sync(napi_env env, napi_callback_info info);
    static napi_value Put(napi_env env, napi_callback_info info);
    static napi_value Get(napi_env env, napi_callback_info info);
    static napi_value Delete(napi_env env, napi_callback_info info);
private:
    enum JSSubscribeType {
        SUBSCRIBE_LOCAL = 0,
        SUBSCRIBE_REMOTE = 1,
        SUBSCRIBE_ALL = 2,
    };

    struct ContextInfo : public AsyncCall::Context {
        SingleKVStore *proxy = nullptr;
        std::string key;
        std::vector<uint8_t> value;
        napi_status status = napi_generic_failure;
        ContextInfo() : Context(nullptr, nullptr) { };
        ContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) { };
        virtual ~ContextInfo() {};

        napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
        {
            NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
            NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
            NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native kv store", napi_invalid_arg);
            return Context::operator()(env, argc, argv, self);
        }
        napi_status operator()(napi_env env, napi_value *result) override
        {
            if (status != napi_ok) {
                return status;
            }
            return Context::operator()(env, result);
        }
    };

    using Exec = std::function<napi_status(napi_env, size_t, napi_value *, napi_value, napi_value *)>;
    static napi_value Initialize(napi_env env, napi_callback_info info);

    static napi_status OnDataChange(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status OnSyncComplete(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static std::map<std::string, Exec> eventHandlers_;
    static napi_ref ctor_;

    std::unique_ptr<DistributedKv::SingleKvStore> kvStore_ = nullptr;
    std::shared_ptr<DistributedKv::KvStoreSyncCallback> syncObserver_ = nullptr;
    std::shared_ptr<DistributedKv::KvStoreObserver> dataObserver_[SUBSCRIBE_ALL + 1];
};

class DataObserver : public DistributedKv::KvStoreObserver {
public:
    DataObserver(napi_env env, napi_value callback);
    virtual ~DataObserver();
    void OnChange(const DistributedKv::ChangeNotification &notification,
                  std::unique_ptr<DistributedKv::KvStoreSnapshot> snapshot) override;
    void OnChange(const DistributedKv::ChangeNotification &notification) override;
private:
    struct EventDataWorker {
        const DataObserver *observer = nullptr;
        const DistributedKv::ChangeNotification data;
        EventDataWorker(const DataObserver * const & observerIn, const DistributedKv::ChangeNotification &dataIn)
            : observer(observerIn),
              data(dataIn.GetInsertEntries(), dataIn.GetUpdateEntries(),
                   dataIn.GetDeleteEntries(), dataIn.GetDeviceId(), false) {}
    };
    napi_ref callback_ = nullptr;
    napi_env env_;
    uv_loop_s *loop_ = nullptr;
};

class SyncObserver : public DistributedKv::KvStoreSyncCallback {
public:
    SyncObserver(napi_env env, napi_value callback);
    virtual ~SyncObserver();
    void SyncCompleted(const std::map<std::string, DistributedKv::Status> &results) override;
private:
    struct EventDataWorker {
        const SyncObserver *observer = nullptr;
        std::map<std::string, DistributedKv::Status> data;
    };
    napi_ref callback_ = nullptr;
    napi_env env_;
    uv_loop_s *loop_ = nullptr;
};
}
#endif // OHOS_SINGLE_KV_STORE_H
