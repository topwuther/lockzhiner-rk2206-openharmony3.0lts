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
#include "store_manager.h"

#include <cstdio>
#include <memory>
#include <string>

#include "doc_db.h"
#include "doc_store.h"
#include "ejdb2.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
constexpr HiLogLabel LABEL = {LOG_CORE, 0xD002D10, "HiView-DOCDB"};
static iwkv_openflags GetEjdbFlag(const Option& option)
{
    switch (option.flag) {
        case Option::RDONLY:
            return IWKV_RDONLY;
        case Option::TRUNC:
            return IWKV_TRUNC;
        default:
            return IWKV_NO_TRIM_ON_CLOSE;
    }
}

std::shared_ptr<DocStore> StoreManager::GetDocStore(const Option& option)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = stores_.find(option.db);
    if (it != stores_.end()) {
        return it->second;
    }

    std::shared_ptr<DocStore> docStore = std::make_shared<DocStore>();
    docStore->dbPtr = std::make_shared<DocDB>();
    if (docStore->dbPtr->OpenDB(option.db, GetEjdbFlag(option)) != 0) {
        HiLog::Error(LABEL, "can not open doc store");
    } else {
        HiLog::Info(LABEL, "open doc store");
    }
    stores_[option.db] = docStore;
    return docStore;
}

int StoreManager::CloseDocStore(const Option& option)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = stores_.find(option.db);
    if (it == stores_.end()) {
        return -1;
    }

    std::shared_ptr<DocStore> docStore = it->second;
    if (docStore->dbPtr == nullptr) {
        return -1;
    }
    if (docStore->dbPtr->CloseDB() != 0) {
        HiLog::Error(LABEL, "can not close doc store");
        return -1;
    }
    return 0;
}

int StoreManager::DeleteDocStore(const Option& option)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int retCode = CloseDocStore(option);
    if (retCode != 0) {
        HiLog::Error(LABEL, "close doc store fail");
        return -1;
    }
    retCode = std::remove(option.db.c_str());
    if (retCode != 0) {
        HiLog::Error(LABEL, "remove doc store fail");
        return -1;
    }

    HiLog::Info(LABEL, "remove doc store success");
    stores_.erase(option.db.c_str());
    return 0;
}
} // HiviewDFX
} // OHOS