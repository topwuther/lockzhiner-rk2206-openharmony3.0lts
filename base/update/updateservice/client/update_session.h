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

#ifndef UPDATE_SESSION_H
#define UPDATE_SESSION_H

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "iupdate_service.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "node_api.h"
#include "node_api_types.h"
#include "update_client.h"
#include "update_helper.h"

namespace updateClient {
class UpdateSession {
public:
    UpdateSession() = default;

    UpdateSession(UpdateClient *client, int32_t type, size_t argc, size_t callbackNumber);

    virtual ~UpdateSession() {};

    napi_value StartWork(napi_env env, size_t startIndex,
        const napi_value *args, UpdateClient::DoWorkFunction worker, void *context);

    UpdateClient* GetUpdateClient() const
    {
        return client_;
    }

    int32_t GetType() const
    {
        return type_;
    }

    uint32_t GetSessionId() const
    {
        return sessionId;
    }

    virtual void CompleteWork(napi_env env, napi_status status) {}
    virtual void ExecuteWork(napi_env env);
    virtual napi_value StartWork(napi_env env, size_t startIndex, const napi_value *args) = 0;
    virtual void NotifyJS(napi_env env, napi_value thisVar, int32_t retcode, const UpdateResult &result)
    {
        return;
    }

    // JS thread, which is used to notify the JS page upon completion of the operation.
    static void CompleteWork(napi_env env, napi_status status, void *data)
    {
        auto sess = reinterpret_cast<UpdateSession*>(data);
        CLIENT_CHECK(sess != nullptr && sess->GetUpdateClient() != nullptr, return, "Session is null pointer");
        sess->CompleteWork(env, status);
        // If the share ptr is used, you can directly remove the share ptr.
        UpdateClient *client = sess->GetUpdateClient();
        if (client != nullptr) {
            client->RemoveSession(sess->GetSessionId());
        }
    }

    // The C++ thread executes the synchronization operation. After the synchronization is complete,
    // the CompleteWork is called to notify the JS page of the completion of the operation.
    static void ExecuteWork(napi_env env, void* data)
    {
        auto sess = reinterpret_cast<UpdateSession*>(data);
        CLIENT_CHECK(sess != nullptr, return, "sess is null");
        sess->ExecuteWork(env);
    }
protected:
    napi_value CreateWorkerName(napi_env env) const;
    int32_t CreateReference(napi_env env, napi_value arg, uint32_t refcount, napi_ref &reference) const;

protected:
    uint32_t sessionId {0};
    UpdateClient *client_ = nullptr;
    int32_t type_ {};
    size_t totalArgc_ = 0;
    size_t callbackNumber_ = 0;
    void* context_ {};
    UpdateClient::DoWorkFunction doWorker_ {};
};

class UpdateAsyncession : public UpdateSession {
public:
    UpdateAsyncession(UpdateClient *client, int32_t type, size_t argc, size_t callbackNumber) :
        UpdateSession(client, type, argc, callbackNumber)
    {
        callbackRef_.resize(callbackNumber);
    }

    ~UpdateAsyncession() override
    {
        callbackRef_.clear();
    }

    void CompleteWork(napi_env env, napi_status status) override;
    napi_value StartWork(napi_env env, size_t startIndex, const napi_value *args) override;
private:
    napi_async_work worker_ = nullptr;
    std::vector<napi_ref> callbackRef_ = {0};
};

class UpdateAsyncessionNoCallback : public UpdateAsyncession {
public:
    UpdateAsyncessionNoCallback(UpdateClient *client, int32_t type, size_t argc, size_t callbackNumber) :
        UpdateAsyncession(client, type, argc, callbackNumber) {}

    ~UpdateAsyncessionNoCallback() override {   }

    void CompleteWork(napi_env env, napi_status status) override;
};

class UpdatePromiseSession : public UpdateSession {
public:
    UpdatePromiseSession(UpdateClient *client, int32_t type, size_t argc, size_t callbackNumber) :
        UpdateSession(client, type, argc, callbackNumber) {}

    ~UpdatePromiseSession() override {}

    void CompleteWork(napi_env env, napi_status status) override;
    napi_value StartWork(napi_env env, size_t startIndex, const napi_value *args) override;
private:
    napi_async_work worker_ = nullptr;
    napi_deferred deferred_ = nullptr;
};

class UpdateListener : public UpdateSession {
public:
    UpdateListener(UpdateClient *client, int32_t type, size_t argc, size_t callbackNumber, bool isOnce) :
        UpdateSession(client, type, argc, callbackNumber), isOnce_(isOnce) {}

    ~UpdateListener() override {}

    napi_value StartWork(napi_env env, size_t startIndex, const napi_value *args) override;

    void NotifyJS(napi_env env, napi_value thisVar, int32_t retcode, const UpdateResult &result) override;

    bool IsOnce() const
    {
        return isOnce_;
    }

    std::string GetEventType() const
    {
        return eventType_;
    }

    bool CheckEqual(napi_env env, napi_value handler, const std::string &type);

    void RemoveHandlerRef(napi_env env);
private:
    bool isOnce_ = false;
    std::string eventType_;
    napi_ref handlerRef_ = nullptr;
    std::mutex mutex_;
};
} // namespace updateClient
#endif // UPDATE_SESSION_H
