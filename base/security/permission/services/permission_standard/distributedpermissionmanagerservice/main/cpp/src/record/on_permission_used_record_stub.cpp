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

#include "on_permission_used_record_stub.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "OnPermissionUsedRecordStub"};
}

using namespace std;

OnPermissionUsedRecordStub::OnPermissionUsedRecordStub()
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s construct", __func__);
}

OnPermissionUsedRecordStub::~OnPermissionUsedRecordStub()
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s de-construct", __func__);
}

int OnPermissionUsedRecordStub::OnRemoteRequest(
    uint32_t code, OHOS::MessageParcel &data, OHOS::MessageParcel &reply, OHOS::MessageOption &option)
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s called", __func__);
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != OnPermissionUsedRecord::GetDescriptor()) {
        PERMISSION_LOG_ERROR(
            LABEL, "%{public}s get unexpected descriptor %{public}s", __func__, Str16ToStr8(descriptor).c_str());
        return INVALID_DATA;
    }
    int msgCode = static_cast<int>(code);
    if (msgCode == OnPermissionUsedRecord::ON_QUERIED) {
        QueryPermissionUsedResult failedResult;
        int errCode = NO_ERROR;
        bool succeed = data.ReadInt32(errCode);
        if (!succeed || errCode != NO_ERROR) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s ReadInt32 fail", __func__);
            OnQueried(errCode, failedResult);
            return ERR_TRANSACTION_FAILED;
        }
        sptr<QueryPermissionUsedResult> result = data.ReadParcelable<QueryPermissionUsedResult>();
        if (result == nullptr) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s ReadParcelable fail", __func__);
            OnQueried(errCode, failedResult);
            return ERR_TRANSACTION_FAILED;
        }
        PERMISSION_LOG_INFO(LABEL,
            "%{public}s, errCode: %{public}s, result: %{public}s",
            __func__,
            to_string(errCode).c_str(),
            to_string(result->bundlePermissionUsedRecords.size()).c_str());
        OnQueried(errCode, *result);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
