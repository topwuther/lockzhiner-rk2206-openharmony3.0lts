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

#include "multi_ver_vacuum_executor_stub.h"
#include <thread>
#include "db_errno.h"

using namespace DistributedDB;

MultiVerVacuumExecutorStub::MultiVerVacuumExecutorStub(const DbScale &inScale, int timeCostEachCall)
    : dbScale_(inScale), timeCostEachCall_(timeCostEachCall), transactionOccupied_(false)
{
}

MultiVerVacuumExecutorStub::~MultiVerVacuumExecutorStub()
{
}

bool MultiVerVacuumExecutorStub::IsTransactionOccupied()
{
    return transactionOccupied_;
}

int MultiVerVacuumExecutorStub::GetVacuumAbleCommits(std::list<MultiVerCommitInfo> &leftBranchCommits,
    std::list<MultiVerCommitInfo> &rightBranchCommits) const
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    for (uint8_t i = dbScale_.left + dbScale_.right; i > dbScale_.right; i--) {
        MultiVerCommitInfo commit;
        commit.version = i;
        commit.commitId.push_back(i);
        leftBranchCommits.push_back(commit);
    }
    for (uint8_t i = dbScale_.right; i > 0; i--) {
        MultiVerCommitInfo commit;
        commit.version = i;
        commit.commitId.push_back(i);
        rightBranchCommits.push_back(commit);
    }
    return E_OK;
}

int MultiVerVacuumExecutorStub::GetVacuumNeedRecordsByVersion(uint64_t version,
    std::list<MultiVerRecordInfo> &vacuumNeedRecords)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    for (uint8_t i = dbScale_.vacuumNeed; i > 0; i--) {
        MultiVerRecordInfo record;
        record.type = RecordType::VALID;
        record.version = version;
        record.hashKey.push_back(i);
        vacuumNeedRecords.push_back(record);
    }
    return E_OK;
}

int MultiVerVacuumExecutorStub::GetShadowRecordsOfClearTypeRecord(uint64_t version,
    const std::vector<uint8_t> &hashKey, std::list<MultiVerRecordInfo> &shadowRecords)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    return E_OK;
}

int MultiVerVacuumExecutorStub::GetShadowRecordsOfNonClearTypeRecord(uint64_t version,
    const std::vector<uint8_t> &hashKey, std::list<MultiVerRecordInfo> &shadowRecords)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    for (uint8_t i = dbScale_.shadow; i > 0; i--) {
        MultiVerRecordInfo record;
        record.type = RecordType::VALID;
        record.version = i;
        record.hashKey = hashKey;
        shadowRecords.push_back(record);
    }
    return E_OK;
}

int MultiVerVacuumExecutorStub::StartTransactionForVacuum()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    transactionOccupied_ = true;
    return E_OK;
}

int MultiVerVacuumExecutorStub::CommitTransactionForVacuum()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    transactionOccupied_ = false;
    return E_OK;
}

int MultiVerVacuumExecutorStub::RollBackTransactionForVacuum()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    transactionOccupied_ = false;
    return E_OK;
}

int MultiVerVacuumExecutorStub::DeleteRecordTotally(uint64_t version, const std::vector<uint8_t> &hashKey)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    return E_OK;
}

int MultiVerVacuumExecutorStub::MarkRecordAsVacuumDone(uint64_t version, const std::vector<uint8_t> &hashKey)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    return E_OK;
}

int MultiVerVacuumExecutorStub::MarkCommitAsVacuumDone(const std::vector<uint8_t> &commitId)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeCostEachCall_));
    return E_OK;
}
