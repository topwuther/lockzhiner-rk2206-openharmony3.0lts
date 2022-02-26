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

#ifndef MULTI_VER_VACUUM_EXECUTOR_H
#define MULTI_VER_VACUUM_EXECUTOR_H

#include <list>
#include <string>
#include <vector>
#include <cstdint>

namespace DistributedDB {
enum class RecordType {
    CLEAR,
    DELETE,
    VALID, // Not clear nor delete
};

struct MultiVerRecordInfo {
    RecordType type;
    uint64_t version;
    std::vector<uint8_t> hashKey;
};

struct MultiVerCommitInfo {
    uint64_t version;
    std::vector<uint8_t> commitId;
};

// All functions will not be concurrently called
class MultiVerVacuumExecutor {
public:
    // Call this always beyond transaction
    virtual int GetVacuumAbleCommits(std::list<MultiVerCommitInfo> &leftBranchCommits,
        std::list<MultiVerCommitInfo> &rightBranchCommits) const = 0;

    // Call this within or beyond transaction
    virtual int GetVacuumNeedRecordsByVersion(uint64_t version, std::list<MultiVerRecordInfo> &vacuumNeedRecords) = 0;

    // Call this within or beyond transaction
    virtual int GetShadowRecordsOfClearTypeRecord(uint64_t version, const std::vector<uint8_t> &hashKey,
        std::list<MultiVerRecordInfo> &shadowRecords) = 0;

    // Call this within or beyond transaction
    virtual int GetShadowRecordsOfNonClearTypeRecord(uint64_t version, const std::vector<uint8_t> &hashKey,
        std::list<MultiVerRecordInfo> &shadowRecords) = 0;

    // Call this before change the database
    virtual int StartTransactionForVacuum() = 0;

    // Call this if nothing error happened, if this itself failed, do not need to call rollback
    virtual int CommitTransactionForVacuum() = 0;

    // Call this if anything wrong happened after start transaction except commit fail
    virtual int RollBackTransactionForVacuum() = 0;

    // Call this always within transaction
    virtual int DeleteRecordTotally(uint64_t version, const std::vector<uint8_t> &hashKey) = 0;

    // Call this always within transaction
    virtual int MarkRecordAsVacuumDone(uint64_t version, const std::vector<uint8_t> &hashKey) = 0;

    // Call this always within transaction
    virtual int MarkCommitAsVacuumDone(const std::vector<uint8_t> &commitId) = 0;

    virtual ~MultiVerVacuumExecutor() {};
};
} // namespace DistributedDB

#endif // MULTI_VER_VACUUM_EXECUTOR_H
