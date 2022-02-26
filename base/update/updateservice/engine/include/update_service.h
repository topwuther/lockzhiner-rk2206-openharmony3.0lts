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
#ifndef UPDATER_SERVICE_H
#define UPDATER_SERVICE_H

#include <iostream>
#include <thread>
#include "cJSON.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "iupdate_service.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include "progress_thread.h"
#include "system_ability.h"
#include "update_service_stub.h"

namespace OHOS {
namespace update_engine {
class UpdateService : public SystemAbility, public UpdateServiceStub {
public:
    DECLARE_SYSTEM_ABILITY(UpdateService);
    DISALLOW_COPY_AND_MOVE(UpdateService);
    explicit UpdateService(int32_t systemAbilityId, bool runOnCreate = true);
    ~UpdateService() override;

    int32_t RegisterUpdateCallback(const UpdateContext &ctx, const sptr<IUpdateCallback> &updateCallback) override;

    int32_t UnregisterUpdateCallback() override;

    int32_t CheckNewVersion() override;

    int32_t DownloadVersion() override;

    int32_t DoUpdate() override;

    int32_t GetNewVersion(VersionInfo &versionInfo) override;

    int32_t GetUpgradeStatus (UpgradeInfo &info) override;

    int32_t SetUpdatePolicy(const UpdatePolicy &policy) override;

    int32_t GetUpdatePolicy(UpdatePolicy &policy) override;

    int32_t Cancel(int32_t service) override;

    int32_t RebootAndClean(const std::string &miscFile, const std::string &cmd) override;

    int32_t RebootAndInstall(const std::string &miscFile, const std::string &packageName) override;
    static int32_t ParseJsonFile(const std::vector<char> &buffer, VersionInfo &info);
    static int32_t ReadCheckVersionResult(const cJSON* results, VersionInfo &info);
    static int32_t ReadCheckVersiondescriptInfo(const cJSON *descriptInfo, VersionInfo &info);
#ifndef UPDATER_UT
protected:
#else
public:
#endif
    void OnStart() override;
    void OnStop() override;
private:
    void DownloadCallback(const std::string &fileName, const Progress &progress);
    void UpgradeCallback(const Progress &progress);
    std::string GetDownloadServerUrl() const;
    void InitVersionInfo(VersionInfo &versionInfo) const;
#ifndef UPDATER_UT
private:
#else
public:
#endif
    void SearchCallback(const std::string &msg, SearchStatus status);
    bool VerifyDownloadPkg(const std::string &pkgName, Progress &progress);
    void ReadDataFromSSL(int32_t engineSocket);
private:
    UpdatePolicy policy_ = {
        1, 1, INSTALLMODE_AUTO, AUTOUPGRADECONDITION_IDLE, { 10, 20 }
    };
    UpgradeStatus upgradeStatus_ = UPDATE_STATE_INIT;
    VersionInfo versionInfo_ {};

    sptr<IUpdateCallback> updateCallback_ { nullptr };
    DownloadThread *downloadThread_  { nullptr };
    UpdateContext updateContext_ {};
};
}
} // namespace OHOS
#endif // UPDATER_SERVICE_H