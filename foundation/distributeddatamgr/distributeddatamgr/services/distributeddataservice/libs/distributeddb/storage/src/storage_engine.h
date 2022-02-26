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

#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include <condition_variable>
#include <list>
#include <mutex>
#include <shared_mutex>

#include "db_types.h"
#include "macro_utils.h"
#include "storage_executor.h"
#include "kvdb_commit_notify_filterable_data.h"

namespace DistributedDB {
struct StorageEngineAttr {
    uint32_t minWriteNum = 1;
    uint32_t maxWriteNum = 1;
    uint32_t minReadNum = 1;
    uint32_t maxReadNum = 1;
};

class StorageEngine {
public:
    StorageEngine();
    virtual ~StorageEngine();

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(StorageEngine);

    int Init();

    StorageExecutor *FindExecutor(bool writable, OperatePerm perm, int &errCode, int waitTime = MAX_WAIT_TIME);

    void Recycle(StorageExecutor *&handle);

    void Release();

    int TryToDisable(bool isNeedCheckAll, OperatePerm disableType = OperatePerm::DISABLE_PERM);

    void Enable(OperatePerm enableType = OperatePerm::NORMAL_PERM);

    void Abort(OperatePerm enableType = OperatePerm::NORMAL_PERM);

    virtual bool IsNeedTobeReleased() const;

    virtual const std::string &GetIdentifier() const;

    virtual EngineState GetEngineState() const;

    virtual void SetEngineState(EngineState state);

    virtual bool IsNeedMigrate() const;

    virtual int ExecuteMigrate();

    virtual void SetNotifiedCallback(const std::function<void(int, KvDBCommitNotifyFilterAbleData *)> &callback);

    void SetConnectionFlag(bool isExisted);

    bool IsExistConnection() const;

    virtual void ClearEnginePasswd();

    virtual int CheckEngineOption(const KvDBProperties &kvdbOption) const;

protected:
    virtual int CreateNewExecutor(bool isWrite, StorageExecutor *&handle) = 0;

    void CloseExecutor();

    virtual void AddStorageExecutor(StorageExecutor *handle);

    static bool CheckEngineAttr(const StorageEngineAttr &poolSize);

    StorageEngineAttr engineAttr_;
    bool isUpdated_;
    std::string identifier_;
    EngineState engineState_;
    // Mutex for commitNotifyFunc_.
    mutable std::shared_mutex notifyMutex_;
    // Callback function for commit notify.
    std::function<void(int, KvDBCommitNotifyFilterAbleData *)> commitNotifyFunc_;

private:
    StorageExecutor *FetchStorageExecutor(bool isWrite, std::list<StorageExecutor *> &idleList,
        std::list<StorageExecutor *> &usingList, int &errCode);

    StorageExecutor *FindWriteExecutor(OperatePerm perm, int &errCode, int waitTime);
    StorageExecutor *FindReadExecutor(OperatePerm perm, int &errCode, int waitTime);

    virtual void ClearCorruptedFlag();

    static const int MAX_WAIT_TIME;
    static const int MAX_WRITE_SIZE;
    static const int MAX_READ_SIZE;

    bool isInitialized_;
    OperatePerm perm_;
    bool operateAbort_;

    std::mutex readMutex_;
    std::mutex writeMutex_;
    std::condition_variable writeCondition_;
    std::condition_variable readCondition_;
    std::list<StorageExecutor *> writeUsingList_;
    std::list<StorageExecutor *> writeIdleList_;
    std::list<StorageExecutor *> readUsingList_;
    std::list<StorageExecutor *> readIdleList_;
    std::atomic<bool> isExistConnection_;
};
} // namespace DistributedDB
#endif // STORAGE_ENGINE_H
