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

#include "storage_engine_manager.h"
#include "log_print.h"
#include "db_errno.h"
#include "runtime_context.h"
#include "sqlite_single_ver_storage_engine.h"

namespace DistributedDB {
bool StorageEngineManager::isRegLockStatusListener_ = false;
std::mutex StorageEngineManager::instanceLock_;
StorageEngineManager *StorageEngineManager::instance_ = nullptr;
std::mutex StorageEngineManager::storageEnginesLock_;

namespace {
    std::string GetIdentifier(const KvDBProperties &property)
    {
        return property.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    }

    int GetDatabaseType(const KvDBProperties &property)
    {
        return property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    }
}

StorageEngineManager::StorageEngineManager() : lockStatusListener_(nullptr)
{}

StorageEngineManager::~StorageEngineManager()
{
    if (lockStatusListener_ != nullptr) {
        lockStatusListener_->Drop(true);
    }
}

StorageEngine *StorageEngineManager::GetStorageEngine(const KvDBProperties &property, int &errCode)
{
    StorageEngineManager *manager = GetInstance();
    if (manager == nullptr) {
        LOGE("[StorageEngineManager] GetInstance failed");
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    std::string identifier = GetIdentifier(property);
    manager->EnterGetEngineProcess(identifier);
    auto storageEngine = manager->FindStorageEngine(identifier);
    if (storageEngine == nullptr) {
        storageEngine = manager->CreateStorageEngine(property, errCode);
        if (errCode == E_OK) {
            manager->InsertStorageEngine(identifier, storageEngine);
        }
    } else {
        errCode = storageEngine->CheckEngineOption(property);
        if (errCode != E_OK) {
            LOGE("kvdb property mismatch engine option! errCode = [%d]", errCode);
            return nullptr;
        }
    }

    manager->ExitGetEngineProcess(identifier);
    return storageEngine;
}

int StorageEngineManager::ReleaseStorageEngine(StorageEngine *storageEngine)
{
    if (storageEngine == nullptr) {
        LOGE("[StorageEngineManager] The engine to be released is nullptr");
        return -E_INVALID_ARGS;
    }

    // Clear commit notify callback function.
    storageEngine->SetNotifiedCallback(nullptr);

    // If the cacheDB is valid, the storageEngine is not released to prevent the cache mechanism failed
    bool isRelease = storageEngine->IsNeedTobeReleased();
    if (!isRelease) {
        LOGW("[StorageEngineManager] storageEngine do not need to be released.");
        return E_OK;
    }

    StorageEngineManager *manager = GetInstance();
    if (manager == nullptr) {
        LOGE("[StorageEngineManager] Release GetInstance failed");
        return -E_OUT_OF_MEMORY;
    }

    LOGD("[StorageEngineManager] storageEngine to be released.");
    return manager->ReleaseEngine(storageEngine);
}

int StorageEngineManager::ForceReleaseStorageEngine(const std::string &identifier)
{
    StorageEngineManager *manager = GetInstance();
    if (manager == nullptr) {
        LOGE("[StorageEngineManager] Force release GetInstance failed");
        return -E_OUT_OF_MEMORY;
    }

    LOGD("[StorageEngineManager] Force release engine.");
    manager->ReleaseResources(identifier);
    return E_OK;
}

int StorageEngineManager::ExecuteMigration(StorageEngine *storageEngine)
{
    if (storageEngine == nullptr) {
        LOGE("storage engine is nullptr can not execute migration!");
        return -E_INVALID_ARGS;
    }
    if (storageEngine->IsExistConnection()) {
        return storageEngine->ExecuteMigrate();
    }
    LOGI("connection is not existed, not need execute migration!");
    return -E_INVALID_DB;
}

StorageEngineManager *StorageEngineManager::GetInstance()
{
    std::lock_guard<std::mutex> lockGuard(instanceLock_);
    if (instance_ == nullptr) {
        instance_ = new (std::nothrow) StorageEngineManager();
        if (instance_ == nullptr) {
            LOGE("[StorageEngineManager] Failed to alloc the engine manager!");
            return nullptr;
        }
    }

    if (!isRegLockStatusListener_) {
        int errCode = instance_->RegisterLockStatusListener();
        if (errCode != E_OK) {
            LOGW("[StorageEngineManager] Failed to regitster lock status listener:%d", errCode);
        } else {
            isRegLockStatusListener_ = true;
        }
    }
    return instance_;
}

int StorageEngineManager::RegisterLockStatusListener()
{
    int errCode = E_OK;
    lockStatusListener_ = RuntimeContext::GetInstance()->RegisterLockStatusLister(
        [this](void *lockStatus) {
            if (lockStatus == nullptr) {
                return;
            }
            bool isLocked = *static_cast<bool *>(lockStatus);
            LOGD("[StorageEngineManager] Lock status to %d", isLocked);
            if (isLocked) {
                return;
            }
            int taskErrCode = RuntimeContext::GetInstance()->ScheduleTask(
                std::bind(&StorageEngineManager::LockStatusNotifier, this, isLocked));
            if (taskErrCode != E_OK) {
                LOGE("[StorageEngineManager] LockStatusNotifier ScheduleTask failed : %d", taskErrCode);
            }
        }, errCode);
    if (errCode != E_OK) {
        LOGW("[StorageEngineManager] Failed to register lock status listener: %d.", errCode);
    }
    return errCode;
}

void StorageEngineManager::LockStatusNotifier(bool isAccessControlled)
{
    (void)isAccessControlled;
    std::lock_guard<std::mutex> lockGuard(storageEnginesLock_);
    StorageEngine *storageEngine = nullptr;
    for (const auto &item : storageEngines_) {
        storageEngine = item.second;
        LOGD("Begin to migrate for lock status change");
        (void)ExecuteMigration(storageEngine);
    }
}

void StorageEngineManager::RemoveEngineFromCache(const std::string &identifier)
{
    StorageEngineManager *manager = GetInstance();
    if (manager != nullptr) {
        manager->EraseStorageEngine(identifier);
    }
}

StorageEngine *StorageEngineManager::CreateStorageEngine(const KvDBProperties &property, int &errCode)
{
    int databaseType = GetDatabaseType(property);
    if (databaseType != KvDBProperties::SINGLE_VER_TYPE) {
        LOGE("[StorageEngineManager] Database type error : %d", databaseType);
        errCode = -E_NOT_SUPPORT;
        return nullptr;
    }

    auto storageEngine = new (std::nothrow) SQLiteSingleVerStorageEngine();
    if (storageEngine == nullptr) {
        LOGE("[StorageEngineManager] Create storage engine failed");
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    errCode = E_OK;
    return storageEngine;
}

StorageEngine *StorageEngineManager::FindStorageEngine(const std::string &identifier)
{
    std::lock_guard<std::mutex> lockGuard(storageEnginesLock_);
    auto iter = storageEngines_.find(identifier);
    if (iter != storageEngines_.end()) {
        auto storageEngine = iter->second;
        if (storageEngine == nullptr) {
            LOGE("[StorageEngineManager] storageEngine in cache is nullptr");
            storageEngines_.erase(identifier);
            return nullptr;
        }

        return storageEngine;
    }

    return nullptr;
}

void StorageEngineManager::InsertStorageEngine(const std::string &identifier, StorageEngine *&storageEngine)
{
    std::lock_guard<std::mutex> lockGuard(storageEnginesLock_);
    storageEngines_.insert(std::pair<std::string, StorageEngine *>(identifier, storageEngine));
}

void StorageEngineManager::EraseStorageEngine(const std::string &identifier)
{
    std::lock_guard<std::mutex> lockGuard(storageEnginesLock_);
    storageEngines_.erase(identifier);
}

void StorageEngineManager::ReleaseResources(const std::string &identifier)
{
    StorageEngine *storageEngine = nullptr;

    {
        std::lock_guard<std::mutex> lockGuard(storageEnginesLock_);
        auto iter = storageEngines_.find(identifier);
        if (iter != storageEngines_.end()) {
            storageEngine = iter->second;
            storageEngines_.erase(identifier);
        }
    }

    if (storageEngine != nullptr) {
        LOGI("[StorageEngineManager] Release storage engine");
        delete storageEngine;
        storageEngine = nullptr;
    }

    return;
}

int StorageEngineManager::ReleaseEngine(StorageEngine *releaseEngine)
{
    const std::string identifier = releaseEngine->GetIdentifier();
    StorageEngine *cacheEngine = nullptr;

    {
        std::lock_guard<std::mutex> lockGuard(storageEnginesLock_);
        auto iter = storageEngines_.find(identifier);
        if (iter != storageEngines_.end()) {
            cacheEngine = iter->second;
            storageEngines_.erase(identifier);
        }
    }

    if (cacheEngine == nullptr) {
        LOGE("[StorageEngineManager] cache engine is null");
        return -E_ALREADY_RELEASE;
    }
    if (cacheEngine != releaseEngine) {
        LOGE("[StorageEngineManager] cache engine is not equal the input engine");
        return -E_INVALID_ARGS;
    }

    delete releaseEngine;
    releaseEngine = nullptr;
    return E_OK;
}

void StorageEngineManager::EnterGetEngineProcess(const std::string &identifier)
{
    std::unique_lock<std::mutex> lock(getEngineMutex_);
    getEngineCondition_.wait(lock, [this, &identifier]() {
        return this->getEngineSet_.count(identifier) == 0;
    });
    (void)getEngineSet_.insert(identifier);
}

void StorageEngineManager::ExitGetEngineProcess(const std::string &identifier)
{
    std::unique_lock<std::mutex> lock(getEngineMutex_);
    (void)getEngineSet_.erase(identifier);
    getEngineCondition_.notify_all();
}
} // namespace DistributedDB
