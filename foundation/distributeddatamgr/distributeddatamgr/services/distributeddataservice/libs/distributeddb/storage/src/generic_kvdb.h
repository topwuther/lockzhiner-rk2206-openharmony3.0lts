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

#ifndef GENERIC_KVDB_H
#define GENERIC_KVDB_H

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "types.h"
#include "version.h"
#include "ikvdb.h"
#include "generic_kvdb_connection.h"
#include "performance_analysis.h"
#include "kvdb_conflict_entry.h"
#include "db_types.h"

namespace DistributedDB {
class KvDBCommitNotifyFilterAbleData;

struct ImportFileInfo {
    std::string backupDir; // the directory of the current database backup
    std::string unpackedDir; // the directory of the unpacked import file
    std::string currentDir; // the directory of the current database
    std::string curValidFile; // the file imply that the current directory is valid
    std::string backValidFile; // the file imply that the backup directory is valid
};

enum RegisterFuncType {
    OBSERVER_SINGLE_VERSION_NS_PUT_EVENT = 0,
    OBSERVER_SINGLE_VERSION_NS_SYNC_EVENT,
    OBSERVER_SINGLE_VERSION_NS_LOCAL_EVENT,
    OBSERVER_SINGLE_VERSION_NS_CONFLICT_EVENT,
    OBSERVER_MULTI_VERSION_NS_COMMIT_EVENT,
    CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY,
    CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG,
    CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL,
    REGISTER_FUNC_TYPE_MAX
};

class GenericKvDB : public IKvDB {
public:
    GenericKvDB();
    ~GenericKvDB() override;

    DISABLE_COPY_ASSIGN_MOVE(GenericKvDB);

    // Get properties of this database.
    const KvDBProperties &GetMyProperties() const override;

    // Create a db connection.
    IKvDBConnection *GetDBConnection(int &errCode) final;

    // Called when all connections of this database closed.
    void OnClose(const std::function<void(void)> &notifier) final;

    // Publish event when a commit action happened.
    virtual void CommitNotify(int notifyEvent, KvDBCommitNotifyFilterAbleData *data);

    // Invoked automatically when connection count is zero
    virtual void Close() = 0;

    virtual int TryToDisableConnection(OperatePerm perm);

    virtual void ReEnableConnection(OperatePerm perm);

    virtual int Rekey(const CipherPassword &passwd) = 0;

    // Empty passwords represent non-encrypted files.
    // Export existing database files to a specified database file in the specified directory.
    virtual int Export(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Import the existing database files to the specified database file in the specified directory.
    virtual int Import(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Release a db connection.
    void ReleaseDBConnection(GenericKvDBConnection *connection);

    // Register an event listener.
    NotificationChain::Listener *RegisterEventListener(EventType type,
        const NotificationChain::Listener::OnEvent &onEvent,
        const NotificationChain::Listener::OnFinalize &onFinalize, int &errCode);

    // Get event notify counter.
    uint64_t GetEventNotifyCounter() const;

    void OpenPerformanceAnalysis() override;

    void ClosePerformanceAnalysis() override;

    void WakeUpSyncer() override {};

    void EnableAutonomicUpgrade() override {};

    void SetCorruptHandler(const DatabaseCorruptHandler &handler) override;

    int RegisterFunction(RegisterFuncType type);

    int UnregisterFunction(RegisterFuncType type);

    uint32_t GetRegisterFunctionCount(RegisterFuncType type) const;

    virtual int TransObserverTypeToRegisterFunctionType(int observerType, RegisterFuncType &type) const;

    virtual int TransConflictTypeToRegisterFunctionType(int conflictType, RegisterFuncType &type) const;

    virtual int CheckDataStatus(const Key &key, const Value &value, bool isDeleted) const;

    virtual bool CheckWritePermission() const;

    virtual bool IsDataMigrating() const;

    virtual void SetConnectionFlag(bool isExisted) const;

protected:
    // Create a connection object, no DB ref increased.
    virtual GenericKvDBConnection *NewConnection(int &errCode) = 0;

    // Delete a connection object.
    virtual void DelConnection(GenericKvDBConnection *connection);

    // Called when a new connection created.
    void IncreaseConnectionCounter();

    // Called when a connection released.
    void DecreaseConnectionCounter();

    // Register a new notification event type.
    int RegisterNotificationEventType(int eventType);

    // Unregister a notification event type.
    void UnRegisterNotificationEventType(int eventType);

    // Access 'properties_' for derived class.
    const KvDBProperties &MyProp() const;
    KvDBProperties &MyProp();

    static int GetWorkDir(const KvDBProperties &kvDBProp, std::string &workDir);

    void CorruptNotify() const;

    std::string GetStoreIdOnlyIdentifier(const KvDBProperties &properties) const;

    void GetStoreDirectory(const KvDBProperties &properties, int dbType,
        std::string &storeDir, std::string &storeOnlyDir) const;

    PerformanceAnalysis *performance_;
    DatabaseCorruptHandler corruptHandler_;
    DeviceID devId_;

private:
    // Do commit notify in task pool.
    void CommitNotifyAsync(int notifyEvent, KvDBCommitNotifyFilterAbleData *data);

    void CorruptNotifyAsync() const;

    // Get the ID of this kvdb.
    std::string GetStoreId() const;

    DECLARE_OBJECT_TAG(GenericKvDB);

    // Databasse  event notify counter.
    std::atomic<uint64_t> eventNotifyCounter_;

    // Fields for tracking the connection count and invoking callbacks.
    std::atomic<int> connectionCount_;
    std::vector<std::function<void(void)>> closeNotifiers_;
    NotificationChain *notificationChain_;
    KvDBProperties properties_;
    std::mutex connectMutex_;
    mutable std::mutex corruptMutex_;
    OperatePerm operatePerm_;
    mutable std::mutex regFuncCountMutex_;
    std::vector<uint32_t> registerFunctionCount_;
};
} // namespace DistributedDB

#endif // GENERIC_KVDB_H
