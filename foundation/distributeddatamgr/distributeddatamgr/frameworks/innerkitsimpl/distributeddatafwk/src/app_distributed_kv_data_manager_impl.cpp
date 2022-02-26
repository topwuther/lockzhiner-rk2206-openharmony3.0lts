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

#define LOG_TAG "AppDistributedKvDataManagerImpl"

#include "app_distributed_kv_data_manager_impl.h"
#include <directory_ex.h>
#include <unistd.h>
#include <memory>
#include <cinttypes>
#include "app_kvstore_impl.h"
#include "communication_provider.h"
#include "constant.h"
#include "types.h"
#include "log_print.h"
#include "reporter.h"
#include "account_delegate.h"
#include "delegate_mgr_callback.h"
#include "process_communicator_impl.h"
#include "kvstore_utils.h"
namespace OHOS {
namespace AppDistributedKv {
using namespace OHOS::DistributedKv;
const std::string DATABASE_DIR = "distributeddb";
const std::string DEVICE_COLLABORATION_ABBRE = "_DDC";

std::mutex AppDistributedKvDataManagerImpl::storeMutex_;
std::map<std::string, std::shared_ptr<AppDistributedKvDataManager>> AppDistributedKvDataManagerImpl::managers_;

std::shared_ptr<AppDistributedKvDataManager> AppDistributedKvDataManagerImpl::GetInstance(const std::string &bundleName,
                                                                                          const std::string &dataDir,
                                                                                          const std::string &userId)
{
    ZLOGI("start");
    std::lock_guard<std::mutex> lck(storeMutex_);
    auto it = managers_.find(bundleName + userId);
    if (it != managers_.end()) {
        return it->second;
    }

    std::string tempDataDir = dataDir;
    ZLOGD("tempDataDir : %s", tempDataDir.c_str());
    if (!ForceCreateDirectory(tempDataDir)) {
        ZLOGE("create directories %s failed", tempDataDir.c_str());
        FaultMsg msg = {FaultType::SERVICE_FAULT, "device", "GetInstance", Fault::SF_CREATE_DIR};
        Reporter::GetInstance()->ServiceFault()->Report(msg);
        return nullptr;
    }
    // default mode is 0755
    if (!ChangeModeDirectory(tempDataDir, DistributedKv::Constant::DEFAULT_MODE)) {
        return nullptr;
    }

    std::string appId = KvStoreUtils::GetAppIdByBundleName(bundleName);
    if (appId.empty()) {
        appId = bundleName;
    }
    auto status = DistributedDB::KvStoreDelegateManager::SetProcessLabel(appId + DEVICE_COLLABORATION_ABBRE, userId);
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate SetProcessLabel failed: %d.", static_cast<int>(status));
        FaultMsg msg = {FaultType::SERVICE_FAULT, "device", "GetInstance", Fault::SF_PROCESS_LABEL};
        Reporter::GetInstance()->ServiceFault()->Report(msg);
        return nullptr;
    }

    auto communicator = std::make_shared<ProcessCommunicatorImpl>();
    auto commStatus = DistributedDB::KvStoreDelegateManager::SetProcessCommunicator(communicator);
    if (commStatus != DistributedDB::DBStatus::OK) {
        ZLOGW("set distributed db communicator failed.");
        return nullptr;
    }

    auto *delegateManager = new(std::nothrow) DistributedDB::KvStoreDelegateManager(appId, userId);
    if (delegateManager == nullptr) {
        ZLOGW("new kvStoredelegateManager failed");
        return nullptr;
    }
    DistributedDB::KvStoreConfig kvStoreConfig { tempDataDir };
    status = delegateManager->SetKvStoreConfig(kvStoreConfig);
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate SetKvStoreConfig failed: %d.", static_cast<int>(status));
        FaultMsg msg = {FaultType::SERVICE_FAULT, "device", "GetInstance", Fault::SF_DATABASE_CONFIG};
        Reporter::GetInstance()->ServiceFault()->Report(msg);
        delete delegateManager;
        delegateManager = nullptr;
        return nullptr;
    }

    auto temp = std::make_shared<AppDistributedKvDataManagerImpl>(delegateManager, appId);
    if (temp == nullptr) {
        delete delegateManager;
        delegateManager = nullptr;
        ZLOGW("new AppDistributedKvDataManagerImpl failed");
        return nullptr;
    }

    managers_.insert({bundleName + userId, temp});
    ZLOGD("return a new managerSingleton_.");
    return temp;
}

AppDistributedKvDataManagerImpl::AppDistributedKvDataManagerImpl(
    DistributedDB::KvStoreDelegateManager *delegateManager, const std::string &appId)
    : kvStoreDelegateManager_(delegateManager), appId_(appId)
{
    ZLOGI("construct");
}

AppDistributedKvDataManagerImpl::~AppDistributedKvDataManagerImpl()
{
    ZLOGI("destruct");
    delete kvStoreDelegateManager_;
}

Status AppDistributedKvDataManagerImpl::GetKvStore(
    const Options &options, const std::string &storeId,
    const std::function<void(std::unique_ptr<AppKvStore> appKvStore)> &callback)
{
    ZLOGI("start.");
    std::string trimmedStoreId = DistributedKv::Constant::TrimCopy<std::string>(storeId);
    if (trimmedStoreId.size() == 0 || trimmedStoreId.size() > DistributedKv::Constant::MAX_STORE_ID_LENGTH) {
        ZLOGE("storeId is invalid.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreDelegateManager_ == nullptr) {
        ZLOGE("kvStoreDelegateManager_ is nullptr.");
        return Status::ILLEGAL_STATE;
    }
    Status status = Status::ERROR;
    DistributedDB::KvStoreNbDelegate::Option dbOption;
    dbOption.createIfNecessary = options.createIfMissing;
    dbOption.isMemoryDb = !options.persistant;
    dbOption.secOption = ConvertSecurityLevel(options.securityLevel);
    kvStoreDelegateManager_->GetKvStore(
        trimmedStoreId, dbOption,
        [&](DistributedDB::DBStatus dbStatus, DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate) {
            if (dbStatus == DistributedDB::DBStatus::OK && kvStoreNbDelegate != nullptr) {
                status = Status::SUCCESS;
                callback(std::make_unique<AppKvStoreImpl>(trimmedStoreId, kvStoreNbDelegate));
                ZLOGI("succeed.");
                auto statDelegateMgr = std::make_shared<DelegateMgrCallback>(kvStoreDelegateManager_);
                auto statDelegate = std::static_pointer_cast<DbMetaCallbackDelegate>(statDelegateMgr);
                Reporter::GetInstance()->DatabaseStatistic()->Report(
                    {AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId(), appId_, storeId, 0, statDelegate});
                return;
            }

            status = AppKvStoreImpl::ConvertErrorCode(dbStatus);
            if (kvStoreNbDelegate == nullptr) {
                status = Status::STORE_NOT_FOUND;
            }
            ZLOGW("appKvStore return nullptr.");
            callback(nullptr);
            FaultMsg msg = {FaultType::RUNTIME_FAULT, ModuleName::DEVICE, "GetKvStore", Fault::RF_GET_DB};
            Reporter::GetInstance()->ServiceFault()->Report(msg);
        });

    ZLOGI("get status: %d.", static_cast<int>(status));
    return status;
}

Status AppDistributedKvDataManagerImpl::CloseKvStore(std::unique_ptr<AppKvStore> appKvStore)
{
    ZLOGI("start.");
    if (appKvStore == nullptr) {
        ZLOGE("appKvStore is nullptr.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreDelegateManager_ == nullptr) {
        ZLOGE("kvStoreDelegateManager_ is nullptr.");
        return Status::ILLEGAL_STATE;
    }
    return reinterpret_cast<AppKvStoreImpl *>(appKvStore.get())->Close(kvStoreDelegateManager_);
}

Status AppDistributedKvDataManagerImpl::DeleteKvStore(const std::string &storeId)
{
    ZLOGI("start.");
    if (kvStoreDelegateManager_ == nullptr) {
        ZLOGE("kvStoreDelegateManager_ is nullptr.");
        return Status::ILLEGAL_STATE;
    }
    std::string trimmedStoreId = DistributedKv::Constant::TrimCopy<std::string>(storeId);
    if (trimmedStoreId.empty() || trimmedStoreId.size() > DistributedKv::Constant::MAX_STORE_ID_LENGTH) {
        ZLOGW("invalid storeId.");
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::DBStatus status = kvStoreDelegateManager_->DeleteKvStore(trimmedStoreId);
    if (status == DistributedDB::DBStatus::OK) {
        ZLOGI("delete KVStore succeed.");
        return Status::SUCCESS;
    }
    ZLOGE("delete KVStore failed.");
    return Status::ERROR;
}

Status AppDistributedKvDataManagerImpl::GetKvStoreDiskSize(const std::string &storeId, uint64_t &size)
{
    ZLOGI("start");
    if (kvStoreDelegateManager_ == nullptr) {
        ZLOGE("kvStoreDelegateManager_ is nullptr.");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::DBStatus status = kvStoreDelegateManager_->GetKvStoreDiskSize(storeId, size);
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("Failed to getStoreDiskSize, storeID: %s", storeId.c_str());
        return Status::ERROR;
    }
    ZLOGI("end, size:%" PRIu64, size);
    return Status::SUCCESS;
}

Status AppDistributedKvDataManagerImpl::RegisterKvStoreCorruptionObserver(
    const std::shared_ptr<AppKvStoreCorruptionObserver> observer)
{
    ZLOGI("start");
    if (observer == nullptr) {
        ZLOGE("observer is nullptr.");
        return Status::INVALID_ARGUMENT;
    }
    if (corruptionObserver_ != nullptr) {
        return Status::REPEATED_REGISTER;
    }
    if (kvStoreDelegateManager_ == nullptr) {
        return Status::ERROR;
    }
    corruptionObserver_ = observer;
    kvStoreDelegateManager_->SetKvStoreCorruptionHandler([&](const std::string& appId, const std::string& userId,
                                                             const std::string& storeId) {
        corruptionObserver_->OnCorruption(appId, userId, storeId);
    });
    ZLOGD("end");
    return Status::SUCCESS;
}

DistributedDB::SecurityOption AppDistributedKvDataManagerImpl::ConvertSecurityLevel(int securityLevel)
{
    if (securityLevel < SecurityLevel::NO_LABEL || securityLevel > SecurityLevel::S4) {
        return {SecurityLevel::NO_LABEL, DistributedDB::ECE};
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
}  // namespace AppDistributedKv
}  // namespace OHOS
