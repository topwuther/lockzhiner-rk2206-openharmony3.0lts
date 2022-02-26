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

#include "on_permission_used_record_proxy.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "OnPermissionUsedRecordProxy"};
}
using namespace std;

OnPermissionUsedRecordProxy::OnPermissionUsedRecordProxy(const OHOS::sptr<OHOS::IRemoteObject> &object)
    : IRemoteProxy(object)
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s construct", __func__);
}

OnPermissionUsedRecordProxy::~OnPermissionUsedRecordProxy()
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s de-construct", __func__);
}

void OnPermissionUsedRecordProxy::OnQueried(OHOS::ErrCode code, QueryPermissionUsedResult &result)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s remote service null", __func__);
        return;
    }

    PERMISSION_LOG_DEBUG(LABEL, "%{public}s Begin SendRequest", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(OnPermissionUsedRecord::GetDescriptor());
    if (result.bundlePermissionUsedRecords.size() > MAX_RECORDS_THRESHOLD) {
        PERMISSION_LOG_WARN(LABEL,
            "%{public}s GetPermissionUsedRecords has a lot of size: %{public}s",
            __func__,
            to_string(result.bundlePermissionUsedRecords.size()).c_str());
        data.SetDataCapacity(MAX_CAPACITY);
    }
    bool succeed = data.WriteInt32(code);
    if (!succeed) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s WriteParcelable fail", __func__);
        return;
    }

    data.WriteParcelable(&result);
    int32_t ret = remote->SendRequest(OnPermissionUsedRecord::ON_QUERIED, data, reply, option);
    if (ret != 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s SendRequest fail, error: %{public}d", __func__, ret);
        return;
    }
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s SendRequest success", __func__);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS