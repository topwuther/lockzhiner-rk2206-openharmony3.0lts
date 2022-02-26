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

#include "on_request_permissions_result_stub.h"
#include "permission_log.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "OnRequestPermissionsResultStub"};
}
OnRequestPermissionsResultStub::OnRequestPermissionsResultStub()
{
    PERMISSION_LOG_INFO(LABEL, "create instance (%{public}p)", this);
}

OnRequestPermissionsResultStub::~OnRequestPermissionsResultStub()
{
    PERMISSION_LOG_INFO(LABEL, "destroy instance (%{public}p)", this);
}

int32_t OnRequestPermissionsResultStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    PERMISSION_LOG_INFO(LABEL, "message code is %{public}u", code);
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != OnRequestPermissionsResult::GetDescriptor()) {
        PERMISSION_LOG_INFO(
            LABEL, "%{public}s get unexpected descriptor %{public}s", __func__, Str16ToStr8(descriptor).c_str());
        return INVALID_DATA;
    }
    OnRequestPermissionsResult::Message msgCode = static_cast<OnRequestPermissionsResult::Message>(code);
    switch (msgCode) {
        case OnRequestPermissionsResult::Message::ON_RESULT: {
            std::string nodeId = data.ReadString();
            std::vector<std::string> permissions;
            data.ReadStringVector(&permissions);
            std::vector<int32_t> grantResults;
            data.ReadInt32Vector(&grantResults);
            OnResult(nodeId, permissions, grantResults);
            break;
        }
        case OnRequestPermissionsResult::Message::ON_CANCEL: {
            std::string nodeId = data.ReadString();
            std::vector<std::string> permissions;
            data.ReadStringVector(&permissions);
            std::vector<int32_t> grantResults;
            OnCancel(nodeId, permissions);
            break;
        }
        case OnRequestPermissionsResult::Message::ON_TIME_OUT: {
            std::string nodeId = data.ReadString();
            std::vector<std::string> permissions;
            data.ReadStringVector(&permissions);
            std::vector<int32_t> grantResults;
            OnTimeOut(nodeId, permissions);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
