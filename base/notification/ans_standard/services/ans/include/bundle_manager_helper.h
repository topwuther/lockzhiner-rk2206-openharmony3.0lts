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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_BUNDLE_MANAGER_HELPER_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_BUNDLE_MANAGER_HELPER_H

#include <memory>
#include <mutex>
#include <string>

#include "bundle_mgr_interface.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "refbase.h"
#include "remote_death_recipient.h"
#include "singleton.h"

namespace OHOS {
namespace Notification {

class BundleManagerHelper : public DelayedSingleton<BundleManagerHelper> {
public:
    std::string GetBundleNameByUid(int uid);
    bool IsSystemApp(int uid);
    int GetDefaultUidByBundleName(const std::string& bundle);

private:
    void Connect();
    void Disconnect();

    void OnRemoteDied(const wptr<IRemoteObject> &object);

private:
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    std::mutex connectionMutex_;
    sptr<RemoteDeathRecipient> deathRecipient_ = nullptr;

    DECLARE_DELAYED_SINGLETON(BundleManagerHelper)
};

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_BUNDLE_MANAGER_HELPER_H