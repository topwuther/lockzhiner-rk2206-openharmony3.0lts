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

#include "on_using_permission_reminder_proxy.h"
#include "permission_log.h"
#include "ipc_types.h"
#include "parcel.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION,
    "OnUsingPermissionReminderProxy"};
}
OnUsingPermissionReminderProxy::OnUsingPermissionReminderProxy(const sptr<IRemoteObject>& object)
    : IRemoteProxy<OnUsingPermissionReminder>(object)
{
    PERMISSION_LOG_INFO(LABEL, "create proxy instance (%{public}p)", this);
}

OnUsingPermissionReminderProxy::~OnUsingPermissionReminderProxy()
{
    PERMISSION_LOG_INFO(LABEL, "destroy proxy instance (%{public}p)", this);
}

void OnUsingPermissionReminderProxy::StartUsingPermission(const PermissionReminderInfo& permReminderInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "StartUsingPermission remote service null.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(OnUsingPermissionReminder::GetDescriptor());
    if (!data.WriteParcelable(&permReminderInfo)) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s WriteParcelable fail", __func__);
        return;
    }

    PERMISSION_LOG_INFO(LABEL, "StartUsingPermission::Begin SendRequest");
    int32_t ret = remote->SendRequest(static_cast<int32_t>(OnUsingPermissionReminder::Message::START_USING_PERMISSION),
        data, reply, option);
    if (ret != 0) {
        PERMISSION_LOG_INFO(LABEL, "StartUsingPermission SendRequest fail, error: %{public}d", ret);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "StartUsingPermission::SendRequest success");
}

void OnUsingPermissionReminderProxy::StopUsingPermission(const PermissionReminderInfo& permReminderInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "StopUsingPermission remote service null.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(OnUsingPermissionReminder::GetDescriptor());
    if (!data.WriteParcelable(&permReminderInfo)) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s WriteParcelable fail", __func__);
        return;
    }

    PERMISSION_LOG_INFO(LABEL, "StopUsingPermission::Begin SendRequest");
    int32_t ret = remote->SendRequest(static_cast<int32_t>(OnUsingPermissionReminder::Message::STOP_USING_PERMISSION),
        data, reply, option);
    if (ret != 0) {
        PERMISSION_LOG_INFO(LABEL, "StopUsingPermission SendRequest fail, error: %{public}d", ret);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "StopUsingPermission::SendRequest success");
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
