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

#ifndef UPDATER_SERVICE_KITS_IMPL_H
#define UPDATER_SERVICE_KITS_IMPL_H

#include "update_service_kits.h"
#include "update_helper.h"
#include "singleton.h"
#include "iupdate_callback.h"
#include "update_callback.h"

namespace OHOS {
namespace update_engine {
class UpdateServiceKitsImpl final : public UpdateServiceKits,
    public DelayedRefSingleton<UpdateServiceKitsImpl> {
    DECLARE_DELAYED_REF_SINGLETON(UpdateServiceKitsImpl);
public:
    DISALLOW_COPY_AND_MOVE(UpdateServiceKitsImpl);

    int32_t RegisterUpdateCallback(const UpdateContext &ctx, const UpdateCallbackInfo &cb) final;

    int32_t UnregisterUpdateCallback() final;

    int32_t CheckNewVersion() final;

    int32_t DownloadVersion() final;

    int32_t DoUpdate() final;

    int32_t GetNewVersion(VersionInfo &versionInfo) final;

    int32_t GetUpgradeStatus(UpgradeInfo &info) final;

    int32_t SetUpdatePolicy(const UpdatePolicy &policy) final;

    int32_t GetUpdatePolicy(UpdatePolicy &policy) final;

    int32_t Cancel(int32_t service);

    int32_t RebootAndClean(const std::string &miscFile, const std::string &cmd) final;

    int32_t RebootAndInstall(const std::string &miscFile, const std::string &packageName) final;
#ifndef UPDATER_UT
private:
#else
public:
#endif
    // For call event procession
    class RemoteUpdateCallback final : public UpdateCallback {
    public:
        RemoteUpdateCallback(const UpdateCallbackInfo &cb);
        ~RemoteUpdateCallback();

        DISALLOW_COPY_AND_MOVE(RemoteUpdateCallback);

        void OnCheckVersionDone(const VersionInfo &info) final;

        void OnDownloadProgress(const Progress &progress) final;

        void OnUpgradeProgress(const Progress &progress) final;
    private:
        UpdateCallbackInfo updateCallback_ {};
    };

    // For death event procession
    class DeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        DeathRecipient() = default;
        ~DeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(DeathRecipient);
        void OnRemoteDied(const wptr<IRemoteObject>& remote) final;
    };
    void ResetService(const wptr<IRemoteObject>& remote);
    sptr<IUpdateService> GetService();
    std::mutex updateServiceLock_;
    sptr<IUpdateService> updateService_ {};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {};
    sptr<IUpdateCallback> remoteUpdateCallback_ {};
    UpdateContext updateContext_ {};
};
} // namespace update_engine
} // namespace OHOS
#endif // UPDATER_SERVICE_KITS_IMPL_H
