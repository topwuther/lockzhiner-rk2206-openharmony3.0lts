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

#ifndef PERMISSION_VALIDATOR_H
#define PERMISSION_VALIDATOR_H
#include <set>
#include <string>
#include "types.h"
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
// Compare function for kvStoreTupleMap_.
struct KvStoreTupleCmp {
    bool operator()(const KvStoreTuple &lKey, const KvStoreTuple &rKey) const
    {
        if (lKey.userId != rKey.userId) {
            return lKey.userId < rKey.userId;
        }
        if (lKey.appId != rKey.appId) {
            return lKey.appId < rKey.appId;
        }
        if (lKey.storeId != rKey.storeId) {
            return lKey.storeId < rKey.storeId;
        }

        return false;
    }
};

class PermissionValidator {
public:
    // check whether the client process have enough privilege to share data with the other devices.
    // uid: client process uid
    KVSTORE_API static bool CheckSyncPermission(const std::string &userId, const std::string &appId,
        std::int32_t uid = 0);

    KVSTORE_API static bool RegisterPermissionChanged(
        const KvStoreTuple &kvStoreTuple, const AppThreadInfo &appThreadInfo);

    KVSTORE_API static void UnregisterPermissionChanged(const KvStoreTuple &kvStoreTuple);

    KVSTORE_API static void UpdateKvStoreTupleMap(const KvStoreTuple &srcKvStoreTuple,
                                                  const KvStoreTuple &dstKvStoreTuple);

    // Check whether the bundle name is in the system service list.
    KVSTORE_API static bool IsSystemService(const std::string &bundleName);

    // Check whether the app with this bundle name is auto launch enabled.
    KVSTORE_API static bool IsAutoLaunchEnabled(const std::string &bundleName);

private:
    static std::set<std::string> systemServiceList_; // the full list for system services.
    static std::set<std::string> autoLaunchEnableList_; // the list for auto launch enabled app.
};
} // namespace DistributedKv
} // namespace OHOS
#endif // PERMISSION_VALIDATOR_H
