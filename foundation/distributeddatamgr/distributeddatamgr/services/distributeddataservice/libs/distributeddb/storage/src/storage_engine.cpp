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

#include "storage_engine.h"

#include <algorithm>

#include "db_common.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
const int StorageEngine::MAX_WAIT_TIME = 30;
const int StorageEngine::MAX_WRITE_SIZE = 1;
const int StorageEngine::MAX_READ_SIZE = 16;

StorageEngine::StorageEngine()
    : isUpdated_(false),
      engineState_(EngineState::INVALID),
      commitNotifyFunc_(nullptr),
      isInitialized_(false),
      perm_(OperatePerm::NORMAL_PERM),
      operateAbort_(false),
      isExistConnection_(false)
{}

StorageEngine::~StorageEngine()
{
    Release();
}

int StorageEngine::Init()
{
    if (isInitialized_) {
        LOGD("Storage engine has been initialized!");
        return E_OK;
    }

    // only for create the database avoid the minimum number is 0.
    int errCode = E_OK;
    StorageExecutor *handle = nullptr;
    if (engineAttr_.minReadNum == 0 && engineAttr_.minWriteNum == 0) {
        errCode = CreateNewExecutor(true, handle);
        if (errCode != E_OK) {
            goto ERROR;
        }

        if (handle != nullptr) {
            delete handle;
            handle = nullptr;
        }
    }

    for (uint32_t i = 0; i < engineAttr_.minWriteNum; i++) {
        handle = nullptr;
        errCode = CreateNewExecutor(true, handle);
        if (errCode != E_OK) {
            goto ERROR;
        }
        AddStorageExecutor(handle);
    }

    for (uint32_t i = 0; i < engineAttr_.minReadNum; i++) {
        handle = nullptr;
        errCode = CreateNewExecutor(false, handle);
        if (errCode != E_OK) {
            goto ERROR;
        }
        AddStorageExecutor(handle);
    }
    isInitialized_ = true;

ERROR:
    if (errCode != E_OK) {
        // Assumed file system has classification function, can only get one write handle
        if (errCode == -E_EKEYREVOKED && !writeIdleList_.empty()) {
            return E_OK;
        }
        Release();
    }
    return errCode;
}

StorageExecutor *StorageEngine::FindExecutor(bool writable, OperatePerm perm, int &errCode, int waitTime)
{
    if (GetEngineState() == EngineState::ENGINE_BUSY) {
        LOGI("Storage engine is busy!");
        errCode = -E_BUSY;
        return nullptr;
    }

    if (writable) {
        return FindWriteExecutor(perm, errCode, waitTime);
    }

    return FindReadExecutor(perm, errCode, waitTime);
}

StorageExecutor *StorageEngine::FindWriteExecutor(OperatePerm perm, int &errCode, int waitTime)
{
    std::unique_lock<std::mutex> lock(writeMutex_);
    errCode = -E_BUSY;
    if (perm_ == OperatePerm::DISABLE_PERM || perm_ != perm) {
        LOGI("Not permitted to get the executor[%d]", perm_);
        return nullptr;
    }
    if (waitTime <= 0) { // non-blocking.
        if (writeIdleList_.empty() &&
            writeIdleList_.size() + writeUsingList_.size() == engineAttr_.maxWriteNum) {
            return nullptr;
        }
        return FetchStorageExecutor(true, writeIdleList_, writeUsingList_, errCode);
    }

    // Not prohibited and there is an available handle
    bool result = writeCondition_.wait_for(lock, std::chrono::seconds(waitTime),
        [this, &perm]() {
            return (perm_ == OperatePerm::NORMAL_PERM || perm_ == perm) && (!writeIdleList_.empty() ||
                (writeIdleList_.size() + writeUsingList_.size() < engineAttr_.maxWriteNum) ||
                operateAbort_);
        });
    if (operateAbort_) {
        LOGI("Abort write executor and executor and busy for operate!");
        return nullptr;
    }
    if (!result) {
        LOGI("Get write handle result[%d], permissType[%d], operType[%d], write[%d-%d-%d]",
            result, perm_, perm, writeIdleList_.size(), writeUsingList_.size(), engineAttr_.maxWriteNum);
        return nullptr;
    }
    return FetchStorageExecutor(true, writeIdleList_, writeUsingList_, errCode);
}

StorageExecutor *StorageEngine::FindReadExecutor(OperatePerm perm, int &errCode, int waitTime)
{
    std::unique_lock<std::mutex> lock(readMutex_);
    errCode = -E_BUSY;
    if (perm_ == OperatePerm::DISABLE_PERM || perm_ != perm) {
        LOGI("Not permitted to get the executor[%d]", perm_);
        return nullptr;
    }

    if (waitTime <= 0) { // non-blocking.
        if (readIdleList_.empty() &&
            readIdleList_.size() + readUsingList_.size() == engineAttr_.maxReadNum) {
            return nullptr;
        }
        return FetchStorageExecutor(false, readIdleList_, readUsingList_, errCode);
    }

    // Not prohibited and there is an available handle
    bool result = readCondition_.wait_for(lock, std::chrono::seconds(waitTime),
        [this, &perm]() {
            return (perm_ == OperatePerm::NORMAL_PERM || perm_ == perm) &&
                (!readIdleList_.empty() || (readIdleList_.size() + readUsingList_.size() < engineAttr_.maxReadNum) ||
                operateAbort_);
        });
    if (operateAbort_) {
        LOGI("Abort find read executor and busy for operate!");
        return nullptr;
    }
    if (!result) {
        LOGI("Get read handle result[%d], permissType[%d], operType[%d], read[%d-%d-%d]",
            result, perm_, perm, readIdleList_.size(), readUsingList_.size(), engineAttr_.maxReadNum);
        return nullptr;
    }
    return FetchStorageExecutor(false, readIdleList_, readUsingList_, errCode);
}

void StorageEngine::Recycle(StorageExecutor *&handle)
{
    if (handle == nullptr) {
        return;
    }
    std::string id = DBCommon::TransferStringToHex(identifier_);
    LOGD("Recycle executor[%d] for id[%.6s]", handle->GetWritable(), id.c_str());
    if (handle->GetWritable()) {
        std::unique_lock<std::mutex> lock(writeMutex_);
        auto iter = std::find(writeUsingList_.begin(), writeUsingList_.end(), handle);
        if (iter != writeUsingList_.end()) {
            writeUsingList_.remove(handle);
            if (writeIdleList_.size() >= 1) {
                delete handle;
                handle = nullptr;
                return;
            }
            handle->Reset();
            writeIdleList_.push_back(handle);
            writeCondition_.notify_one();
        }
    } else {
        std::unique_lock<std::mutex> lock(readMutex_);
        auto iter = std::find(readUsingList_.begin(), readUsingList_.end(), handle);
        if (iter != readUsingList_.end()) {
            readUsingList_.remove(handle);
            if (readIdleList_.size() >= 1) {
                delete handle;
                handle = nullptr;
                return;
            }
            handle->Reset();
            readIdleList_.push_back(handle);
            readCondition_.notify_one();
        }
    }
    handle = nullptr;
}

void StorageEngine::ClearCorruptedFlag()
{
    return;
}

void StorageEngine::Release()
{
    CloseExecutor();
    isInitialized_ = false;
    isUpdated_ = false;
    ClearCorruptedFlag();
    SetEngineState(EngineState::INVALID);
}

int StorageEngine::TryToDisable(bool isNeedCheckAll, OperatePerm disableType)
{
    if (engineState_ != EngineState::MAINDB && engineState_ != EngineState::INVALID) {
        LOGE("Not support disable handle when cacheDB existed! state = [%d]", engineState_);
        return(engineState_ == EngineState::CACHEDB) ? -E_NOT_SUPPORT : -E_BUSY;
    }

    std::lock(writeMutex_, readMutex_);
    std::lock_guard<std::mutex> writeLock(writeMutex_, std::adopt_lock);
    std::lock_guard<std::mutex> readLock(readMutex_, std::adopt_lock);

    if (!isNeedCheckAll) {
        goto END;
    }

    if (!writeUsingList_.empty() || !readUsingList_.empty()) {
        LOGE("Database handle used");
        return -E_BUSY;
    }
END:
    if (perm_ == OperatePerm::NORMAL_PERM) {
        LOGI("database is disable for re-build:%d", static_cast<int>(disableType));
        perm_ = disableType;
        writeCondition_.notify_all();
        readCondition_.notify_all();
    }
    return E_OK;
}

void StorageEngine::Enable(OperatePerm enableType)
{
    std::lock(writeMutex_, readMutex_);
    std::lock_guard<std::mutex> writeLock(writeMutex_, std::adopt_lock);
    std::lock_guard<std::mutex> readLock(readMutex_, std::adopt_lock);
    if (perm_ == enableType) {
        LOGI("Re-enable the database");
        perm_ = OperatePerm::NORMAL_PERM;
        writeCondition_.notify_all();
        readCondition_.notify_all();
    }
}

void StorageEngine::Abort(OperatePerm enableType)
{
    std::lock(writeMutex_, readMutex_);
    std::lock_guard<std::mutex> writeLock(writeMutex_, std::adopt_lock);
    std::lock_guard<std::mutex> readLock(readMutex_, std::adopt_lock);
    if (perm_ == enableType) {
        LOGI("Abort the handle occupy, release all!");
        perm_ = OperatePerm::NORMAL_PERM;
        operateAbort_ = true;

        writeCondition_.notify_all();
        readCondition_.notify_all();
    }
}

bool StorageEngine::IsNeedTobeReleased() const
{
    return true;
}

const std::string &StorageEngine::GetIdentifier() const
{
    return identifier_;
}

EngineState StorageEngine::GetEngineState() const
{
    return engineState_;
}

void StorageEngine::SetEngineState(EngineState state)
{
    LOGI("Set storage engine state to [%d]!", state);
    engineState_ = state;
}

bool StorageEngine::IsNeedMigrate() const
{
    LOGI("No need to migrate!");
    return false;
}

int StorageEngine::ExecuteMigrate()
{
    LOGW("Migration is not supported!");
    return -E_NOT_SUPPORT;
}

void StorageEngine::SetNotifiedCallback(const std::function<void(int, KvDBCommitNotifyFilterAbleData *)> &callback)
{
    std::unique_lock<std::shared_mutex> lock(notifyMutex_);
    commitNotifyFunc_ = callback;
    return;
}

void StorageEngine::SetConnectionFlag(bool isExisted)
{
    return isExistConnection_.store(isExisted);
}

bool StorageEngine::IsExistConnection() const
{
    return isExistConnection_.load();
}

void StorageEngine::ClearEnginePasswd()
{
    return;
}

int StorageEngine::CheckEngineOption(const KvDBProperties &kvdbOption) const
{
    return E_OK;
}

void StorageEngine::AddStorageExecutor(StorageExecutor *handle)
{
    if (handle == nullptr) {
        return;
    }

    if (handle->GetWritable()) {
        writeIdleList_.push_back(handle);
    } else {
        readIdleList_.push_back(handle);
    }
}

void StorageEngine::CloseExecutor()
{
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        for (auto &item : writeIdleList_) {
            if (item != nullptr) {
                delete item;
                item = nullptr;
            }
        }
        writeIdleList_.clear();
    }

    {
        std::lock_guard<std::mutex> lock(readMutex_);
        for (auto &item : readIdleList_) {
            if (item != nullptr) {
                delete item;
                item = nullptr;
            }
        }
        readIdleList_.clear();
    }
}

StorageExecutor *StorageEngine::FetchStorageExecutor(bool isWrite, std::list<StorageExecutor *> &idleList,
    std::list<StorageExecutor *> &usingList, int &errCode)
{
    if (idleList.empty()) {
        StorageExecutor *handle = nullptr;
        errCode = CreateNewExecutor(isWrite, handle);
        if ((errCode != E_OK) || (handle == nullptr)) {
            if (errCode == -E_EKEYREVOKED) {
                LOGE("Key revoked status, couldn't create the new executor");
                if (!usingList.empty()) {
                    LOGE("Can't create new executor for revoked");
                    errCode = -E_BUSY;
                }
            }
            return nullptr;
        }

        AddStorageExecutor(handle);
    }
    auto item = idleList.front();
    usingList.push_back(item);
    idleList.remove(item);
    std::string id = DBCommon::TransferStringToHex(identifier_);
    LOGD("Get executor from [%.6s], write[%d], using[%d], idle[%d]",
        id.c_str(), isWrite, usingList.size(), idleList.size());
    errCode = E_OK;
    return item;
}

bool StorageEngine::CheckEngineAttr(const StorageEngineAttr &poolSize)
{
    return (poolSize.maxReadNum > MAX_READ_SIZE ||
            poolSize.maxWriteNum > MAX_WRITE_SIZE ||
            poolSize.minReadNum > poolSize.maxReadNum ||
            poolSize.minWriteNum > poolSize.maxWriteNum);
}
}
