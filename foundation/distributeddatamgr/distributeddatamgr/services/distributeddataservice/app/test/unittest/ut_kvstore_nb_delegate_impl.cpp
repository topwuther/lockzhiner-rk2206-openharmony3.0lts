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

#include "ut_kvstore_nb_delegate_impl.h"
#include <algorithm>
#include <functional>
#include <string>

namespace DistributedDB {
UtKvStoreNbDelegateImpl::UtKvStoreNbDelegateImpl(const std::string &storeId, const std::string &deviceId)
    : storeId_(storeId), deviceId_(deviceId), syncStatus_(OK)
{}

UtKvStoreNbDelegateImpl::~UtKvStoreNbDelegateImpl()
{}

bool UtKvStoreNbDelegateImpl::IsValidKey(const Key &key)
{
    if (key.size() > MAX_KEY_SIZE) {
        return false;
    }

    if (key.empty()) {
        return false;
    }
    return true;
}

bool UtKvStoreNbDelegateImpl::IsValidValue(const Value &value)
{
    if (value.size() > MAX_VALUE_SIZE) {
        return false;
    }
    return true;
}

DBStatus UtKvStoreNbDelegateImpl::Get(const Key &key, Value &value) const
{
    if (!IsValidKey(key)) {
        return INVALID_ARGS;
    }
    const auto it = db_.find(key);
    if (it != db_.end()) {
        if (!it->second.isDeleted) {
            value = it->second.value;
            return OK;
        }
    }
    return NOT_FOUND;
}

DBStatus UtKvStoreNbDelegateImpl::GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    if (keyPrefix.size() > MAX_KEY_SIZE) {
        return INVALID_ARGS;
    }

    size_t sizeOld = entries.size();
    for (const auto &data : db_) {
        if (std::equal(keyPrefix.begin(), keyPrefix.end(), data.first.begin())) {
            if (!data.second.isDeleted) {
                Entry entry{ data.first, data.second.value };
                entries.push_back(entry);
            }
        }
    }
    return (entries.size() > sizeOld) ? OK : NOT_FOUND;
}

DBStatus UtKvStoreNbDelegateImpl::GetEntries(const Key &keyPrefix, KvStoreResultSet *&resultSet) const
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::CloseResultSet(KvStoreResultSet *&resultSet)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Put(const Key &key, const Value &value)
{
    DbValue dbValue{ value, true, false, false };
    return DoPut(key, dbValue);
}

DBStatus UtKvStoreNbDelegateImpl::DoPut(const Key &key, const DbValue &dbValue)
{
    if (!IsValidKey(key) || !IsValidValue(dbValue.value)) {
        return INVALID_ARGS;
    }
    db_.insert_or_assign(key, dbValue);
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::PutBatch(const std::vector<Entry> &entries)
{
    return NOT_SUPPORT;
}

DBStatus UtKvStoreNbDelegateImpl::PutByOtherDevice(const Key &key, const Value &value)
{
    DbValue dbValue{ value, false, false, true };
    auto ret = DoPut(key, dbValue);
    if (ret != OK) {
        return ret;
    }

    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Delete(const Key &key)
{
    return DoDelete(key, true);
}

DBStatus UtKvStoreNbDelegateImpl::DoDelete(const Key &key, bool isLocalDelete)
{
    if (!IsValidKey(key)) {
        return INVALID_ARGS;
    }
    auto it = db_.find(key);
    if (it != db_.end()) {
        it->second.isLocalPut = isLocalDelete;
        it->second.isDeleted = true;
        it->second.isSynced = !isLocalDelete;
        return OK;
    }

    return NOT_FOUND;
}

DBStatus UtKvStoreNbDelegateImpl::DeleteBatch(const std::vector<Key> &keys)
{
    return NOT_SUPPORT;
}

DBStatus UtKvStoreNbDelegateImpl::DeleteByOtherDevice(const Key &key)
{
    auto ret = DoDelete(key, false);
    if (ret != OK) {
        return ret;
    }

    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::GetLocal(const Key &key, Value &value) const
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::PutLocal(const Key &key, const Value &value)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::DeleteLocal(const Key &key)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::RegisterObserver(const Key &key, unsigned int mode, KvStoreObserver *observer)
{
    if (key.size() > MAX_KEY_SIZE) {
        return INVALID_ARGS;
    }

    if (mode > OBSERVER_CHANGES_LOCAL_ONLY || mode < OBSERVER_CHANGES_NATIVE) {
        return INVALID_ARGS;
    }

    if (observer == nullptr) {
        return INVALID_ARGS;
    }

    std::vector<const KvStoreObserver *>::iterator it = find(observerMap_.begin(), observerMap_.end(), observer);
    if (it != observerMap_.end()) {
        return DB_ERROR;
    }

    observerMap_.push_back(observer);

    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::UnRegisterObserver(const KvStoreObserver *observer)
{
    if (observer == nullptr) {
        return INVALID_ARGS;
    }

    std::vector<const KvStoreObserver *>::iterator it = find(observerMap_.begin(), observerMap_.end(), observer);
    if (it == observerMap_.end()) {
        return NOT_FOUND;
    }

    observerMap_.erase(it);
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::StartTransaction()
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Commit()
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Rollback()
{
    return OK;
}

std::string UtKvStoreNbDelegateImpl::GetStoreId() const
{
    return storeId_;
}

void UtKvStoreNbDelegateImpl::SetSyncStatus(DBStatus status)
{
    syncStatus_ = status;
}

void UtKvStoreNbDelegateImpl::SetNeighbor(const std::shared_ptr<UtKvStoreNbDelegateImpl> &neighbor)
{
    neighbor_ = neighbor;
}

DBStatus UtKvStoreNbDelegateImpl::Sync(
    const std::vector<std::string> &devices, SyncMode,
    const std::function<void(const std::map<std::string, DBStatus> &devicesMap)> &onComplete, bool)
{
    if (syncStatus_ != OK) {
        return syncStatus_;
    }
    auto neighbor = neighbor_.lock();
    if (neighbor != nullptr) {
        for (auto &it : db_) {
            if (it.second.isLocalPut && (!it.second.isSynced)) {
                if (!it.second.isDeleted) {
                    neighbor->PutByOtherDevice(it.first, it.second.value);
                } else {
                    neighbor->DeleteByOtherDevice(it.first);
                }
                it.second.isSynced = true;
            }
        }
    }

    if (onComplete) {
        std::map<std::string, DBStatus> deviceResults;
        for (const auto &dev : devices) {
            deviceResults.emplace(dev, syncStatus_);
        }
        onComplete(deviceResults);
    }

    return syncStatus_;
}

DBStatus UtKvStoreNbDelegateImpl::Pragma(PragmaCmd cmd, PragmaData &paramData)
{
    if (cmd == AUTO_SYNC) {
        if (paramData == nullptr) {
            return INVALID_ARGS;
        }
        bool isSync = *(static_cast<bool *>(paramData));
        if (isSync) {
            std::vector<std::string> devices;
            return Sync(devices, SYNC_MODE_PUSH_ONLY, nullptr, false);
        }
    }
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::SetConflictNotifier(int conflictType, const KvStoreNbConflictNotifier &notifier)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Rekey(const CipherPassword &password)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Export(const std::string &filePath, const CipherPassword &passwd)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::Import(const std::string &filePath, const CipherPassword &passwd)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::RemoveDeviceData(const std::string &device)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::GetEntries(const Query &query, std::vector<Entry> &entries) const
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::GetEntries(const Query &query, KvStoreResultSet *&resultSet) const
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::GetCount(const Query &query, int &count) const
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::GetLocalEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::PublishLocal(const Key &key, bool deleteLocal, bool updateTimestamp,
    const KvStoreNbPublishOnConflict &onConflict)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::UnpublishToLocal(const Key &key, bool deletePublic, bool updateTimestamp)
{
    return OK;
}
DBStatus UtKvStoreNbDelegateImpl::PutLocalBatch(const std::vector<Entry> &entries)
{
    return OK;
}

DBStatus UtKvStoreNbDelegateImpl::DeleteLocalBatch(const std::vector<Key> &keys)
{
    return OK;
}
}  // namespace DistributedDB
