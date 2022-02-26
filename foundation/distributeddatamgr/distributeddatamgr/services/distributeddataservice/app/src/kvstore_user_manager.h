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

#ifndef KV_STORE_USER_MANAGER_H
#define KV_STORE_USER_MANAGER_H

#include <map>
#include <mutex>
#include "kvstore_app_manager.h"
#include "kvstore_impl.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

class KvStoreUserManager {
public:
    explicit KvStoreUserManager(const std::string &deviceAccountId);

    virtual ~KvStoreUserManager();

    Status GetKvStore(const Options &options, const std::string &appId, const std::string &storeId,
                      const std::vector<uint8_t> &cipherKey, std::function<void(sptr<IKvStoreImpl>)> callback);

    Status GetSingleKvStore(const Options &options, const std::string &appId, const std::string &storeId,
                            const std::vector<uint8_t> &cipherKey, std::function<void(sptr<ISingleKvStore>)> callback);

    Status CloseKvStore(const std::string &appId, const std::string &storeId);

    Status CloseAllKvStore(const std::string &appId);

    void CloseAllKvStore();

    Status DeleteKvStore(const std::string &bundleName, const std::string &storeId);

    void DeleteAllKvStore();

    Status MigrateAllKvStore(const std::string &harmonyAccountId);

    std::string GetDbDir(const std::string &bundleName, const Options &options);

    void Dump(int fd) const;

private:
    std::mutex appMutex_;
    std::map<std::string, KvStoreAppManager> appMap_;
    std::string deviceAccountId_;
    std::string userId_;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // KV_STORE_USER_MANAGER_H
