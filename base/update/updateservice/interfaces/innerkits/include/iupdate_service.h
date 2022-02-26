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

#ifndef IUPDATER_SERVICE_H
#define IUPDATER_SERVICE_H

#include <iostream>
#include "update_helper.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iupdate_callback.h"

namespace OHOS {
namespace update_engine {
class IUpdateService : public OHOS::IRemoteBroker {
public:
    enum {
        CHECK_VERSION = 1,
        DOWNLOAD,
        UPGRADE,
        SET_POLICY,
        GET_POLICY,
        GET_NEW_VERSION,
        GET_STATUS,
        REGISTER_CALLBACK,
        UNREGISTER_CALLBACK,
        CANCEL,
        REBOOT_CLEAN,
        REBOOT_INSTALL,
        VERIFY_PACKAGE
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Updater.IUpdateService");
public:
    virtual int32_t RegisterUpdateCallback(const UpdateContext &ctx, const sptr<IUpdateCallback>& updateCallback) = 0;

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
#endif // IUPDATER_SERVICE_H
