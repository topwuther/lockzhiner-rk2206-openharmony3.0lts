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

#ifndef DISTRIBUTEDDATAMGR2_SINGLE_KVSTORE_CLIENT_H
#define DISTRIBUTEDDATAMGR2_SINGLE_KVSTORE_CLIENT_H

#include "data_query.h"
#include "ikvstore_single.h"
#include "single_kvstore.h"

namespace OHOS::DistributedKv {
class SingleKvStoreClient : public SingleKvStore {
public:
    explicit SingleKvStoreClient(sptr<ISingleKvStore> kvStoreProxy, const std::string &storeId);

    ~SingleKvStoreClient()
    {}

    StoreId GetStoreId() const override;

    Status GetEntries(const Key &prefixKey, std::vector<Entry> &entries) const override;

    Status GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries) const override;

    Status GetEntriesWithQuery(const DataQuery &query, std::vector<Entry> &entries) const override;

    void GetResultSet(const Key &prefixKey,
                      std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const override;

    void GetResultSetWithQuery(const std::string &query,
                               std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const override;

    void GetResultSetWithQuery(const DataQuery &query,
                               std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const override;

    Status CloseResultSet(std::unique_ptr<KvStoreResultSet> resultSet) override;

    Status GetCountWithQuery(const std::string &query, int &result) const override;

    Status GetCountWithQuery(const DataQuery &query, int &result) const override;

    Status Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode, uint32_t allowedDelayMs) override;

    Status RemoveDeviceData(const std::string &device) override;

    Status Delete(const Key &key) override;

    Status Put(const Key &key, const Value &value) override;

    Status Get(const Key &key, Value &value) override;

    Status SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) override;

    Status UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) override;

    Status RegisterSyncCallback(std::shared_ptr<KvStoreSyncCallback> callback) override;

    Status UnRegisterSyncCallback() override;

    Status PutBatch(const std::vector<Entry> &entries) override;

    Status DeleteBatch(const std::vector<Key> &keys) override;

    Status StartTransaction() override;

    Status Commit() override;

    Status Rollback() override;

    Status SetSyncParam(const KvSyncParam &syncParam) override;

    Status GetSyncParam(KvSyncParam &syncParam) override;
    Status SetCapabilityEnabled(bool enabled) const override;
    Status SetCapabilityRange(const std::vector<std::string> &localLabels,
                              const std::vector<std::string> &remoteSupportLabels) const override;

    Status GetSecurityLevel(SecurityLevel &securityLevel) const override;
protected:
    Status Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &outputParam) override;

private:
    sptr<ISingleKvStore> kvStoreProxy_;
    std::map<KvStoreObserver *, sptr<IKvStoreObserver>> registeredObservers_;
    std::mutex observerMapMutex_;
    std::string storeId_;
};
} // namespace OHOS::DistributedKv
#endif // DISTRIBUTEDDATAMGR2_SINGLE_KVSTORE_CLIENT_H
