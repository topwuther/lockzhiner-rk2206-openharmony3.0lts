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
#include "multi_ver_natural_store.h"

#include <cstdio>
#include <openssl/rand.h>

#include "securec.h"

#include "db_constant.h"
#include "ikvdb_factory.h"
#include "db_common.h"
#include "endian_convert.h"
#include "log_print.h"
#include "db_errno.h"
#include "multi_ver_storage_engine.h"
#include "multi_ver_natural_store_connection.h"
#include "generic_multi_ver_kv_entry.h"
#include "sqlite_multi_ver_data_storage.h"
#include "multi_ver_natural_store_commit_storage.h"
#include "multi_ver_vacuum_executor_impl.h"
#include "kvdb_utils.h"
#include "sqlite_utils.h"
#include "platform_specific.h"
#include "package_file.h"
#include "multi_ver_database_oper.h"

namespace DistributedDB {
namespace {
    // file block doesn't support the atomic of the upgrade temporarily.
    struct VersionFileBlock {
        static const uint64_t MAGIC_NUMBER = 0x37F8C35AULL;
        uint64_t magic = MAGIC_NUMBER; // magic number.
        uint32_t fileVersion = VERSION_FILE_VERSION_CURRENT; // file format version.
        uint32_t version = 0U; // version of the database.
        uint8_t tag[MULTI_VER_TAG_SIZE] = {0}; // tag of the multi ver branch.
        uint8_t reserved[72] = {0}; // reserved data.
        uint8_t checkSum[32] = {0}; // check sum
    };

    void TransferHostFileBlockToNet(VersionFileBlock &block)
    {
        block.magic = HostToNet(block.magic);
        block.fileVersion = HostToNet(block.fileVersion);
        block.version = HostToNet(block.version);
    }

    void TransferNetFileBlockToHost(VersionFileBlock &block)
    {
        block.magic = NetToHost(block.magic);
        block.fileVersion = NetToHost(block.fileVersion);
        block.version = NetToHost(block.version);
    }

    int CalcFileBlockCheckSum(VersionFileBlock &block)
    {
        std::vector<uint8_t> vect(reinterpret_cast<uint8_t *>(&block),
            reinterpret_cast<uint8_t *>(&block) + sizeof(block) - sizeof(block.checkSum));
        std::vector<uint8_t> hashVect;
        int errCode = DBCommon::CalcValueHash(vect, hashVect);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = memcpy_s(block.checkSum, sizeof(block.checkSum), hashVect.data(), hashVect.size());
        if (errCode != EOK) {
            return -E_SECUREC_ERROR;
        }
        return E_OK;
    }

    int CheckFileBlock(VersionFileBlock &block)
    {
        uint64_t readMagic = NetToHost(block.magic);
        if (readMagic != block.MAGIC_NUMBER) {
            LOGE("Invalid file head");
            return -E_UNEXPECTED_DATA;
        }

        std::vector<uint8_t> vect(reinterpret_cast<uint8_t *>(&block),
            reinterpret_cast<uint8_t *>(&block) + sizeof(block) - sizeof(block.checkSum));
        std::vector<uint8_t> hashVect;
        int errCode = DBCommon::CalcValueHash(vect, hashVect);
        if (errCode != E_OK) {
            return errCode;
        }
        if (memcmp(hashVect.data(), block.checkSum, sizeof(block.checkSum)) != 0) {
            LOGE("Check block error");
            return -E_UNEXPECTED_DATA;
        }

        return E_OK;
    }

    int CreateNewVersionFile(const std::string &versionFileDir, uint32_t version, std::vector<uint8_t> &tag)
    {
        VersionFileBlock block;
        block.version = version;
        RAND_bytes(block.tag, sizeof(block.tag));
        int errCode = memset_s(block.reserved, sizeof(block.reserved), 0, sizeof(block.reserved));
        if (errCode != EOK) {
            return -E_SECUREC_ERROR;
        }

        TransferHostFileBlockToNet(block);
        errCode = CalcFileBlockCheckSum(block);
        if (errCode != E_OK) {
            return errCode;
        }
        FILE *versionFile = fopen(versionFileDir.c_str(), "wb+");
        if (versionFile == nullptr) {
            LOGE("Open the version file error:%d", errno);
            return -E_SYSTEM_API_FAIL;
        }
        size_t writeSize = fwrite(static_cast<void *>(&block), 1, sizeof(VersionFileBlock), versionFile);
        if (writeSize != sizeof(VersionFileBlock)) {
            LOGE("Write version file head error:%d", errno);
            errCode = -E_SYSTEM_API_FAIL;
        } else {
            errCode = E_OK;
            tag.assign(block.tag, block.tag + sizeof(block.tag));
        }

        fclose(versionFile);
        versionFile = nullptr;
        return errCode;
    }

    int ChangeVersionFile(const std::string &versionFileDir, uint32_t version, std::vector<uint8_t> &tag,
        bool isChangeTag)
    {
        FILE *versionFile = fopen(versionFileDir.c_str(), "rb+");
        if (versionFile == nullptr) {
            LOGE("Open the version file error:%d", errno);
            return -E_SYSTEM_API_FAIL;
        }
        VersionFileBlock block;
        size_t operateSize = fread(static_cast<void *>(&block), 1, sizeof(VersionFileBlock), versionFile);
        if (operateSize != sizeof(VersionFileBlock)) {
            fclose(versionFile);
            LOGE("Read file error:%d", errno);
            return -E_SYSTEM_API_FAIL;
        };
        int errCode = CheckFileBlock(block);
        if (errCode != E_OK) {
            goto END;
        }
        TransferHostFileBlockToNet(block);
        block.version = version;

        if (isChangeTag) {
            RAND_bytes(block.tag, sizeof(block.tag));
            tag.assign(block.tag, block.tag + sizeof(block.tag));
        }

        TransferHostFileBlockToNet(block);
        errCode = CalcFileBlockCheckSum(block);
        if (errCode != E_OK) {
            goto END;
        }

        fseeko64(versionFile, 0LL, SEEK_SET);
        operateSize = fwrite(&block, 1, sizeof(VersionFileBlock), versionFile);
        if (operateSize != sizeof(VersionFileBlock)) {
            LOGE("write the file error:%d", errno);
            errCode = -E_SYSTEM_API_FAIL;
            goto END;
        }
    END:
        fclose(versionFile);
        versionFile = nullptr;
        return errCode;
    }

    int GetVersionAndTag(const std::string &versionFileDir, uint32_t &version, std::vector<uint8_t> &tag)
    {
        FILE *versionFile = fopen(versionFileDir.c_str(), "rb+");
        if (versionFile == nullptr) {
            LOGE("Open the version file error:%d", errno);
            return -E_SYSTEM_API_FAIL;
        }
        int errCode = E_OK;
        VersionFileBlock block;
        size_t readSize = fread(static_cast<void *>(&block), 1, sizeof(VersionFileBlock), versionFile);
        if (readSize != sizeof(VersionFileBlock)) {
            LOGE("read the file error:%d", errno);
            errCode = -E_SYSTEM_API_FAIL;
            goto END;
        };
        errCode = CheckFileBlock(block);
        if (errCode != E_OK) {
            LOGE("Check the file block error");
            goto END;
        }
        TransferNetFileBlockToHost(block);
        version = block.version;
        tag.assign(block.tag, block.tag + sizeof(block.tag));
    END:
        fclose(versionFile);
        versionFile = nullptr;
        return errCode;
    }
}

MultiVerVacuum MultiVerNaturalStore::shadowTrimmer_;
MultiVerNaturalStore::MultiVerNaturalStore()
    : multiVerData_(nullptr),
      commitHistory_(nullptr),
      multiVerKvStorage_(nullptr),
      multiVerEngine_(nullptr),
      trimmerImpl_(nullptr),
      maxRecordTimestamp_(0),
      maxCommitVersion_(0)
{}

MultiVerNaturalStore::~MultiVerNaturalStore()
{
    Clear();
    UnRegisterNotificationEventType(NATURAL_STORE_COMMIT_EVENT);
}

void MultiVerNaturalStore::Clear()
{
    if (trimmerImpl_ != nullptr) {
        shadowTrimmer_.Abort(GetStringIdentifier());
        delete trimmerImpl_;
        trimmerImpl_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(commitHistMutex_);
        if (commitHistory_ != nullptr) {
            commitHistory_->Close();
            delete commitHistory_;
            commitHistory_ = nullptr;
        }
    }
    {
        std::lock_guard<std::mutex> lock(multiDataMutex_);
        if (multiVerData_ != nullptr) {
            multiVerData_->Close();
            delete multiVerData_;
            multiVerData_ = nullptr;
        }
    }

    {
        std::lock_guard<std::mutex> lock(syncerKvMutex_);
        if (multiVerKvStorage_ != nullptr) {
            multiVerKvStorage_->Close();
            delete multiVerKvStorage_;
            multiVerKvStorage_ = nullptr;
        }
    }
    multiVerEngine_ = nullptr;
}

int MultiVerNaturalStore::InitStorages(const KvDBProperties &kvDBProp, bool isChangeTag)
{
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    bool isNeedCreate = kvDBProp.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    CipherType cipherType;
    CipherPassword passwd;
    kvDBProp.GetPassword(cipherType, passwd);

    IKvDBMultiVerDataStorage::Property multiVerProp = {dataDir, identifierDir, isNeedCreate, cipherType, passwd};
    IKvDBCommitStorage::Property commitProp = {dataDir, identifierDir, isNeedCreate, cipherType, passwd};
    MultiVerKvDataStorage::Property multiVerKvProp = {dataDir, identifierDir, isNeedCreate, cipherType, passwd};

    int errCode = DBCommon::CreateStoreDirectory(dataDir, identifierDir, DBConstant::MULTI_SUB_DIR, isNeedCreate);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = CheckVersion(kvDBProp);
    if (errCode != E_OK) {
        LOGE("Upgrade multi ver failed:%d", errCode);
        return errCode;
    }

    errCode = multiVerData_->Open(multiVerProp);
    if (errCode != E_OK) {
        LOGE("MultiVer::InitStorages open  multiVerData fail! errCode[%d]", errCode);
        return errCode;
    }

    errCode = commitHistory_->Open(commitProp);
    if (errCode != E_OK) {
        LOGE("MultiVer::InitStorages open  commitHistory fail! errCode[%d]", errCode);
        return errCode;
    }

    errCode = multiVerKvStorage_->Open(multiVerKvProp);
    if (errCode != E_OK) {
        LOGE("Open multi ver kv storage failed:%d", errCode);
        return errCode;
    }

    errCode = RecoverFromException();
    if (errCode != E_OK) {
        LOGE("Recover multi version storage failed:%d", errCode);
        return errCode;
    }
    return InitStorageContext(isChangeTag);
}

int MultiVerNaturalStore::CheckSubStorageVersion(const KvDBProperties &kvDBProp, bool &isSubStorageAllExist) const
{
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    bool isNeedCreate = kvDBProp.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    CipherType cipherType;
    CipherPassword passwd;
    kvDBProp.GetPassword(cipherType, passwd);

    IKvDBMultiVerDataStorage::Property multiVerProp = {dataDir, identifierDir, isNeedCreate, cipherType, passwd};
    IKvDBCommitStorage::Property commitProp = {dataDir, identifierDir, isNeedCreate, cipherType, passwd};
    MultiVerKvDataStorage::Property multiVerKvProp = {dataDir, identifierDir, true, cipherType, passwd};

    bool isDataStorageExist = false;
    bool isCommitStorageExist = false;
    bool isKvStorageAllExist = false;
    int errCode = multiVerData_->CheckVersion(multiVerProp, isDataStorageExist);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = commitHistory_->CheckVersion(commitProp, isCommitStorageExist);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = multiVerKvStorage_->CheckVersion(multiVerKvProp, isKvStorageAllExist);
    if (errCode != E_OK) {
        return errCode;
    }
    if ((isDataStorageExist != isCommitStorageExist) || (isCommitStorageExist != isKvStorageAllExist)) {
        // In case failure happens during open progress, some dbFile will not exist, we should recover from this
        LOGW("[MultiVerStore][CheckSubVer] Detect File Lost, isDataExist=%d, isCommitExist=%d, isKvAllExist=%d.",
            isDataStorageExist, isCommitStorageExist, isKvStorageAllExist);
    }
    isSubStorageAllExist = isDataStorageExist && isCommitStorageExist && isKvStorageAllExist;
    return E_OK;
}

int MultiVerNaturalStore::CreateStorages()
{
    int errCode = E_OK;
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    if (factory == nullptr) {
        return -E_INVALID_DB;
    }
    multiVerData_ = factory->CreateMultiVerStorage(errCode);
    if (multiVerData_ == nullptr) {
        return errCode;
    }

    commitHistory_ = factory->CreateMultiVerCommitStorage(errCode);
    if (commitHistory_ == nullptr) {
        return errCode;
    }

    multiVerKvStorage_ = new (std::nothrow) MultiVerKvDataStorage;
    if (multiVerKvStorage_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    return E_OK;
}

int MultiVerNaturalStore::ClearTempFile(const KvDBProperties &kvDBProp)
{
    std::unique_ptr<MultiVerDatabaseOper> operation = std::make_unique<MultiVerDatabaseOper>(this, multiVerData_,
        commitHistory_, multiVerKvStorage_);
    (void)operation->ClearExportedTempFiles(kvDBProp);
    int errCode = operation->RekeyRecover(kvDBProp);
    if (errCode != E_OK) {
        LOGE("Recover for open db failed in multi version:%d", errCode);
        return errCode;
    }

    errCode = operation->ClearImportTempFile(kvDBProp);
    if (errCode != E_OK) {
        LOGE("Recover import temp file for open db failed in multi version:%d", errCode);
    }
    return errCode;
}

// Open the database
int MultiVerNaturalStore::Open(const KvDBProperties &kvDBProp)
{
    StorageEngineAttr poolSize = {0, 1, 0, 16}; // 1 write 16 read at most.
    int errCode = CreateStorages();
    if (errCode != E_OK) {
        goto ERROR;
    }

    MyProp() = kvDBProp;
    errCode = ClearTempFile(kvDBProp);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = InitStorages(kvDBProp);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = RegisterNotificationEventType(NATURAL_STORE_COMMIT_EVENT);
    if (errCode != E_OK) {
        LOGE("RegisterEventType failed!");
        goto ERROR;
    }

    multiVerEngine_ = std::make_unique<MultiVerStorageEngine>();
    if (multiVerEngine_ == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        goto ERROR;
    }
    errCode = multiVerEngine_->InitDatabases(this, multiVerData_, commitHistory_, multiVerKvStorage_, poolSize);
    if (errCode != E_OK) {
        goto ERROR;
    }
    // Start the trimming;
    trimmerImpl_ = new (std::nothrow) MultiVerVacuumExecutorImpl(this);
    if (trimmerImpl_ == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        goto ERROR;
    }

    shadowTrimmer_.Launch(GetStringIdentifier(), trimmerImpl_);
    StartSyncer();
    return E_OK;
ERROR:
    Clear();
    return errCode;
}

void MultiVerNaturalStore::Close()
{
    // Abort the trimming;
    SyncAbleKvDB::Close();
    Clear();
}

GenericKvDBConnection *MultiVerNaturalStore::NewConnection(int &errCode)
{
    auto connection = new (std::nothrow) MultiVerNaturalStoreConnection(this);
    if (connection == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }

    errCode = E_OK;
    return connection;
}

// Get interface for syncer.
IKvDBSyncInterface *MultiVerNaturalStore::GetSyncInterface()
{
    return this;
}

// Get interface type of this kvdb.
int MultiVerNaturalStore::GetInterfaceType() const
{
    return SYNC_MVD;
}

// Get the interface ref-count, in order to access asynchronously.
void MultiVerNaturalStore::IncRefCount()
{
    IncObjRef(this);
}

// Drop the interface ref-count.
void MultiVerNaturalStore::DecRefCount()
{
    DecObjRef(this);
}

// Get the identifier of this kvdb.
std::vector<uint8_t> MultiVerNaturalStore::GetIdentifier() const
{
    std::string identifier = MyProp().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    std::vector<uint8_t> identifierVect(identifier.begin(), identifier.end());
    return identifierVect;
}

std::string MultiVerNaturalStore::GetStringIdentifier() const
{
    std::string identifier = MyProp().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    std::vector<uint8_t> idVect(identifier.begin(), identifier.end());
    return VEC_TO_STR(idVect);
}

// Get the max timestamp of all entries in database.
void MultiVerNaturalStore::GetMaxTimeStamp(TimeStamp &stamp) const
{
    std::lock_guard<std::mutex> lock(maxTimeMutex_);
    stamp = maxRecordTimestamp_;
}

void MultiVerNaturalStore::SetMaxTimeStamp(TimeStamp stamp)
{
    std::lock_guard<std::mutex> lock(maxTimeMutex_);
    maxRecordTimestamp_ = (stamp > maxRecordTimestamp_) ? stamp : maxRecordTimestamp_;
}

// Get meta data associated with the given key.
int MultiVerNaturalStore::GetMetaData(const Key &key, Value &value) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->GetMetaData(key, value);
    ReleaseHandle(handle);
    return errCode;
}

// Put meta data as a key-value entry.
int MultiVerNaturalStore::PutMetaData(const Key &key, const Value &value)
{
    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->PutMetaData(key, value);
    ReleaseHandle(handle);
    return errCode;
}

// Get all meta data keys.
int MultiVerNaturalStore::GetAllMetaKeys(std::vector<Key> &keys) const
{
    return E_OK;
}

bool MultiVerNaturalStore::IsCommitExisted(const MultiVerCommitNode &commit) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return false;
    }

    bool result = handle->IsCommitExisted(commit, errCode);
    ReleaseHandle(handle);
    return result;
}

int MultiVerNaturalStore::GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &commitMap) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->GetDeviceLatestCommit(commitMap);
    ReleaseHandle(handle);
    return errCode;
}

int MultiVerNaturalStore::GetCommitTree(const std::map<std::string, MultiVerCommitNode> &commitMap,
    std::vector<MultiVerCommitNode> &commits) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->GetCommitTree(commitMap, commits);
    ReleaseHandle(handle);
    return errCode;
}

int MultiVerNaturalStore::GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->GetCommitData(commit, entries);
    ReleaseHandle(handle);
    return errCode;
}

MultiVerKvEntry *MultiVerNaturalStore::CreateKvEntry(const std::vector<uint8_t> &data)
{
    auto kvEntry = new (std::nothrow) GenericMultiVerKvEntry;
    if (kvEntry == nullptr) {
        return nullptr;
    }

    int errCode = kvEntry->DeSerialData(data);
    if (errCode != E_OK) {
        LOGE("deserialize data into kv entry failed:%d", errCode);
        delete kvEntry;
        kvEntry = nullptr;
    }
    return kvEntry;
}

void MultiVerNaturalStore::ReleaseKvEntry(const MultiVerKvEntry *entry)
{
    if (entry != nullptr) {
        delete entry;
        entry = nullptr;
    }
}

bool MultiVerNaturalStore::IsValueSliceExisted(const ValueSliceHash &value) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return false;
    }

    bool result = handle->IsValueSliceExisted(value, errCode);
    ReleaseHandle(handle);
    return result;
}

int MultiVerNaturalStore::GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const
{
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->GetValueSlice(hashValue, sliceValue);
    ReleaseHandle(handle);
    return errCode;
}

int MultiVerNaturalStore::PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue) const
{
    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->PutValueSlice(hashValue, sliceValue, false);
    ReleaseHandle(handle);
    return errCode;
}

int MultiVerNaturalStore::PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
    const std::string &deviceName)
{
    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->PutCommitData(commit, entries, deviceName);
    ReleaseHandle(handle);
    return errCode;
}

int MultiVerNaturalStore::MergeSyncCommit(const MultiVerCommitNode &commit,
    const std::vector<MultiVerCommitNode> &commits)
{
    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return -E_BUSY;
    }

    errCode = handle->MergeSyncCommit(commit, commits);
    ReleaseHandle(handle);
    return errCode;
}

void MultiVerNaturalStore::NotifyStartSyncOperation()
{
    shadowTrimmer_.Pause(GetStringIdentifier());
}

void MultiVerNaturalStore::NotifyFinishSyncOperation()
{
    shadowTrimmer_.Continue(GetStringIdentifier(), true);
}

int MultiVerNaturalStore::TransferSyncCommitDevInfo(MultiVerCommitNode &commit, const std::string &devId,
    bool isSyncedIn) const
{
    std::string hashDevId = DBCommon::TransferHashString(devId);
    if (isSyncedIn) {
        // The size of the device info must be hash_size + tag_size;
        if (commit.deviceInfo.size() == hashDevId.size() + MULTI_VER_TAG_SIZE) {
            // If the hash device info is matched with the local, just remove the hash device info.
            if (commit.deviceInfo.compare(0, hashDevId.size(), hashDevId) == 0) {
                commit.deviceInfo = commit.deviceInfo.substr(hashDevId.size(), MULTI_VER_TAG_SIZE);
            }
            return E_OK;
        }
        LOGE("Unexpected dev info for sync in:%zu", commit.deviceInfo.size());
        return -E_UNEXPECTED_DATA;
    } else {
        // If the device info only contains the tag info, it must be local node.
        if (commit.deviceInfo.size() == MULTI_VER_TAG_SIZE) {
            commit.deviceInfo.insert(0, hashDevId);
        } else if (commit.deviceInfo.size() != hashDevId.size() + MULTI_VER_TAG_SIZE) {
            LOGE("Unexpected dev info for sync out:%zu", commit.deviceInfo.size());
            return -E_UNEXPECTED_DATA;
        }
        return E_OK;
    }
}

int MultiVerNaturalStore::Rekey(const CipherPassword &passwd)
{
    if (multiVerEngine_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = multiVerEngine_->TryToDisable(false, OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }
    StopSyncer();
    shadowTrimmer_.Pause(GetStringIdentifier());
    errCode = multiVerEngine_->TryToDisable(true, OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        multiVerEngine_->Enable(OperatePerm::REKEY_MONOPOLIZE_PERM);
        shadowTrimmer_.Continue(GetStringIdentifier(), true);
        StartSyncer();
        return errCode;
    }

    std::unique_ptr<MultiVerDatabaseOper> operation = std::make_unique<MultiVerDatabaseOper>(this, multiVerData_,
        commitHistory_, multiVerKvStorage_);
    errCode = operation->Rekey(passwd);

    multiVerEngine_->Enable(OperatePerm::REKEY_MONOPOLIZE_PERM);
    shadowTrimmer_.Continue(GetStringIdentifier(), true);
    StartSyncer();

    return errCode;
}

int MultiVerNaturalStore::Export(const std::string &filePath, const CipherPassword &passwd)
{
    if (multiVerEngine_ == nullptr) {
        return -E_INVALID_DB;
    }
    std::string localDev;
    int errCode = GetLocalIdentity(localDev);
    if (errCode != E_OK) {
        LOGE("Failed to GetLocalIdentity!");
    }
    // Exclusively write resources
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    std::unique_ptr<MultiVerDatabaseOper> operation = std::make_unique<MultiVerDatabaseOper>(this, multiVerData_,
        commitHistory_, multiVerKvStorage_);
    operation->SetLocalDevId(localDev);
    errCode = operation->Export(filePath, passwd);

    ReleaseHandle(handle);

    return errCode;
}

int MultiVerNaturalStore::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (multiVerEngine_ == nullptr) {
        return -E_INVALID_DB;
    }
    std::string localDev;
    int errCode = GetLocalIdentity(localDev);
    if (errCode != E_OK) {
        LOGE("Failed to get the local identity!");
        localDev.resize(0);
    }
    errCode = multiVerEngine_->TryToDisable(false, OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }
    StopSyncer();
    shadowTrimmer_.Abort(GetStringIdentifier());
    std::unique_ptr<MultiVerDatabaseOper> operation;
    errCode = multiVerEngine_->TryToDisable(true, OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        goto END;
    }
    operation = std::make_unique<MultiVerDatabaseOper>(this, multiVerData_, commitHistory_, multiVerKvStorage_);
    operation->SetLocalDevId(localDev);
    errCode = operation->Import(filePath, passwd);
END:
    multiVerEngine_->Enable(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    shadowTrimmer_.Launch(GetStringIdentifier(), trimmerImpl_);
    StartSyncer();
    return errCode;
}

uint64_t MultiVerNaturalStore::GetCurrentTimeStamp()
{
    return GetTimeStamp();
}

int MultiVerNaturalStore::GetDiffEntries(const CommitID &begin, const CommitID &end, MultiVerDiffData &data) const
{
    // Get one connection.
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->GetDiffEntries(begin, end, data);
    ReleaseHandle(handle);
    return errCode;
}

int MultiVerNaturalStore::RecoverFromException()
{
    // Get the latest local version and the head node.
    if (multiVerData_ == nullptr || commitHistory_ == nullptr) {
        return -E_INVALID_DB;
    }

    IKvDBMultiVerTransaction *transaction = nullptr;
    int errCode = multiVerData_->StartWrite(KvDataType::KV_DATA_SYNC_P2P, transaction);
    if (transaction == nullptr) {
        goto END;
    }
    errCode = transaction->StartTransaction();
    if (errCode != E_OK) {
        goto END;
    }

    errCode = CompareVerDataAndLog(transaction);
    if (errCode != E_OK) {
        LOGE("Compare the version data and log failed:%d", errCode);
        transaction->RollBackTransaction();
        goto END;
    }
    errCode = transaction->CommitTransaction();
END:
    if (transaction != nullptr) {
        multiVerData_->ReleaseTransaction(transaction);
        transaction = nullptr;
    }
    return errCode;
}

int MultiVerNaturalStore::CompareVerDataAndLog(IKvDBMultiVerTransaction *transaction) const
{
    // Get the latest local version, we only care the local data.
    Version maxLocalVersion = 0;
    int errCode = transaction->GetMaxVersion(MultiVerDataType::NATIVE_TYPE, maxLocalVersion);
    if (errCode != E_OK) {
        return errCode;
    }

    CommitID headerId = commitHistory_->GetHeader(errCode);
    if (errCode != E_OK) {
        return errCode;
    }

    if (headerId.empty()) {
        if (maxLocalVersion != 0) {
            return transaction->ClearEntriesByVersion(maxLocalVersion);
        }
        return E_OK;
    }

    IKvDBCommit *commitHead = commitHistory_->GetCommit(headerId, errCode);
    if (commitHead == nullptr) {
        return errCode;
    }

    // compare the version;
    if (commitHead->GetCommitVersion() < maxLocalVersion) {
        LOGD("Delete entries");
        errCode = transaction->ClearEntriesByVersion(maxLocalVersion);
    } else {
        errCode = E_OK;
    }

    commitHistory_->ReleaseCommit(commitHead);
    commitHead = nullptr;
    return errCode;
}

Version MultiVerNaturalStore::GetMaxCommitVersion() const
{
    return maxCommitVersion_;
}

void MultiVerNaturalStore::SetMaxCommitVersion(const Version &version)
{
    maxCommitVersion_ = (version > maxCommitVersion_) ? version : maxCommitVersion_;
}

MultiVerStorageExecutor *MultiVerNaturalStore::GetHandle(bool isWrite, int &errCode,
    bool isTrimming, OperatePerm perm) const
{
    if (multiVerEngine_ == nullptr) {
        errCode = -E_INVALID_DB;
        return nullptr;
    }

    if (isWrite && !isTrimming) {
        // stop the trimming;
        shadowTrimmer_.Pause(GetStringIdentifier());
    }
    StorageExecutor *handle = nullptr;
    if (isTrimming) {
        handle = multiVerEngine_->FindExecutor(isWrite, OperatePerm::NORMAL_PERM, errCode, 0);
    } else {
        handle = multiVerEngine_->FindExecutor(isWrite, perm, errCode);
    }

    if (handle == nullptr) {
        if (isWrite && !isTrimming) {
            // restart the trimming;
            shadowTrimmer_.Continue(GetStringIdentifier(), false);
        }
    } else {
        if (!handle->GetWritable() && isTrimming) {
            static_cast<MultiVerStorageExecutor *>(handle)->InitCurrentReadVersion();
        }
    }
    return static_cast<MultiVerStorageExecutor *>(handle);
}

void MultiVerNaturalStore::ReleaseHandle(MultiVerStorageExecutor *&handle, bool isTrimming) const
{
    if (multiVerEngine_ == nullptr || handle == nullptr) {
        return;
    }
    bool isCorrupted = handle->GetCorruptedStatus();
    bool isWrite = handle->GetWritable();
    StorageExecutor *databaseHandle = handle;
    multiVerEngine_->Recycle(databaseHandle);
    handle = nullptr;
    if (isCorrupted) {
        CorruptNotify();
    }
    if (isWrite && !isTrimming) {
        // restart the trimming.
        LOGI("Release handle and continue vacuum data!");
        shadowTrimmer_.Continue(GetStringIdentifier(), true);
    }
}

int MultiVerNaturalStore::InitStorageContext(bool isChangeTag)
{
    int errCode = InitStorageContextVersion(isChangeTag);
    if (errCode != E_OK) {
        return errCode;
    }

    maxCommitVersion_ = commitHistory_->GetMaxCommitVersion(errCode);
    if (errCode != E_OK) {
        LOGE("Get the max commit version failed:%d", errCode);
    }
    return errCode;
}

int MultiVerNaturalStore::InitStorageContextVersion(bool isChangeTag)
{
    std::string verFilePath;
    int errCode = GetVersionFilePath(MyProp(), verFilePath);
    if (errCode != E_OK) {
        return errCode;
    }

    if (!OS::CheckPathExistence(verFilePath)) {
        return CreateNewVersionFile(verFilePath, MULTI_VER_STORE_VERSION_CURRENT, branchTag_);
    }
    if (isChangeTag) {
        return ChangeVersionFile(verFilePath, MULTI_VER_STORE_VERSION_CURRENT, branchTag_, isChangeTag);
    }
    uint32_t version = 0;
    return GetVersionAndTag(verFilePath, version, branchTag_);
}

void MultiVerNaturalStore::GetCurrentTag(std::vector<uint8_t> &tag) const
{
    tag = branchTag_;
}

void MultiVerNaturalStore::AddVersionConstraintToList(Version version)
{
    std::lock_guard<std::mutex> lock(versionConstraintMutex_);
    versionConstraints_.insert(version);
}

void MultiVerNaturalStore::RemoveVersionConstraintFromList(Version version)
{
    std::lock_guard<std::mutex> lock(versionConstraintMutex_);
    auto iter = versionConstraints_.find(version);
    if (iter != versionConstraints_.end()) {
        versionConstraints_.erase(iter);
        // Auto launch the vacuum.
        shadowTrimmer_.AutoRelaunchOnce(GetStringIdentifier());
    }
}

Version MultiVerNaturalStore::GetMaxTrimmableVersion() const
{
    std::lock_guard<std::mutex> lock(versionConstraintMutex_);
    if (versionConstraints_.empty()) {
        return UINT64_MAX;
    }
    return *(versionConstraints_.begin());
}

int MultiVerNaturalStore::TransObserverTypeToRegisterFunctionType(int observerType, RegisterFuncType &type) const
{
    if (observerType == static_cast<uint32_t>(NATURAL_STORE_COMMIT_EVENT)) {
        type = OBSERVER_MULTI_VERSION_NS_COMMIT_EVENT;
        return E_OK;
    }
    return -E_NOT_SUPPORT;
}

const KvDBProperties &MultiVerNaturalStore::GetDbProperties() const
{
    return GetMyProperties();
}

int MultiVerNaturalStore::RemoveKvDB(const KvDBProperties &properties)
{
    std::string storeOnlyDir;
    std::string storeDir;
    GenericKvDB::GetStoreDirectory(properties, KvDBProperties::MULTI_VER_TYPE, storeDir, storeOnlyDir);
    int errCodeVersion = KvDBUtils::RemoveKvDB(storeDir, storeOnlyDir, DBConstant::MULTI_VER_DATA_STORE);
    int errCodeCommit = KvDBUtils::RemoveKvDB(storeDir, storeOnlyDir, DBConstant::MULTI_VER_COMMIT_STORE);
    int errCodeValue = KvDBUtils::RemoveKvDB(storeDir, storeOnlyDir, DBConstant::MULTI_VER_VALUE_STORE);
    int errCodeMeta = KvDBUtils::RemoveKvDB(storeDir, storeOnlyDir, DBConstant::MULTI_VER_META_STORE);
    LOGD("Delete the versionStorage:%d, commitStorage:%d, valueStorage:%d, metaStorage:%d",
        errCodeVersion, errCodeCommit, errCodeValue, errCodeMeta);
    DBCommon::RemoveAllFilesOfDirectory(storeDir, true);
    DBCommon::RemoveAllFilesOfDirectory(storeOnlyDir, true);
    if (errCodeVersion == E_OK && errCodeCommit == E_OK) {
        return E_OK;
    }
    if (errCodeVersion == -E_NOT_FOUND && errCodeCommit == -E_NOT_FOUND) {
        return -E_NOT_FOUND;
    }
    if (errCodeVersion == E_OK && errCodeCommit == -E_NOT_FOUND) {
        return E_OK;
    }
    if (errCodeVersion == -E_NOT_FOUND && errCodeCommit == E_OK) {
        return E_OK;
    }
    return errCodeCommit;
}

int MultiVerNaturalStore::GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const
{
    std::string storeOnlyDir;
    std::string storeDir;
    GenericKvDB::GetStoreDirectory(properties, KvDBProperties::MULTI_VER_TYPE, storeDir, storeOnlyDir);

    std::vector<std::string> storageNames = {
        DBConstant::MULTI_VER_DATA_STORE,
        DBConstant::MULTI_VER_COMMIT_STORE,
        DBConstant::MULTI_VER_VALUE_STORE,
        DBConstant::MULTI_VER_META_STORE
    };

    // there only calculate db related file size
    for (const auto &storageName : storageNames) {
        uint64_t dbSize = 0;
        int errCode = KvDBUtils::GetKvDbSize(storeDir, storeOnlyDir, storageName, dbSize);
        if (errCode == E_OK) {
            size += dbSize;
            continue;
        }

        if (errCode == -E_NOT_FOUND) {
            return -E_NOT_FOUND;
        }

        size = 0;
        return errCode;
    }
    return E_OK;
}

KvDBProperties &MultiVerNaturalStore::GetDbPropertyForUpdate()
{
    return MyProp();
}

int MultiVerNaturalStore::CheckVersion(const KvDBProperties &kvDBProp) const
{
    LOGD("[MultiVerStore][CheckVer] Current Overall Version: %u.", MULTI_VER_STORE_VERSION_CURRENT);
    bool isVerFileExist = false;
    int errCode = CheckOverallVersionViaVersionFile(kvDBProp, isVerFileExist);
    if (errCode != E_OK) {
        return errCode;
    }
    bool isSubStorageExist = false;
    errCode = CheckSubStorageVersion(kvDBProp, isSubStorageExist);
    if (errCode != E_OK) {
        return errCode;
    }
    if (isVerFileExist != isSubStorageExist) {
        LOGW("[MultiVerStore][CheckVer] Detect File Lost, isVerFileExist=%d, isSubStorageExist=%d.",
            isVerFileExist, isSubStorageExist);
    }
    return E_OK;
}

int MultiVerNaturalStore::CheckOverallVersionViaVersionFile(const KvDBProperties &kvDBProp, bool &isVerFileExist) const
{
    std::string verFilePath;
    int errCode = GetVersionFilePath(kvDBProp, verFilePath);
    if (errCode != E_OK) {
        return errCode;
    }
    // Absent of version file may because: 1: Newly created database; 2: An already created database lost version file.
    // In both case, we returned E_OK here. After each sub storage be successfully open and upgrade, create verFile.
    if (!OS::CheckPathExistence(verFilePath)) {
        LOGD("[MultiVerStore][CheckOverVer] No Version File.");
        isVerFileExist = false;
        return E_OK;
    }
    isVerFileExist = true;

    uint32_t overallVersion = 0;
    std::vector<uint8_t> branchTagInVerFile;
    errCode = GetVersionAndTag(verFilePath, overallVersion, branchTagInVerFile);
    if (errCode != E_OK) {
        LOGE("[MultiVerStore][CheckOverVer] GetVersionAndTag fail, errCode=%d.", errCode);
        return errCode;
    }
    LOGD("[MultiVerStore][CheckOverVer] overallVersion=%u, tag=%s.", overallVersion, VEC_TO_STR(branchTagInVerFile));
    if (overallVersion > MULTI_VER_STORE_VERSION_CURRENT) {
        LOGE("[MultiVerStore][CheckOverVer] Version Not Support!");
        return -E_VERSION_NOT_SUPPORT;
    }
    return E_OK;
}

int MultiVerNaturalStore::GetVersionFilePath(const KvDBProperties &kvDBProp, std::string &outPath) const
{
    std::string verFiledir;
    int errCode = GetWorkDir(kvDBProp, verFiledir);
    if (errCode != E_OK) {
        LOGE("[MultiVerStore][GetVerFilePath] GetWorkDir fail, errCode=%d", errCode);
        return errCode;
    }
    outPath = verFiledir + "/" + DBConstant::MULTI_SUB_DIR + "/version";
    return E_OK;
}

DEFINE_OBJECT_TAG_FACILITIES(MultiVerNaturalStore)
} // namespace DistributedDB
#endif
