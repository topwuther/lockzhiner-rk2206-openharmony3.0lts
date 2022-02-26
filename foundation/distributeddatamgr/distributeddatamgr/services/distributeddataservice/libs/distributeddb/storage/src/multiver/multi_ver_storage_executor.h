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

#ifndef MULTI_VER_STORAGE_EXECUTOR_H
#define MULTI_VER_STORAGE_EXECUTOR_H

#ifndef OMIT_MULTI_VER
#include "storage_executor.h"
#include "ikvdb.h"
#include "ikvdb_commit_storage.h"
#include "ikvdb_multi_ver_data_storage.h"
#include "macro_utils.h"
#include "multi_ver_kvdata_storage.h"

namespace DistributedDB {
enum class MultiTransactionType {
    NORMAL_DATA,
    ALL_DATA,
};

class MultiVerStorageExecutor : public StorageExecutor {
public:
    MultiVerStorageExecutor(IKvDB *kvDB, IKvDBMultiVerDataStorage *dataStorage, IKvDBCommitStorage *commitStorage,
        MultiVerKvDataStorage *kvDataStorage, bool writable);
    ~MultiVerStorageExecutor() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(MultiVerStorageExecutor);

    int Reset() override;

    int Put(const Key &key, const Value &value);

    int Get(const Key &key, Value &value) const;

    int GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const;

    int Delete(const Key &key);

    int Clear();

    int PutMetaData(const Key &key, const Value &value);

    int GetMetaData(const Key &key, Value &value) const;

    int GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &commitMap) const;

    int GetCommitTree(const std::map<std::string, MultiVerCommitNode> &commitMap,
        std::vector<MultiVerCommitNode> &commits) const;

    bool IsCommitExisted(const MultiVerCommitNode &commit, int &errCode) const;

    int GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries) const;

    bool IsValueSliceExisted(const ValueSliceHash &value, int &errCode) const;

    int GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const;

    int PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue, bool isAddCount);

    int PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
        const std::string &deviceName);

    int MergeSyncCommit(const MultiVerCommitNode &commit, const std::vector<MultiVerCommitNode> &commits);

    int GetDiffEntries(const CommitID &begin, const CommitID &end, MultiVerDiffData &data) const;

    int StartTransaction(MultiTransactionType type = MultiTransactionType::NORMAL_DATA);

    int CommitTransaction(MultiTransactionType type = MultiTransactionType::NORMAL_DATA);

    int RollBackTransaction(MultiTransactionType type = MultiTransactionType::NORMAL_DATA);

    int InitCurrentReadVersion();

    void Close();

    Version GetCurrentReadVersion() const;

    // Get all the commits with the view of one commit.
    int GetAllCommitsInTree(std::list<MultiVerCommitNode> &commits) const;

    // Get all the hash key of one version.
    int GetEntriesByVersion(Version version, std::list<MultiVerTrimedVersionData> &data) const;

    // Get all the overwritten record whose version is less than the specified version and tag is less the cleard data.
    int GetOverwrittenClearTypeEntries(Version clearVersion, std::list<MultiVerTrimedVersionData> &data) const;

    // Get all the overwritten non-cleared record whose version is less than the specified version.
    int GetOverwrittenNonClearTypeEntries(Version version, const Key &hashKey,
        std::list<MultiVerTrimedVersionData> &data) const;

    // Delete the data whose hash key is equal to the hashKey and version is less than the specified.
    int DeleteEntriesByHashKey(Version version, const Key &hashKey);

    // Update the trimmed flag for the hash key with the specified version.
    int UpdateTrimedFlag(Version version, const Key &hashKey);

    // Update the trimmed flag for the commit.
    int UpdateTrimedFlag(const CommitID &commit);

private:
    static void ReleaseMultiVerKvEntries(std::vector<MultiVerKvEntry *> &entries);

    int GetSliceCount(std::vector<Entry> &&entries, uint32_t &count) const;

    int PutSliceCount(const Key &sliceKey, uint32_t count) const;

    int CommitTransaction(const MultiVerCommitNode &multiVerCommit, bool isMerge);

    int GetResolvedConflictEntries(const MultiVerCommitNode &commitItem, std::vector<MultiVerKvEntry *> &entries) const;

    int TransferDiffEntries(MultiVerDiffData &data) const;

    int TransferToUserValue(const Value &savedValue, Value &value) const;

    int TransferToSavedValue(const Value &value, Value &savedValue);

    void CommitNotifiedData(const CommitID &commitId);

    int GetParentCommitId(const CommitID &commitId, CommitID &parentId, Version &curVersion) const;

    int AllocNewCommitId(CommitID &commitId) const;

    int FillAndCommitLogEntry(const Version &versionInfo, CommitID &commitId, uint64_t timestamp) const;

    int FillCommitByForeign(IKvDBCommit *commit, const MultiVerCommitNode &multiVerCommit,
        const Version &versionInfo, const CommitID &commitId, bool isMerge) const;

    int FillAndCommitLogEntry(const Version &versionInfo, const MultiVerCommitNode &multiVerCommit,
        CommitID &commitId, bool isMerge, TimeStamp &timestamp) const;

    int MergeOneCommit(const MultiVerCommitNode &commit);

    int MergeCommits(const std::vector<MultiVerCommitNode> &commits);

    int CommitSyncCommits();

    int StartAllDbTransaction();

    int TransferToValueObject(const Value &value, MultiVerValueObject &valueObject);

    int RollBackAllDbTransaction();

    int CommitAllDbTransaction();

    int ReInitTransactionVersion(const MultiVerCommitNode &commit);

    int StartSliceTransaction();

    int CommitSliceTransaction();

    int RollbackSliceTransaction();

    int GetValueSliceInner(const SliceTransaction *sliceTransaction, const ValueSliceHash &hashValue,
        ValueSlice &sliceValue) const;

    int PutValueSliceInner(SliceTransaction *sliceTransaction, const ValueSliceHash &hashValue,
        const ValueSlice &sliceValue, bool isAddCount);

    int DeleteValueSliceInner(SliceTransaction *sliceTransaction, const ValueSliceHash &hashValue);

    int AddSliceDataCount(const std::vector<Value> &values);

    static const int COMMIT_ID_LENGTH = 20;
    IKvDB *kvDB_;
    IKvDBMultiVerDataStorage *dataStorage_;
    IKvDBCommitStorage *commitStorage_;
    MultiVerKvDataStorage *kvDataStorage_;
    IKvDBMultiVerTransaction *transaction_;
    SliceTransaction *sliceTransaction_;
    Version readVersion_ = 0;
};
} // namespace DistributedDB

#endif // MULTI_VER_STORAGE_EXECUTOR_H
#endif