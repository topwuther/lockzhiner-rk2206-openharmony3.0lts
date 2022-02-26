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

#ifndef KV_DB_MANAGER_H
#define KV_DB_MANAGER_H

#include <string>
#include <map>
#include <mutex>
#include <set>
#include <condition_variable>

#include "db_errno.h"
#include "ikvdb.h"
#include "ikvdb_factory.h"

namespace DistributedDB {
class KvDBManager final {
public:
    // used to generate process label
    static const std::string PROCESS_LABEL_CONNECTOR;

    // used to open a kvdb with the given property
    static IKvDB *OpenDatabase(const KvDBProperties &property, int &errCode);

    // used to open a kvdb with the given property
    static IKvDBConnection *GetDatabaseConnection(const KvDBProperties &property, int &errCode,
        bool isNeedIfOpened = true);

    // used to close the connection.
    static int ReleaseDatabaseConnection(IKvDBConnection *connection);

    // used to delete a kvdb with the given property.
    static int RemoveDatabase(const KvDBProperties &property);

    // Used to set the process userid and appid
    static int SetProcessLabel(const std::string &appId, const std::string &userId);

    static int CalculateKvStoreSize(const KvDBProperties &property, uint64_t &size);

    // used to restore the sync module of the store.
    static void RestoreSyncableKvStore();

    // used to set the corruption handler.
    static void SetDatabaseCorruptionHandler(const KvStoreCorruptionHandler &handler);

    // Attention. After call FindKvDB and kvdb is not null, you need to call DecObjRef.
    IKvDB* FindKvDB(const std::string &identifier) const;

    // Get a KvDBManager instance, Singleton mode
    static KvDBManager *GetInstance();
private:
    // Generate a KvDB unique Identifier
    static std::string GenerateKvDBIdentifier(const KvDBProperties &property);

    // used to judge Db opened, can not remove Db file
    static int CheckDatabaseFileStatus(const KvDBProperties &properties);

    IKvDB *OpenNewDatabase(const KvDBProperties &property, int &errCode);

    // Save to IKvDB to the global map
    IKvDB *SaveKvDBToCache(IKvDB *kvDB);

    // Get IKvdb From global map
    IKvDB *FindAndGetKvDBFromCache(const KvDBProperties &property, int &errCode) const;

    // Get IKvdb From global map
    void RemoveKvDBFromCache(const IKvDB *kvDB);

    // Find a IKvdb From the given cache. the IKvDB will IncObjRef if found.
    IKvDB *FindKvDBFromCache(const KvDBProperties &property,
        const std::map<std::string, IKvDB *> &cache, bool isNeedCheckPasswd, int &errCode) const;

    bool IsOpenMemoryDb(const KvDBProperties &properties, const std::map<std::string, IKvDB *> &cache) const;

    void RestoreSyncerOfAllKvStore();

    void SetAllDatabaseCorruptionHander(const KvStoreCorruptionHandler &handler);

    IKvDB *GetDataBase(const KvDBProperties &property, int &errCode, bool isNeedIfOpened);

    void DataBaseCorruptNotify(const std::string &appId, const std::string &userId, const std::string &storeId);

    void DataBaseCorruptNotifyAsync(const std::string &appId, const std::string &userId, const std::string &storeId);

    void EnterDBOpenCloseProcess(const std::string &identifier);

    void ExitDBOpenCloseProcess(const std::string &identifier);

    void SetCorruptHandlerForDatabases(const std::map<std::string, IKvDB *> &kvDBMap);

    // Compare two schema objects and return true if both are empty,
    // or both are not empty and the schemas are equal, otherwise return false.
    static bool CompareSchemaObject(const SchemaObject &newSchema, const SchemaObject &oldSchema);

    // check schema is valid
    static int CheckSchema(const IKvDB *kvDB, const KvDBProperties &properties);

    static int ExecuteRemoveDatabase(const KvDBProperties &properties);

    static void RemoveDBDirectory(const KvDBProperties &properties);

    int CheckKvDBProperties(const IKvDB *kvDB, const KvDBProperties &properties,
        bool isNeedCheckPasswd) const;

    IKvDB *GetKvDBFromCacheByIdentify(const std::string &identifier, const std::map<std::string, IKvDB *> &cache) const;

    static KvDBManager *instance_;
    static std::mutex kvDBLock_;
    static std::mutex instanceLock_;

    std::map<std::string, IKvDB *> localKvDBs_;
    std::map<std::string, IKvDB *> multiVerNaturalStores_;
    std::map<std::string, IKvDB *> singleVerNaturalStores_;

    std::mutex corruptMutex_;
    std::mutex kvDBOpenMutex_;
    std::condition_variable kvDBOpenCondition_;
    std::set<std::string> kvDBOpenSet_;
    KvStoreCorruptionHandler corruptHandler_;
};
} // namespace DistributedDB

#endif // KV_DB_MANAGER_H
