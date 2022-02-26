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

#include "common_event_manager_service.h"
#include "bundle_manager_helper.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
#include "ipc_skeleton.h"
#include "publish_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace EventFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<CommonEventManagerService>::GetInstance().get());

CommonEventManagerService::CommonEventManagerService()
    : SystemAbility(COMMON_EVENT_SERVICE_ID, true),
      serviceRunningState_(ServiceRunningState::STATE_NOT_START),
      runner_(nullptr),
      handler_(nullptr)
{
    EVENT_LOGI("instance created");
    innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
}

CommonEventManagerService::~CommonEventManagerService()
{
    EVENT_LOGI("instance destroyed");
}

void CommonEventManagerService::OnStart()
{
    EVENT_LOGI("ready to start service");

    if (serviceRunningState_ == ServiceRunningState::STATE_RUNNING) {
        EVENT_LOGW("Failed to start service since it's already running");
        return;
    }

    ErrCode errCode = Init();
    if (FAILED(errCode)) {
        EVENT_LOGE("Failed to init, errCode: %{public}08x", errCode);
        return;
    }

    serviceRunningState_ = ServiceRunningState::STATE_RUNNING;

    EVENT_LOGI("start service success");
}

void CommonEventManagerService::OnStop()
{
    EVENT_LOGI("ready to stop service");

    serviceRunningState_ = ServiceRunningState::STATE_NOT_START;

    if (handler_) {
        handler_.reset();
    }

    if (runner_) {
        runner_.reset();
    }

    EVENT_LOGI("stop service success");
}

ErrCode CommonEventManagerService::Init()
{
    EVENT_LOGI("ready to init");

    if (!innerCommonEventManager_) {
        EVENT_LOGE("Failed to init without inner service");
        return ERR_INVALID_OPERATION;
    }

    runner_ = EventRunner::Create(true);
    if (!runner_) {
        EVENT_LOGE("Failed to init due to create runner error");
        return ERR_INVALID_OPERATION;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    if (!handler_) {
        EVENT_LOGE("Failed to init due to create handler error");
        return ERR_INVALID_OPERATION;
    }

    if (!Publish(this)) {
        EVENT_LOGE("Failed to publish CommonEventManagerService to SystemAbilityMgr");
        return ERR_INVALID_OPERATION;
    }

    EVENT_LOGI("init success");

    return ERR_OK;
}

bool CommonEventManagerService::IsReady() const
{
    if (!innerCommonEventManager_) {
        EVENT_LOGE("innerCommonEventManager is null");
        return false;
    }

    if (!handler_) {
        EVENT_LOGE("handler is null");
        return false;
    }

    return true;
}

bool CommonEventManagerService::PublishCommonEvent(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    uid_t callingUid = IPCSkeleton::GetCallingUid();

    return PublishCommonEventDetailed(event, publishinfo, commonEventListener, callingUid, callingPid);
}

bool CommonEventManagerService::PublishCommonEvent(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener, const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    return PublishCommonEventDetailed(event, publishinfo, commonEventListener, uid, -1);
}

bool CommonEventManagerService::PublishCommonEventDetailed(const CommonEventData &event,
    const CommonEventPublishInfo &publishinfo, const sptr<IRemoteObject> &commonEventListener, const uid_t &uid,
    const pid_t &pid)
{
    EVENT_LOGI("enter");

    struct tm recordTime = {0};
    if (!GetSystemCurrentTime(&recordTime)) {
        EVENT_LOGE("Failed to GetSystemCurrentTime");
        return false;
    }

    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(uid);

    if (!DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(uid)) {
        EVENT_LOGE("Too many common events have been sent in a short period from %{public}s (pid = %{public}d, uid = "
                   "%{public}d)",
            bundleName.c_str(),
            pid,
            uid);
        return false;
    }

    std::function<void()> PublishCommonEventFunc = std::bind(&InnerCommonEventManager::PublishCommonEvent,
        innerCommonEventManager_,
        event,
        publishinfo,
        commonEventListener,
        recordTime,
        pid,
        uid,
        bundleName);
    return handler_->PostTask(PublishCommonEventFunc);
}

bool CommonEventManagerService::SubscribeCommonEvent(
    const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    struct tm recordTime = {0};
    if (!GetSystemCurrentTime(&recordTime)) {
        EVENT_LOGE("Failed to GetSystemCurrentTime");
        return false;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    uid_t callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(callingUid);

    std::function<void()> SubscribeCommonEventFunc = std::bind(&InnerCommonEventManager::SubscribeCommonEvent,
        innerCommonEventManager_,
        subscriberInfo,
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        bundleName);
    return handler_->PostTask(SubscribeCommonEventFunc);
}

bool CommonEventManagerService::UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    std::function<void()> UnsubscribeCommonEventFunc =
        std::bind(&InnerCommonEventManager::UnsubscribeCommonEvent, innerCommonEventManager_, commonEventListener);
    return handler_->PostTask(UnsubscribeCommonEventFunc);
}

bool CommonEventManagerService::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    if (event.empty()) {
        EVENT_LOGE("event is empty");
        return false;
    }

    uid_t callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = DelayedSingleton<BundleManagerHelper>::GetInstance()->GetBundleName(callingUid);
    const std::string permission = "ohos.permission.COMMONEVENT_STICKY";
    bool ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(bundleName, permission);
    if (!ret) {
        EVENT_LOGE("No permission to get a sticky common event from %{public}s (uid = %{public}d)",
            bundleName.c_str(),
            callingUid);
        return false;
    }

    return innerCommonEventManager_->GetStickyCommonEvent(event, eventData);
}

bool CommonEventManagerService::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    innerCommonEventManager_->DumpState(event, state);

    return true;
}

bool CommonEventManagerService::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int &code, const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    std::function<void()> FinishReceiverFunc = std::bind(
        &InnerCommonEventManager::FinishReceiver, innerCommonEventManager_, proxy, code, receiverData, abortEvent);
    return handler_->PostTask(FinishReceiverFunc);
}

bool CommonEventManagerService::Freeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    std::function<void()> FreezeFunc = std::bind(&InnerCommonEventManager::Freeze, innerCommonEventManager_, uid);
    return handler_->PostImmediateTask(FreezeFunc);
}

bool CommonEventManagerService::Unfreeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        return false;
    }

    std::function<void()> UnfreezeFunc = std::bind(&InnerCommonEventManager::Unfreeze, innerCommonEventManager_, uid);
    return handler_->PostImmediateTask(UnfreezeFunc);
}
}  // namespace EventFwk
}  // namespace OHOS
