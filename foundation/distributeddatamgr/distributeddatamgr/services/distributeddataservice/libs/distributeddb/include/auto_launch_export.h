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

#ifndef DISTRIBUTEDDB_AUTO_LAUNCH_EXPORT_H
#define DISTRIBUTEDDB_AUTO_LAUNCH_EXPORT_H

#include "types_export.h"
#include "kv_store_observer.h"
#include "kv_store_nb_delegate.h"

namespace DistributedDB {
struct AutoLaunchOption {
    bool createIfNecessary = true;
    bool isEncryptedDb = false;
    CipherType cipher = CipherType::DEFAULT;
    CipherPassword passwd;
    std::string schema;
    bool createDirByStoreIdOnly = false;
    std::string dataDir;
    KvStoreObserver *observer = nullptr;
    int conflictType = 0;
    KvStoreNbConflictNotifier notifier;
    SecurityOption secOption;
};

struct AutoLaunchParam {
    std::string userId;
    std::string appId;
    std::string storeId;
    AutoLaunchOption option;
    AutoLaunchNotifier notifier;
};

using AutoLaunchRequestCallback = std::function<bool (const std::string &identifier, AutoLaunchParam &param)>;
} // namespace DistributedDB

#endif // DISTRIBUTEDDB_AUTO_LAUNCH_EXPORT_H
