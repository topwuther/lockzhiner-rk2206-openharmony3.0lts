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

#include "hdf_remote_adapter.h"
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "hdf_sbuf_ipc.h"

static constexpr int32_t THREAD_POOL_BASE_THREAD_COUNT = 1;
static int32_t g_remoteThreadMax = THREAD_POOL_BASE_THREAD_COUNT;

HdfRemoteServiceStub::HdfRemoteServiceStub(struct HdfRemoteService *service)
    : IPCObjectStub(std::u16string(u"")), service_(service)
{
}

int HdfRemoteServiceStub::OnRemoteRequest(uint32_t code,
    OHOS::MessageParcel &data, OHOS::MessageParcel &reply, OHOS::MessageOption &option)
{
    (void)option;
    if (service_ == nullptr) {
        return HDF_ERR_INVALID_OBJECT;
    }

    int ret = HDF_FAILURE;
    struct HdfSBuf *dataSbuf = ParcelToSbuf(&data);
    struct HdfSBuf *replySbuf = ParcelToSbuf(&reply);

    struct HdfRemoteDispatcher *dispatcher = service_->dispatcher;
    if (dispatcher != nullptr && dispatcher->Dispatch != nullptr) {
        ret = dispatcher->Dispatch((HdfRemoteService *)service_->target, code, dataSbuf, replySbuf);
    } else {
        HDF_LOGE("dispatcher or dispatcher->Dispatch is null, flags is: %{public}d", option.GetFlags());
    }

    HdfSBufRecycle(dataSbuf);
    HdfSBufRecycle(replySbuf);
    return ret;
}

HdfRemoteServiceStub::~HdfRemoteServiceStub()
{
}

HdfDeathNotifier::HdfDeathNotifier(struct HdfRemoteService *service, struct HdfDeathRecipient *recipient)
    : recipient_(recipient), service_(service)
{
}

HdfDeathNotifier::~HdfDeathNotifier()
{
}

void HdfDeathNotifier::OnRemoteDied(const OHOS::wptr<OHOS::IRemoteObject> &) /* who = 0 */
{
    if (recipient_ != nullptr) {
        recipient_->OnRemoteDied(recipient_, service_);
    }
}

static int HdfRemoteAdapterDispatch(struct HdfRemoteService *service, int code, HdfSBuf *data, HdfSBuf *reply)
{
    if (service == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }

    OHOS::MessageParcel *dataParcel = nullptr;
    OHOS::MessageParcel *replyParcel = nullptr;

    if (reply == nullptr) {
        static OHOS::MessageParcel dummyReply;
        dummyReply.FlushBuffer();
        replyParcel = &dummyReply;
    } else if (SbufToParcel(reply, &replyParcel)) {
        HDF_LOGE("%{public}s:invalid reply sbuf object to dispatch", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (SbufToParcel(data, &dataParcel)) {
        HDF_LOGE("%{public}s:invalid data sbuf object to dispatch", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    OHOS::MessageOption option;
    struct HdfRemoteServiceHolder *holder = reinterpret_cast<struct HdfRemoteServiceHolder *>(service);
    if (dataParcel != nullptr) {
        OHOS::sptr<OHOS::IRemoteObject> remote = holder->remote_;
        if (remote != nullptr) {
            return remote->SendRequest(code, *dataParcel, *replyParcel, option);
        }
    }
    return HDF_FAILURE;
}

HdfRemoteServiceHolder::HdfRemoteServiceHolder() : remote_(nullptr)
{
    service_.object_.objectId = HDF_OBJECT_ID_REMOTE_SERVICE;
    service_.dispatcher = nullptr;
    service_.target = nullptr;
}

void HdfRemoteAdapterAddDeathRecipient(
    struct HdfRemoteService *service, struct HdfDeathRecipient *recipient)
{
    struct HdfRemoteServiceHolder *holder = reinterpret_cast<struct HdfRemoteServiceHolder *>(service);
    if (holder == NULL) {
        return;
    }
    OHOS::sptr<OHOS::IRemoteObject> remote = holder->remote_;
    if (remote == nullptr) {
        return;
    }
    HdfDeathNotifier *notifier = new HdfDeathNotifier(service, recipient);
    remote->AddDeathRecipient(notifier);
}

struct HdfRemoteService *HdfRemoteAdapterBind(OHOS::sptr<OHOS::IRemoteObject> binder)
{
    struct HdfRemoteService *remoteService = nullptr;
    static HdfRemoteDispatcher dispatcher = {
        .Dispatch = HdfRemoteAdapterDispatch
    };

    struct HdfRemoteServiceHolder *holder = new HdfRemoteServiceHolder();
    if (holder != nullptr) {
        holder->remote_ = binder;
        remoteService = &holder->service_;
        remoteService->dispatcher = &dispatcher;
        return remoteService;
    }
    return nullptr;
}

struct HdfRemoteService *HdfRemoteAdapterObtain(void)
{
    struct HdfRemoteServiceHolder *holder = new HdfRemoteServiceHolder();
    holder->remote_ = new HdfRemoteServiceStub(&holder->service_);
    return &holder->service_;
}

void HdfRemoteAdapterRecycle(struct HdfRemoteService *object)
{
    struct HdfRemoteServiceHolder *holder = reinterpret_cast<struct HdfRemoteServiceHolder *>(object);
    if (holder != nullptr) {
        if (holder->remote_ != nullptr) {
            holder->remote_ = nullptr;
        }
        delete holder;
    }
}

int HdfRemoteAdapterAddService(const char *name, struct HdfRemoteService *service)
{
    if (name == nullptr || service == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }

    OHOS::sptr<OHOS::IServiceRegistry> sr = OHOS::ServiceRegistry::GetInstance();
    if (sr == nullptr) {
        HDF_LOGE("failed to get service registry");
        return HDF_FAILURE;
    }
    struct HdfRemoteServiceHolder *holder = reinterpret_cast<struct HdfRemoteServiceHolder *>(service);
    int ret = sr->AddService(OHOS::Str8ToStr16(name), holder->remote_);
    if (ret == 0) {
        (void)OHOS::IPCSkeleton::GetInstance().SetMaxWorkThreadNum(g_remoteThreadMax++);
    }
    return ret;
}

struct HdfRemoteService *HdfRemoteAdapterGetService(const char *name)
{
    if (name == nullptr) {
        return nullptr;
    }

    OHOS::sptr<OHOS::IServiceRegistry> sr = OHOS::ServiceRegistry::GetInstance();
    if (sr == nullptr) {
        HDF_LOGE("failed to get service registry");
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> remote = sr->GetService(OHOS::Str8ToStr16(name));
    if (remote != nullptr) {
        return HdfRemoteAdapterBind(remote);
    }
    return nullptr;
}

int HdfRemoteAdapterAddSa(int32_t saId, struct HdfRemoteService *service)
{
    if (service == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }

    auto saManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HDF_LOGE("failed to get sa manager");
        return HDF_FAILURE;
    }
    struct HdfRemoteServiceHolder *holder = reinterpret_cast<struct HdfRemoteServiceHolder *>(service);
    int ret = saManager->AddSystemAbility(saId, holder->remote_);
    (void)OHOS::IPCSkeleton::GetInstance().SetMaxWorkThreadNum(g_remoteThreadMax++);
    HDF_LOGI("add sa %{public}d, ret = %{public}s", saId, (ret == 0) ? "succ" : "fail");

    return HDF_SUCCESS;
}

struct HdfRemoteService *HdfRemoteAdapterGetSa(int32_t saId)
{
    auto saManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HDF_LOGE("failed to get sa manager");
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> remote = saManager->GetSystemAbility(saId);
    if (remote != nullptr) {
        return HdfRemoteAdapterBind(remote);
    } else {
        HDF_LOGE("failed to get sa %{public}d", saId);
    }
    return nullptr;
}
