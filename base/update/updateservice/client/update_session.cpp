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

#include "update_session.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "node_api.h"
#include "update_client.h"
#include "update_helper.h"
#include "package/package.h"
#include "securec.h"

using namespace std;
using namespace OHOS::update_engine;

namespace updateClient {
const int32_t RESULT_ARGC = 2;

uint32_t g_sessionId = 0;
UpdateSession::UpdateSession(UpdateClient *client, int32_t type, size_t argc, size_t callbackNumber)
    : sessionId(++g_sessionId), client_(client), type_(type), totalArgc_(argc), callbackNumber_(callbackNumber) {}

int32_t UpdateSession::CreateReference(napi_env env, napi_value arg, uint32_t refcount,
    napi_ref &reference) const
{
    napi_valuetype valuetype;
    napi_status status = napi_typeof(env, arg, &valuetype);
    CLIENT_CHECK(status == napi_ok, return status, "Failed to napi_typeof");
    CLIENT_CHECK(valuetype == napi_function, return -1, "Invalid callback type");

    status = napi_create_reference(env, arg, refcount, &reference);
    CLIENT_CHECK(status == napi_ok, return status, "Failed to create reference");
    return status;
}

napi_value UpdateSession::CreateWorkerName(napi_env env) const
{
    napi_value workName;
    std::string name = "Async Work" + std::to_string(sessionId);
    napi_status status = napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &workName);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to worker name");
    return workName;
}

napi_value UpdateSession::StartWork(napi_env env,
    size_t startIndex, const napi_value *args, UpdateClient::DoWorkFunction worker, void *context)
{
    static std::string sessName[SESSION_MAX] = {
        "check version", "download", "upgrade", "set policy", "get policy",
        "get new version", "get upgrade status", "subscribe", "unsubscribe", "get update",
        "apply new version", "reboot and clean", "verify package", "Cancel Upgrade"
    };

    CLIENT_LOGI("StartWork type: %s", sessName[type_].c_str());
    doWorker_ = worker;
    context_ = context;
    return StartWork(env, startIndex, args);
}

void UpdateSession::ExecuteWork(napi_env env)
{
    if (doWorker_ != nullptr) {
#ifndef UPDATER_UT
        int32_t ret = doWorker_(type_, context_);
        CLIENT_CHECK_NAPI_CALL(env, ret == 0, return, "execute work");
#else
        doWorker_(type_, context_);
#endif
    }
    return;
}

napi_value UpdateAsyncession::StartWork(napi_env env, size_t startIndex, const napi_value *args)
{
    CLIENT_LOGI("UpdateAsyncession::StartWork startIndex: %zu", startIndex);
    CLIENT_LOGI("UpdateAsyncession::totalArgc_ %zu callbackNumber_: %zu", totalArgc_, callbackNumber_);
    CLIENT_CHECK_NAPI_CALL(env, args != nullptr && totalArgc_ >= startIndex, return nullptr, "Invalid para");
    napi_value workName = CreateWorkerName(env);
    CLIENT_CHECK_NAPI_CALL(env, workName != nullptr, return nullptr, "Failed to worker name");

    // Check whether a callback exists. Only one callback is allowed.
    for (size_t i = 0; (i < (totalArgc_ - startIndex)) && (i < callbackNumber_); i++) {
        CLIENT_LOGI("CreateReference index:%u", static_cast<unsigned int>(i + startIndex));
        int32_t ret = CreateReference(env, args[i + startIndex], 1, callbackRef_[i]);
        CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Failed to create reference");
    }

    // Create an asynchronous call.
    napi_status status = napi_create_async_work(env, nullptr, workName, UpdateSession::ExecuteWork,
        UpdateSession::CompleteWork, this, &(worker_));
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to create worker");

    // Put the thread in the task execution queue.
    status = napi_queue_async_work(env, worker_);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to queue worker");
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

void UpdateAsyncession::CompleteWork(napi_env env, napi_status status)
{
    CLIENT_LOGI("UpdateAsyncession::CompleteWork callbackNumber_: %d", static_cast<int32_t>(callbackNumber_));
    napi_value callback;
    napi_value undefined;
    napi_value callResult;
    napi_get_undefined(env, &undefined);
    napi_value retArgs[RESULT_ARGC] = { 0 };

    UpdateResult result;
    int32_t fail = 0;
    client_->GetUpdateResult(type_, result, fail);
    int ret = UpdateClient::BuildErrorResult(env, retArgs[0], fail);
    ret |= result.buildJSObject(env, retArgs[1], result);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return, "Failed to build json");

    status = napi_get_reference_value(env, callbackRef_[0], &callback);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return, "Failed to get reference");
    const int callBackNumber = 2;
    status = napi_call_function(env, undefined, callback, callBackNumber, retArgs, &callResult);
    // Release resources.
    for (size_t i = 0; i < callbackNumber_; i++) {
        napi_delete_reference(env, callbackRef_[i]);
        callbackRef_[i] = nullptr;
    }
    napi_delete_async_work(env, worker_);
    worker_ = nullptr;
}

void UpdateAsyncessionNoCallback::CompleteWork(napi_env env, napi_status status)
{
    CLIENT_LOGI("UpdateAsyncessionNoCallback::CompleteWork callbackNumber_: %d",
        static_cast<int32_t>(callbackNumber_));
}

napi_value UpdatePromiseSession::StartWork(napi_env env, size_t startIndex, const napi_value *args)
{
    CLIENT_LOGI("UpdatePromiseSession::StartWork");
    CLIENT_CHECK_NAPI_CALL(env, args != nullptr, return nullptr, "Invalid para");
    napi_value workName = CreateWorkerName(env);
    CLIENT_CHECK_NAPI_CALL(env, workName != nullptr, return nullptr, "Failed to worker name");

    napi_value promise;
    napi_status status = napi_create_promise(env, &deferred_, &promise);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to napi_create_promise");

    // Create an asynchronous call.
    status = napi_create_async_work(env, nullptr, workName, UpdateSession::ExecuteWork,
        UpdateSession::CompleteWork, this, &(worker_));
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to napi_create_async_work");
    // Put the thread in the task execution queue.
    status = napi_queue_async_work(env, worker_);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return nullptr, "Failed to napi_queue_async_work");
    return promise;
}

void UpdatePromiseSession::CompleteWork(napi_env env, napi_status status)
{
    CLIENT_LOGI("UpdatePromiseSession::CompleteWork status: %d", static_cast<int32_t>(status));
    // Get the return result.
    napi_value processResult;
    UpdateResult result;
    int32_t fail = 0;
    client_->GetUpdateResult(type_, result, fail);
    if (fail == 0) {
        result.buildJSObject(env, processResult, result);
        napi_resolve_deferred(env, deferred_, processResult);
    } else {
        UpdateClient::BuildErrorResult(env, processResult, fail);
        napi_reject_deferred(env, deferred_, processResult);
    }
    napi_delete_async_work(env, worker_);
    worker_ = nullptr;
}

napi_value UpdateListener::StartWork(napi_env env, size_t startIndex, const napi_value *args)
{
    CLIENT_LOGI("UpdateListener::StartWork");
    CLIENT_CHECK_NAPI_CALL(env, args != nullptr && totalArgc_ > startIndex, return nullptr, "Invalid para");
    int ret = UpdateClient::GetStringValue(env, args[0], eventType_);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Failed to get event type");

    // reference count is 1
    ret = CreateReference(env, args[startIndex], 1, handlerRef_);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return nullptr, "Failed to create reference");
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

void UpdateListener::NotifyJS(napi_env env, napi_value thisVar, int32_t retcode, const UpdateResult &result)
{
    CLIENT_LOGI("NotifyJS retcode:%d", retcode);
    napi_value jsEvent;
    napi_value handler = nullptr;
    napi_value callResult;
    int32_t ret = result.buildJSObject(env, jsEvent, result);
    CLIENT_CHECK_NAPI_CALL(env, ret == napi_ok, return, "Failed to build json");
    {
        std::unique_lock<std::mutex> lock(mutex_);
        CLIENT_CHECK_NAPI_CALL(env, handlerRef_ != nullptr, return, "handlerRef_ has beed freed");
        napi_status status = napi_get_reference_value(env, handlerRef_, &handler);
        CLIENT_CHECK_NAPI_CALL(env, status == napi_ok && handler != nullptr, return, "Failed to get reference");
    }
     CLIENT_CHECK_NAPI_CALL(env, handler != nullptr, return, "handlerRef_ has beed freed");
    napi_call_function(env, thisVar, handler, 1, &jsEvent, &callResult);
}

bool UpdateListener::CheckEqual(napi_env env, napi_value handler, const std::string &type)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool isEquals = false;
    napi_value handlerTemp = nullptr;
    napi_status status = napi_get_reference_value(env, handlerRef_, &handlerTemp);
    CLIENT_CHECK_NAPI_CALL(env, status == napi_ok, return false, "Failed to get reference");
    napi_strict_equals(env, handler, handlerTemp, &isEquals);
    return isEquals && (type.compare(eventType_) == 0);
}

void UpdateListener::RemoveHandlerRef(napi_env env)
{
    std::unique_lock<std::mutex> lock(mutex_);
    CLIENT_LOGI("RemoveHandlerRef handlerRef_:%{public}p %{public}u", handlerRef_, GetSessionId());
    napi_delete_reference(env, handlerRef_);
    handlerRef_ = nullptr;
}
} // namespace updateClient
