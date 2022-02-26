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

#ifndef BASE_NOTIFICATION_CES_STANDARD_CESFWK_KITS_NAPI_COMMON_EVENT_INCLUDE_COMMON_EVENT_H
#define BASE_NOTIFICATION_CES_STANDARD_CESFWK_KITS_NAPI_COMMON_EVENT_INCLUDE_COMMON_EVENT_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common_event_publish_info.h"
#include "common_event_subscriber.h"

#include "want.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace EventFwkNapi {
using namespace OHOS::EventFwk;

static const std::int32_t CREATE_MAX_PARA = 2;
static const std::int32_t SUBSCRIBE_MAX_PARA = 2;
static const std::int32_t UNSUBSCRIBE_MAX_PARA = 2;
static const std::int32_t PUBLISH_MAX_PARA_BY_PUBLISHDATA = 3;

class SubscriberInstance;
struct AsyncCallbackInfoSubscribe;

struct subscriberInstanceInfo {
    std::vector<AsyncCallbackInfoSubscribe *> asyncCallbackInfo;
    std::shared_ptr<AsyncCommonEventResult> commonEventResult = nullptr;
};

static napi_ref g_CommonEventSubscriber = nullptr;
static std::map<std::shared_ptr<SubscriberInstance>, subscriberInstanceInfo> subscriberInstances;

struct AsyncCallbackInfoSubscribe {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
};

struct AsyncCallbackInfoUnsubscribe {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    size_t argc = 0;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
};

struct AsyncCallbackInfoSubscribeInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    std::vector<std::string> events;
    std::string permission;
    std::string deviceId;
    int32_t priority = 0;
    bool isCallback = false;
};

struct AsyncCallbackInfoOrderedCommonEvent {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    bool isCallback = false;
    bool isOrdered = false;
};

struct AsyncCallbackInfoStickyCommonEvent {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    bool isSticky = false;
    bool isCallback = false;
};

struct AsyncCallbackInfoGetCode {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    int code = 0;
    bool isCallback = false;
};

struct AsyncCallbackInfoSetCode {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    int code = 0;
    bool isCallback = false;
};

struct AsyncCallbackInfoGetData {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    std::string data;
    bool isCallback = false;
};

struct AsyncCallbackInfoSetData {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    std::string data;
    bool isCallback = false;
};

struct AsyncCallbackInfoSetCodeAndData {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    int code = 0;
    std::string data;
    bool isCallback = false;
};

struct AsyncCallbackInfoAbort {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    bool isCallback = false;
};

struct AsyncCallbackInfoClearAbort {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    bool isCallback = false;
};

struct AsyncCallbackInfoGetAbort {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    bool abortEvent = false;
    bool isCallback = false;
};

struct AsyncCallbackInfoFinish {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    bool isCallback = false;
};

struct AsyncCallbackInfoCreate {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref subscriberInfo = nullptr;
    bool isCallback = false;
    napi_value result = nullptr;
};

struct AsyncCallbackInfoPublish {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    CommonEventData commonEventData;
    CommonEventPublishInfo commonEventPublishInfo;
};

struct CommonEventPublishDataByjs {
    std::string bundleName;
    std::string data;
    int code = 0;
    std::vector<std::string> subscriberPermissions;
    bool isOrdered = false;
    bool isSticky = false;
    AAFwk::WantParams wantParams;
};

struct CommonEventDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    Want want;
    int code = 0;
    std::string data;
};

class SubscriberInstance : public CommonEventSubscriber {
public:
    SubscriberInstance(const CommonEventSubscribeInfo &sp);
    ~SubscriberInstance();

    virtual void OnReceiveEvent(const CommonEventData &data) override;

    void SetEnv(const napi_env &env);
    void SetCallbackRef(const napi_ref &ref);

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
};

napi_value NapiGetNull(napi_env env);

napi_value GetCallbackErrorValue(napi_env env, int errCode);

napi_value ParseParametersByCreateSubscriber(
    const napi_env &env, const napi_value (&argv)[CREATE_MAX_PARA], const size_t &argc, napi_ref &callback);

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value &result);

void SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result);

void ReturnCallbackPromise(const napi_env &env, const bool &isCallback, const napi_ref &callback,
    const napi_deferred &deferred, const napi_value &result);

void PaddingAsyncCallbackInfoCreateSubscriber(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoCreate *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value CreateSubscriber(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetSubscribeInfo(
    const napi_env &env, const size_t &argc, const napi_value (&argv)[1], napi_ref &callback);

void SetEventsResult(const napi_env &env, const std::vector<std::string> &events, napi_value &commonEventSubscribeInfo);

void SetPublisherPermissionResult(
    const napi_env &env, const std::string &permission, napi_value &commonEventSubscribeInfo);

void SetPublisherDeviceIdResult(const napi_env &env, const std::string &deviceId, napi_value &commonEventSubscribeInfo);

void SetPublisherPriorityResult(const napi_env &env, const int32_t &priority, napi_value &commonEventSubscribeInfo);

void PaddingAsyncCallbackInfoGetSubscribeInfo(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

void PaddingNapiCreateAsyncWorkCallbackInfo(
    AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, SubscriberInstance *&subscriber);

void SetNapiResult(const napi_env &env, const AsyncCallbackInfoSubscribeInfo *asynccallbackinfo, napi_value &result);

napi_value GetSubscribeInfo(napi_env env, napi_callback_info info);

napi_value ParseParametersByIsOrderedCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoIsOrderedCommonEvent(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoOrderedCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value IsOrderedCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByIsStickyCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoIsStickyCommonEvent(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoStickyCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value IsStickyCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetCode(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoGetCode(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetCode *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

std::shared_ptr<AsyncCommonEventResult> GetAsyncResult(const SubscriberInstance *objectInfo);

napi_value GetCode(napi_env env, napi_callback_info info);

napi_value ParseParametersBySetCode(
    const napi_env &env, const napi_value (&argv)[2], size_t argc, int &code, napi_ref &callback);

void PaddingAsyncCallbackInfoSetCode(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetCode *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value SetCode(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetData(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoGetData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value GetData(napi_env env, napi_callback_info info);

napi_value ParseParametersBySetData(
    const napi_env &env, const napi_value (&argv)[2], size_t argc, std::string &data, napi_ref &callback);

void PaddingAsyncCallbackInfoSetData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value SetData(napi_env env, napi_callback_info info);

napi_value ParseParametersBySetCodeAndData(
    const napi_env &env, const napi_value (&argv)[3], size_t argc, int &code, std::string &data, napi_ref &callback);

void PaddingAsyncCallbackInfoSetCodeAndData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetCodeAndData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value SetCodeAndData(napi_env env, napi_callback_info info);

napi_value ParseParametersByAbort(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoAbort(const napi_env &env, const size_t &argc, AsyncCallbackInfoAbort *&asynccallbackinfo,
    const napi_ref &callback, napi_value &promise);

napi_value AbortCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByClearAbort(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoClearAbort(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoClearAbort *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value ClearAbortCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetAbort(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoGetAbort(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetAbort *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value GetAbortCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByFinish(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoFinish(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoFinish *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value FinishCommonEvent(napi_env env, napi_callback_info info);

napi_value GetSubscriberBySubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber);

napi_value ParseParametersBySubscribe(const napi_env &env, const napi_value (&argv)[SUBSCRIBE_MAX_PARA],
    std::shared_ptr<SubscriberInstance> &subscriber, napi_ref &callback);

napi_value Subscribe(napi_env env, napi_callback_info info);

napi_value GetBundlenameByPublish(const napi_env &env, const napi_value &value, std::string &bundleName);

napi_value GetDataByPublish(const napi_env &env, const napi_value &value, std::string &data);

napi_value GetCodeByPublish(const napi_env &env, const napi_value &value, int &code);

napi_value GetSubscriberPermissionsByPublish(
    const napi_env &env, const napi_value &value, std::vector<std::string> &subscriberPermissions);

napi_value GetIsOrderedByPublish(const napi_env &env, const napi_value &value, bool &isOrdered);

napi_value GetIsStickyByPublish(const napi_env &env, const napi_value &value, bool &isSticky);

napi_value GetParametersByPublish(const napi_env &env, const napi_value &value, AAFwk::WantParams &wantParams);

napi_value ParseParametersByPublish(const napi_env &env, const napi_value (&argv)[PUBLISH_MAX_PARA_BY_PUBLISHDATA],
    const size_t &argc, std::string &event, CommonEventPublishDataByjs &commonEventPublishData, napi_ref &callback);

void PaddingCallbackInfoPublish(Want &want, AsyncCallbackInfoPublish *&asynccallbackinfo,
    const CommonEventPublishDataByjs &commonEventPublishDatajs);

napi_value Publish(napi_env env, napi_callback_info info);

napi_value GetSubscriberByUnsubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber, bool &isFind);

napi_value ParseParametersByUnsubscribe(const napi_env &env, const size_t &argc,
    const napi_value (&argv)[UNSUBSCRIBE_MAX_PARA], std::shared_ptr<SubscriberInstance> &subscriber,
    napi_ref &callback);

void NapiDeleteSubscribe(const napi_env &env, std::shared_ptr<SubscriberInstance> &subscriber);

napi_value Unsubscribe(napi_env env, napi_callback_info info);

napi_value GetEventsByCreateSubscriber(const napi_env &env, const napi_value &argv, std::vector<std::string> &events);

napi_value GetPublisherPermissionByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &permission, bool &hasProperty);

napi_value GetPublisherDeviceIdByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &publisherDeviceId, bool &hasProperty);

napi_value GetPriorityByCreateSubscriber(const napi_env &env, const napi_value &argv, int &priority, bool &hasProperty);

napi_value CommonEventSubscriberConstructor(napi_env env, napi_callback_info info);

napi_value CommonEventSubscriberInit(napi_env env, napi_value exports);

napi_value CommonEventInit(napi_env env, napi_value exports);
}  // namespace EventFwkNapi
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_CES_STANDARD_CESFWK_KITS_NAPI_COMMON_EVENT_INCLUDE_COMMON_EVENT_H