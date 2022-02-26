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
#define LOG_TAG "SingleKVStore"

#include "single_kv_store.h"

#include <uv.h>

#include "async_call.h"
#include "js_util.h"
#include "log_print.h"
#include "single_kvstore.h"

using namespace OHOS::DistributedKv;
namespace OHOS::DistributedData {
napi_ref SingleKVStore::ctor_ = nullptr;
std::map<std::string, SingleKVStore::Exec> SingleKVStore::eventHandlers_ = {
    {"dataChange", SingleKVStore::OnDataChange},
    {"syncComplete", SingleKVStore::OnSyncComplete},
};

SingleKVStore::~SingleKVStore()
{
    if (kvStore_ == nullptr) {
        return;
    }

    for (int i = 0; i < SUBSCRIBE_ALL + 1; ++i) {
        if (dataObserver_[i] == nullptr) {
            continue;
        }
        kvStore_->UnSubscribeKvStore(static_cast<SubscribeType>(i + 1), dataObserver_[i]);
    }

    if (syncObserver_ != nullptr) {
        kvStore_->UnRegisterSyncCallback();
    }
}

napi_value SingleKVStore::GetCtor(napi_env env)
{
    if (ctor_ != nullptr) {
        napi_value cons = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, ctor_, &cons));
        return cons;
    }

    napi_property_descriptor clzDes[] = {
        DECLARE_NAPI_METHOD("put", SingleKVStore::Put),
        DECLARE_NAPI_METHOD("get", SingleKVStore::Get),
        DECLARE_NAPI_METHOD("delete", SingleKVStore::Delete),
        DECLARE_NAPI_METHOD("on", SingleKVStore::OnEvent),
        DECLARE_NAPI_METHOD("sync", SingleKVStore::Sync),
    };
    napi_value cons;
    NAPI_CALL(env, napi_define_class(env, "SingleKVStore", NAPI_AUTO_LENGTH, Initialize, nullptr,
                                     sizeof(clzDes) / sizeof(napi_property_descriptor), clzDes, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &ctor_));
    return cons;
}

napi_value SingleKVStore::Put(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<ContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        // 2 is the max number of parameters
        NAPI_ASSERT_BASE(env, argc >= 2, " should 2 or more parameters!", napi_invalid_arg);
        context->key = JSUtil::Convert2String(env, argv[0]);
        context->value = JSUtil::Convert2Vector(env, argv[1]);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        OHOS::DistributedKv::Key key(context->key);
        OHOS::DistributedKv::Value value(context->value);
        Status status = context->proxy->kvStore_->Put(key, value);
        context->status = (status != Status::SUCCESS) ? napi_generic_failure : napi_ok;
    };
    context->SetAction(std::move(input));
    // 2 is the callback position
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 2);
    return asyncCall.Call(env, exec);
}

napi_value SingleKVStore::Get(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<ContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc >= 1, " should 1 or more parameters!", napi_invalid_arg);
        context->key = JSUtil::Convert2String(env, argv[0]);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value data = JSUtil::Convert2JSValue(env, context->value);
        return napi_get_named_property(env, data, "value", result);
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        OHOS::DistributedKv::Key key(context->key);
        OHOS::DistributedKv::Value value;
        Status status = context->proxy->kvStore_->Get(key, value);
        if (status == Status::SUCCESS) {
            context->status = napi_ok;
            context->value = value.Data();
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1);
    return asyncCall.Call(env, exec);
}

napi_value SingleKVStore::Delete(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<ContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc >= 1, " should 1 or more parameters!", napi_invalid_arg);
        context->key = JSUtil::Convert2String(env, argv[0]);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        OHOS::DistributedKv::Key key(context->key);
        Status status = context->proxy->kvStore_->Delete(key);
        if (status == Status::SUCCESS) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1);
    return asyncCall.Call(env, exec);
}

napi_value SingleKVStore::OnEvent(napi_env env, napi_callback_info info)
{
    napi_value self = nullptr;
    size_t argc = JSUtil::MAX_ARGC;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    NAPI_ASSERT(env, argc > 0, "there is no args");
    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "key not string type");
    std::string event = JSUtil::Convert2String(env, argv[0]);

    ZLOGI("subscribe event:%{public}s", event.c_str());
    auto handle = eventHandlers_.find(event);
    NAPI_ASSERT(env, handle != eventHandlers_.end(), "invalid event");
    napi_value result = nullptr;
    handle->second(env, argc - 1, &argv[1], self, &result);
    return nullptr;
}

napi_value SingleKVStore::Sync(napi_env env, napi_callback_info info)
{
    napi_value self = nullptr;
    size_t argc = JSUtil::MAX_ARGC;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    // the number of parameters is greater than 2 is error
    NAPI_ASSERT_BASE(env, argc >= 2, "args is out of range", nullptr);
    NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", nullptr);
    SingleKVStore *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), nullptr);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native kv store", nullptr);

    std::vector<std::string> devices = JSUtil::Convert2StringArray(env, argv[0]);
    int32_t mode = int32_t(SyncMode::PUSH_PULL);
    napi_get_value_int32(env, argv[1], &mode);
    uint32_t delay = 0;
    // 3 is the max number of parameters
    if (argc >= 3) {
        napi_get_value_uint32(env, argv[2], &delay); // argv[2] is the third parameter
    }
    ZLOGD("sync data %{public}d, mode:%{public}d, devices:%{public}zu", static_cast<int>(argc), mode, devices.size());

    Status status = proxy->kvStore_->Sync(devices, static_cast<SyncMode>(mode), delay);
    NAPI_ASSERT_BASE(env, status == Status::SUCCESS, "call sync failed", nullptr);
    return nullptr;
}

napi_value SingleKVStore::Initialize(napi_env env, napi_callback_info info)
{
    ZLOGD("constructor single kv store!");
    napi_value self = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &self, nullptr));
    auto *proxy = new SingleKVStore();
    auto release = [proxy]() -> napi_value {
        delete proxy;
        return nullptr;
    };
    auto finalize = [](napi_env env, void *data, void *hint) {
        auto *proxy = reinterpret_cast<SingleKVStore *>(data);
        delete proxy;
    };
    NAPI_CALL_BASE(env, napi_wrap(env, self, proxy, finalize, nullptr, nullptr), release());
    return self;
}

napi_status SingleKVStore::OnDataChange(napi_env env, size_t argc, napi_value *argv, napi_value self,
                                        napi_value *result)
{
    // on(event: 'dataChange', type: SubscribeType, observer: Callback<ChangeNotification>): void;
    // except event,  there are 2 args
    NAPI_ASSERT_BASE(env, argc >= 2, "args is out of range", napi_invalid_arg);
    NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
    int32_t type = 0;
    napi_get_value_int32(env, argv[0], &type);
    NAPI_ASSERT_BASE(env, SUBSCRIBE_LOCAL <= type && type <= SUBSCRIBE_ALL, "type is out of range", napi_invalid_arg);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[1], &valueType);
    NAPI_ASSERT_BASE(env, valueType == napi_function, "callback is not a function", napi_invalid_arg);

    SingleKVStore *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native kv store", napi_invalid_arg);

    ZLOGI("subscribe data change type %{public}d", type);
    std::shared_ptr<KvStoreObserver> observer = std::make_shared<DataObserver>(env, argv[1]);
    Status status = proxy->kvStore_->SubscribeKvStore(static_cast<SubscribeType>(type + 1), observer);
    if (status != Status::SUCCESS) {
        return napi_generic_failure;
    }
    if (proxy->dataObserver_[type] != nullptr) {
        proxy->kvStore_->UnSubscribeKvStore(static_cast<SubscribeType>(type + 1), proxy->dataObserver_[type]);
    }
    proxy->dataObserver_[type] = std::move(observer);
    return napi_ok;
}

napi_status SingleKVStore::OnSyncComplete(napi_env env, size_t argc, napi_value *argv, napi_value self,
                                          napi_value *result)
{
    // on(event: 'syncComplete', syncCallback: Callback<Array<[string, number]>>): void;
    // except event,  there is 1 arg
    NAPI_ASSERT_BASE(env, argc >= 1, "args is out of range", napi_invalid_arg);
    NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT_BASE(env, valueType == napi_function, "callback is not function", napi_invalid_arg);

    SingleKVStore *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native kv store", napi_invalid_arg);
    std::shared_ptr<KvStoreSyncCallback> observer = std::make_shared<SyncObserver>(env, argv[0]);
    Status status = proxy->kvStore_->RegisterSyncCallback(observer);
    if (status != Status::SUCCESS) {
        return napi_generic_failure;
    }
    proxy->syncObserver_ = observer;
    return napi_ok;
}

SingleKVStore &SingleKVStore::operator=(std::unique_ptr<DistributedKv::SingleKvStore> &&singleKvStore)
{
    if (kvStore_ == singleKvStore) {
        return *this;
    }
    kvStore_ = std::move(singleKvStore);
    return *this;
}

bool SingleKVStore::operator==(const std::unique_ptr<DistributedKv::SingleKvStore> &singleKvStore)
{
    return kvStore_ == singleKvStore;
}

DataObserver::DataObserver(napi_env env, napi_value callback)
    : env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

DataObserver::~DataObserver()
{
    napi_delete_reference(env_, callback_);
}

void DataObserver::OnChange(const ChangeNotification &notification, std::unique_ptr<KvStoreSnapshot> snapshot)
{
    ZLOGD("data change insert:%{public}zu, update:%{public}zu, delete:%{public}zu",
          notification.GetInsertEntries().size(), notification.GetUpdateEntries().size(),
          notification.GetDeleteEntries().size());
}

void DataObserver::OnChange(const ChangeNotification &notification)
{
    ZLOGD("data change insert:%{public}zu, update:%{public}zu, delete:%{public}zu",
          notification.GetInsertEntries().size(), notification.GetUpdateEntries().size(),
          notification.GetDeleteEntries().size());
    KvStoreObserver::OnChange(notification);
    EventDataWorker *eventDataWorker = new EventDataWorker(this, notification);
    uv_work_t *work = new uv_work_t;
    work->data = eventDataWorker;
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            EventDataWorker *eventDataInner = reinterpret_cast<EventDataWorker *>(work->data);
            napi_value jsNotification = JSUtil::Convert2JSNotification(eventDataInner->observer->env_,
                eventDataInner->data);
            napi_value callback = nullptr;
            napi_value args[1] = {jsNotification};
            napi_get_reference_value(eventDataInner->observer->env_, eventDataInner->observer->callback_, &callback);
            napi_value global = nullptr;
            napi_get_global(eventDataInner->observer->env_, &global);
            napi_value result;
            napi_status callStatus = napi_call_function(eventDataInner->observer->env_, global, callback,
                1, args, &result);
            if (callStatus != napi_ok) {
                ZLOGE("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus, callback);
            }
            delete eventDataInner;
            eventDataInner = nullptr;
            delete work;
            work = nullptr;
        });
}

SyncObserver::SyncObserver(napi_env env, napi_value callback)
    : env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

SyncObserver::~SyncObserver()
{
    napi_delete_reference(env_, callback_);
}

void SyncObserver::SyncCompleted(const std::map<std::string, DistributedKv::Status> &results)
{
    EventDataWorker *eventDataWorker = new EventDataWorker();
    eventDataWorker->observer = this;
    eventDataWorker->data = results;
    uv_work_t *work = new uv_work_t;
    work->data = eventDataWorker;
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            EventDataWorker *eventDataInner = reinterpret_cast<EventDataWorker *>(work->data);
            std::map<std::string, int32_t> dataMap;
            for (const auto &[key, value] : eventDataInner->data) {
                dataMap.emplace(key, int32_t(value));
            }
            napi_value notification = JSUtil::Convert2JSTupleArray(eventDataInner->observer->env_, dataMap);
            napi_value callback = nullptr;
            napi_value args[1] = {notification};
            napi_get_reference_value(eventDataInner->observer->env_, eventDataInner->observer->callback_, &callback);
            napi_value global = nullptr;
            napi_value result = nullptr;
            napi_get_global(eventDataInner->observer->env_, &global);
            napi_call_function(eventDataInner->observer->env_, global, callback, 1, args, &result);
            delete eventDataInner;
            eventDataInner = nullptr;
            delete work;
            work = nullptr;
        });
}
}
