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

#ifndef I_KV_DB_COMMIT_STORAGE_H
#define I_KV_DB_COMMIT_STORAGE_H

#include <vector>
#include <string>
#include <list>
#include <map>

#include "db_types.h"
#include "ikvdb_commit.h"
#include "multi_ver_def.h"

namespace DistributedDB {
class IKvDBCommitStorage {
public:
    struct Property final {
        std::string path;
        std::string identifierName;
        bool isNeedCreate = true;
        CipherType cipherType = CipherType::AES_256_GCM;
        CipherPassword passwd;
    };

    virtual ~IKvDBCommitStorage() {};
    virtual int CheckVersion(const Property &property, bool &isDbExist) const = 0;
    virtual int Open(const Property &property) = 0;
    virtual void Close() = 0;
    virtual int Remove(const Property &property) = 0;
    virtual IKvDBCommit *AllocCommit(int &errCode) const = 0;
    virtual IKvDBCommit *GetCommit(const CommitID &commitId, int &errCode) const = 0;
    virtual int AddCommit(const IKvDBCommit &commitEntry, bool isHeader) = 0;
    virtual int RemoveCommit(const CommitID &commitId) = 0;
    virtual void ReleaseCommit(const IKvDBCommit *commit) const = 0;
    virtual int SetHeader(const CommitID &commitId) = 0;
    virtual CommitID GetHeader(int &errCode) const = 0;
    virtual bool CommitExist(const CommitID &commitId, int &errCode) const = 0;
    virtual int GetLatestCommits(std::map<DeviceID, IKvDBCommit *> &latestCommits) const = 0;
    virtual int GetCommitTree(const std::map<DeviceID, CommitID> &latestCommits,
        std::list<IKvDBCommit *> &commits) const = 0;
    virtual Version GetMaxCommitVersion(int &errCode) const = 0;
    virtual int BackupCurrentDatabase(const Property &property, const std::string &dir) = 0;
    virtual int ImportDatabase(const Property &property, const std::string &dir, const CipherPassword &passwd) = 0;
    virtual int StartVacuum() = 0;
    virtual int CancelVacuum() = 0;
    virtual int FinishlVacuum() = 0;
    virtual int GetAllCommitsInTree(std::list<MultiVerCommitNode> &commits) const = 0;
};
} // namespace DistributedDB

#endif  // I_KV_DB_COMMIT_STORAGE_H
