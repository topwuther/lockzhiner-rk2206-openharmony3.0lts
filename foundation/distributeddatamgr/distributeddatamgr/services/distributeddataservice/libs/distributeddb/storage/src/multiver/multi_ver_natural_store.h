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

#ifndef MULTI_VER_NATURAL_STORE_H
#define MULTI_VER_NATURAL_STORE_H

#ifndef OMIT_MULTI_VER
#include "sync_able_kvdb.h"
#include "multi_ver_kvdb_sync_interface.h"
#include "kv_store_changed_data.h"
#include "ikvdb_multi_ver_data_storage.h"
#include "ikvdb_commit_storage.h"
#include "macro_utils.h"
#include "multi_ver_kvdata_storage.h"
#include "multi_ver_storage_executor.h"
#include "multi_ver_storage_engine.h"
#include "multi_ver_vacuum.h"

namespace DistributedDB {
enum NaturalStoreNotificationEventType {
    NATURAL_STORE_COMMIT_EVENT = 0
};
class MultiVerVacuumExecutorImpl;
class MultiVerNaturalStore final: public SyncAbleKvDB, public MultiVerKvDBSyncInterface {
public:
    MultiVerNaturalStore();
    ~MultiVerNaturalStore() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(MultiVerNaturalStore);

    // Open the database
    int Open(const KvDBProperties &kvDBProp) override;

    // Invoked automatically when connection count is zero
    void Close() override;

    // Create a connection object.
    GenericKvDBConnection *NewConnection(int &errCode) override;

    // Get interface for syncer.
    IKvDBSyncInterface *GetSyncInterface() override;

    // Get interface type of this kvdb.
    int GetInterfaceType() const override;

    // Get the interface ref-count, in order to access asynchronously.
    void IncRefCount() override;

    // Drop the interface ref-count.
    void DecRefCount() override;

    // Get the identifier of this kvdb.
    std::vector<uint8_t> GetIdentifier() const override;

    // Get the max timestamp of all entries in database.
    void GetMaxTimeStamp(TimeStamp &stamp) const override;

    // Get meta data associated with the given key.
    int GetMetaData(const Key &key, Value &value) const override;

    // Put meta data as a key-value entry.
    int PutMetaData(const Key &key, const Value &value) override;

    // Get all meta data keys.
    int GetAllMetaKeys(std::vector<Key> &keys) const override;

    bool IsCommitExisted(const MultiVerCommitNode &commit) const override;

    int GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &) const override;

    int GetCommitTree(const std::map<std::string, MultiVerCommitNode> &,
        std::vector<MultiVerCommitNode> &) const override;

    int GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries) const override;

    MultiVerKvEntry *CreateKvEntry(const std::vector<uint8_t> &data) override;

    void ReleaseKvEntry(const MultiVerKvEntry *entry) override;

    bool IsValueSliceExisted(const ValueSliceHash &value) const override;

    int GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const override;

    int PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue) const override;

    int PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
        const std::string &deviceName) override;

    int MergeSyncCommit(const MultiVerCommitNode &commit, const std::vector<MultiVerCommitNode> &commits) override;

    void NotifyStartSyncOperation() override;

    void NotifyFinishSyncOperation() override;

    int TransferSyncCommitDevInfo(MultiVerCommitNode &commit, const std::string &devId, bool isSyncedIn) const override;

    int Rekey(const CipherPassword &passwd) override;

    int Export(const std::string &filePath, const CipherPassword &passwd) override;

    int Import(const std::string &filePath, const CipherPassword &passwd) override;

    int GetDiffEntries(const CommitID &begin, const CommitID &end, MultiVerDiffData &data) const;

    uint64_t GetCurrentTimeStamp();

    // Set the max timestamp
    void SetMaxTimeStamp(TimeStamp stamp);

    Version GetMaxCommitVersion() const;

    void SetMaxCommitVersion(const Version &version);

    MultiVerStorageExecutor *GetHandle(bool isWrite, int &errCode,
        bool isTrimming = false, OperatePerm perm = OperatePerm::NORMAL_PERM) const;

    void ReleaseHandle(MultiVerStorageExecutor *&handle, bool isTrimming = false) const;

    void GetCurrentTag(std::vector<uint8_t> &tag) const;

    // Just provide the version constraint for trimmming data(include observer and the snapshot)
    void AddVersionConstraintToList(Version version);

    void RemoveVersionConstraintFromList(Version version);

    // Get the max trimmable version, if no need trimming, return 0; if need trimming all return the MAX_UINT64.
    Version GetMaxTrimmableVersion() const;

    int TransObserverTypeToRegisterFunctionType(int observerType, RegisterFuncType &type) const override;

    const KvDBProperties &GetDbProperties() const override;

    int RemoveKvDB(const KvDBProperties &properties) override;

    int GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const override;

    KvDBProperties &GetDbPropertyForUpdate();

    int InitStorages(const KvDBProperties &kvDBProp, bool isChangeTag = false);

private:

    int CheckSubStorageVersion(const KvDBProperties &kvDBProp, bool &isSubStorageAllExist) const;

    int CreateStorages();

    int CreateStoreDirectory(const std::string &directory, const std::string &identifierName);

    void Clear();

    int RecoverFromException();

    int CompareVerDataAndLog(IKvDBMultiVerTransaction *transaction) const;

    int ClearTempFile(const KvDBProperties &kvDBProp);

    int InitStorageContext(bool isChangeTag);

    int InitStorageContextVersion(bool isChangeTag);

    std::string GetStringIdentifier() const;

    int CheckVersion(const KvDBProperties &kvDBProp) const;

    int CheckOverallVersionViaVersionFile(const KvDBProperties &kvDBProp, bool &isVerFileExist) const;

    int GetVersionFilePath(const KvDBProperties &kvDBProp, std::string &outPath) const;

    DECLARE_OBJECT_TAG(MultiVerNaturalStore);

    static MultiVerVacuum shadowTrimmer_;
    IKvDBMultiVerDataStorage *multiVerData_;
    IKvDBCommitStorage *commitHistory_;
    MultiVerKvDataStorage *multiVerKvStorage_;
    std::unique_ptr<MultiVerStorageEngine> multiVerEngine_;
    MultiVerVacuumExecutorImpl *trimmerImpl_;
    mutable std::mutex commitHistMutex_;
    mutable std::mutex multiDataMutex_;
    mutable std::mutex syncerKvMutex_;
    mutable std::mutex maxTimeMutex_;
    mutable std::mutex versionConstraintMutex_;
    mutable uint64_t maxRecordTimestamp_;
    Version maxCommitVersion_;
    std::vector<uint8_t> branchTag_;
    std::multiset<Version> versionConstraints_;
};
} // namespace DistributedDB

#endif  // MULTI_VER_NATURAL_STORE_H
#endif