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

#ifndef DISTRIBUTEDDATAMGR_BACKUP_HANDLER_H
#define DISTRIBUTEDDATAMGR_BACKUP_HANDLER_H

#include "kv_store_nb_delegate.h"
#include "kv_store_delegate.h"
#include "types.h"
#include "kv_scheduler.h"
#include "kvstore_meta_manager.h"
#include "ikvstore_data_service.h"

namespace OHOS::DistributedKv {
class BackupHandler {
public:
    explicit BackupHandler(IKvStoreDataService *kvStoreDataService);
    BackupHandler();
    ~BackupHandler();
    void BackSchedule();
    void SingleKvStoreBackup(const MetaData &metaData);
    void MultiKvStoreBackup(const MetaData &metaData);
    bool SingleKvStoreRecover(MetaData &metaData, DistributedDB::KvStoreNbDelegate *delegate);
    bool MultiKvStoreRecover(MetaData &metaData, DistributedDB::KvStoreDelegate *delegate);

    static const std::string &GetBackupPath(const std::string &deviceAccountId, int pathType);
    static bool RenameFile(const std::string &oldPath, const std::string &newPath);
    static bool RemoveFile(const std::string &path);
    static bool FileExists(const std::string &path);
    static std::string GetHashedBackupName(const std::string &bundleName);
private:
    bool CheckNeedBackup();

    static std::string backupDirCe_;
    static std::string backupDirDe_;

    KvScheduler scheduler_ {};
    int64_t backupSuccessTime_ = 0;
};
} // namespace OHOS::DistributedKv
#endif // DISTRIBUTEDDATAMGR_BACKUP_HANDLER_H
