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

#ifndef I_KV_DB_CONNECTION_H
#define I_KV_DB_CONNECTION_H

#include <string>
#include <functional>

#include "types.h"
#include "db_types.h"
#include "macro_utils.h"
#include "query.h"

namespace DistributedDB {
class IKvDB;
class IKvDBSnapshot;
class KvDBObserverHandle;
class KvDBCommitNotifyData;
class IKvDBResultSet;

using KvDBObserverAction = std::function<void(const KvDBCommitNotifyData &data)>;
using KvDBConflictAction = std::function<void(const KvDBCommitNotifyData &data)>;

class IKvDBConnection {
public:
    IKvDBConnection() = default;
    virtual ~IKvDBConnection() {};

    DISABLE_COPY_ASSIGN_MOVE(IKvDBConnection);

    // Get the value from the database.
    virtual int Get(const IOption &option, const Key &key, Value &value) const = 0;

    // Put the value to the database.
    virtual int Put(const IOption &option, const Key &key, const Value &value) = 0;

    // Delete the value from the database.
    virtual int Delete(const IOption &option, const Key &key) = 0;

    // Clear all the data from the database.
    virtual int Clear(const IOption &option) = 0;

    // Get all the data from the database.
    virtual int GetEntries(const IOption &option, const Key &keyPrefix, std::vector<Entry> &entries) const = 0;

    virtual int GetEntries(const IOption &option, const Query &query, std::vector<Entry> &entries) const = 0;

    virtual int GetCount(const IOption &option, const Query &query, int &count) const = 0;

    // Put the batch values to the database.
    virtual int PutBatch(const IOption &option, const std::vector<Entry> &entries) = 0;

    // Delete the batch values from the database.
    virtual int DeleteBatch(const IOption &option, const std::vector<Key> &keys) = 0;

    // Get the snapshot.
    virtual int GetSnapshot(IKvDBSnapshot *&snapshot) const = 0;

    // Release the created snapshot.
    virtual void ReleaseSnapshot(IKvDBSnapshot *&snapshot) = 0;

    // Start the transaction.
    virtual int StartTransaction() = 0;

    // Commit the transaction.
    virtual int Commit() = 0;

    // Roll back the transaction.
    virtual int RollBack() = 0;

    // Check if the transaction already started manually
    virtual bool IsTransactionStarted() const = 0;

    // Register observer.
    virtual KvDBObserverHandle *RegisterObserver(unsigned mode, const Key &key,
        const KvDBObserverAction &action, int &errCode) = 0;

    // Unregister observer.
    virtual int UnRegisterObserver(const KvDBObserverHandle *observerHandle) = 0;

    // Register a conflict notifier.
    virtual int SetConflictNotifier(int conflictType, const KvDBConflictAction &action) = 0;

    // Close and release the connection.
    virtual int Close() = 0;

    virtual std::string GetIdentifier() const = 0;

    // Pragma interface.
    virtual int Pragma(int cmd, void *parameter) = 0;

    // Rekey the database.
    virtual int Rekey(const CipherPassword &passwd) = 0;

    // Empty passwords represent non-encrypted files.
    // Export existing database files to a specified database file in the specified directory.
    virtual int Export(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Import the existing database files to the specified database file in the specified directory.
    virtual int Import(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Get the result set
    virtual int GetResultSet(const IOption &option, const Key &keyPrefix, IKvDBResultSet *&resultSet) const = 0;

    virtual int GetResultSet(const IOption &option, const Query &query, IKvDBResultSet *&resultSet) const = 0;

    // Release the result set
    virtual void ReleaseResultSet(IKvDBResultSet *&resultSet) = 0;

    virtual int RegisterLifeCycleCallback(const DatabaseLifeCycleNotifier &notifier) = 0;

    // Get the securityLabel and securityFlag
    virtual int GetSecurityOption(int &securityLabel, int &securityFlag) const = 0;
};
} // namespace DistributedDB

#endif // I_KV_DB_CONNECTION_H
