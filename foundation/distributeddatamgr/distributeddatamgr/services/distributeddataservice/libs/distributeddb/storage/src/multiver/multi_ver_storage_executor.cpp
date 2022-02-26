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
#include "multi_ver_storage_executor.h"

#include <openssl/rand.h>

#include "db_common.h"
#include "db_errno.h"
#include "log_print.h"
#include "multi_ver_natural_store.h"
#include "multi_ver_natural_store_commit_notify_data.h"
#include "multi_ver_natural_store_transfer_data.h"
#include "value_hash_calc.h"

namespace DistributedDB {
MultiVerStorageExecutor::MultiVerStorageExecutor(IKvDB *kvDB, IKvDBMultiVerDataStorage *dataStorage,
    IKvDBCommitStorage *commitStorage, MultiVerKvDataStorage *kvDataStorage, bool writable)
    : StorageExecutor(writable),
      kvDB_(kvDB),
      dataStorage_(dataStorage),
      commitStorage_(commitStorage),
      kvDataStorage_(kvDataStorage),
      transaction_(nullptr),
      sliceTransaction_(nullptr)
{}

MultiVerStorageExecutor::~MultiVerStorageExecutor()
{
    kvDB_ = nullptr;
    dataStorage_ = nullptr;
    commitStorage_ = nullptr;
    kvDataStorage_ = nullptr;
    transaction_ = nullptr;
}

int MultiVerStorageExecutor::Reset()
{
    return E_OK;
}

int MultiVerStorageExecutor::PutMetaData(const Key &key, const Value &value)
{
    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = kvDataStorage_->PutMetaData(key, value);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::GetMetaData(const Key &key, Value &value) const
{
    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = kvDataStorage_->GetMetaData(key, value);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &commitMap) const
{
    if (commitStorage_ == nullptr) {
        LOGE("The commit history module is null.");
        return -E_INVALID_DB;
    }
    std::map<DeviceID, IKvDBCommit *> latestCommits;
    int errCode = commitStorage_->GetLatestCommits(latestCommits);
    if (errCode != E_OK) {
        LOGE("Get latest commits failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    for (auto &latestCommit : latestCommits) {
        uint64_t localFlag = (latestCommit.second->GetLocalFlag() ?
            MultiVerCommitNode::LOCAL_FLAG : MultiVerCommitNode::NON_LOCAL_FLAG);
        MultiVerCommitNode commit = {
            latestCommit.second->GetCommitId(), // commitId
            latestCommit.second->GetLeftParentId(), // leftParent
            latestCommit.second->GetRightParentId(), // rightParent
            latestCommit.second->GetTimestamp(), // timestamp
            latestCommit.second->GetCommitVersion(), // version
            localFlag, // isLocal
            latestCommit.second->GetDeviceInfo() // deviceInfo
        };

        commitStorage_->ReleaseCommit(latestCommit.second);
        latestCommit.second = nullptr;
        commitMap.insert(std::make_pair(latestCommit.first, std::move(commit)));
    }
    latestCommits.clear();
    return E_OK;
}

int MultiVerStorageExecutor::GetCommitTree(const std::map<std::string, MultiVerCommitNode> &commitMap,
    std::vector<MultiVerCommitNode> &commits) const
{
    if (commitStorage_ == nullptr) {
        LOGE("The commit history module is null.");
        return -E_INVALID_DB;
    }
    std::map<DeviceID, CommitID> latestCommits;
    for (auto &latestCommit : commitMap) {
        latestCommits.insert(std::make_pair(latestCommit.first, latestCommit.second.commitId));
    }
    std::list<IKvDBCommit *> commitTree;
    int errCode = commitStorage_->GetCommitTree(latestCommits, commitTree);
    if (errCode != E_OK) {
        LOGE("Get commit tree failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    LOGD("Get commit tree size:%zu", commitTree.size());
    for (auto &commitNode : commitTree) {
        if (commitNode == nullptr) {
            continue;
        }
        uint64_t localFlag = (commitNode->GetLocalFlag() ?
            MultiVerCommitNode::LOCAL_FLAG : MultiVerCommitNode::NON_LOCAL_FLAG);
        MultiVerCommitNode commit = {
            commitNode->GetCommitId(), // commitId
            commitNode->GetLeftParentId(), // leftParent
            commitNode->GetRightParentId(), // rightParent
            commitNode->GetTimestamp(), // timestamp
            commitNode->GetCommitVersion(), // version
            localFlag, // isLocal
            commitNode->GetDeviceInfo() // deviceInfo
        };

        commitStorage_->ReleaseCommit(commitNode);
        commitNode = nullptr;
        commits.push_back(std::move(commit));
    }
    commitTree.clear();
    return E_OK;
}

int MultiVerStorageExecutor::GetCommitData(const MultiVerCommitNode &commit,
    std::vector<MultiVerKvEntry *> &entries) const
{
    if ((commitStorage_ == nullptr) || (dataStorage_ == nullptr)) {
        return -E_INVALID_DB;
    }
    // call the putting value method.
    CommitID commitId = commit.commitId;
    int errCode = E_OK;
    Version version;
    IKvDBCommit *commitNode = commitStorage_->GetCommit(commitId, errCode);
    if (commitNode == nullptr) {
        LOGE("Failed to get the commit:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    // Get the commit and the version.
    std::string devInfo = commitNode->GetDeviceInfo();
    version = commitNode->GetCommitVersion();
    commitStorage_->ReleaseCommit(commitNode);
    commitNode = nullptr;
    if (devInfo.size() != MULTI_VER_TAG_SIZE) {
        LOGD("skip the foreign data");
        entries.clear();
        entries.shrink_to_fit();
        return E_OK;
    }

    IKvDBMultiVerTransaction *transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, version, errCode);
    if (transaction == nullptr) {
        LOGE("Failed to get the transaction:%d", errCode);
        goto END;
    }

    errCode = transaction->GetEntriesByVersion(version, entries);
    if (errCode != E_OK) {
        LOGE("Get entries by version failed:%d", errCode);
    }
END:
    if (transaction != nullptr) {
        dataStorage_->ReleaseTransaction(transaction);
        transaction = nullptr;
    }
    return CheckCorruptedStatus(errCode);
}

bool MultiVerStorageExecutor::IsCommitExisted(const MultiVerCommitNode &commit, int &errCode) const
{
    if ((commitStorage_ == nullptr) || (dataStorage_ == nullptr)) {
        LOGE("The commit history module or data storage is null.");
        return false;
    }
    auto readCommit = commitStorage_->GetCommit(commit.commitId, errCode);
    if (readCommit == nullptr) {
        return false;
    }
    commitStorage_->ReleaseCommit(readCommit);

    bool result = false;
    std::vector<MultiVerKvEntry *> entries;
    auto transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, commit.version, errCode);
    if (transaction == nullptr) {
        LOGE("Failed to get the transaction:%d", errCode);
        goto END;
    }

    errCode = transaction->GetEntriesByVersion(commit.version, entries);
    if (errCode != E_OK) {
        LOGE("Get entries by version failed:%d", errCode);
        goto END;
    }
    if (!entries.empty()) {
        result = true;
    }
END:
    if (errCode != E_OK) {
        result = false;
    }
    if (transaction != nullptr) {
        dataStorage_->ReleaseTransaction(transaction);
    }

    ReleaseMultiVerKvEntries(entries);
    errCode = CheckCorruptedStatus(errCode);
    return result;
}

bool MultiVerStorageExecutor::IsValueSliceExisted(const ValueSliceHash &value, int &errCode) const
{
    if (kvDataStorage_ == nullptr) {
        errCode = -E_INVALID_DB;
        return false;
    }
    auto sliceTransaction = kvDataStorage_->GetSliceTransaction(false, errCode);
    if (sliceTransaction == nullptr) {
        (void)(CheckCorruptedStatus(errCode));
        return false;
    }
    Value valueReal;
    errCode = sliceTransaction->GetData(value, valueReal);
    kvDataStorage_->ReleaseSliceTransaction(sliceTransaction);
    if (errCode == E_OK) {
        return true;
    }
    (void)(CheckCorruptedStatus(errCode));
    return false;
}

int MultiVerStorageExecutor::GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const
{
    return GetValueSliceInner(nullptr, hashValue, sliceValue);
}

int MultiVerStorageExecutor::PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue,
    bool isAddCount)
{
    return PutValueSliceInner(nullptr, hashValue, sliceValue, isAddCount);
}

int MultiVerStorageExecutor::GetValueSliceInner(const SliceTransaction *sliceTransaction,
    const ValueSliceHash &hashValue, ValueSlice &sliceValue) const
{
    int errCode;
    if (sliceTransaction != nullptr) {
        errCode = sliceTransaction->GetData(hashValue, sliceValue);
        return CheckCorruptedStatus(errCode);
    }
    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    auto sliceTransact = kvDataStorage_->GetSliceTransaction(false, errCode);
    if (sliceTransact == nullptr) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = sliceTransact->GetData(hashValue, sliceValue);
    kvDataStorage_->ReleaseSliceTransaction(sliceTransact);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::PutValueSliceInner(SliceTransaction *sliceTransaction, const ValueSliceHash &hashValue,
    const ValueSlice &sliceValue, bool isAddCount)
{
    int errCode;
    if (sliceTransaction != nullptr) {
        errCode = sliceTransaction->PutData(hashValue, sliceValue, isAddCount);
        return CheckCorruptedStatus(errCode);
    }

    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    auto transaction = kvDataStorage_->GetSliceTransaction(true, errCode);
    if (transaction == nullptr) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = transaction->PutData(hashValue, sliceValue, isAddCount);
    kvDataStorage_->ReleaseSliceTransaction(transaction);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::DeleteValueSliceInner(SliceTransaction *sliceTransaction,
    const ValueSliceHash &hashValue)
{
    int errCode;
    if (sliceTransaction != nullptr) {
        errCode = sliceTransaction->DeleteData(hashValue);
        return CheckCorruptedStatus(errCode);
    }

    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    auto transaction = kvDataStorage_->GetSliceTransaction(true, errCode);
    if (transaction == nullptr) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = transaction->DeleteData(hashValue);
    kvDataStorage_->ReleaseSliceTransaction(transaction);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::StartSliceTransaction()
{
    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (sliceTransaction_ != nullptr) {
        return -E_UNEXPECTED_DATA;
    }
    int errCode;
    sliceTransaction_ = kvDataStorage_->GetSliceTransaction(true, errCode);
    if (sliceTransaction_ == nullptr) {
        return errCode;
    }
    errCode = sliceTransaction_->StartTransaction();
    if (errCode != E_OK) {
        kvDataStorage_->ReleaseSliceTransaction(sliceTransaction_);
    }
    return errCode;
}

int MultiVerStorageExecutor::CommitSliceTransaction()
{
    if (sliceTransaction_ == nullptr) {
        return -E_UNEXPECTED_DATA;
    }
    int errCode = sliceTransaction_->CommitTransaction();
    if (errCode != E_OK) {
        LOGE("Commit slice transaction failed:%d", errCode);
    }
    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    kvDataStorage_->ReleaseSliceTransaction(sliceTransaction_);
    sliceTransaction_ = nullptr;
    return errCode;
}

int MultiVerStorageExecutor::RollbackSliceTransaction()
{
    if (sliceTransaction_ == nullptr) {
        return -E_UNEXPECTED_DATA;
    }
    int errCode = sliceTransaction_->RollbackTransaction();
    if (errCode != E_OK) {
        LOGE("Commit slice transaction failed:%d", errCode);
    }
    if (kvDataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    kvDataStorage_->ReleaseSliceTransaction(sliceTransaction_);
    sliceTransaction_ = nullptr;
    return errCode;
}

int MultiVerStorageExecutor::ReInitTransactionVersion(const MultiVerCommitNode &commit)
{
    if (commitStorage_ == nullptr) {
        LOGE("The commit history module is null when reinit transaction version.");
        return -E_INVALID_DB;
    }
    int errCode = StartTransaction();
    if (errCode != E_OK) {
        LOGE("Start transaction failed:%d", errCode);
        return errCode;
    }
    auto readCommit = commitStorage_->GetCommit(commit.commitId, errCode);
    if (readCommit == nullptr) {
        if (errCode != -E_NOT_FOUND) {
            RollBackTransaction();
            LOGE("Get the commit error:%d", errCode);
            return errCode;
        } else {
            errCode = E_OK;
        }
    } else {
        LOGD("Reput the version:%llu", readCommit->GetCommitVersion());
        transaction_->SetVersion(readCommit->GetCommitVersion());
        commitStorage_->ReleaseCommit(readCommit);
    }

    if (errCode != E_OK) {
        RollBackTransaction();
    }
    return errCode;
}

int MultiVerStorageExecutor::AddSliceDataCount(const std::vector<Value> &values)
{
    for (const auto &item : values) {
        MultiVerValueObject valueObject;
        int errCode = valueObject.DeSerialData(item);
        if (errCode != E_OK) {
            return errCode;
        }
        if (!valueObject.IsHash()) {
            continue;
        }
        std::vector<ValueSliceHash> valueHashList;
        valueObject.GetValueHash(valueHashList);
        for (auto &iter : valueHashList) {
            Value filledData;
            errCode = PutValueSliceInner(sliceTransaction_, iter, filledData, true);
            if (errCode != E_OK) {
                LOGE("Add the slice value count failed:%d", errCode);
                return errCode;
            }
        }
    }
    return E_OK;
}
int MultiVerStorageExecutor::PutCommitData(const MultiVerCommitNode &commit,
    const std::vector<MultiVerKvEntry *> &entries, const std::string &deviceName)
{
    // Update the version while the commit has been put.
    int errCode = ReInitTransactionVersion(commit);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }
    errCode = StartSliceTransaction();
    if (errCode != E_OK) {
        RollBackTransaction();
        return CheckCorruptedStatus(errCode);
    }

    if (transaction_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::vector<Value> values;
    errCode = transaction_->PutBatch(entries, false, values);
    if (errCode != E_OK) {
        LOGE("Put batch synced data failed:%d", errCode);
        goto END;
    }
    errCode = AddSliceDataCount(values);
    if (errCode != E_OK) {
        goto END;
    }
    errCode = CommitSliceTransaction();
    if (errCode != E_OK) {
        RollBackTransaction();
    } else {
        errCode = CommitTransaction(commit, false);
    }
    return CheckCorruptedStatus(errCode);
END:
    if (errCode != E_OK) {
        (void)(RollbackSliceTransaction());
        RollBackTransaction();
    }
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::MergeSyncCommit(const MultiVerCommitNode &commit,
    const std::vector<MultiVerCommitNode> &commits)
{
    if (commits.empty()) {
        return E_OK;
    }
    // if all the nodes have two parents, no need to merge.
    bool isAllMerged = true;
    for (const auto &item : commits) {
        if (item.rightParent.empty()) {
            isAllMerged = false;
        }
    }

    if (isAllMerged) {
        LOGI("all nodes have been merged");
        return E_OK;
    }

    int errCode = MergeCommits(commits);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::MergeOneCommit(const MultiVerCommitNode &commit)
{
    std::vector<MultiVerKvEntry *> entries;
    int errCode = GetResolvedConflictEntries(commit, entries);
    if (errCode != E_OK) {
        return errCode;
    }

    if (transaction_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::vector<Value> values;
    errCode = transaction_->PutBatch(entries, true, values);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = AddSliceDataCount(values);
END:
    ReleaseMultiVerKvEntries(entries);
    return errCode;
}

int MultiVerStorageExecutor::MergeCommits(const std::vector<MultiVerCommitNode> &commits)
{
    const MultiVerCommitNode &rootCommitNode = commits.back();
    std::string rootNodeDeviceInfo = rootCommitNode.deviceInfo;
    if (rootNodeDeviceInfo.size() != SHA256_DIGEST_LENGTH + MULTI_VER_TAG_SIZE) {
        return -E_UNEXPECTED_DATA;
    }
    int errCode = StartTransaction();
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = StartSliceTransaction();
    if (errCode != E_OK) {
        RollBackTransaction();
        return errCode;
    }
    for (const auto &item : commits) {
        // only need to merge the node data which is from the same device
        if (item.deviceInfo.size() != SHA256_DIGEST_LENGTH + MULTI_VER_TAG_SIZE &&
            item.deviceInfo.size() != MULTI_VER_TAG_SIZE) {
            errCode = -E_UNEXPECTED_DATA;
            break;
        }
        if (item.deviceInfo.size() == MULTI_VER_TAG_SIZE ||
            item.deviceInfo.compare(0, SHA256_DIGEST_LENGTH, rootNodeDeviceInfo, 0, SHA256_DIGEST_LENGTH) != 0) {
            LOGD("Skip the version:%llu", item.version);
            continue;
        }
        errCode = MergeOneCommit(item);
        if (errCode != E_OK) {
            break;
        }
    }

    if (errCode != E_OK) {
        (void)(RollbackSliceTransaction());
        errCode = RollBackTransaction();
    } else {
        errCode = CommitSliceTransaction();
        if (errCode == E_OK) {
            errCode = CommitTransaction(rootCommitNode, true);
        } else {
            LOGE("Commit the slice transaction error, rollback the data transaction");
            RollBackTransaction();
        }
    }
    return errCode;
}

int MultiVerStorageExecutor::GetDiffEntries(const CommitID &begin, const CommitID &end, MultiVerDiffData &data) const
{
    if ((commitStorage_ == nullptr) || (dataStorage_ == nullptr)) {
        return -E_INVALID_DB;
    }

    int errCode = E_OK;
    Version verBegin;
    if (begin.empty()) {
        verBegin = 0;
    } else {
        IKvDBCommit *commitBegin = commitStorage_->GetCommit(begin, errCode);
        if (commitBegin == nullptr) {
            verBegin = 0;
        } else {
            verBegin = commitBegin->GetCommitVersion();
        }
        commitStorage_->ReleaseCommit(commitBegin);
        commitBegin = nullptr;
    }

    IKvDBCommit *commitEnd = commitStorage_->GetCommit(end, errCode);
    if (commitEnd == nullptr) {
        return CheckCorruptedStatus(errCode);
    }

    Version verEnd = commitEnd->GetCommitVersion();
    commitStorage_->ReleaseCommit(commitEnd);
    commitEnd = nullptr;

    IKvDBMultiVerTransaction *transaction =
        dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, verBegin, errCode);
    if (transaction == nullptr) {
        LOGE("Get diff data start read failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    errCode = transaction->GetDiffEntries(verBegin, verEnd, data);
    if (errCode != E_OK) {
        LOGE("get diff entries failed:%d", errCode);
        goto END;
    }

    errCode = TransferDiffEntries(data);
END:
    dataStorage_->ReleaseTransaction(transaction);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::Get(const Key &key, Value &value) const
{
    if (dataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    auto transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, readVersion_, errCode);
    if (transaction == nullptr) {
        LOGE("Get read transaction failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    Value rawValue;
    errCode = transaction->Get(key, rawValue);

    dataStorage_->ReleaseTransaction(transaction);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    return TransferToUserValue(rawValue, value);
}

int MultiVerStorageExecutor::GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    if (dataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    auto transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, readVersion_, errCode);
    if (transaction == nullptr) {
        LOGE("Get read transaction failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    errCode = transaction->GetEntries(keyPrefix, entries);

    dataStorage_->ReleaseTransaction(transaction);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    for (auto &item : entries) {
        Value userValue;
        errCode = TransferToUserValue(item.value, userValue);
        if (errCode != E_OK) {
            entries.clear();
            entries.shrink_to_fit();
            break;
        }
        std::swap(userValue, item.value);
    }

    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::Put(const Key &key, const Value &value)
{
    if (transaction_ == nullptr) {
        return -E_INVALID_DB;
    }
    Value savedValue;
    int errCode = TransferToSavedValue(value, savedValue);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }
    errCode = transaction_->Put(key, savedValue);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::Delete(const Key &key)
{
    if (transaction_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = transaction_->Delete(key);
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::Clear()
{
    if (transaction_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = transaction_->Clear();
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::StartAllDbTransaction()
{
    if (dataStorage_ == nullptr || commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    IKvDBMultiVerTransaction *transaction = nullptr;
    int errCode = dataStorage_->StartWrite(KvDataType::KV_DATA_SYNC_P2P, transaction);
    if (transaction == nullptr) {
        LOGE("start write transaction failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    // start data storage transaction
    Version maxVersion = static_cast<MultiVerNaturalStore *>(kvDB_)->GetMaxCommitVersion();
    transaction->SetVersion(maxVersion);

    errCode = transaction->StartTransaction();
    if (errCode != E_OK) {
        LOGE("Start dataStorage transaction failed:%d", errCode);
        goto END;
    }

    // start commit history transaction
    errCode = commitStorage_->StartVacuum();
    if (errCode != E_OK) {
        transaction->RollBackTransaction();
        LOGE("Start commitStorage transaction failed:%d", errCode);
        goto END;
    }

    // start slice data transaction
    errCode = StartSliceTransaction();
    if (errCode != E_OK) {
        transaction->RollBackTransaction();
        commitStorage_->CancelVacuum();
        LOGE("Start kvDataStorage transaction failed:%d", errCode);
        goto END;
    }
    transaction_ = transaction;
END:
    if (errCode != E_OK) {
        dataStorage_->ReleaseTransaction(transaction);
        transaction = nullptr;
        transaction_ = nullptr;
        return CheckCorruptedStatus(errCode);
    }

    return errCode;
}

int MultiVerStorageExecutor::StartTransaction(MultiTransactionType type)
{
    if (type == MultiTransactionType::ALL_DATA) {
        return StartAllDbTransaction();
    }

    if (dataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    IKvDBMultiVerTransaction *transaction = nullptr;
    int errCode = dataStorage_->StartWrite(KvDataType::KV_DATA_SYNC_P2P, transaction);
    if (transaction == nullptr) {
        LOGE("start write transaction failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    // Get the current max version, and the current version is max version + 1.
    Version maxVersion = static_cast<MultiVerNaturalStore *>(kvDB_)->GetMaxCommitVersion();
    transaction->SetVersion(++maxVersion);
    errCode = transaction->StartTransaction();
    if (errCode != E_OK) {
        dataStorage_->ReleaseTransaction(transaction);
        transaction = nullptr;
        LOGE("Start transaction failed:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    transaction_ = transaction;
    return E_OK;
}

int MultiVerStorageExecutor::CommitAllDbTransaction()
{
    if (dataStorage_ == nullptr || commitStorage_ == nullptr || transaction_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = transaction_->CommitTransaction();
    if (errCode != E_OK) {
        (void)(RollbackSliceTransaction());
        commitStorage_->CancelVacuum();
        LOGE("commit phase one failed:%d", errCode);
        goto END;
    }

    // start slice data transaction
    errCode = CommitSliceTransaction();
    if (errCode != E_OK) {
        commitStorage_->CancelVacuum();
        LOGE("Finish kvDataStorage transaction failed:%d", errCode);
        goto END;
    }

    // start commit history transaction
    errCode = commitStorage_->FinishlVacuum();
    if (errCode != E_OK) {
        LOGE("Finish commitStorage transaction failed:%d", errCode);
        goto END;
    }

END:
    dataStorage_->ReleaseTransaction(transaction_);
    transaction_ = nullptr;

    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::CommitTransaction(MultiTransactionType type)
{
    if (type == MultiTransactionType::ALL_DATA) {
        return CommitAllDbTransaction();
    }

    if ((dataStorage_ == nullptr) || (transaction_ == nullptr)) {
        return -E_INVALID_DB;
    }
    UpdateVerTimeStamp multiVerTimeStamp = {static_cast<MultiVerNaturalStore *>(kvDB_)->GetCurrentTimeStamp(), true};
    Version commitVersion;
    CommitID commitId;
    int errCode = E_OK;
    bool isDataChanged = transaction_->IsDataChanged();
    if (!isDataChanged) {
        transaction_->RollBackTransaction();
        goto END;
    }

    errCode = dataStorage_->CommitWritePhaseOne(transaction_, multiVerTimeStamp);
    if (errCode != E_OK) {
        LOGE("commit phase one failed:%d", errCode);
        goto END;
    }

    commitVersion = transaction_->GetVersion();
    errCode = FillAndCommitLogEntry(commitVersion, commitId, multiVerTimeStamp.timestamp);
    if (errCode != E_OK) {
        LOGE("rollback commit phase one failed:%d", errCode);
        dataStorage_->RollbackWritePhaseOne(transaction_, commitVersion);
        goto END;
    }
    LOGD("local commit version:%llu", commitVersion);
    static_cast<MultiVerNaturalStore *>(kvDB_)->SetMaxTimeStamp(multiVerTimeStamp.timestamp);
    dataStorage_->CommitWritePhaseTwo(transaction_);
    static_cast<MultiVerNaturalStore *>(kvDB_)->SetMaxCommitVersion(commitVersion);
END:
    dataStorage_->ReleaseTransaction(transaction_);
    transaction_ = nullptr;
    if (errCode == E_OK && isDataChanged) {
        CommitNotifiedData(commitId);
    }

    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::RollBackAllDbTransaction()
{
    if ((dataStorage_ == nullptr) || (commitStorage_ == nullptr)) {
        return -E_INVALID_DB;
    }
    int errCode = dataStorage_->RollbackWrite(transaction_);
    if (errCode != E_OK) {
        LOGE("Data storage rollback fail!");
        (void)(commitStorage_->CancelVacuum());
        (void)(RollbackSliceTransaction());
        goto END;
    }

    errCode = commitStorage_->CancelVacuum();
    if (errCode != E_OK) {
        LOGE("Commit storage rollback fail!");
        (void)(RollbackSliceTransaction());
        goto END;
    }

    errCode = RollbackSliceTransaction();
    if (errCode != E_OK) {
        LOGE("Value slice rollback fail!");
    }

END:
    dataStorage_->ReleaseTransaction(transaction_);
    transaction_ = nullptr;
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::RollBackTransaction(MultiTransactionType type)
{
    if (dataStorage_ == nullptr || transaction_ == nullptr) {
        LOGE("invalid transaction for rollback");
        return -E_INVALID_DB;
    }

    if (type == MultiTransactionType::ALL_DATA) {
        return RollBackAllDbTransaction();
    }

    int errCode = dataStorage_->RollbackWrite(transaction_);
    dataStorage_->ReleaseTransaction(transaction_);
    transaction_ = nullptr;
    return CheckCorruptedStatus(errCode);
}

void MultiVerStorageExecutor::Close()
{
    MultiVerStorageExecutor *handle = this;

    MultiVerNaturalStore *multiVerNatureStore = static_cast<MultiVerNaturalStore *>(kvDB_);
    if (multiVerNatureStore == nullptr) {
        return;
    }

    if (readVersion_ != 0) {
        multiVerNatureStore->RemoveVersionConstraintFromList(readVersion_);
        readVersion_ = 0;
    }
    multiVerNatureStore->ReleaseHandle(handle);
}

int MultiVerStorageExecutor::InitCurrentReadVersion()
{
    if (commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    CommitID commitId = commitStorage_->GetHeader(errCode);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    Version version = 0;
    // if no head, just use the initial version.
    if (!commitId.empty()) {
        IKvDBCommit *commit = commitStorage_->GetCommit(commitId, errCode);
        if (commit == nullptr) {
            LOGE("get the header commit failed:%d", errCode);
            return CheckCorruptedStatus(errCode);
        }

        version = commit->GetCommitVersion();
        commitStorage_->ReleaseCommit(commit);
        commit = nullptr;
    }
    readVersion_ = version;
    return E_OK;
}

int MultiVerStorageExecutor::TransferDiffEntries(MultiVerDiffData &data) const
{
    int errCode;
    Value valueTmp;
    for (auto &insertedItem : data.inserted) {
        errCode = TransferToUserValue(insertedItem.value, valueTmp);
        if (errCode != E_OK) {
            return errCode;
        }
        std::swap(insertedItem.value, valueTmp);
    }

    for (auto &updatedItem : data.updated) {
        errCode = TransferToUserValue(updatedItem.value, valueTmp);
        if (errCode != E_OK) {
            return errCode;
        }
        std::swap(updatedItem.value, valueTmp);
    }

    for (auto &deletedItem : data.deleted) {
        errCode = TransferToUserValue(deletedItem.value, valueTmp);
        if (errCode != E_OK) {
            return errCode;
        }
        std::swap(deletedItem.value, valueTmp);
    }

    return E_OK;
}

int MultiVerStorageExecutor::TransferToUserValue(const Value &savedValue, Value &value) const
{
    MultiVerValueObject valueObject;
    int errCode = valueObject.DeSerialData(savedValue);
    if (errCode != E_OK) {
        LOGE("Deserialize the multi ver saved value failed:%d", errCode);
        return errCode;
    }
    if (!valueObject.IsHash()) {
        return valueObject.GetValue(value);
    }

    std::vector<ValueSliceHash> sliceHashVect;
    errCode = valueObject.GetValueHash(sliceHashVect);
    if (errCode != E_OK) {
        return errCode;
    }
    value.clear();
    value.shrink_to_fit();
    for (const auto &item : sliceHashVect) {
        Value itemValue;
        errCode = GetValueSlice(item, itemValue);
        if (errCode != E_OK) {
            LOGE("Get hash entry error:%d", errCode);
            break;
        }
        value.insert(value.end(), itemValue.begin(), itemValue.end());
    }

    return errCode;
}

int MultiVerStorageExecutor::TransferToValueObject(const Value &value, MultiVerValueObject &valueObject)
{
    MultiVerNaturalStoreTransferData splitData;
    std::vector<Value> partValues;
    // Segment data into blocks by fixed size
    // You can set Threshold and blocksize by SetSliceLengthThreshold, SetBlockSizeByte;
    int errCode = splitData.SegmentAndTransferValueToHash(value, partValues);
    if (errCode == E_OK) {
        valueObject.SetFlag(MultiVerValueObject::HASH_FLAG);

        // Tansfer blocks data to hash value list
        std::vector<ValueSliceHash> hashValues;
        ValueSliceHash hashValue;
        for (const auto &partValue : partValues) {
            if (DBCommon::CalcValueHash(partValue, hashValue) != E_OK) {
                return -E_INTERNAL_ERROR;
            }
            // Put hash value into table
            errCode = PutValueSlice(hashValue, partValue, true);
            if (errCode != E_OK) {
                return errCode;
            }
            hashValues.push_back(std::move(hashValue));
        }

        valueObject.SetValueHash(hashValues);
    } else {
        valueObject.SetFlag(0);
        valueObject.SetValue(value);
    }
    valueObject.SetDataLength(value.size());
    return E_OK;
}

int MultiVerStorageExecutor::TransferToSavedValue(const Value &value, Value &savedValue)
{
    MultiVerValueObject valueObject;
    int errCode = TransferToValueObject(value, valueObject);
    if (errCode != E_OK) {
        LOGE("Failed to get the serialize data of value object:%d", errCode);
        return errCode;
    }

    errCode = valueObject.GetSerialData(savedValue);
    if (errCode != E_OK) {
        LOGE("failed to get the serialize data of savedValue:%d", errCode);
        return errCode;
    }

    return E_OK;
}

int MultiVerStorageExecutor::GetResolvedConflictEntries(const MultiVerCommitNode &commitItem,
    std::vector<MultiVerKvEntry *> &entries) const
{
    if (commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    auto commit = commitStorage_->GetCommit(commitItem.commitId, errCode);
    if (commit == nullptr) {
        LOGE("failed to get the commit in merge:%d", errCode);
        return errCode;
    }
    entries.clear();
    entries.shrink_to_fit();
    Version version = commit->GetCommitVersion();
    LOGD("Version is %llu", version);
    if (transaction_ != nullptr) {
        errCode = transaction_->GetEntriesByVersion(version, entries);
        if (errCode != E_OK) {
            LOGE("failed to get the entries by version:%d", errCode);
        }
    }
    commitStorage_->ReleaseCommit(commit);
    return errCode;
}

void MultiVerStorageExecutor::CommitNotifiedData(const CommitID &commitId)
{
    CommitID startId;
    Version currentVersion;
    int errCode = GetParentCommitId(commitId, startId, currentVersion);
    if (errCode != E_OK || currentVersion == 0) { // make sure that the version - 1 is valid.
        LOGE("Notify: get the parent commit failed:%d", errCode);
        return;
    }
    MultiVerNaturalStoreCommitNotifyData *committedData =
        new (std::nothrow) MultiVerNaturalStoreCommitNotifyData(
        static_cast<MultiVerNaturalStore *>(kvDB_), startId, commitId, currentVersion - 1);
    if (committedData != nullptr) {
        static_cast<MultiVerNaturalStore *>(kvDB_)->AddVersionConstraintToList(currentVersion - 1);
        static_cast<MultiVerNaturalStore *>(kvDB_)->CommitNotify(NATURAL_STORE_COMMIT_EVENT, committedData);
        committedData->DecObjRef(committedData);
        committedData = nullptr;
    } else {
        LOGE("Failed to do commit notify because of OOM.");
    }
}

int MultiVerStorageExecutor::GetParentCommitId(const CommitID &commitId, CommitID &parentId, Version &curVersion) const
{
    if (commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    IKvDBCommit *commit = commitStorage_->GetCommit(commitId, errCode);
    if (commit == nullptr) {
        LOGE("Get commit failed while getting the parent id:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    parentId = commit->GetLeftParentId();
    curVersion = commit->GetCommitVersion();
    commitStorage_->ReleaseCommit(commit);
    commit = nullptr;
    return E_OK;
}

int MultiVerStorageExecutor::AllocNewCommitId(CommitID &commitId) const
{
    // Only for allocate for temporary.
    commitId.resize(COMMIT_ID_LENGTH);
    RAND_bytes(commitId.data(), COMMIT_ID_LENGTH);
    return E_OK;
}

int MultiVerStorageExecutor::FillAndCommitLogEntry(const Version &versionInfo, CommitID &commitId,
    uint64_t timestamp) const
{
    if (kvDB_ == nullptr || commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    // Get the commit id.
    int errCode = E_OK;
    IKvDBCommit *commit = commitStorage_->AllocCommit(errCode);
    if (commit == nullptr) {
        LOGE("Failed to alloc the commit locally:%d", errCode);
        return errCode;
    }

    (void)(AllocNewCommitId(commitId));
    std::vector<uint8_t> vectTag;
    static_cast<MultiVerNaturalStore *>(kvDB_)->GetCurrentTag(vectTag);
    std::string strTag(vectTag.begin(), vectTag.end());

    // Get the commit struct.
    CommitID header = commitStorage_->GetHeader(errCode);
    if (errCode != E_OK) {
        goto END;
    }

    commit->SetLeftParentId(header);
    commit->SetCommitId(commitId);
    commit->SetCommitVersion(versionInfo);
    commit->SetLocalFlag(true);
    commit->SetTimestamp(timestamp);
    commit->SetDeviceInfo(strTag);

    // write the commit history.
    errCode = commitStorage_->AddCommit(*commit, true);
    if (errCode != E_OK) {
        LOGE("Add commit history failed:%d", errCode);
    }

END:
    if (commit != nullptr) {
        commitStorage_->ReleaseCommit(commit);
        commit = nullptr;
    }
    return errCode;
}

int MultiVerStorageExecutor::FillCommitByForeign(IKvDBCommit *commit,
    const MultiVerCommitNode &multiVerCommit, const Version &versionInfo, const CommitID &commitId, bool isMerge) const
{
    if (isMerge) {
        if (commitStorage_ == nullptr || kvDB_ == nullptr) {
            return -E_INVALID_DB;
        }

        int errCode = E_OK;
        CommitID header = commitStorage_->GetHeader(errCode);
        if (errCode != E_OK) {
            return errCode;
        }
        std::vector<uint8_t> vectTag;
        static_cast<MultiVerNaturalStore *>(kvDB_)->GetCurrentTag(vectTag);
        std::string strTag(vectTag.begin(), vectTag.end());

        commit->SetCommitId(commitId);
        commit->SetLeftParentId(header);
        commit->SetRightParentId(multiVerCommit.commitId);
        commit->SetLocalFlag(true);
        TimeStamp timestamp = static_cast<MultiVerNaturalStore *>(kvDB_)->GetCurrentTimeStamp();
        commit->SetTimestamp(timestamp);
        commit->SetDeviceInfo(strTag);
    } else {
        commit->SetCommitId(multiVerCommit.commitId);
        commit->SetLeftParentId(multiVerCommit.leftParent);
        commit->SetRightParentId(multiVerCommit.rightParent);
        commit->SetTimestamp(multiVerCommit.timestamp);
        commit->SetLocalFlag(false);
        commit->SetDeviceInfo(multiVerCommit.deviceInfo);
    }

    commit->SetCommitVersion(versionInfo);
    return E_OK;
}

int MultiVerStorageExecutor::FillAndCommitLogEntry(const Version &versionInfo,
    const MultiVerCommitNode &multiVerCommit, CommitID &commitId, bool isMerge, TimeStamp &timestamp) const
{
    if (commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    IKvDBCommit *commit = commitStorage_->AllocCommit(errCode);
    if (commit == nullptr) {
        return errCode;
    }

    if (isMerge) {
        (void)(AllocNewCommitId(commitId));
    }

    errCode = FillCommitByForeign(commit, multiVerCommit, versionInfo, commitId, isMerge);
    if (errCode != E_OK) {
        LOGE("Failed to fill the sync commit:%d", errCode);
        goto END;
    }

    timestamp = isMerge ? static_cast<MultiVerNaturalStore *>(kvDB_)->GetCurrentTimeStamp() : multiVerCommit.timestamp;
    commit->SetTimestamp(timestamp);

    // write the commit history.
    errCode = commitStorage_->AddCommit(*commit, isMerge);
    if (errCode != E_OK) {
        LOGE("Add commit history failed:%d", errCode);
    }
END:
    if (commit != nullptr) {
        commitStorage_->ReleaseCommit(commit);
        commit = nullptr;
    }

    return errCode;
}

int MultiVerStorageExecutor::CommitTransaction(const MultiVerCommitNode &multiVerCommit, bool isMerge)
{
    if ((transaction_ == nullptr) || (dataStorage_ == nullptr)) {
        LOGE("invalid transaction for commit");
        return -E_INVALID_DB;
    }

    Version commitVersion;
    CommitID commitId;
    UpdateVerTimeStamp multiVerTimeStamp = {0ull, false};
    bool isDataChanged = transaction_->IsDataChanged();

    int errCode = dataStorage_->CommitWritePhaseOne(transaction_, multiVerTimeStamp);
    if (errCode != E_OK) {
        LOGE("commit phase one failed:%d", errCode);
        goto END;
    }

    commitVersion = transaction_->GetVersion();
    errCode = FillAndCommitLogEntry(commitVersion, multiVerCommit, commitId, isMerge, multiVerTimeStamp.timestamp);
    if (errCode != E_OK) {
        LOGE("rollback commit phase one failed:%d", errCode);
        dataStorage_->RollbackWritePhaseOne(transaction_, commitVersion);
        goto END;
    }

    dataStorage_->CommitWritePhaseTwo(transaction_);
    static_cast<MultiVerNaturalStore *>(kvDB_)->SetMaxTimeStamp(multiVerTimeStamp.timestamp);
    static_cast<MultiVerNaturalStore *>(kvDB_)->SetMaxCommitVersion(commitVersion);
    LOGD("sync commit version:%llu", commitVersion);
END:
    dataStorage_->ReleaseTransaction(transaction_);
    transaction_ = nullptr;

    if (errCode == E_OK && isMerge && isDataChanged) {
        CommitNotifiedData(commitId);
    }

    return CheckCorruptedStatus(errCode);
}

void MultiVerStorageExecutor::ReleaseMultiVerKvEntries(std::vector<MultiVerKvEntry *> &entries)
{
    for (auto &item : entries) {
        if (item != nullptr) {
            delete item;
            item = nullptr;
        }
    }
    entries.clear();
    entries.shrink_to_fit();
}

Version MultiVerStorageExecutor::GetCurrentReadVersion() const
{
    return readVersion_;
}

int MultiVerStorageExecutor::GetAllCommitsInTree(std::list<MultiVerCommitNode> &commits) const
{
    if (commitStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    return commitStorage_->GetAllCommitsInTree(commits);
}

int MultiVerStorageExecutor::GetEntriesByVersion(Version version, std::list<MultiVerTrimedVersionData> &data) const
{
    if (dataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = E_OK;
    IKvDBMultiVerTransaction *transaction = nullptr;
    if (transaction_ == nullptr) {
        transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, version, errCode);
        if (transaction == nullptr) {
            LOGE("Failed to get the transaction:%d", errCode);
            goto END;
        }
    } else {
        transaction = transaction_;
    }

    // Note that the transaction fails and the parameters are empty.
    errCode = transaction->GetEntriesByVersion(version, data);
END:
    if (transaction != transaction_) {
        dataStorage_->ReleaseTransaction(transaction);
        transaction = nullptr;
    }
    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::GetOverwrittenClearTypeEntries(Version clearVersion,
    std::list<MultiVerTrimedVersionData> &data) const
{
    if (dataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = E_OK;
    IKvDBMultiVerTransaction *transaction = nullptr;
    if (transaction_ == nullptr) {
        transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, clearVersion, errCode);
        if (transaction == nullptr) {
            LOGE("Failed to get the transaction:%d", errCode);
            goto END;
        }
    } else {
        transaction = transaction_;
    }

    errCode = transaction->GetOverwrittenClearTypeEntries(clearVersion, data);
END:
    if (transaction != transaction_) {
        dataStorage_->ReleaseTransaction(transaction);
        transaction = nullptr;
    }

    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::GetOverwrittenNonClearTypeEntries(Version version, const Key &hashKey,
    std::list<MultiVerTrimedVersionData> &data) const
{
    if (dataStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = E_OK;
    IKvDBMultiVerTransaction *transaction = nullptr;
    if (transaction_ == nullptr) {
        transaction = dataStorage_->StartRead(KvDataType::KV_DATA_SYNC_P2P, version, errCode);
        if (transaction == nullptr) {
            LOGE("Failed to get the transaction:%d", errCode);
            goto END;
        }
    } else {
        transaction = transaction_;
    }

    errCode = transaction->GetOverwrittenNonClearTypeEntries(version, hashKey, data);
END:
    if (transaction != transaction_) {
        dataStorage_->ReleaseTransaction(transaction);
        transaction = nullptr;
    }

    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::DeleteEntriesByHashKey(Version version, const Key &hashKey)
{
    if (transaction_ == nullptr) {
        LOGI("You need start transaction before this operation!");
        return -E_NOT_PERMIT;
    }

    Value savedValue;
    int errCode = transaction_->GetValueForTrimSlice(hashKey, version, savedValue);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = transaction_->DeleteEntriesByHashKey(version, hashKey);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    MultiVerValueObject valueObject;
    errCode = valueObject.DeSerialData(savedValue);
    // savedValue empty is del or clear record
    if (!valueObject.IsHash() || savedValue.empty()) {
        return E_OK;
    }
    if (errCode != E_OK) {
        return errCode;
    }

    std::vector<ValueSliceHash> sliceHashVect;
    errCode = valueObject.GetValueHash(sliceHashVect);
    if (errCode != E_OK) {
        return errCode;
    }

    for (const auto &item : sliceHashVect) {
        errCode = DeleteValueSliceInner(sliceTransaction_, item);
        if (errCode != E_OK) {
            LOGI("Value slice delete fail!");
            break;
        }
    }

    return CheckCorruptedStatus(errCode);
}

int MultiVerStorageExecutor::UpdateTrimedFlag(Version version, const Key &hashKey)
{
    return E_OK;
}

int MultiVerStorageExecutor::UpdateTrimedFlag(const CommitID &commit)
{
    return E_OK;
}
} // namespace DistributedDB
#endif
