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

#include "on_request_permissions_result_proxy.h"
#include "permission_log.h"
#include "ipc_types.h"
#include "parcel.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "OnRequestPermissionsResultProxy"};
}
OnRequestPermissionsResultProxy::OnRequestPermissionsResultProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<OnRequestPermissionsResult>(object)
{
    PERMISSION_LOG_INFO(LABEL, "create proxy instance (%{public}p)", this);
}

OnRequestPermissionsResultProxy::~OnRequestPermissionsResultProxy()
{
    PERMISSION_LOG_INFO(LABEL, "destroy proxy instance (%{public}p)", this);
}

void OnRequestPermissionsResultProxy::OnResult(
    const std::string nodeId, std::vector<std::string> permissions, std::vector<int32_t> grantResults)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "OnResult remote service null.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(OnRequestPermissionsResult::GetDescriptor());
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write nodeId", __func__);
        return;
    }
    if (!data.WriteStringVector(permissions)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissions", __func__);
        return;
    }
    if (!data.WriteInt32Vector(grantResults)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write grantResults", __func__);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "OnResult::Begin SendRequest");
    int32_t ret =
        remote->SendRequest(static_cast<int32_t>(OnRequestPermissionsResult::Message::ON_RESULT), data, reply, option);
    if (ret != 0) {
        PERMISSION_LOG_INFO(LABEL, "OnResult SendRequest fail, error: %{public}d", ret);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "OnResult::SendRequest success");
}

void OnRequestPermissionsResultProxy::OnCancel(const std::string nodeId, std::vector<std::string> permissions)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "OnCancel remote service null.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(OnRequestPermissionsResult::GetDescriptor());
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write nodeId", __func__);
        return;
    }
    if (!data.WriteStringVector(permissions)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissions", __func__);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "OnCancel::Begin SendRequest");
    int32_t ret =
        remote->SendRequest(static_cast<int32_t>(OnRequestPermissionsResult::Message::ON_CANCEL), data, reply, option);
    if (ret != 0) {
        PERMISSION_LOG_INFO(LABEL, "OnCancel SendRequest fail, error: %{public}d", ret);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "OnCancel::SendRequest success");
}
void OnRequestPermissionsResultProxy::OnTimeOut(const std::string nodeId, std::vector<std::string> permissions)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "OnTimeOut remote service null.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(OnRequestPermissionsResult::GetDescriptor());
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write nodeId", __func__);
        return;
    }
    if (!data.WriteStringVector(permissions)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissions", __func__);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "OnTimeOut::Begin SendRequest");
    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(OnRequestPermissionsResult::Message::ON_TIME_OUT), data, reply, option);
    if (ret != 0) {
        PERMISSION_LOG_INFO(LABEL, "OnTimeOut SendRequest fail, error: %{public}d", ret);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "OnTimeOut::SendRequest success");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
