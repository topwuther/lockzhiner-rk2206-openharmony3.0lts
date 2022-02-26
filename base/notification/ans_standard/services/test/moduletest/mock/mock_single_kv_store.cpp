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

#include "mock_single_kv_store.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

Status AnsTestSingleKvStore::GetEntries(const Key &prefixKey, std::vector<Entry> &entries) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::GetEntriesWithQuery(const DataQuery &query, std::vector<Entry> &entries) const
{
    return Status::SUCCESS;
}

void AnsTestSingleKvStore::GetResultSet(
    const Key &prefixKey, std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const
{}

void AnsTestSingleKvStore::GetResultSetWithQuery(
    const std::string &query, std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const
{}

void AnsTestSingleKvStore::GetResultSetWithQuery(
    const DataQuery &query, std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const
{}

Status AnsTestSingleKvStore::CloseResultSet(std::unique_ptr<KvStoreResultSet> resultSet)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::GetCountWithQuery(const std::string &query, int &result) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::GetCountWithQuery(const DataQuery &query, int &result) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::Sync(
    const std::vector<std::string> &deviceIdList, const SyncMode &mode, uint32_t allowedDelayMs)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::RemoveDeviceData(const std::string &device)
{
    return Status::SUCCESS;
}

StoreId AnsTestSingleKvStore::GetStoreId() const
{
    StoreId storeId;
    storeId.storeId = "";
    return storeId;
}

Status AnsTestSingleKvStore::Delete(const Key &key)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::Put(const Key &key, const Value &value)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::Get(const Key &key, Value &value)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::RegisterSyncCallback(std::shared_ptr<KvStoreSyncCallback> callback)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::UnRegisterSyncCallback()
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::PutBatch(const std::vector<Entry> &entries)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::DeleteBatch(const std::vector<Key> &keys)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::StartTransaction()
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::Commit()
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::Rollback()
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::SetSyncParam(const KvSyncParam &syncParam)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::GetSyncParam(KvSyncParam &syncParam)
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::SetCapabilityEnabled(bool enabled) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::SetCapabilityRange(
    const std::vector<std::string> &localLabels, const std::vector<std::string> &remoteSupportLabels) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::GetSecurityLevel(SecurityLevel &securityLevel) const
{
    return Status::SUCCESS;
}

Status AnsTestSingleKvStore::Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output)
{
    return Status::SUCCESS;
}

}  // namespace DistributedKv
}  // namespace OHOS