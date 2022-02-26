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

#ifndef MULTI_VER_VACUUM_EXECUTOR_IMPL_H
#define MULTI_VER_VACUUM_EXECUTOR_IMPL_H

#ifndef OMIT_MULTI_VER
#include "multi_ver_vacuum_executor.h"
#include "multi_ver_natural_store.h"

namespace DistributedDB {
// All functions will not be concurrently called
class MultiVerVacuumExecutorImpl final : public MultiVerVacuumExecutor {
public:
    explicit MultiVerVacuumExecutorImpl(MultiVerNaturalStore *multiKvDB);
    ~MultiVerVacuumExecutorImpl() override;

    // Call this always beyond transaction
    int GetVacuumAbleCommits(std::list<MultiVerCommitInfo> &leftBranchCommits,
        std::list<MultiVerCommitInfo> &rightBranchCommits) const override;

    // Call this within or beyond transaction
    int GetVacuumNeedRecordsByVersion(uint64_t version, std::list<MultiVerRecordInfo> &vacuumNeedRecords) override;

    // Call this within or beyond transaction
    int GetShadowRecordsOfClearTypeRecord(uint64_t version, const std::vector<uint8_t> &hashKey,
        std::list<MultiVerRecordInfo> &shadowRecords) override;

    // Call this within or beyond transaction
    int GetShadowRecordsOfNonClearTypeRecord(uint64_t version, const std::vector<uint8_t> &hashKey,
        std::list<MultiVerRecordInfo> &shadowRecords) override;

    // Call this before change the database
    int StartTransactionForVacuum() override;

    // Call this if nothing error happened, if this itself failed, do not need to call rollback
    int CommitTransactionForVacuum() override;

    // Call this if anything wrong happened after start transaction except commit fail
    int RollBackTransactionForVacuum() override;

    // Call this always within transaction
    int DeleteRecordTotally(uint64_t version, const std::vector<uint8_t> &hashKey) override;

    // Call this always within transaction
    int MarkRecordAsVacuumDone(uint64_t version, const std::vector<uint8_t> &hashKey) override;

    // Call this always within transaction
    int MarkCommitAsVacuumDone(const std::vector<uint8_t> &commitId) override;
private:
    MultiVerStorageExecutor *GetCorrectHandleForUse() const;
    void ReleaseHandleIfNeed(MultiVerStorageExecutor *inHandle);

    RecordType GetRecordType(const MultiVerTrimedVersionData &inRecord) const;

    MultiVerNaturalStore *multiKvDB_;
    MultiVerStorageExecutor *writeHandle_;
};
} // namespace DistributedDB

#endif // MULTI_VER_VACUUM_EXECUTOR_IMPL_H
#endif