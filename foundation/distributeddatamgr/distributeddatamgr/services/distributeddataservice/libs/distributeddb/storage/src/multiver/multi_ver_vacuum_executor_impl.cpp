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

#ifndef OMIT_MULTI_VER
#include "multi_ver_vacuum_executor_impl.h"
#include "db_errno.h"
#include "log_print.h"
#include "multi_ver_storage_executor.h"

namespace DistributedDB {
namespace {
    const uint64_t DEL_FLAG = 0x02; // Del type record flag in OperFlag
    const uint64_t CLEAR_FLAG = 0x03; // Clear type record flag in OperFlag
    const uint64_t MASK_FLAG = 0x07; // mask.
}

MultiVerVacuumExecutorImpl::MultiVerVacuumExecutorImpl(MultiVerNaturalStore *multiKvDB)
    : multiKvDB_(multiKvDB), writeHandle_(nullptr)
{
}

MultiVerVacuumExecutorImpl::~MultiVerVacuumExecutorImpl()
{
    // In abnormal case that transaction not commit or rollback
    if (multiKvDB_ != nullptr && writeHandle_ != nullptr) {
        multiKvDB_->ReleaseHandle(writeHandle_, true);
    }
}

// Call this always beyond transaction
int MultiVerVacuumExecutorImpl::GetVacuumAbleCommits(std::list<MultiVerCommitInfo> &leftBranchCommits,
    std::list<MultiVerCommitInfo> &rightBranchCommits) const
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ != nullptr) {
        LOGE("[VacuumExec][GetCommit] Mis-Called Within Transaction");
        return -E_NOT_PERMIT;
    }

    // It will return at least zero, it's ok. return at most UINT64_MAX to means that all left commit are vacuumable.
    uint64_t maxVersionOfVacuumAbleLeftCommit = multiKvDB_->GetMaxTrimmableVersion();

    int errCode = E_OK;
    MultiVerStorageExecutor *readHandle = multiKvDB_->GetHandle(false, errCode, true);
    if (errCode != E_OK || readHandle == nullptr) {
        LOGE("[VacuumExec][GetCommit] GetHandle fail, errCode=%d", errCode);
        return errCode;
    }

    std::list<MultiVerCommitNode> commitsInTree;
    errCode = readHandle->GetAllCommitsInTree(commitsInTree);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][GetCommit] GetAllCommitsInTree fail, errCode=%d", errCode);
        multiKvDB_->ReleaseHandle(readHandle, true);
        return errCode;
    }

    // As discussed and agreed, the commit in commitsInTree had already be sorted in descending order by version
    for (auto &eachCommit : commitsInTree) {
        if (eachCommit.isLocal) {
            if (eachCommit.version > maxVersionOfVacuumAbleLeftCommit) {
                continue;
            }
            leftBranchCommits.emplace_back(MultiVerCommitInfo{eachCommit.version, eachCommit.commitId});
        } else {
            rightBranchCommits.emplace_back(MultiVerCommitInfo{eachCommit.version, eachCommit.commitId});
        }
    }

    multiKvDB_->ReleaseHandle(readHandle, true);
    return E_OK;
}

// Call this within or beyond transaction
int MultiVerVacuumExecutorImpl::GetVacuumNeedRecordsByVersion(uint64_t version,
    std::list<MultiVerRecordInfo> &vacuumNeedRecords)
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    MultiVerStorageExecutor *handle = GetCorrectHandleForUse();
    if (handle == nullptr) {
        return -E_NO_RESOURCE_FOR_USE;
    }

    std::list<MultiVerTrimedVersionData> recordsInCommit;
    int errCode = handle->GetEntriesByVersion(version, recordsInCommit);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][GetVacuumNeed] GetEntriesByVersion fail, errCode=%d", errCode);
        ReleaseHandleIfNeed(handle);
        return errCode;
    }

    for (auto &eachRecord : recordsInCommit) {
        vacuumNeedRecords.emplace_back(MultiVerRecordInfo{GetRecordType(eachRecord), eachRecord.version,
            eachRecord.key});
    }

    ReleaseHandleIfNeed(handle);
    return E_OK;
}

// Call this within or beyond transaction
int MultiVerVacuumExecutorImpl::GetShadowRecordsOfClearTypeRecord(uint64_t version,
    const std::vector<uint8_t> &hashKey, std::list<MultiVerRecordInfo> &shadowRecords)
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    MultiVerStorageExecutor *handle = GetCorrectHandleForUse();
    if (handle == nullptr) {
        return -E_NO_RESOURCE_FOR_USE;
    }

    std::list<MultiVerTrimedVersionData> clearShadowRecords;
    int errCode = handle->GetOverwrittenClearTypeEntries(version, clearShadowRecords);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][GetShadowClear] GetOverwrittenClearTypeEntries:%zu fail, err=%d", hashKey.size(), errCode);
        ReleaseHandleIfNeed(handle);
        return errCode;
    }

    for (auto &eachRecord : clearShadowRecords) {
        shadowRecords.emplace_back(MultiVerRecordInfo{GetRecordType(eachRecord), eachRecord.version, eachRecord.key});
    }

    ReleaseHandleIfNeed(handle);
    return E_OK;
}

// Call this within or beyond transaction
int MultiVerVacuumExecutorImpl::GetShadowRecordsOfNonClearTypeRecord(uint64_t version,
    const std::vector<uint8_t> &hashKey, std::list<MultiVerRecordInfo> &shadowRecords)
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    MultiVerStorageExecutor *handle = GetCorrectHandleForUse();
    if (handle == nullptr) {
        return -E_NO_RESOURCE_FOR_USE;
    }

    std::list<MultiVerTrimedVersionData> nonClearShadowRecords;
    int errCode = handle->GetOverwrittenNonClearTypeEntries(version, hashKey, nonClearShadowRecords);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][GetShadowNonClear] GetOverwrittenNonClearTypeEntries fail, errCode=%d", errCode);
        ReleaseHandleIfNeed(handle);
        return errCode;
    }

    for (auto &eachRecord : nonClearShadowRecords) {
        shadowRecords.emplace_back(MultiVerRecordInfo{GetRecordType(eachRecord), eachRecord.version, eachRecord.key});
    }

    ReleaseHandleIfNeed(handle);
    return E_OK;
}

// Call this before change the database
int MultiVerVacuumExecutorImpl::StartTransactionForVacuum()
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ != nullptr) {
        LOGE("[VacuumExec][Start] Transaction Already Started.");
        return -E_NOT_PERMIT;
    }

    int errCode = E_OK;
    writeHandle_ = multiKvDB_->GetHandle(true, errCode, true);
    if (errCode != E_OK || writeHandle_ == nullptr) {
        LOGE("[VacuumExec][Start] GetHandle fail, errCode=%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->StartTransaction(MultiTransactionType::ALL_DATA);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][Start] StartTransaction fail, errCode=%d", errCode);
        multiKvDB_->ReleaseHandle(writeHandle_, true);
        writeHandle_ = nullptr;
        return errCode;
    }
    return E_OK;
}

// Call this if nothing error happened, if this itself failed, do not need to call rollback
int MultiVerVacuumExecutorImpl::CommitTransactionForVacuum()
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ == nullptr) {
        LOGE("[VacuumExec][Commit] Transaction Had Not Been Started.");
        return -E_NOT_PERMIT;
    }

    int errCode = writeHandle_->CommitTransaction(MultiTransactionType::ALL_DATA);
    if (errCode != E_OK) {
        // Commit fail do not need to call rollback which is automatically
        LOGE("[VacuumExec][Commit] CommitTransaction fail, errCode=%d", errCode);
    }
    multiKvDB_->ReleaseHandle(writeHandle_, true);
    writeHandle_ = nullptr;
    return errCode;
}

// Call this if anything wrong happened after start transaction except commit fail
int MultiVerVacuumExecutorImpl::RollBackTransactionForVacuum()
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ == nullptr) {
        LOGE("[VacuumExec][RollBack] Transaction Had Not Been Started.");
        return -E_NOT_PERMIT;
    }

    int errCode = writeHandle_->RollBackTransaction(MultiTransactionType::ALL_DATA);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][RollBack] RollBackTransaction fail, errCode=%d", errCode);
    }
    multiKvDB_->ReleaseHandle(writeHandle_, true);
    writeHandle_ = nullptr;
    return errCode;
}

// Call this always within transaction
int MultiVerVacuumExecutorImpl::DeleteRecordTotally(uint64_t version, const std::vector<uint8_t> &hashKey)
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ == nullptr) {
        LOGE("[VacuumExec][Delete] Transaction Had Not Been Started.");
        return -E_NOT_PERMIT;
    }

    int errCode = writeHandle_->DeleteEntriesByHashKey(version, hashKey);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][Delete] DeleteEntriesByHashKey fail, errCode=%d", errCode);
    }
    return errCode;
}

// Call this always within transaction
int MultiVerVacuumExecutorImpl::MarkRecordAsVacuumDone(uint64_t version, const std::vector<uint8_t> &hashKey)
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ == nullptr) {
        LOGE("[VacuumExec][MarkRecord] Transaction Had Not Been Started.");
        return -E_NOT_PERMIT;
    }

    int errCode = writeHandle_->UpdateTrimedFlag(version, hashKey);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][MarkRecord] UpdateTrimedFlag fail, errCode=%d", errCode);
    }
    return errCode;
}

// Call this always within transaction
int MultiVerVacuumExecutorImpl::MarkCommitAsVacuumDone(const std::vector<uint8_t> &commitId)
{
    if (multiKvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ == nullptr) {
        LOGE("[VacuumExec][MarkCommit] Transaction Had Not Been Started.");
        return -E_NOT_PERMIT;
    }

    int errCode = writeHandle_->UpdateTrimedFlag(commitId);
    if (errCode != E_OK) {
        LOGE("[VacuumExec][MarkCommit] UpdateTrimedFlag fail, errCode=%d", errCode);
    }
    return errCode;
}

MultiVerStorageExecutor *MultiVerVacuumExecutorImpl::GetCorrectHandleForUse() const
{
    if (writeHandle_ != nullptr) {
        return writeHandle_;
    }
    int errCode = E_OK;
    MultiVerStorageExecutor *handle = multiKvDB_->GetHandle(false, errCode, true);
    if (errCode != E_OK || handle == nullptr) {
        LOGE("[VacuumExec][GetHandle] GetHandle fail, errCode=%d", errCode);
        return nullptr;
    }
    return handle;
}

void MultiVerVacuumExecutorImpl::ReleaseHandleIfNeed(MultiVerStorageExecutor *inHandle)
{
    if (inHandle != writeHandle_) {
        multiKvDB_->ReleaseHandle(inHandle, true);
    }
}

RecordType MultiVerVacuumExecutorImpl::GetRecordType(const MultiVerTrimedVersionData &inRecord) const
{
    if ((inRecord.operFlag & MASK_FLAG) == CLEAR_FLAG) {
        return RecordType::CLEAR;
    } else if ((inRecord.operFlag & MASK_FLAG) == DEL_FLAG) {
        return RecordType::DELETE;
    } else {
        return RecordType::VALID;
    }
}
} // namespace DistributedDB
#endif