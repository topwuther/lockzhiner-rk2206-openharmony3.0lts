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

#ifndef KV_STORE_DELEGATE_MANAGER_H
#define KV_STORE_DELEGATE_MANAGER_H

#include <string>
#include <functional>
#include <mutex>
#include <memory>

#ifndef OMIT_MULTI_VER
#include "kv_store_delegate.h"
#endif
#include "kv_store_nb_delegate.h"
#include "types.h"
#include "iprocess_communicator.h"
#include "iprocess_system_api_adapter.h"
#include "auto_launch_export.h"

namespace DistributedDB {
class KvStoreDelegateManager final {
public:
    DB_API KvStoreDelegateManager(const std::string &appId, const std::string &userId);
    DB_API ~KvStoreDelegateManager();

    KvStoreDelegateManager(const KvStoreDelegateManager &) = delete;
    KvStoreDelegateManager(KvStoreDelegateManager &&) = delete;
    KvStoreDelegateManager &operator=(const KvStoreDelegateManager &) = delete;
    KvStoreDelegateManager &operator=(KvStoreDelegateManager &&) = delete;

    // Used to set global config of the KvStores, such dataDir, return OK if set config susccess.
    DB_API DBStatus SetKvStoreConfig(const KvStoreConfig &kvStoreConfig);

#ifndef OMIT_MULTI_VER
    // Used to open or create a KvStore.
    // Return OK and a KvStoreDelegate* if there is no error. else return ERROR and nullptr;
    DB_API void GetKvStore(const std::string &storeId, const KvStoreDelegate::Option &option,
        const std::function<void(DBStatus, KvStoreDelegate *)> &callback);
#endif
    // Used to open or create a KvStore(Natural store).
    // Suggest: Not to use encrypted database in S3 SECE access controlled;
    // Warning: Access controlled prevents access to files so cannot verify passwords,
    // So that a cacheDb with incorrect passwd will be created or opened and lose these data.
    DB_API void GetKvStore(const std::string &storeId, const KvStoreNbDelegate::Option &option,
        const std::function<void(DBStatus, KvStoreNbDelegate *)> &callback);

#ifndef OMIT_MULTI_VER
    // Close a KvStore, return OK if close susccess.
    DB_API DBStatus CloseKvStore(KvStoreDelegate *kvStore);
#endif

    DB_API DBStatus CloseKvStore(KvStoreNbDelegate *kvStore);

    // Used to delete a KvStore, return OK if delete susccess.
    DB_API DBStatus DeleteKvStore(const std::string &storeId);

    // Get the database size.
    DB_API DBStatus GetKvStoreDiskSize(const std::string &storeId, uint64_t &size);

    // Used to set the process userid and appid
    DB_API static DBStatus SetProcessLabel(const std::string &appId, const std::string &userId);

    // Set process communicator.
    DB_API static DBStatus SetProcessCommunicator(const std::shared_ptr<IProcessCommunicator> &inCommunicator);

    DB_API static void SetKvStoreCorruptionHandler(const KvStoreCorruptionHandler &handler);

    // Get database directory by storeId + appId + userId
    DB_API static DBStatus GetDatabaseDir(const std::string &storeId, const std::string &appId,
        const std::string &userId, std::string &directory);

    // Get database directory by storeId
    DB_API static DBStatus GetDatabaseDir(const std::string &storeId, std::string &directory);

    DB_API static DBStatus SetPermissionCheckCallback(const PermissionCheckCallback &callback);

    DB_API static DBStatus SetPermissionCheckCallback(const PermissionCheckCallbackV2 &callback);

    DB_API static DBStatus EnableKvStoreAutoLaunch(const std::string &userId, const std::string &appId,
        const std::string &storeId, const AutoLaunchOption &option, const AutoLaunchNotifier &notifier);

    DB_API static DBStatus DisableKvStoreAutoLaunch(const std::string &userId, const std::string &appId,
        const std::string &storeId);

    DB_API static void SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback);

    DB_API static std::string GetKvStoreIdentifier(const std::string &userId, const std::string &appId,
        const std::string &storeId);

    DB_API static DBStatus SetProcessSystemAPIAdapter(const std::shared_ptr<IProcessSystemApiAdapter> &adapter);

private:

    // Check if the dataDir is safe arg.
    bool IsDataDirSafe(const std::string &dataDir, std::string &canonicalDir) const;
    bool GetKvStoreParamCheck(const std::string &storeId, const KvStoreNbDelegate::Option &option,
        const std::function<void(DBStatus, KvStoreNbDelegate *)> &callback) const;
    DBStatus SetObserverNotifier(KvStoreNbDelegate *kvStore, const KvStoreNbDelegate::Option &option);

    const std::string &GetKvStorePath() const;
    static const std::string DEFAULT_PROCESS_APP_ID;
    static std::mutex communicatorMutex_;
    static std::shared_ptr<IProcessCommunicator> processCommunicator_;

    KvStoreConfig kvStoreConfig_;
    std::string appId_;
    std::string userId_;

    mutable std::mutex mutex_;
};
} // namespace DistributedDB

#endif // KV_STORE_DELEGATE_MANAGER_H
