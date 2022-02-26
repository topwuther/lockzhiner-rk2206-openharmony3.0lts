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

#ifndef MULTI_VER_NATURAL_STORE_COMMIT_STORAGE_H
#define MULTI_VER_NATURAL_STORE_COMMIT_STORAGE_H

#ifndef OMIT_MULTI_VER
#include <map>
#include <stack>

#include "db_types.h"
#include "ikvdb.h"
#include "ikvdb_commit_storage.h"
#include "macro_utils.h"

namespace DistributedDB {
class MultiVerNaturalStoreCommitStorage final : public IKvDBCommitStorage {
public:
    MultiVerNaturalStoreCommitStorage();
    ~MultiVerNaturalStoreCommitStorage() override;

    DISABLE_COPY_ASSIGN_MOVE(MultiVerNaturalStoreCommitStorage);

    int CheckVersion(const Property &property, bool &isDbExist) const override;

    int Open(const IKvDBCommitStorage::Property &property) override;

    void Close() override;

    int Remove(const IKvDBCommitStorage::Property &property) override;

    IKvDBCommit *AllocCommit(int &errCode) const override;

    IKvDBCommit *GetCommit(const CommitID &commitId, int &errCode) const override;

    int AddCommit(const IKvDBCommit &commitEntry, bool isHeader) override;
    int RemoveCommit(const CommitID &commitId) override;

    void ReleaseCommit(const IKvDBCommit *commit) const override;

    int SetHeader(const CommitID &commitId) override;
    CommitID GetHeader(int &errCode) const override;

    bool CommitExist(const CommitID &commitId, int &errCode) const override;

    int GetLatestCommits(std::map<DeviceID, IKvDBCommit *> &latestCommits) const override;

    Version GetMaxCommitVersion(int &errCode) const override;

    int GetCommitTree(const std::map<DeviceID, CommitID> &latestCommits,
        std::list<IKvDBCommit *> &commits) const override;

    int RunRekeyLogic(CipherType type, const CipherPassword &passwd);

    int RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &dbDir);

    int BackupCurrentDatabase(const Property &property, const std::string &dir) override;

    int ImportDatabase(const Property &property, const std::string &dir, const CipherPassword &passwd) override;

    int StartVacuum() override;

    int CancelVacuum() override;

    int FinishlVacuum() override;

    int GetAllCommitsInTree(std::list<MultiVerCommitNode> &commits) const override;

private:
    static void TransferCommitIDToKey(const CommitID &commitID, Key &key);

    static int TransferCommitToValue(const IKvDBCommit &commit, Value &value);
    static int TransferValueToCommit(const Value &value, IKvDBCommit &commit);

    static void TransferStringToKey(const std::string &str, Key &key);

    static void TransferCommitIDToValue(const CommitID &commitID, Value &value);
    static void TransferValueToCommitID(const Value &value, CommitID &commitID);

    static bool CompareCommit(const IKvDBCommit *first, const IKvDBCommit *second);

    static void GetLocalVersionThredForLatestCommits(const std::map<CommitID, IKvDBCommit *> &allCommits,
        const std::map<DeviceID, CommitID> &latestCommits, std::map<DeviceID, Version> &latestCommitVersions);

    static void AddParentsToStack(const IKvDBCommit *commit,
        const std::map<CommitID, IKvDBCommit *> &allCommits, std::stack<CommitID> &commitStack);

    static void RefreshCommitTree(std::list<IKvDBCommit *> &commits);

    int GetAllCommits(std::map<CommitID, IKvDBCommit *> &commits, CommitID &header) const;

    int SetHeaderInner(const CommitID &commitId);

    int CheckAddedCommit(const IKvDBCommit &commitEntry) const;

    // Release the latest commits for exception.
    void ReleaseLatestCommits(std::map<DeviceID, IKvDBCommit *> &latestCommits) const;

    // Release the untraveled commits
    void ReleaseUnusedCommits(std::map<CommitID, IKvDBCommit *> &commits) const;

    void ReleaseCommitList(std::list<IKvDBCommit *> &commits) const;

    static int GetVersion(const IKvDBCommitStorage::Property &property, int &version, bool &isDbExisted);

private:
    static const std::string HEADER_KEY;
    std::string branchTag_;
    IKvDB *commitStorageDatabase_;
    IKvDBConnection *commitStorageDBConnection_;
};
} // namespace DistributedDB

#endif // MULTI_VER_NATURAL_STORE_COMMIT_STORAGE_H
#endif