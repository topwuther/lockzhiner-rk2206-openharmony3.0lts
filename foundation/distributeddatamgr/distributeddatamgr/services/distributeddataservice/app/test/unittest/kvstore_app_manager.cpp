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

#define LOG_TAG "KvStoreAppManager"

#include "kvstore_app_manager.h"
#include <directory_ex.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include "account_delegate.h"
#include "broadcast_sender.h"
#include "constant.h"
#include "directory_utils.h"
#include "device_kvstore_impl.h"
#include "ikvstore.h"
#include "kv_store_delegate.h"
#include "kvstore_app_accessor.h"
#include "kvstore_utils.h"
#include "log_print.h"
#include "process_communicator_impl.h"
#include "permission_validator.h"
#include "reporter.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {
KvStoreAppManager::KvStoreAppManager(const std::string &bundleName, const std::string &deviceAccountId)
    : bundleName_(bundleName), deviceAccountId_(deviceAccountId), flowCtrlManager_(BURST_CAPACITY, SUSTAINED_CAPACITY)
{
    ZLOGI("begin.");
    GetDelegateManager(PATH_DE);
    GetDelegateManager(PATH_CE);
}

KvStoreAppManager::~KvStoreAppManager()
{
    ZLOGD("begin.");
    stores_[PATH_DE].clear();
    stores_[PATH_CE].clear();

    {
        std::lock_guard<std::mutex> guard(delegateMutex_);
        delete delegateManagers_[PATH_DE];
        delete delegateManagers_[PATH_CE];
        delegateManagers_[PATH_DE] = nullptr;
        delegateManagers_[PATH_CE] = nullptr;
    }
}

Status KvStoreAppManager::ConvertErrorStatus(DistributedDB::DBStatus dbStatus, bool createIfMissing)
{
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error: %d.", static_cast<int>(dbStatus));
        switch (dbStatus) {
            case DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB:
                return Status::CRYPT_ERROR;
            case DistributedDB::DBStatus::SCHEMA_MISMATCH:
                return Status::SCHEMA_MISMATCH;
            case DistributedDB::DBStatus::INVALID_SCHEMA:
                return Status::INVALID_SCHEMA;
            case DistributedDB::DBStatus::NOT_SUPPORT:
                return Status::NOT_SUPPORT;
            case DistributedDB::DBStatus::EKEYREVOKED_ERROR: // fallthrough
            case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
                return Status::SECURITY_LEVEL_ERROR;
            default:
                break;
        }
        if (createIfMissing) {
            return Status::DB_ERROR;
        } else {
            return Status::STORE_NOT_FOUND;
        }
    }
    return Status::SUCCESS;
}

Status KvStoreAppManager::GetKvStore(const Options &options, const std::string &storeId,
                                     const std::vector<uint8_t> &cipherKey,
                                     std::function<void(sptr<IKvStoreImpl>)> callback)
{
    ZLOGI("begin");
    PathType type = ConvertPathType(bundleName_, options.securityLevel);
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManagers[%d] is nullptr.", type);
        callback(nullptr);
        return Status::ILLEGAL_STATE;
    }

    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }

    std::lock_guard<std::mutex> lg(storeMutex_);
    auto it = stores_[type].find(storeId);
    if (it != stores_[type].end()) {
        sptr<IKvStoreImpl> kvStoreImpl = it->second;
        ZLOGI("find store in map refcount: %d.", kvStoreImpl->GetSptrRefCount());
        static_cast<KvStoreImpl *>(kvStoreImpl.GetRefPtr())->IncreaseOpenCount();
        callback(std::move(kvStoreImpl));
        return Status::SUCCESS;
    }

    if ((GetTotalKvStoreNum()) >= static_cast<size_t>(Constant::MAX_OPEN_KVSTORES)) {
        ZLOGE("limit %d KvStores can be opened.", Constant::MAX_OPEN_KVSTORES);
        callback(nullptr);
        return Status::ERROR;
    }

    DistributedDB::KvStoreDelegate::Option dbOption;
    auto status = InitDbOption(options, cipherKey, dbOption);
    if (status != Status::SUCCESS) {
        ZLOGE("InitDbOption failed.");
        callback(nullptr);
        return status;
    }

    DistributedDB::KvStoreDelegate *storeDelegate = nullptr;
    DistributedDB::DBStatus dbStatusTmp;
    delegateManager->GetKvStore(storeId, dbOption,
        [&storeDelegate, &dbStatusTmp](DistributedDB::DBStatus dbStatus, DistributedDB::KvStoreDelegate *delegate) {
            storeDelegate = delegate;
            dbStatusTmp = dbStatus;
        });

    if (storeDelegate == nullptr) {
        ZLOGE("storeDelegate is nullptr, status:%d.", static_cast<int>(dbStatusTmp));
        callback(nullptr);
        return ConvertErrorStatus(dbStatusTmp, options.createIfMissing);
    }

    ZLOGD("get delegate");
    sptr<KvStoreImpl> store = new (std::nothrow)KvStoreImpl(options, deviceAccountId_, bundleName_,
                                                            storeId, GetDbDir(options), storeDelegate);
    if (store == nullptr) {
        callback(nullptr);
        delegateManager->CloseKvStore(storeDelegate);
        return Status::ERROR;
    }
    auto result = stores_[type].emplace(storeId, store);
    if (!result.second) {
        ZLOGE("emplace failed.");
        callback(nullptr);
        delegateManager->CloseKvStore(storeDelegate);
        return Status::ERROR;
    }

    sptr<IKvStoreImpl> kvStoreImpl = result.first->second;
    ZLOGD("after emplace refcount: %d", kvStoreImpl->GetSptrRefCount());
    callback(std::move(kvStoreImpl));
    return Status::SUCCESS;
}

Status KvStoreAppManager::GetKvStore(const Options &options, const std::string &storeId,
                                     const std::vector<uint8_t> &cipherKey,
                                     std::function<void(sptr<ISingleKvStore>)> callback)
{
    ZLOGI("begin");
    PathType type = ConvertPathType(bundleName_, options.securityLevel);
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManagers[%d] is nullptr.", type);
        callback(nullptr);
        return Status::ILLEGAL_STATE;
    }

    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    std::lock_guard<std::mutex> lg(storeMutex_);
    auto it = singleStores_[type].find(storeId);
    if (it != singleStores_[type].end()) {
        sptr<ISingleKvStore> singleKvStoreImpl = it->second;
        ZLOGI("find store in map refcount: %d.", singleKvStoreImpl->GetSptrRefCount());
        static_cast<SingleKvStoreImpl *>(singleKvStoreImpl.GetRefPtr())->IncreaseOpenCount();
        callback(std::move(singleKvStoreImpl));
        return Status::SUCCESS;
    }

    if ((GetTotalKvStoreNum()) >= static_cast<size_t>(Constant::MAX_OPEN_KVSTORES)) {
        ZLOGE("limit %d KvStores can be opened.", Constant::MAX_OPEN_KVSTORES);
        callback(nullptr);
        return Status::ERROR;
    }

    DistributedDB::KvStoreNbDelegate::Option dbOption;
    auto status = InitNbDbOption(options, cipherKey, dbOption);
    if (status != Status::SUCCESS) {
        ZLOGE("InitNbDbOption failed.");
        callback(nullptr);
        return status;
    }

    DistributedDB::KvStoreNbDelegate *storeDelegate = nullptr;
    DistributedDB::DBStatus dbStatusTmp;
    delegateManager->GetKvStore(storeId, dbOption,
        [&](DistributedDB::DBStatus dbStatus, DistributedDB::KvStoreNbDelegate *kvStoreDelegate) {
            storeDelegate = kvStoreDelegate;
            dbStatusTmp = dbStatus;
        });

    if (storeDelegate == nullptr) {
        ZLOGE("storeDelegate is nullptr.");
        callback(nullptr);
        return ConvertErrorStatus(dbStatusTmp, options.createIfMissing);
    }
    std::string kvStorePath = GetDbDir(options);
    auto store = new (std::nothrow) DeviceKvStoreImpl({
        options, options.kvStoreType == KvStoreType::DEVICE_COLLABORATION, deviceAccountId_, bundleName_, storeId,
        kvStorePath}, storeDelegate);
    if (store == nullptr) {
        ZLOGE("store is nullptr.");
        callback(nullptr);
        delegateManager->CloseKvStore(storeDelegate);
        return Status::ERROR;
    }
    auto result = singleStores_[type].emplace(storeId, store);
    if (!result.second) {
        ZLOGE("emplace failed.");
        callback(nullptr);
        delegateManager->CloseKvStore(storeDelegate);
        delete store;
        return Status::ERROR;
    }

    sptr<ISingleKvStore> singleKvStoreImpl = result.first->second;
    ZLOGI("after emplace refcount: %d autoSync: %d",
        singleKvStoreImpl->GetSptrRefCount(), static_cast<int>(options.autoSync));
    if (options.autoSync) {
        bool autoSync = true;
        DistributedDB::PragmaData data = static_cast<DistributedDB::PragmaData>(&autoSync);
        auto pragmaStatus = storeDelegate->Pragma(DistributedDB::PragmaCmd::AUTO_SYNC, data);
        if (pragmaStatus != DistributedDB::DBStatus::OK) {
            ZLOGE("pragmaStatus: %d", static_cast<int>(pragmaStatus));
        }
    }

    callback(std::move(singleKvStoreImpl));
    DistributedDB::AutoLaunchOption launchOption = {
        options.createIfMissing, options.encrypt, dbOption.cipher, dbOption.passwd, dbOption.schema,
        dbOption.createDirByStoreIdOnly, kvStorePath, nullptr
    };
    launchOption.secOption = ConvertSecurity(options.securityLevel);
    AppAccessorParam accessorParam = {Constant::DEFAULT_GROUP_ID, trueAppId_, storeId, launchOption};
    KvStoreAppAccessor::GetInstance().EnableKvStoreAutoLaunch(accessorParam);
    return Status::SUCCESS;
}

Status KvStoreAppManager::CloseKvStore(const std::string &storeId)
{
    ZLOGI("CloseKvStore");
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    std::lock_guard<std::mutex> lg(storeMutex_);
    Status status = CloseKvStore(storeId, PATH_DE);
    if (status != Status::STORE_NOT_OPEN) {
        return status;
    }

    status = CloseKvStore(storeId, PATH_CE);
    if (status != Status::STORE_NOT_OPEN) {
        return status;
    }

    ZLOGW("store not open");
    return Status::STORE_NOT_OPEN;
}

Status KvStoreAppManager::CloseAllKvStore()
{
    ZLOGI("begin.");
    std::lock_guard<std::mutex> lg(storeMutex_);
    if (GetTotalKvStoreNum() == 0) {
        return Status::STORE_NOT_OPEN;
    }

    ZLOGI("close %zu KvStores.", GetTotalKvStoreNum());
    Status status = CloseAllKvStore(PATH_DE);
    if (status == Status::DB_ERROR) {
        return status;
    }
    status = CloseAllKvStore(PATH_CE);
    if (status == Status::DB_ERROR) {
        return status;
    }
    return Status::SUCCESS;
}

Status KvStoreAppManager::DeleteKvStore(const std::string &storeId)
{
    ZLOGI("%s", storeId.c_str());
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }

    Status statusDE = DeleteKvStore(storeId, PATH_DE);
    Status statusCE = DeleteKvStore(storeId, PATH_CE);
    if (statusDE == Status::SUCCESS || statusCE == Status::SUCCESS) {
        return Status::SUCCESS;
    }

    ZLOGE("delegate close error.");
    return Status::DB_ERROR;
}

Status KvStoreAppManager::DeleteAllKvStore()
{
    ZLOGI("begin.");
    std::lock_guard<std::mutex> lg(storeMutex_);
    if (GetTotalKvStoreNum() == 0) {
        return Status::STORE_NOT_OPEN;
    }
    ZLOGI("delete %d KvStores.", int32_t(GetTotalKvStoreNum()));

    Status status = DeleteAllKvStore(PATH_DE);
    if (status != Status::SUCCESS) {
        ZLOGE("path de delegate delete error: %d.", static_cast<int>(status));
        return Status::DB_ERROR;
    }
    status = DeleteAllKvStore(PATH_CE);
    if (status != Status::SUCCESS) {
        ZLOGE("path ce delegate delete error: %d.", static_cast<int>(status));
        return Status::DB_ERROR;
    }
    return Status::SUCCESS;
}

Status KvStoreAppManager::MigrateAllKvStore(const std::string &harmonyAccountId)
{
    ZLOGI("begin");
    if (PermissionValidator::IsAutoLaunchEnabled(bundleName_)) {
        return Status::SUCCESS;
    }

    std::lock_guard<std::mutex> lg(storeMutex_);
    // update userid in kvstore tuple map of permission adapter.
    KvStoreTuple srcKvStoreTuple {userId_, bundleName_};
    KvStoreTuple dstKvStoreTuple {harmonyAccountId, bundleName_};
    PermissionValidator::UpdateKvStoreTupleMap(srcKvStoreTuple, dstKvStoreTuple);
    userId_ = harmonyAccountId;
    ZLOGI("path de migration begin.");
    Status statusDE = MigrateAllKvStore(harmonyAccountId, PATH_DE);
    ZLOGI("path ce migration begin.");
    Status statusCE = MigrateAllKvStore(harmonyAccountId, PATH_CE);
    return (statusCE != Status::SUCCESS) ? statusCE : statusDE;
}

Status KvStoreAppManager::InitDbOption(const Options &options, const std::vector<uint8_t> &cipherKey,
                                       DistributedDB::KvStoreDelegate::Option &dbOption)
{
    DistributedDB::CipherPassword password;
    auto status = password.SetValue(cipherKey.data(), cipherKey.size());
    if (status != DistributedDB::CipherPassword::ErrorCode::OK) {
        ZLOGE("Failed to set the passwd:%zu", cipherKey.size());
        return Status::DB_ERROR;
    }
    dbOption.createIfNecessary = options.createIfMissing;
    dbOption.localOnly = false;
    dbOption.isEncryptedDb = options.encrypt;
    if (options.encrypt) {
        dbOption.cipher = DistributedDB::CipherType::AES_256_GCM;
        dbOption.passwd = password;
    }
    dbOption.createDirByStoreIdOnly = options.dataOwnership;
    return Status::SUCCESS;
}

Status KvStoreAppManager::InitNbDbOption(const Options &options, const std::vector<uint8_t> &cipherKey,
                                         DistributedDB::KvStoreNbDelegate::Option &dbOption)
{
    DistributedDB::CipherPassword password;
    auto status = password.SetValue(cipherKey.data(), cipherKey.size());
    if (status != DistributedDB::CipherPassword::ErrorCode::OK) {
        ZLOGE("Failed to set the passwd:%zu", cipherKey.size());
        return Status::DB_ERROR;
    }

    dbOption.createIfNecessary = options.createIfMissing;
    dbOption.isEncryptedDb = options.encrypt;
    if (options.encrypt) {
        dbOption.cipher = DistributedDB::CipherType::AES_256_GCM;
        dbOption.passwd = password;
    }

    if (options.kvStoreType == KvStoreType::SINGLE_VERSION) {
        dbOption.conflictResolvePolicy = DistributedDB::LAST_WIN;
    } else if (options.kvStoreType == KvStoreType::DEVICE_COLLABORATION) {
        dbOption.conflictResolvePolicy = DistributedDB::DEVICE_COLLABORATION;
    } else {
        ZLOGE("kvStoreType is invalid");
        return Status::INVALID_ARGUMENT;
    }

    dbOption.schema = options.schema;
    dbOption.createDirByStoreIdOnly = options.dataOwnership;
    dbOption.secOption = ConvertSecurity(options.securityLevel);
    return Status::SUCCESS;
}

std::string KvStoreAppManager::GetDbDir(const Options &options) const
{
    return GetDataStoragePath(deviceAccountId_, bundleName_, ConvertPathType(bundleName_, options.securityLevel));
}

KvStoreAppManager::PathType KvStoreAppManager::ConvertPathType(const std::string &bundleName, int securityLevel)
{
    PathType type = PATH_CE;
    if ((securityLevel == NO_LABEL && PermissionValidator::IsSystemService(bundleName)) ||
        securityLevel == S0 ||
        securityLevel == S1) {
        type = PATH_DE;
    }
    return type;
}

DistributedDB::KvStoreDelegateManager *KvStoreAppManager::GetDelegateManager(PathType type)
{
    std::lock_guard<std::mutex> guard(delegateMutex_);
    if (delegateManagers_[type] != nullptr) {
        return delegateManagers_[type];
    }

    std::string directory = GetDataStoragePath(deviceAccountId_, bundleName_, type);
    bool ret = ForceCreateDirectory(directory);
    if (!ret) {
        ZLOGE("create directory[%s] failed, errstr=[%d].", directory.c_str(), errno);
        return nullptr;
    }
    // change mode for directories to 0755, and for files to 0600.
    DirectoryUtils::ChangeModeDirOnly(directory, Constant::DEFAULT_MODE_DIR);
    DirectoryUtils::ChangeModeFileOnly(directory, Constant::DEFAULT_MODE_FILE);

    trueAppId_ = KvStoreUtils::GetAppIdByBundleName(bundleName_);
    if (trueAppId_.empty()) {
        delegateManagers_[type] = nullptr;
        ZLOGW("trueAppId_ empty(permission issues?)");
        return nullptr;
    }

    userId_ = AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId(bundleName_);
    ZLOGD("accountId: %s bundleName: %s", KvStoreUtils::ToBeAnonymous(userId_).c_str(), bundleName_.c_str());
    delegateManagers_[type] = new (std::nothrow) DistributedDB::KvStoreDelegateManager(trueAppId_, userId_);
    if (delegateManagers_[type] == nullptr) {
        ZLOGE("delegateManagers_[%d] is nullptr.", type);
        return nullptr;
    }

    DistributedDB::KvStoreConfig kvStoreConfig;
    kvStoreConfig.dataDir = directory;
    delegateManagers_[type]->SetKvStoreConfig(kvStoreConfig);
    DistributedDB::KvStoreDelegateManager::SetProcessLabel(Constant::PROCESS_LABEL, "default");
    auto communicator = std::make_shared<AppDistributedKv::ProcessCommunicatorImpl>();
    auto result = DistributedDB::KvStoreDelegateManager::SetProcessCommunicator(communicator);
    ZLOGI("app set communicator result:%d.", static_cast<int>(result));
    return delegateManagers_[type];
}

DistributedDB::KvStoreDelegateManager *KvStoreAppManager::SwitchDelegateManager(PathType type,
    DistributedDB::KvStoreDelegateManager *delegateManager)
{
    std::lock_guard<std::mutex> guard(delegateMutex_);
    DistributedDB::KvStoreDelegateManager *oldDelegateManager = delegateManagers_[type];
    delegateManagers_[type] = delegateManager;
    return oldDelegateManager;
}

Status KvStoreAppManager::CloseKvStore(const std::string &storeId, PathType type)
{
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManager[%d] is null.", type);
        return Status::ILLEGAL_STATE;
    }

    auto it = stores_[type].find(storeId);
    if (it != stores_[type].end()) {
        ZLOGD("find store and close delegate.");
        InnerStatus status = it->second->Close(delegateManager);
        if (status == InnerStatus::SUCCESS) {
            stores_[type].erase(it);
            return Status::SUCCESS;
        }
        if (status == InnerStatus::DECREASE_REFCOUNT) {
            return Status::SUCCESS;
        }
        ZLOGE("delegate close error: %d.", static_cast<int>(status));
        return Status::DB_ERROR;
    }

    auto itSingle = singleStores_[type].find(storeId);
    if (itSingle != singleStores_[type].end()) {
        ZLOGD("find single store and close delegate.");
        InnerStatus status = itSingle->second->Close(delegateManager);
        if (status == InnerStatus::SUCCESS) {
            singleStores_[type].erase(itSingle);
            return Status::SUCCESS;
        }
        if (status == InnerStatus::DECREASE_REFCOUNT) {
            return Status::SUCCESS;
        }
        ZLOGE("delegate close error: %d.", static_cast<int>(status));
        return Status::DB_ERROR;
    }

    return Status::STORE_NOT_OPEN;
}

Status KvStoreAppManager::CloseAllKvStore(PathType type)
{
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManager[%d] is null.", type);
        return Status::ILLEGAL_STATE;
    }

    for (auto it = stores_[type].begin(); it != stores_[type].end(); it = stores_[type].erase(it)) {
        KvStoreImpl *currentStore = it->second.GetRefPtr();
        ZLOGI("close kvstore, refcount %d.", it->second->GetSptrRefCount());
        Status status = currentStore->ForceClose(delegateManager);
        if (status != Status::SUCCESS) {
            ZLOGE("delegate close error: %d.", static_cast<int>(status));
            return Status::DB_ERROR;
        }
    }
    stores_[type].clear();

    for (auto it = singleStores_[type].begin(); it != singleStores_[type].end(); it = singleStores_[type].erase(it)) {
        SingleKvStoreImpl *currentStore = it->second.GetRefPtr();
        ZLOGI("close kvstore, refcount %d.", it->second->GetSptrRefCount());
        Status status = currentStore->ForceClose(delegateManager);
        if (status != Status::SUCCESS) {
            ZLOGE("delegate close error: %d.", static_cast<int>(status));
            return Status::DB_ERROR;
        }
    }
    singleStores_[type].clear();
    return Status::SUCCESS;
}

Status KvStoreAppManager::DeleteKvStore(const std::string &storeId, PathType type)
{
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManager[%d] is null.", type);
        return Status::ILLEGAL_STATE;
    }
    std::lock_guard<std::mutex> lg(storeMutex_);
    auto it = stores_[type].find(storeId);
    if (it != stores_[type].end()) {
        Status status = it->second->ForceClose(delegateManager);
        if (status != Status::SUCCESS) {
            return Status::DB_ERROR;
        }
        stores_[type].erase(it);
    }

    auto itSingle = singleStores_[type].find(storeId);
    if (itSingle != singleStores_[type].end()) {
        Status status = itSingle->second->ForceClose(delegateManager);
        if (status != Status::SUCCESS) {
            return Status::DB_ERROR;
        }
        singleStores_[type].erase(itSingle);
    }

    DistributedDB::DBStatus status = delegateManager->DeleteKvStore(storeId);
    if (singleStores_[type].empty() && stores_[type].empty()) {
        SwitchDelegateManager(type, nullptr);
        delete delegateManager;
    }
    return (status != DistributedDB::DBStatus::OK) ? Status::DB_ERROR : Status::SUCCESS;
}

Status KvStoreAppManager::DeleteAllKvStore(PathType type)
{
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManager[%d] is null.", type);
        return Status::ILLEGAL_STATE;
    }

    for (auto it = stores_[type].begin(); it != stores_[type].end(); it = stores_[type].erase(it)) {
        std::string storeId = it->first;
        KvStoreImpl *currentStore = it->second.GetRefPtr();
        Status status = currentStore->ForceClose(delegateManager);
        if (status != Status::SUCCESS) {
            ZLOGE("delegate delete close failed error: %d.", static_cast<int>(status));
            return Status::DB_ERROR;
        }

        ZLOGI("delete kvstore, refcount %d.", it->second->GetSptrRefCount());
        DistributedDB::DBStatus dbStatus = delegateManager->DeleteKvStore(storeId);
        if (dbStatus != DistributedDB::DBStatus::OK) {
            ZLOGE("delegate delete error: %d.", static_cast<int>(dbStatus));
            return Status::DB_ERROR;
        }
    }
    stores_[type].clear();

    for (auto it = singleStores_[type].begin(); it != singleStores_[type].end(); it = singleStores_[type].erase(it)) {
        std::string storeId = it->first;
        SingleKvStoreImpl *currentStore = it->second.GetRefPtr();
        Status status = currentStore->ForceClose(delegateManager);
        if (status != Status::SUCCESS) {
            ZLOGE("delegate delete close failed error: %d.", static_cast<int>(status));
            return Status::DB_ERROR;
        }

        ZLOGI("close kvstore, refcount %d.", it->second->GetSptrRefCount());
        DistributedDB::DBStatus dbStatus = delegateManager->DeleteKvStore(storeId);
        if (dbStatus != DistributedDB::DBStatus::OK) {
            ZLOGE("delegate delete error: %d.", static_cast<int>(dbStatus));
            return Status::DB_ERROR;
        }
    }
    singleStores_[type].clear();
    SwitchDelegateManager(type, nullptr);
    delete delegateManager;
    return Status::SUCCESS;
}

Status KvStoreAppManager::MigrateAllKvStore(const std::string &harmonyAccountId, PathType type)
{
    auto *delegateManager = GetDelegateManager(type);
    if (delegateManager == nullptr) {
        ZLOGE("delegateManager is nullptr.");
        return Status::ILLEGAL_STATE;
    }

    std::string dirPath = GetDataStoragePath(deviceAccountId_, bundleName_, type);
    DistributedDB::KvStoreDelegateManager *newDelegateManager = nullptr;
    Status status = Status::SUCCESS;
    ZLOGI("KvStore migration begin.");
    for (auto &it : stores_[type]) {
        sptr<KvStoreImpl> impl = it.second;
        if (impl->MigrateKvStore(harmonyAccountId, dirPath, delegateManager, newDelegateManager) != Status::SUCCESS) {
            status = Status::MIGRATION_KVSTORE_FAILED;
            ZLOGE("migrate kvstore for appId-%s failed.", bundleName_.c_str());
            // skip this failed, continue to migrate other kvstore.
        }
    }

    ZLOGI("SingleKvStore migration begin.");
    for (auto &it : singleStores_[type]) {
        sptr<SingleKvStoreImpl> impl = it.second;
        if (impl->MigrateKvStore(harmonyAccountId, dirPath, delegateManager, newDelegateManager) != Status::SUCCESS) {
            status = Status::MIGRATION_KVSTORE_FAILED;
            ZLOGE("migrate single kvstore for appId-%s failed.", bundleName_.c_str());
            // skip this failed, continue to migrate other kvstore.
        }
    }

    if (newDelegateManager != nullptr) {
        delegateManager = SwitchDelegateManager(type, newDelegateManager);
        delete delegateManager;
    }
    return status;
}

size_t KvStoreAppManager::GetTotalKvStoreNum() const
{
    size_t total = stores_[PATH_DE].size();
    total += stores_[PATH_CE].size();
    total += singleStores_[PATH_DE].size();
    total += singleStores_[PATH_CE].size();
    return int(total);
};

std::string KvStoreAppManager::GetDataStoragePath(const std::string &deviceAccountId, const std::string &bundleName,
                                                  PathType type)
{
    std::string miscPath = (type == PATH_DE) ? Constant::ROOT_PATH_DE : Constant::ROOT_PATH_CE;
    return Constant::Concatenate({
        miscPath, "/", Constant::SERVICE_NAME, "/", deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(),
        "/", bundleName, "/"
    });
}

DistributedDB::SecurityOption KvStoreAppManager::ConvertSecurity(int securityLevel)
{
    if (securityLevel < SecurityLevel::NO_LABEL || securityLevel > SecurityLevel::S4) {
        return {DistributedDB::NOT_SET, DistributedDB::ECE};
    }
    switch (securityLevel) {
        case SecurityLevel::S3:
            return {DistributedDB::S3, DistributedDB::SECE};
        case SecurityLevel::S4:
            return {DistributedDB::S4, DistributedDB::ECE};
        default:
            return {securityLevel, DistributedDB::ECE};
    }
}

void KvStoreAppManager::Dump(int fd) const
{
    const std::string prefix(8, ' ');
    std::string dePath = GetDataStoragePath(deviceAccountId_, bundleName_, PATH_DE);
    std::string cePath = GetDataStoragePath(deviceAccountId_, bundleName_, PATH_CE);
    size_t singleStoreNum = singleStores_[PATH_DE].size() + singleStores_[PATH_CE].size();
    dprintf(fd, "%s----------------------------------------------------------\n", prefix.c_str());
    dprintf(fd, "%sAppID         : %s\n", prefix.c_str(), trueAppId_.c_str());
    dprintf(fd, "%sBundleName    : %s\n", prefix.c_str(), bundleName_.c_str());
    dprintf(fd, "%sAppDEDirectory: %s\n", prefix.c_str(), dePath.c_str());
    dprintf(fd, "%sAppCEDirectory: %s\n", prefix.c_str(), cePath.c_str());
    dprintf(fd, "%sStore count   : %u\n", prefix.c_str(), static_cast<uint32_t>(singleStoreNum));
    for (const auto &singleStoreMap : singleStores_) {
        for (const auto &pair : singleStoreMap) {
            pair.second->OnDump(fd);
        }
    }
}
}  // namespace DistributedKv
}  // namespace OHOS
