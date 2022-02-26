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

#ifndef UPDATER_PROXY_H
#define UPDATER_PROXY_H

#include "iremote_proxy.h"
#include "iupdate_service.h"

namespace OHOS {
namespace update_engine {
class UpdateServiceProxy : public IRemoteProxy<IUpdateService> {
public:
    explicit UpdateServiceProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IUpdateService>(impl) {}

    int32_t RegisterUpdateCallback(const UpdateContext &ctx, const sptr<IUpdateCallback>& updateCallback) override;

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
private:
    static inline BrokerDelegator<UpdateServiceProxy> delegator_;
};
}
} // namespace OHOS
#endif // UPDATER_PROXY_H