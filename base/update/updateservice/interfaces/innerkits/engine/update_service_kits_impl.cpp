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

#include "update_service_kits_impl.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "iupdate_service.h"
#include "iupdate_callback.h"
#include "securec.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace update_engine {
UpdateServiceKits& UpdateServiceKits::GetInstance()
{
    return DelayedRefSingleton<UpdateServiceKitsImpl>::GetInstance();
}

UpdateServiceKitsImpl::UpdateServiceKitsImpl() {}

UpdateServiceKitsImpl::~UpdateServiceKitsImpl() {}

void UpdateServiceKitsImpl::ResetService(const wptr<IRemoteObject>& remote)
{
    ENGINE_LOGI("Remote is dead, reset service instance");

    std::lock_guard<std::mutex> lock(updateServiceLock_);
    if (updateService_ != nullptr) {
        sptr<IRemoteObject> object = updateService_->AsObject();
        if ((object != nullptr) && (remote == object)) {
            object->RemoveDeathRecipient(deathRecipient_);
            updateService_ = nullptr;
        }
    }
}

sptr<IUpdateService> UpdateServiceKitsImpl::GetService()
{
    std::lock_guard<std::mutex> lock(updateServiceLock_);
    if (updateService_ != nullptr) {
        return updateService_;
    }

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ENGINE_CHECK(samgr != nullptr, return nullptr, "Get samgr failed");
    sptr<IRemoteObject> object = samgr->GetSystemAbility(UPDATE_DISTRIBUTED_SERVICE_ID);
    ENGINE_CHECK(object != nullptr, return nullptr, "Get update object from samgr failed");

    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new DeathRecipient();
    }

    if ((object->IsProxyObject()) && (!object->AddDeathRecipient(deathRecipient_))) {
        ENGINE_LOGE("Failed to add death recipient");
    }

    ENGINE_LOGI("get remote object ok");
    updateService_ = iface_cast<IUpdateService>(object);
    if (updateService_ == nullptr) {
        ENGINE_LOGE("account iface_cast failed");
    }
    return updateService_;
}

void UpdateServiceKitsImpl::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DelayedRefSingleton<UpdateServiceKitsImpl>::GetInstance().ResetService(remote);
}

UpdateServiceKitsImpl::RemoteUpdateCallback::RemoteUpdateCallback(const UpdateCallbackInfo &cb)
    : UpdateCallback()
{
    updateCallback_.checkNewVersionDone = cb.checkNewVersionDone;
    updateCallback_.downloadProgress = cb.downloadProgress;
    updateCallback_.upgradeProgress = cb.upgradeProgress;
}

UpdateServiceKitsImpl::RemoteUpdateCallback::~RemoteUpdateCallback()
{
    updateCallback_.checkNewVersionDone = nullptr;
    updateCallback_.downloadProgress = nullptr;
    updateCallback_.upgradeProgress = nullptr;
}

void UpdateServiceKitsImpl::RemoteUpdateCallback::OnCheckVersionDone(const VersionInfo &info)
{
    ENGINE_LOGE("OnCheckVersionDone VersionInfo status %d", info.status);
    ENGINE_LOGE("OnCheckVersionDone VersionInfo errMsg %s", info.errMsg.c_str());
    ENGINE_LOGE("OnCheckVersionDone VersionInfo versionName : %s", info.result[0].versionName.c_str());
    ENGINE_LOGE("OnCheckVersionDone VersionInfo versionCode : %s", info.result[0].versionCode.c_str());
    ENGINE_LOGE("OnCheckVersionDone VersionInfo verifyInfo : %s", info.result[0].verifyInfo.c_str());
    ENGINE_LOGE("OnCheckVersionDone VersionInfo size : %zu", info.result[0].size);
    if (updateCallback_.checkNewVersionDone != nullptr) {
        updateCallback_.checkNewVersionDone(info);
    }
}

void UpdateServiceKitsImpl::RemoteUpdateCallback::OnDownloadProgress(const Progress &progress)
{
    ENGINE_LOGE("OnDownloadProgress progress %u %d", progress.percent, progress.status);
    if (updateCallback_.downloadProgress != nullptr) {
        updateCallback_.downloadProgress(progress);
    }
}

void UpdateServiceKitsImpl::RemoteUpdateCallback::OnUpgradeProgress(const Progress &progress)
{
    ENGINE_LOGE("OnUpgradeProgress progress %u %d", progress.percent, progress.status);
    if (updateCallback_.upgradeProgress != nullptr) {
        updateCallback_.upgradeProgress(progress);
    }
}

int32_t UpdateServiceKitsImpl::RegisterUpdateCallback(const UpdateContext &ctx, const UpdateCallbackInfo &cb)
{
    updateContext_.upgradeDevId = ctx.upgradeDevId;
    updateContext_.controlDevId = ctx.controlDevId;
    updateContext_.upgradeApp = ctx.upgradeApp;
    updateContext_.type = ctx.type;
    updateContext_.upgradeFile = ctx.upgradeFile;
    if (remoteUpdateCallback_ == nullptr) {
        remoteUpdateCallback_ = new RemoteUpdateCallback(cb);
        ENGINE_CHECK(remoteUpdateCallback_ != nullptr, return -1, "Failed to create remote callback");
        auto updateService = GetService();
        ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
        updateService->RegisterUpdateCallback(ctx, remoteUpdateCallback_);
    }
    return 0;
}

int32_t UpdateServiceKitsImpl::UnregisterUpdateCallback()
{
    delete remoteUpdateCallback_;
    remoteUpdateCallback_ = nullptr;
    return 0;
}

int32_t UpdateServiceKitsImpl::CheckNewVersion()
{
    ENGINE_LOGI("UpdateServiceKitsImpl::CheckNewVersion");

    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->CheckNewVersion();
}

int32_t UpdateServiceKitsImpl::DownloadVersion()
{
    ENGINE_LOGI("UpdateServiceKitsImpl::DownloadVersion");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->DownloadVersion();
}

int32_t UpdateServiceKitsImpl::DoUpdate()
{
    ENGINE_LOGI("UpdateServiceKitsImpl::DoUpdate");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->DoUpdate();
}

int32_t UpdateServiceKitsImpl::GetNewVersion(VersionInfo &versionInfo)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::GetNewversion");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->GetNewVersion(versionInfo);
}

int32_t UpdateServiceKitsImpl::GetUpgradeStatus(UpgradeInfo &info)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::GetUpgradeStatus");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->GetUpgradeStatus(info);
}

int32_t UpdateServiceKitsImpl::SetUpdatePolicy(const UpdatePolicy &policy)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::SetUpdatePolicy");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->SetUpdatePolicy(policy);
}

int32_t UpdateServiceKitsImpl::GetUpdatePolicy(UpdatePolicy &policy)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::GetUpdatePolicy");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->GetUpdatePolicy(policy);
}

int32_t UpdateServiceKitsImpl::Cancel(int32_t service)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::Cancel %d", service);
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->Cancel(service);
}

int32_t UpdateServiceKitsImpl::RebootAndClean(const std::string &miscFile, const std::string &cmd)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::RebootAndCleanUserData");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
#ifndef UPDATER_API_TEST
    return updateService->RebootAndClean(miscFile, cmd);
#endif
    return 1;
}

int32_t UpdateServiceKitsImpl::RebootAndInstall(const std::string &miscFile, const std::string &packageName)
{
    ENGINE_LOGI("UpdateServiceKitsImpl::RebootAndInstall");
    auto updateService = GetService();
    ENGINE_CHECK(updateService != nullptr, return -1, "Get updateService failed");
    return updateService->RebootAndInstall(miscFile, packageName);
}
}
} // namespace OHOS
