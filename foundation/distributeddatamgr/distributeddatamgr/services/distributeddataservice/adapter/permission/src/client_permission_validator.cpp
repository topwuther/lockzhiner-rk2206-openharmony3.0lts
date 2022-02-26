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

#define LOG_TAG "ClientPermissionValidator"

#include "client_permission_validator.h"
#include <cstdint>
#include <string>

namespace OHOS {
namespace DistributedKv {

void ClientPermissionValidator::UpdatePermissionStatus(
    int32_t uid, const std::string &permissionType, bool permissionStatus)
{
    if (permissionType == DISTRIBUTED_DATASYNC) {
        std::lock_guard<std::mutex> permissionLock(permissionMutex_);
        dataSyncPermissionMap_[uid] = permissionStatus;
    }
}

bool ClientPermissionValidator::CheckClientSyncPermission(const KvStoreTuple &kvStoreTuple, std::int32_t curUid)
{
    return true;
}

bool ClientPermissionValidator::RegisterPermissionChanged(
    const KvStoreTuple &kvStoreTuple, const AppThreadInfo &appThreadInfo)
{
    return false;
}

void ClientPermissionValidator::UnregisterPermissionChanged(const KvStoreTuple &kvStoreTuple)
{

}

// after the BMS service restarted, rebuild BundleManager object and re-register permission callbacks for all apps.
void ClientPermissionValidator::RebuildBundleManager()
{

}

void ClientPermissionValidator::UpdateKvStoreTupleMap(const KvStoreTuple &srcKvStoreTuple,
                                                      const KvStoreTuple &dstKvStoreTuple)
{

}
} // namespace DistributedKv
} // namespace OHOS
