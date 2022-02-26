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

#define LOG_TAG "PermissionValidator"

#include "permission_validator.h"

#include <regex>
#include <string>
#include "client_permission_validator.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
// initialize system service full list.
std::set<std::string> PermissionValidator::systemServiceList_ = {
    "bundle_manager_service",  // BMS
    "ivi_config_manager", // IVI
    "form_storage"  // form
};

// initialize auto launch enabled applications white list.
std::set<std::string> PermissionValidator::autoLaunchEnableList_ = {
    "providers.calendar",
    "com.huawei.contacts.sync",
    "com.huawei.ohos.totemweather"
};

// check whether the client process have enough privilege to share data with the other devices.
bool PermissionValidator::CheckSyncPermission(const std::string &userId, const std::string &appId, std::int32_t uid)
{
    KvStoreTuple kvStoreTuple {userId, appId};
    return ClientPermissionValidator::GetInstance().CheckClientSyncPermission(kvStoreTuple, uid);
}

bool PermissionValidator::RegisterPermissionChanged(
    const KvStoreTuple &kvStoreTuple, const AppThreadInfo &appThreadInfo)
{
    return ClientPermissionValidator::GetInstance().RegisterPermissionChanged(kvStoreTuple, appThreadInfo);
}

void PermissionValidator::UnregisterPermissionChanged(const KvStoreTuple &kvStoreTuple)
{
    return ClientPermissionValidator::GetInstance().UnregisterPermissionChanged(kvStoreTuple);
}

void PermissionValidator::UpdateKvStoreTupleMap(const KvStoreTuple &srcKvStoreTuple,
                                                const KvStoreTuple &dstKvStoreTuple)
{
    return ClientPermissionValidator::GetInstance().UpdateKvStoreTupleMap(srcKvStoreTuple, dstKvStoreTuple);
}


// Check whether the bundle name is in the system service list.
bool PermissionValidator::IsSystemService(const std::string &bundleName)
{
    auto it = systemServiceList_.find(bundleName);
    if (it == systemServiceList_.end()) {
        ZLOGD("bundleName:%s is not system service.", bundleName.c_str());
        return false;
    }
    ZLOGD("bundleName:%s is system service.", bundleName.c_str());
    return true;
}

// Check whether the app with this bundle name is auto launch enabled.
bool PermissionValidator::IsAutoLaunchEnabled(const std::string &bundleName)
{
    for (auto it : autoLaunchEnableList_) {
        size_t pos = bundleName.rfind(it);
        if (pos != std::string::npos) {
            return true;
        }
    }
    ZLOGD("AppId:%s is not allowed.", bundleName.c_str());
    return false;
}
} // namespace DistributedKv
} // namespace OHOS
