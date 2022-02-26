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

#ifndef VIRTUAL_MULTI_VER_SYNC_INTERFACE_H
#define VIRTUAL_MULTI_VER_SYNC_INTERFACE_H

#include "multi_ver_natural_store.h"

#include "multi_ver_natural_store_connection.h"
#include "distributeddb_tools_unit_test.h"

namespace DistributedDB {
class VirtualMultiVerSyncDBInterface final : public MultiVerKvDBSyncInterface {
public:
    VirtualMultiVerSyncDBInterface();
    ~VirtualMultiVerSyncDBInterface() override;

    int GetInterfaceType() const override;

    void IncRefCount() override;

    void DecRefCount() override;

    std::vector<uint8_t> GetIdentifier() const override;

    void GetMaxTimeStamp(TimeStamp &stamp) const override;

    int GetMetaData(const Key &key, Value &value) const override;

    int PutMetaData(const Key &key, const Value &value) override;

    int GetAllMetaKeys(std::vector<Key> &keys) const override;

    bool IsCommitExisted(const MultiVerCommitNode &) const override;

    int GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &) const override;

    int GetCommitTree(const std::map<std::string, MultiVerCommitNode> &,
        std::vector<MultiVerCommitNode> &) const override;

    int GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries) const override;

    MultiVerKvEntry *CreateKvEntry(const std::vector<uint8_t> &) override;

    void ReleaseKvEntry(const MultiVerKvEntry *entry) override;

    bool IsValueSliceExisted(const ValueSliceHash &value) const override;

    int GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const override;

    int PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue) const override;

    int PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
        const std::string &deviceName) override;

    int MergeSyncCommit(const MultiVerCommitNode &commit, const std::vector<MultiVerCommitNode> &commits) override;

    void NotifyStartSyncOperation() override {};

    void NotifyFinishSyncOperation() override {};

    int TransferSyncCommitDevInfo(MultiVerCommitNode &commit, const std::string &devId, bool isSyncedIn) const override;

    int Initialize(const std::string &deviceId);

    int GetData(const Key &key, Key &value);

    int PutData(const Key &key, const Key &value);

    int DeleteData(const Key &key);

    int StartTransaction();

    int Commit();

    int DeleteDatabase();

    const KvDBProperties &GetDbProperties() const override;

private:
    DistributedDBUnitTest::DistributedDBToolsUnitTest testTool_;
    MultiVerNaturalStore *kvStore_;
    MultiVerNaturalStoreConnection *connection_;
    KvDBProperties properties_;
};
}  // namespace DistributedDB

#endif // VIRTUAL_MULTI_VER_SYNC_INTERFACE