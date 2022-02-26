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

#ifndef APP_DISTRIBUTED_KV_DATA_MANAGER_H
#define APP_DISTRIBUTED_KV_DATA_MANAGER_H

#include <memory>
#include "app_device_status_change_listener.h"
#include "app_kvstore.h"
#include "app_types.h"
#include "app_kvstore_corruption_observer.h"

namespace OHOS {
namespace AppDistributedKv {
// This is the overall manager of all kvstore.
// This class provides open, close, delete AppKvStore and manage remote device functions.
class AppDistributedKvDataManager {
public:
    // Get AppDistributedKvDataManager singleton for APP process (for jni and dynamic library depended by jni).
    // Parameters:
    //     bundleName: bundleName of your app
    //     dataDir: the directory to save your db file. Please choose a directory you can visit before phone unlock.
    //              this parameter will not be checked or used after first successful call.
    //     userId: name of your user. this stands for multiuser, not for account or linux user.
    // Return:
    //     singleton of AppDistributedKvDataManager, or nullptr on error.
    KVSTORE_API static std::shared_ptr<AppDistributedKvDataManager> GetInstance(const std::string &bundleName,
                                                                                const std::string &dataDir,
                                                                                const std::string &userId = "account0");

    KVSTORE_API AppDistributedKvDataManager()
    {}

    KVSTORE_API virtual ~AppDistributedKvDataManager()
    {}

    // Open kvstore instance with the given storeId, creating it if needed.
    // It is allowed to open the same kvstore concurrently
    // multiple times, but only one database instance will be created.
    // Parameters:
    //     options: the config of the kvstore, including encrypt, create if needed and whether need sync between
    //              devices.
    //     storeId: the name of the kvstore.
    //     callback: KvStore instance returned by this call.
    //     callback will return:
    //         if Options.createIfMissing is false and kvstore has not been created before, nullptr and Status is
    //         STORE_NOT_FOUND,
    //         if storeId is not valid,  nullptr and Status is INVALID_ARGUMENT
    //         otherwise, SUCCESS and the unipue_ptr of AppKvStore, which client can use to operate kvstore, will be
    //         returned.
    // Return:
    //     Status of this get operation.
    KVSTORE_API
    virtual Status GetKvStore(const Options &options, const std::string &storeId,
                              const std::function<void(std::unique_ptr<AppKvStore> appKvStore)> &callback) = 0;

    // WARNING: try to close a KvStore while other thread(s) still using it may cause process crash.
    // Disconnect kvstore connection from database instance with the given storeId,
    // only if all connections to the same database instance are closed, database instance will be freed.
    // after this call, kvstore becomes invalid.
    // call to it will return nullptr exception.
    // Parameters:
    //     appKvStore: kvstore instance created by GetKvStore.
    // Return:
    //     Status of this close operation.
    KVSTORE_API virtual Status CloseKvStore(std::unique_ptr<AppKvStore> appKvStore) = 0;

    // Delete database file with the given storeId.
    // Client should first close all connections to it and then delete it, otherwise delete will return error.
    // Parameters:
    //     storeId: the name of the kvstore.
    // Return:
    //     Status of this delete operation.
    KVSTORE_API virtual Status DeleteKvStore(const std::string &storeId) = 0;

    // Get the database size.
    KVSTORE_API virtual Status GetKvStoreDiskSize(const std::string &storeId, uint64_t &size) = 0;

    // observe
    // Parameters:
    //     observer: observer which will be callback when corrupted.
    // Return:
    //     Status of this operation.
    KVSTORE_API
    virtual Status RegisterKvStoreCorruptionObserver(const std::shared_ptr<AppKvStoreCorruptionObserver> observer) = 0;
protected:
    AppDistributedKvDataManager(const AppDistributedKvDataManager &) = delete;
    AppDistributedKvDataManager& operator=(const AppDistributedKvDataManager&) = delete;
    AppDistributedKvDataManager(AppDistributedKvDataManager &&) = delete;
    AppDistributedKvDataManager& operator=(AppDistributedKvDataManager &&) = delete;
};  // class AppDistributedKvDataManager
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // APP_DISTRIBUTED_KV_DATA_MANAGER_H
