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

#define LOG_TAG "KvStoreSnapshotImpl"

#include "kvstore_snapshot_impl.h"
#include "constant.h"
#include "log_print.h"
#include "dds_trace.h"

namespace OHOS {
namespace DistributedKv {
KvStoreSnapshotImpl::KvStoreSnapshotImpl(DistributedDB::KvStoreSnapshotDelegate *kvStoreSnapshotDelegate,
    KvStoreObserverImpl *kvStoreObserverImpl)
    : kvStoreSnapshotDelegate_(kvStoreSnapshotDelegate), kvStoreObserverImpl_(kvStoreObserverImpl)
{
    ZLOGI("construct");
}

KvStoreSnapshotImpl::~KvStoreSnapshotImpl()
{
    ZLOGI("destruct");
    if (kvStoreObserverImpl_ != nullptr) {
        delete kvStoreObserverImpl_;
    }
}

Status KvStoreSnapshotImpl::Get(const Key &key, Value &value)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());

    if (keyData.empty() || keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    if (kvStoreSnapshotDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }

    DistributedDB::Value retValue;
    DistributedDB::DBStatus retValueStatus;

    auto valueCallbackFunction = [&](DistributedDB::DBStatus status, DistributedDB::Value valueTmp) {
        retValueStatus = status;
        retValue = valueTmp;
    };

    DistributedDB::Key dbKey = keyData;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        kvStoreSnapshotDelegate_->Get(dbKey, valueCallbackFunction);
    }
    if (retValueStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error: %d.", static_cast<int>(retValueStatus));
        if (retValueStatus == DistributedDB::DBStatus::NOT_FOUND) {
            return Status::KEY_NOT_FOUND;
        }
        return Status::DB_ERROR;
    }

    Value valueOut(retValue);
    value = valueOut;
    return Status::SUCCESS;
}

void KvStoreSnapshotImpl::GetEntries(const Key &prefixKey, const Key &nextKey,
                                     std::function<void(Status, std::vector<Entry> &, const Key &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::vector<Entry> retEntries;
    Key trimmedPrefix = Key(Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data()));
    Key trimmedNext = Key(Constant::TrimCopy<std::vector<uint8_t>>(nextKey.Data()));
    // handling parameter errors
    if (trimmedPrefix.Size() > Constant::MAX_KEY_LENGTH || trimmedNext.Size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        callback(Status::INVALID_ARGUMENT, retEntries, nextKey);
        return;
    }

    std::shared_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    if (kvStoreSnapshotDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        callback(Status::DB_ERROR, retEntries, nextKey);
        return;
    }
    // search the buffer to find if this search has already been buffered.
    std::lock_guard<std::mutex> entryLock(entriesMutex_);
    auto restPair = batchEntries_.begin();
    for (; restPair != batchEntries_.end(); restPair++) {
        // firstly compare prefixKey
        if (restPair->first == trimmedPrefix.ToString()) {
            // secondly compare nextKey
            if (restPair->second.front().key.ToString() == trimmedNext.ToString()) {
                break;
            }
        }
    }

    if (restPair != batchEntries_.end()) {
        // buffer of this search has been found. read and remove returned entries from buffer.
        auto &restList = restPair->second;
        size_t retSize = 0;
        // compute if add next entry to retEntries will let retEntries size exceeds IPC limit.
        while (restList.size() > 0 && restList.front().value.Size() + retSize < SOFT_LIMIT) {
            retSize += restList.front().key.Size() + IPC_WRITE_AMPLIFICATION +
                    restList.front().value.Size() + IPC_WRITE_AMPLIFICATION;
            retEntries.push_back(restList.front());
            restList.pop_front();
        }
        if (restList.size() > 0 && retEntries.size() > 0) {
            callback(Status::SUCCESS, retEntries, restList.front().key);
            return;
        }
        batchEntries_.erase(restPair);
        if (restList.size() == 0) {
            callback(Status::SUCCESS, retEntries, Key(""));
        } else {
            callback(Status::ILLEGAL_STATE, retEntries, Key(""));
        }
        return;
    }
    // holding entriesMutex until GetEntriesFromDelegate() finish. GetEntriesFromDelegate will change batchEntries_ too.
    GetEntriesFromDelegateLocked(trimmedPrefix, trimmedNext, callback);
}

void KvStoreSnapshotImpl::GetEntriesFromDelegateLocked(const Key &prefixKey, const Key &nextKey,
    std::function<void(Status, std::vector<Entry> &, const Key &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::vector<Entry> retEntries;
    DistributedDB::DBStatus retValueStatus;
    size_t retSize = 0;
    Key nextStart;
    // the returned entries can be separated into three part:
    // part1: the already returned part. this part should be discarded.
    // part2: entries to be returned this time. size of this part depends on ipc limit.
    // part3: entries that cannot be returned due to ipc limit. part3 should be buffered to batchEntries_.
    auto valueCallbackFunction = [&](DistributedDB::DBStatus status, const std::vector<DistributedDB::Entry> &entries) {
        ZLOGD("delegate return entry size: %zu", entries.size());
        retValueStatus = status;
        auto entry = entries.begin();
        // deal with part1: skip already returned entries.
        if (nextKey.Size() != 0) {
            while (!(Key(entry->key) == nextKey)) {
                entry++;
            }
            if (entry == entries.end()) {
                ZLOGE("search reach end before find nextkey");
                return;
            }
        }
        // deal with part2: put entries to retEntries until put next entry will cause retEntries exceeds its size limit.
        for (; entry != entries.end() && entry->value.size() + retSize < SOFT_LIMIT; entry++) {
            Entry entryTmp;
            entryTmp.key = Key(entry->key);
            entryTmp.value = Value(entry->value);
            retEntries.push_back(entryTmp);
            retSize += entryTmp.value.Size() + IPC_WRITE_AMPLIFICATION + entryTmp.key.Size() + IPC_WRITE_AMPLIFICATION;
        }
        // all returned entries has been put to retEntries so there will not be a part 3.
        if (entry == entries.end() || retEntries.size() == 0) {
            nextStart = "";
            return;
        }
        // deal with part3:
        if (batchEntries_.size() >= BUFFER_SIZE) {
            // buffer is full. firstly remove the oldest buffer(the last element in batchEntries_).
            batchEntries_.pop_back();
        }
        nextStart = entry->key;
        // secondly move the rest entries to buffer
        std::list<Entry> buffer;
        for (; entry != entries.end(); entry++) {
            Entry entryTmp;
            entryTmp.key = Key(entry->key);
            entryTmp.value = Value(entry->value);
            buffer.push_back(entryTmp);
        }
        // thirdly put buffer to the front of batchEntries_, use prefixKey as its key
        batchEntries_.push_front(std::make_pair(prefixKey.ToString(), std::move(buffer)));
    };  // end of valueCallbackFunction

    std::shared_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    DistributedDB::Key dbKey = prefixKey.Data();
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        kvStoreSnapshotDelegate_->GetEntries(dbKey, valueCallbackFunction);
    }
    if (retValueStatus == DistributedDB::DBStatus::NOT_FOUND) {
        callback(Status::KEY_NOT_FOUND, retEntries, nextStart);
        return;
    }
    if (retValueStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error: %d.", static_cast<int>(retValueStatus));
        callback(Status::DB_ERROR, retEntries, nextStart);
        return;
    }
    ZLOGD("retEntries size: %zu : %zu.", retEntries.size(), retSize);
    callback(Status::SUCCESS, retEntries, nextStart);
}

void KvStoreSnapshotImpl::GetKeys(const Key &prefixKey, const Key &nextKey,
                                  std::function<void(Status, std::vector<Key> &, const Key &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("begin.");
    std::vector<Key> retKeys;
    Key trimmedPrefix = Key(Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data()));
    Key trimmedNext = Key(Constant::TrimCopy<std::vector<uint8_t>>(nextKey.Data()));
    if (trimmedPrefix.Size() > Constant::MAX_KEY_LENGTH || trimmedNext.Size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        callback(Status::INVALID_ARGUMENT, retKeys, nextKey);
        return;
    }

    std::shared_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    if (kvStoreSnapshotDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        callback(Status::DB_ERROR, retKeys, nextKey);
        return;
    }

    std::lock_guard<std::mutex> keyLock(keysMutex_);
    // search the buffer to find if this search has been buffered
    auto restPair = batchKeys_.begin();
    for (; restPair != batchKeys_.end(); restPair++) {
        if (restPair->first == trimmedPrefix.ToString()) {
            if (restPair->second.front() == trimmedNext) {
                break;
            }
        }
    }

    // buffer of this search has been found
    if (restPair != batchKeys_.end()) {
        auto restList = restPair->second;
        size_t retSize = 0;
        while (restList.size() > 0 && retSize < SOFT_LIMIT) {
            retSize += restList.front().Size() + IPC_WRITE_AMPLIFICATION;
            retKeys.push_back(restList.front());
            restList.pop_front();
        }
        if (restList.size() > 0) {
            callback(Status::SUCCESS, retKeys, restList.front());
        } else {
            callback(Status::SUCCESS, retKeys, Key(""));
            batchKeys_.erase(restPair);
        }
        return;
    }

    // holding keysMutex until GetKeysFromDelegate() finish. GetKeysFromDelegate will change batchkeys_ too.
    GetKeysFromDelegateLocked(trimmedPrefix, trimmedNext, callback);
}

void KvStoreSnapshotImpl::GetKeysFromDelegateLocked(const Key &prefixKey, const Key &nextKey,
    std::function<void(Status, std::vector<Key> &, const Key &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    // just the same as GetEntriesFromDelegate
    std::vector<Key> retKeys;
    DistributedDB::DBStatus retValueStatus;
    size_t retSize = 0;
    Key nextStart;
    auto valueCallbackFunction = [&](DistributedDB::DBStatus status, const std::vector<DistributedDB::Entry> &entries) {
        ZLOGD("delegate return entry size: %zu", entries.size());
        retValueStatus = status;
        auto entry = entries.begin();
        if (nextKey.Size() != 0) {
            while (!(Key(entry->key) == nextKey)) {
                entry++;
            }
        }
        for (; entry != entries.end() && entry->key.size() + retSize < SOFT_LIMIT; entry++) {
            retKeys.push_back(Key(entry->key));
            retSize += entry->key.size() + IPC_WRITE_AMPLIFICATION;
        }

        if (entry == entries.end()) {
            nextStart = "";
            return;
        }
        if (batchKeys_.size() >= BUFFER_SIZE) {
            batchKeys_.pop_back();
        }
        std::list<Key> buffer;
        nextStart = Key(entry->key);
        for (; entry != entries.end(); entry++) {
            buffer.push_back(Key(entry->key));
        }
        batchKeys_.push_front(std::make_pair(prefixKey.ToString(), std::move(buffer)));
    };

    std::shared_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    DistributedDB::Key dbKey = prefixKey.Data();
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        kvStoreSnapshotDelegate_->GetEntries(dbKey, valueCallbackFunction);
    }
    if (retValueStatus == DistributedDB::DBStatus::NOT_FOUND) {
        callback(Status::KEY_NOT_FOUND, retKeys, nextStart);
        return;
    }
    if (retValueStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error: %d.", static_cast<int>(retValueStatus));
        callback(Status::DB_ERROR, retKeys, nextStart);
        return;
    }
    ZLOGD("retKeys size: %zu : %zu.", retKeys.size(), retSize);
    callback(Status::SUCCESS, retKeys, nextStart);
}

Status KvStoreSnapshotImpl::Release(DistributedDB::KvStoreDelegate *kvStoreDelegate)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    ZLOGI("Releasing KvStoreSnapshot.");
    if (kvStoreDelegate == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    DistributedDB::DBStatus status = kvStoreDelegate->ReleaseKvStoreSnapshot(kvStoreSnapshotDelegate_);
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("Error occurs during Releasing KvStoreSnapshot, error code %d.", status);
        return Status::DB_ERROR;
    }
    kvStoreSnapshotDelegate_ = nullptr;
    return Status::SUCCESS;
}

Status KvStoreSnapshotImpl::MigrateKvStore(DistributedDB::KvStoreDelegate *kvStoreDelegate)
{
    if (kvStoreDelegate == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    ZLOGI("begin.");
    DistributedDB::KvStoreSnapshotDelegate *snapshotDelegate = nullptr;
    DistributedDB::DBStatus dbStatus;
    auto snapshotCallbackFunction = [&](DistributedDB::DBStatus status,
                                        DistributedDB::KvStoreSnapshotDelegate *snapshot) {
        dbStatus = status;
        snapshotDelegate = snapshot;
    };
    std::unique_lock<std::shared_mutex> lock(snapshotDelegateMutex_);
    kvStoreDelegate->GetKvStoreSnapshot(kvStoreObserverImpl_, snapshotCallbackFunction);
    if (dbStatus != DistributedDB::DBStatus::OK || snapshotDelegate == nullptr) {
        ZLOGE("delegate return nullptr or errcode, dbStatus:%d.", static_cast<int>(dbStatus));
        return Status::DB_ERROR;
    }

    kvStoreSnapshotDelegate_ = snapshotDelegate;
    return Status::SUCCESS;
}
}  // namespace DistributedKv
}  // namespace OHOS
