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

#ifndef UPDATER_SERVICE_KITS_H
#define UPDATER_SERVICE_KITS_H

#include <iostream>
#include "iupdate_service.h"
#include "update_helper.h"

namespace OHOS {
namespace update_engine {
class UpdateServiceKits {
public:
    UpdateServiceKits() = default;
    virtual ~UpdateServiceKits() = default;
    DISALLOW_COPY_AND_MOVE(UpdateServiceKits);

    /**
     * Get instance of ohos account manager.
     *
     * @return Instance of ohos account manager.
     */
    static UpdateServiceKits& GetInstance();

    virtual int32_t RegisterUpdateCallback(const UpdateContext &ctx, const UpdateCallbackInfo &cb) = 0;

    virtual int32_t UnregisterUpdateCallback() = 0;

    virtual int32_t CheckNewVersion() = 0;

    virtual int32_t DownloadVersion() = 0;

    virtual int32_t DoUpdate() = 0;

    virtual int32_t GetNewVersion(VersionInfo &versionInfo) = 0;

    virtual int32_t GetUpgradeStatus (UpgradeInfo &info) = 0;

    virtual int32_t SetUpdatePolicy(const UpdatePolicy &policy) = 0;

    virtual int32_t GetUpdatePolicy(UpdatePolicy &policy) = 0;

    virtual int32_t Cancel(int32_t service) = 0;

    virtual int32_t RebootAndClean(const std::string &miscFile, const std::string &cmd) = 0;

    virtual int32_t RebootAndInstall(const std::string &miscFile, const std::string &packageName) = 0;
};
} // namespace update_engine
} // namespace OHOS
#endif // UPDATER_SERVICE_KITS_H
