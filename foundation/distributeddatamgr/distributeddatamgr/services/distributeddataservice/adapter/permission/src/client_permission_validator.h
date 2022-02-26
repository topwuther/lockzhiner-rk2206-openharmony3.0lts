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

#ifndef CLIENT_PERMISSION_VALIDATOR_H
#define CLIENT_PERMISSION_VALIDATOR_H

#include "permission_validator.h"
#include <cstdint>
#include <map>
#include <mutex>

namespace OHOS {
namespace DistributedKv {

const std::string DISTRIBUTED_DATASYNC = "ohos.permission.DISTRIBUTED_DATASYNC";

class ClientPermissionValidator {
public:
    static ClientPermissionValidator &GetInstance()
    {
        static ClientPermissionValidator clientPermissionValidator;
        return clientPermissionValidator;
    }

    bool RegisterPermissionChanged(const KvStoreTuple &kvStoreTuple, const AppThreadInfo &appThreadInfo);

    void UnregisterPermissionChanged(const KvStoreTuple &kvStoreTuple);

    void UpdateKvStoreTupleMap(const KvStoreTuple &srcKvStoreTuple, const KvStoreTuple &dstKvStoreTuple);

    void UpdatePermissionStatus(int32_t uid, const std::string &permissionType, bool permissionStatus);

    bool CheckClientSyncPermission(const KvStoreTuple &kvStoreTuple, std::int32_t curUid);

private:
    ClientPermissionValidator() = default;

    ~ClientPermissionValidator() = default;

    ClientPermissionValidator(const ClientPermissionValidator &clientPermissionValidator);

    const ClientPermissionValidator &operator=(const ClientPermissionValidator &clientPermissionValidator);

    void RebuildBundleManager();

    std::mutex tupleMutex_;
    std::mutex permissionMutex_;
    std::map<std::int32_t, bool> dataSyncPermissionMap_;
};
} // namespace DistributedKv
} // namespace OHOS

#endif // CLIENT_PERMISSION_VALIDATOR_H
