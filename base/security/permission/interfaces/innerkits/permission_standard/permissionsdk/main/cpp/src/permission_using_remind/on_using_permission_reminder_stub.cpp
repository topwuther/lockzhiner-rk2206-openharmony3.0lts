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

#include "on_using_permission_reminder_stub.h"
#include "permission_log.h"
#include "permission_reminder_info.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION,
    "OnUsingPermissionReminderStub"};
}
OnUsingPermissionReminderStub::OnUsingPermissionReminderStub()
{
    PERMISSION_LOG_DEBUG(LABEL, "create instance (%{public}p)", this);
}

OnUsingPermissionReminderStub::~OnUsingPermissionReminderStub()
{
    PERMISSION_LOG_DEBUG(LABEL, "destroy instance (%{public}p)", this);
}

int32_t OnUsingPermissionReminderStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    PERMISSION_LOG_INFO(LABEL, "message code is %{public}u", code);
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != OnUsingPermissionReminder::GetDescriptor()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s get unexpected descriptor %{public}s", __func__,
            Str16ToStr8(descriptor).c_str());
        return INVALID_DATA;
    }
    OnUsingPermissionReminder::Message msgCode = static_cast<OnUsingPermissionReminder::Message>(code);
    switch (msgCode) {
        case OnUsingPermissionReminder::Message::START_USING_PERMISSION: {
            PermissionReminderInfo failedResult;
            sptr<PermissionReminderInfo> info = data.ReadParcelable<PermissionReminderInfo>();
            if (info == nullptr) {
                PERMISSION_LOG_INFO(LABEL, "%{public}s ReadParcelable fail", __func__);
                StartUsingPermission(failedResult);
                return ERR_TRANSACTION_FAILED;
            }
            StartUsingPermission(*info);
            break;
        }
        case OnUsingPermissionReminder::Message::STOP_USING_PERMISSION: {
            PermissionReminderInfo failedResult;
            sptr<PermissionReminderInfo> info = data.ReadParcelable<PermissionReminderInfo>();
            if (info == nullptr) {
                PERMISSION_LOG_INFO(LABEL, "%{public}s ReadParcelable fail", __func__);
                StopUsingPermission(failedResult);
                return ERR_TRANSACTION_FAILED;
            }
            StopUsingPermission(*info);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
