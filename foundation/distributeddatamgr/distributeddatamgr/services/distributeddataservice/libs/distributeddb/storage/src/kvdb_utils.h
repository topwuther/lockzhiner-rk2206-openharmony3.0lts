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

#ifndef KVDB_UTILLS_H
#define KVDB_UTILLS_H
#include <string>

#include "types.h"
#include "macro_utils.h"

namespace DistributedDB {
class KvDBUtils final {
public:
    DISABLE_COPY_ASSIGN_MOVE(KvDBUtils);
    ~KvDBUtils() {}

    static void GetStoreDirectory(std::string &directory, const std::string &identifierName);

    static int GetKvDbSize(const std::string &dirAll, const std::string &dirStoreOnly,
        const std::string &dbName, uint64_t &size);

    static int RemoveKvDB(const std::string &dir, const std::string &dbName);
    static int RemoveKvDB(const std::string &dirAll, const std::string &dirStoreOnly, const std::string &dbName);
};
} // namespace DistributedDB
#endif // LOCAL_KVDB_H