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

#ifndef MULTI_VER_VACUUM_EXECUTOR_STUB_H
#define MULTI_VER_VACUUM_EXECUTOR_STUB_H

#include <atomic>
#include "multi_ver_vacuum_executor.h"

namespace DistributedDB {
struct DbScale {
    uint8_t left = 1;
    uint8_t right = 1;
    uint8_t vacuumNeed = 1;
    uint8_t shadow = 1;
};

class MultiVerVacuumExecutorStub : public MultiVerVacuumExecutor {
public:
    // Total Time: (3 + 2L + 2LT + LTS + 2R + RT) Multiple timeCostEachCall(In Millisecond)
    MultiVerVacuumExecutorStub(const DbScale &inScale, int timeCostEachCall);
    ~MultiVerVacuumExecutorStub();

    bool IsTransactionOccupied();

    int GetVacuumAbleCommits(std::list<MultiVerCommitInfo> &leftBranchCommits,
        std::list<MultiVerCommitInfo> &rightBranchCommits) const;
    int GetVacuumNeedRecordsByVersion(uint64_t version, std::list<MultiVerRecordInfo> &vacuumNeedRecords);
    int GetShadowRecordsOfClearTypeRecord(uint64_t version, const std::vector<uint8_t> &hashKey,
        std::list<MultiVerRecordInfo> &shadowRecords);
    int GetShadowRecordsOfNonClearTypeRecord(uint64_t version, const std::vector<uint8_t> &hashKey,
        std::list<MultiVerRecordInfo> &shadowRecords);

    int StartTransactionForVacuum();
    int CommitTransactionForVacuum();
    int RollBackTransactionForVacuum();

    int DeleteRecordTotally(uint64_t version, const std::vector<uint8_t> &hashKey);
    int MarkRecordAsVacuumDone(uint64_t version, const std::vector<uint8_t> &hashKey);
    int MarkCommitAsVacuumDone(const std::vector<uint8_t> &commitId);
private:
    DbScale dbScale_;
    int timeCostEachCall_;
    std::atomic<bool> transactionOccupied_;
};
}

#endif // MULTI_VER_VACUUM_EXECUTOR_STUB_H