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

#ifndef STORAGE_ENGINE_MANAGER_H
#define STORAGE_ENGINE_MANAGER_H

#include <string>
#include <map>
#include <mutex>
#include <set>
#include <condition_variable>

#include "storage_engine.h"

namespace DistributedDB {
class StorageEngineManager final {
public:
    static StorageEngine *GetStorageEngine(const KvDBProperties &property, int &errCode);

    static int ReleaseStorageEngine(StorageEngine *storageEngine);

    static int ForceReleaseStorageEngine(const std::string &identifier);

    static int ExecuteMigration(StorageEngine *storageEngine);

    DISABLE_COPY_ASSIGN_MOVE(StorageEngineManager);

private:
    StorageEngineManager();
    ~StorageEngineManager();

    // Get a StorageEngineManager instance, Singleton  mode
    static StorageEngineManager *GetInstance();

    int RegisterLockStatusListener();

    void LockStatusNotifier(bool isAccessControlled);

    void RemoveEngineFromCache(const std::string &identifier);

    StorageEngine *CreateStorageEngine(const KvDBProperties &property, int &errCode);

    StorageEngine *FindStorageEngine(const std::string &identifier);

    void InsertStorageEngine(const std::string &identifier, StorageEngine *&storageEngine);

    void EraseStorageEngine(const std::string &identifier);

    void ReleaseResources(const std::string &identifier);

    int ReleaseEngine(StorageEngine *releaseEngine);

    void EnterGetEngineProcess(const std::string &identifier);

    void ExitGetEngineProcess(const std::string &identifier);

    static std::mutex instanceLock_;
    static StorageEngineManager *instance_;
    static bool isRegLockStatusListener_;

    static std::mutex storageEnginesLock_;
    std::map<std::string, StorageEngine *> storageEngines_;

    std::mutex getEngineMutex_;
    std::condition_variable getEngineCondition_;
    std::set<std::string> getEngineSet_;

    NotificationChain::Listener *lockStatusListener_;
};
} // namespace DistributedDB

#endif // STORAGE_ENGINE_MANAGER_H
