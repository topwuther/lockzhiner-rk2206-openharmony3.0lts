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

#include "virtual_multi_ver_sync_db_interface.h"

#include "db_common.h"
#include "kvdb_manager.h"
#include "multi_ver_natural_store_snapshot.h"

namespace DistributedDB {
VirtualMultiVerSyncDBInterface::VirtualMultiVerSyncDBInterface() : kvStore_(nullptr), connection_(nullptr)
{
}

VirtualMultiVerSyncDBInterface::~VirtualMultiVerSyncDBInterface()
{
    DeleteDatabase();
}

int VirtualMultiVerSyncDBInterface::GetInterfaceType() const
{
    return IKvDBSyncInterface::SYNC_MVD;
}

void VirtualMultiVerSyncDBInterface::IncRefCount()
{
    kvStore_->IncRefCount();
}

void VirtualMultiVerSyncDBInterface::DecRefCount()
{
    kvStore_->DecRefCount();
}

std::vector<uint8_t> VirtualMultiVerSyncDBInterface::GetIdentifier() const
{
    return kvStore_->GetIdentifier();
}

void VirtualMultiVerSyncDBInterface::GetMaxTimeStamp(TimeStamp &stamp) const
{
    return kvStore_->GetMaxTimeStamp(stamp);
}

int VirtualMultiVerSyncDBInterface::GetMetaData(const Key &key, Value &value) const
{
    return kvStore_->GetMetaData(key, value);
}

int VirtualMultiVerSyncDBInterface::PutMetaData(const Key &key, const Value &value)
{
    return kvStore_->PutMetaData(key, value);
}

int VirtualMultiVerSyncDBInterface::GetAllMetaKeys(std::vector<Key> &keys) const
{
    return kvStore_->GetAllMetaKeys(keys);
}

bool VirtualMultiVerSyncDBInterface::IsCommitExisted(const MultiVerCommitNode &commit) const
{
    return kvStore_->IsCommitExisted(commit);
}

int VirtualMultiVerSyncDBInterface::GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &commits) const
{
    return kvStore_->GetDeviceLatestCommit(commits);
}

int VirtualMultiVerSyncDBInterface::GetCommitTree(const std::map<std::string, MultiVerCommitNode> &inCommit,
    std::vector<MultiVerCommitNode> &outCommit) const
{
    return kvStore_->GetCommitTree(inCommit, outCommit);
}

int VirtualMultiVerSyncDBInterface::GetCommitData(const MultiVerCommitNode &commit,
    std::vector<MultiVerKvEntry *> &entries) const
{
    return kvStore_->GetCommitData(commit, entries);
}

MultiVerKvEntry *VirtualMultiVerSyncDBInterface::CreateKvEntry(const std::vector<uint8_t> &entries)
{
    return kvStore_->CreateKvEntry(entries);
}

void VirtualMultiVerSyncDBInterface::ReleaseKvEntry(const MultiVerKvEntry *entry)
{
    return kvStore_->ReleaseKvEntry(entry);
}

bool VirtualMultiVerSyncDBInterface::IsValueSliceExisted(const ValueSliceHash &value) const
{
    return kvStore_->IsValueSliceExisted(value);
}

int VirtualMultiVerSyncDBInterface::GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const
{
    return kvStore_->GetValueSlice(hashValue, sliceValue);
}

int VirtualMultiVerSyncDBInterface::PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue) const
{
    return kvStore_->PutValueSlice(hashValue, sliceValue);
}

int VirtualMultiVerSyncDBInterface::PutCommitData(const MultiVerCommitNode &commit,
    const std::vector<MultiVerKvEntry *> &entries, const std::string &deviceName)
{
    return kvStore_->PutCommitData(commit, entries, deviceName);
}

int VirtualMultiVerSyncDBInterface::MergeSyncCommit(const MultiVerCommitNode &commit,
    const std::vector<MultiVerCommitNode> &commits)
{
    return kvStore_->MergeSyncCommit(commit, commits);
}

int VirtualMultiVerSyncDBInterface::TransferSyncCommitDevInfo(MultiVerCommitNode &commit, const std::string &devId,
    bool isSyncedIn) const
{
    return kvStore_->TransferSyncCommitDevInfo(commit, devId, isSyncedIn);
}

int VirtualMultiVerSyncDBInterface::Initialize(const std::string &deviceId)
{
    std::string dir;
    testTool_.TestDirInit(dir);
    KvDBProperties prop;
    prop.SetStringProp(KvDBProperties::USER_ID, "sync_test");
    prop.SetStringProp(KvDBProperties::APP_ID, "sync_test");
    prop.SetStringProp(KvDBProperties::STORE_ID, deviceId);
    std::string identifier = DBCommon::TransferHashString("sync_test-sync_test-" + deviceId);

    prop.SetStringProp(KvDBProperties::IDENTIFIER_DATA, identifier);
    std::string identifierDir = DBCommon::TransferStringToHex(identifier);
    prop.SetStringProp(KvDBProperties::IDENTIFIER_DIR, identifierDir);
    prop.SetStringProp(KvDBProperties::DATA_DIR, dir + "/commitstore");
    prop.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    prop.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);

    int errCode = E_OK;
    IKvDB *kvDB = KvDBManager::OpenDatabase(prop, errCode);
    if (errCode != E_OK) {
        LOGE("[VirtualMultiVerSyncDBInterface] db create failed path, err %d", errCode);
        return errCode;
    }
    kvStore_ = static_cast<MultiVerNaturalStore *>(kvDB);
    IKvDBConnection *conn = kvDB->GetDBConnection(errCode);
    if (errCode != E_OK) {
        LOGE("[VirtualMultiVerSyncDBInterface] connection get failed path, err %d", errCode);
        RefObject::DecObjRef(kvStore_);
        kvStore_ = nullptr;
        return errCode;
    }
    RefObject::DecObjRef(kvStore_);
    connection_ = static_cast<MultiVerNaturalStoreConnection *>(conn);
    return E_OK;
}

int VirtualMultiVerSyncDBInterface::GetData(const Key &key, Key &value)
{
    IKvDBSnapshot *snapshot = nullptr;
    int errCode = connection_->GetSnapshot(snapshot);
    if (errCode != E_OK) {
        LOGE("[VirtualMultiVerSyncDBInterface] GetSnapshot failed err %d", errCode);
        return errCode;
    }
    errCode = snapshot->Get(key, value);
    connection_->ReleaseSnapshot(snapshot);
    return errCode;
}

int VirtualMultiVerSyncDBInterface::PutData(const Key &key, const Key &value)
{
    IOption option;
    return connection_->Put(option, key, value);
}

int VirtualMultiVerSyncDBInterface::DeleteData(const Key &key)
{
    IOption option;
    return connection_->Delete(option, key);
}

int VirtualMultiVerSyncDBInterface::StartTransaction()
{
    return connection_->StartTransaction();
}

int VirtualMultiVerSyncDBInterface::Commit()
{
    return connection_->Commit();
}

int VirtualMultiVerSyncDBInterface::DeleteDatabase()
{
    if (connection_ != nullptr) {
        KvDBProperties prop = kvStore_->GetMyProperties();
        int errCode = connection_->Close();
        if (errCode != E_OK) {
            return errCode;
        }
        connection_ = nullptr;
        kvStore_ = nullptr;
        return KvDBManager::RemoveDatabase(prop);
    }
    return -E_NOT_FOUND;
}

const KvDBProperties &VirtualMultiVerSyncDBInterface::GetDbProperties() const
{
    return properties_;
}
}  // namespace DistributedDB

