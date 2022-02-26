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

#include "update_service_stub.h"
#include "update_helper.h"
#include "securec.h"

using namespace std;

namespace OHOS {
namespace update_engine {
static int32_t GetNewVersionStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    VersionInfo info = {};
    int32_t ret = service->GetNewVersion(info);
    ENGINE_CHECK(ret == 0, return -1, "Failed to get new version");
    UpdateHelper::WriteVersionInfo(reply, info);
    return 0;
}

static int32_t GetUpgradeStatusStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    UpgradeInfo info = {};
    int32_t ret = service->GetUpgradeStatus(info);
    ENGINE_CHECK(ret == 0, return -1, "Failed to get new upgrade status");
    UpdateHelper::WriteUpgradeInfo(reply, info);
    return 0;
}

static int32_t CheckNewVersionStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    int32_t ret = service->CheckNewVersion();
    reply.WriteInt32(ret);
    return 0;
}

static int32_t DownloadVersionStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    int32_t ret = service->DownloadVersion();
    reply.WriteInt32(ret);
    return 0;
}

static int32_t DoUpdateStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    int32_t ret = service->DoUpdate();
    reply.WriteInt32(ret);
    return 0;
}

static int32_t SetUpdatePolicyStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    UpdatePolicy policy = {};
    UpdateHelper::ReadUpdatePolicy(data, policy);
    int32_t ret = service->SetUpdatePolicy(policy);
    reply.WriteInt32(ret);
    return 0;
}

static int32_t GetUpdatePolicyStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    UpdatePolicy policy = {};
    int32_t ret = service->GetUpdatePolicy(policy);
    ENGINE_CHECK(ret == 0, return -1, "Failed to get new get policy");
    UpdateHelper::WriteUpdatePolicy(reply, policy);
    return 0;
}

static int32_t RegisterUpdateCallbackStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    int32_t ret = -1;
    UpdateContext ctx {};
    UpdateHelper::ReadUpdateContext(data, ctx);
    auto remote = data.ReadRemoteObject();
    ENGINE_CHECK(remote != nullptr, reply.WriteInt32(ret); return 0, "Failed to read remote obj");
    ret = service->RegisterUpdateCallback(ctx, iface_cast<IUpdateCallback>(remote));
    reply.WriteInt32(ret);
    return 0;
}

static int32_t UnregisterUpdateCallbackStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    int32_t ret = service->UnregisterUpdateCallback();
    reply.WriteInt32(ret);
    return 0;
}

static int32_t CancelStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    int32_t ret = service->Cancel(data.ReadInt32());
    reply.WriteInt32(ret);
    return 0;
}

static int32_t RebootAndCleanStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    string miscFile = Str16ToStr8(data.ReadString16());
    string cmd = Str16ToStr8(data.ReadString16());
    int32_t ret = service->RebootAndClean(miscFile, cmd);
    reply.WriteInt32(ret);
    return 0;
}

static int32_t RebootAndInstallStub(UpdateServiceStub::UpdateServiceStubPtr service,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    ENGINE_CHECK(service != nullptr, return -1, "Invalid param");
    string miscFile = Str16ToStr8(data.ReadString16());
    string packageName = Str16ToStr8(data.ReadString16());
    int32_t ret = service->RebootAndInstall(miscFile, packageName);
    reply.WriteInt32(ret);
    return 0;
}

int32_t UpdateServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    static std::map<uint32_t, UpdateServiceStub::RequestFuncType> requestFuncMap = {
        {IUpdateService::CHECK_VERSION, CheckNewVersionStub},
        {IUpdateService::DOWNLOAD, DownloadVersionStub},
        {IUpdateService::UPGRADE, DoUpdateStub},
        {IUpdateService::SET_POLICY, SetUpdatePolicyStub},
        {IUpdateService::GET_POLICY, GetUpdatePolicyStub},
        {IUpdateService::GET_NEW_VERSION, GetNewVersionStub},
        {IUpdateService::GET_STATUS, GetUpgradeStatusStub},
        {IUpdateService::REGISTER_CALLBACK, RegisterUpdateCallbackStub},
        {IUpdateService::UNREGISTER_CALLBACK, UnregisterUpdateCallbackStub},
        {IUpdateService::CANCEL, CancelStub},
        {IUpdateService::REBOOT_CLEAN, RebootAndCleanStub},
        {IUpdateService::REBOOT_INSTALL, RebootAndInstallStub},
    };

    ENGINE_LOGI("UpdateServiceStub OnRemoteRequest code %u", code);
    for (auto inter = requestFuncMap.begin(); inter != requestFuncMap.end(); inter++) {
        if (inter->first == code) {
            return inter->second(this, data, reply, option);
        }
    }
    ENGINE_LOGE("UpdateServiceStub OnRemoteRequest code %u not found", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}
} // namespace OHO