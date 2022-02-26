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

#include "update_callback_proxy.h"
#include "securec.h"
#include "update_helper.h"

namespace OHOS {
namespace update_engine {
void UpdateCallbackProxy::OnCheckVersionDone(const VersionInfo &info)
{
    ENGINE_LOGI("UpdateCallbackProxy::OnCheckVersionDone");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC };

    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return, "Can not get remote");

    int32_t result = UpdateHelper::WriteVersionInfo(data, info);
    ENGINE_CHECK(result == 0, return, "Can not WriteVersionInfo");

    result = remote->SendRequest(CHECK_VERSION, data, reply, option);
    ENGINE_CHECK(result == ERR_OK, return, "Can not SendRequest");
    return;
}

void UpdateCallbackProxy::OnDownloadProgress(const Progress &progress)
{
    ENGINE_LOGI("UpdateCallbackProxy::OnDownloadProgress");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC };

    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return, "Can not get remote");

    int32_t result = UpdateHelper::WriteUpdateProgress(data, progress);
    ENGINE_CHECK(result == 0, return, "Can not WriteUpdateProgress");

    result = remote->SendRequest(DOWNLOAD, data, reply, option);
    ENGINE_CHECK(result == ERR_OK, return, "Can not SendRequest %d", result);
    return;
}

void UpdateCallbackProxy::OnUpgradeProgress(const Progress &progress)
{
    ENGINE_LOGI("UpdateCallbackProxy::OnUpgradeProgress");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC };

    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return, "Can not get remote");

    int32_t result = UpdateHelper::WriteUpdateProgress(data, progress);
    ENGINE_CHECK(result == 0, return, "Can not WriteUpdateProgress");

    result = remote->SendRequest(UPGRADE, data, reply, option);
    ENGINE_CHECK(result == ERR_OK, return, "Can not SendRequest");
    return;
}
}
} // namespace OHOS
