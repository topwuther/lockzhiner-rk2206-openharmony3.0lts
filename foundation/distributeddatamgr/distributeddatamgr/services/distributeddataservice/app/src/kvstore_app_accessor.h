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

#ifndef KVSTORE_APP_ACCESSOR_H
#define KVSTORE_APP_ACCESSOR_H

#include "kv_store_delegate_manager.h"
#include <mutex>
#include <set>
#include <string>

namespace OHOS::DistributedKv {
struct AppAccessorParam {
    std::string userId;
    std::string appId;
    std::string storeId;
    DistributedDB::AutoLaunchOption launchOption;
};

class KvStoreAppAccessor {
public:
    ~KvStoreAppAccessor();
    void EnableKvStoreAutoLaunch(const AppAccessorParam &param);

    void DisableKvStoreAutoLaunch(const AppAccessorParam &param);

    void EnableKvStoreAutoLaunch();

    void OnCallback(const std::string &userId, const std::string &appId,
                    const std::string &storeId, DistributedDB::AutoLaunchStatus status);
    static KvStoreAppAccessor &GetInstance();
private:
    KvStoreAppAccessor();
};
}
#endif // KVSTORE_APP_ACCESSOR_H
