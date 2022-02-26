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

#include "update_service_proxy.h"
#include "update_helper.h"
#include "securec.h"

namespace OHOS {
namespace update_engine {
int32_t UpdateServiceProxy::RegisterUpdateCallback(const UpdateContext &ctx,
    const sptr<IUpdateCallback>& updateCallback)
{
    ENGINE_CHECK(updateCallback != nullptr, return ERR_INVALID_VALUE, "Invalid param");
    ENGINE_LOGI("UpdateServiceProxy::RegisterUpdateCallback");

    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    UpdateHelper::WriteUpdateContext(data, ctx);
    bool ret = data.WriteRemoteObject(updateCallback->AsObject());
    ENGINE_CHECK(ret, return ERR_FLATTEN_OBJECT, "Can not get remote");
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC };
    int32_t res = remote->SendRequest(REGISTER_CALLBACK, data, reply, option);
    ENGINE_CHECK(res == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}

int32_t UpdateServiceProxy::UnregisterUpdateCallback()
{
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC };
    int32_t res = remote->SendRequest(UNREGISTER_CALLBACK, data, reply, option);
    ENGINE_CHECK(res == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}

int32_t UpdateServiceProxy::CheckNewVersion()
{
    ENGINE_LOGI("UpdateServiceProxy::CheckNewVersion");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t res = remote->SendRequest(CHECK_VERSION, data, reply, option);
    ENGINE_CHECK(res == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}

int32_t UpdateServiceProxy::DownloadVersion()
{
    ENGINE_LOGI("UpdateServiceProxy::DownloadVersion");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    // Construct a data sending message to the stub.
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(DOWNLOAD, data, reply, option);
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}

int32_t UpdateServiceProxy::DoUpdate()
{
    ENGINE_LOGI("UpdateServiceProxy::DoUpdate");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    // Construct a data sending message to the stub.
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(UPGRADE, data, reply, option);
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}

int32_t UpdateServiceProxy::GetNewVersion(VersionInfo &versionInfo)
{
    ENGINE_LOGI("UpdateServiceProxy::GetNewVersion");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(GET_NEW_VERSION, data, reply, option);
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");

    return UpdateHelper::ReadVersionInfo(reply, versionInfo);
}

int32_t UpdateServiceProxy::GetUpgradeStatus(UpgradeInfo &info)
{
    ENGINE_LOGI("UpdateServiceProxy::GetUpgradeStatus");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(GET_STATUS, data, reply, option);
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");

    return UpdateHelper::ReadUpgradeInfo(reply, info);
}

int32_t UpdateServiceProxy::SetUpdatePolicy(const UpdatePolicy &policy)
{
    ENGINE_LOGI("UpdateServiceProxy::SetUpdatePolicy");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    UpdateHelper::WriteUpdatePolicy(data, policy);
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(SET_POLICY, data, reply, option);
    ENGINE_CHECK(res == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");

    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    ENGINE_CHECK(ret, return ERR_FLATTEN_OBJECT, "Failed to read length");
    return result;
}

int32_t UpdateServiceProxy::GetUpdatePolicy(UpdatePolicy &policy)
{
    ENGINE_LOGI("UpdateServiceProxy::GetUpdatePolicy");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(GET_POLICY, data, reply, option);
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return UpdateHelper::ReadUpdatePolicy(reply, policy);
}

int32_t UpdateServiceProxy::Cancel(int32_t service)
{
    ENGINE_LOGI("UpdateServiceProxy::Cancel");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    data.WriteInt32(static_cast<int32_t>(service));
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(CANCEL, data, reply, option);
    ENGINE_CHECK(res == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error res %d", res);

    int32_t result = -1;
    bool ret = reply.ReadInt32(result);
    ENGINE_CHECK(ret, return ERR_FLATTEN_OBJECT, "Failed to read result");
    return result;
}

int32_t UpdateServiceProxy::RebootAndClean(const std::string &miscFile, const std::string &cmd)
{
    ENGINE_LOGI("UpdateServiceProxy::RebootAndCleanUserData");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    data.WriteString16(Str8ToStr16(miscFile));
    data.WriteString16(Str8ToStr16(cmd));
    MessageParcel reply;
    MessageOption option;
    int32_t ret = ERR_OK;
#ifndef UPDATER_UT
    ret = remote->SendRequest(REBOOT_CLEAN, data, reply, option);
#endif
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}

int32_t UpdateServiceProxy::RebootAndInstall(const std::string &miscFile, const std::string &packageName)
{
    ENGINE_LOGI("UpdateServiceProxy::RebootAndCleanUserData");
    auto remote = Remote();
    ENGINE_CHECK(remote != nullptr, return ERR_FLATTEN_OBJECT, "Can not get remote");

    MessageParcel data;
    data.WriteString16(Str8ToStr16(miscFile));
    data.WriteString16(Str8ToStr16(packageName));
    MessageParcel reply;
    MessageOption option;
    int32_t ret = ERR_OK;
#ifndef UPDATER_UT
    ret = remote->SendRequest(REBOOT_INSTALL, data, reply, option);
#endif
    ENGINE_CHECK(ret == ERR_OK, return ERR_FLATTEN_OBJECT, "Transact error");
    return reply.ReadInt32();
}
}
} // namespace OHOS
