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

#ifndef APP_DISTRIBUTED_KV_DATA_MANAGER_IMPL_H
#define APP_DISTRIBUTED_KV_DATA_MANAGER_IMPL_H

#include <mutex>
#include "app_device_status_change_listener.h"
#include "app_distributed_kv_data_manager.h"
#include "app_kvstore.h"
#include "app_types.h"
#include "kv_store_delegate_manager.h"
#include "process_communicator_impl.h"

namespace OHOS {
namespace AppDistributedKv {
// This is the overall manager of all kvstore.
// This class provides open, close, delete AppKvStore and manage remote device functions.
class AppDistributedKvDataManagerImpl final : public AppDistributedKvDataManager {
public:
    AppDistributedKvDataManagerImpl(DistributedDB::KvStoreDelegateManager *delegateManager, const std::string &appId);

    static std::shared_ptr<AppDistributedKvDataManager> GetInstance(const std::string &bundleName,
                                                                    const std::string &dataDir,
                                                                    const std::string &userId);

    virtual ~AppDistributedKvDataManagerImpl();

    Status GetKvStore(const Options &options, const std::string &storeId,
                      const std::function<void(std::unique_ptr<AppKvStore> appKvStore)> &callback) override;

    Status CloseKvStore(std::unique_ptr<AppKvStore> appKvStore) override;

    Status DeleteKvStore(const std::string &storeId) override;

    Status GetKvStoreDiskSize(const std::string &storeId, uint64_t &size) override;

    Status RegisterKvStoreCorruptionObserver(const std::shared_ptr<AppKvStoreCorruptionObserver> observer) override;
private:
    static DistributedDB::SecurityOption ConvertSecurityLevel(int securityLevel);

    static std::mutex storeMutex_;

    static std::map<std::string, std::shared_ptr<AppDistributedKvDataManager>> managers_;
    // pointer of class DistributedDB::KvStoreDelegateManager. defined as void* to avoid exposing inside implementions.
    DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager_;
    std::string appId_;

    std::shared_ptr<AppKvStoreCorruptionObserver> corruptionObserver_;
};  // class AppDistributedKvDataManagerImpl
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // APP_DISTRIBUTED_KV_DATA_MANAGER_IMPL_H
