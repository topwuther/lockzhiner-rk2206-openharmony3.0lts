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

#ifndef DISTRIBUTEDDB_COMMON_H
#define DISTRIBUTEDDB_COMMON_H

#include <list>
#include <string>
#include "db_types.h"
#include "types.h"
#include "kvdb_properties.h"

namespace DistributedDB {
class DBCommon final {
public:
    static int CreateDirectory(const std::string &directory);

    static void StringToVector(const std::string &src, std::vector<uint8_t> &dst);
    static void VectorToString(const std::vector<uint8_t> &src, std::string &dst);

    static std::string VectorToHexString(const std::vector<uint8_t> &inVec, const std::string &separator = "");

    static void PrintHexVector(const std::vector<uint8_t> &data, int line = 0, const std::string &tag = "");

    static std::string TransferStringToHex(const std::string &origStr);

    static std::string TransferHashString(const std::string &devName);

    static int CalcValueHash(const std::vector<uint8_t> &Value, std::vector<uint8_t> &hashValue);

    static int CreateStoreDirectory(const std::string &directory, const std::string &identifierName,
        const std::string &subDir, bool isCreate);

    static int CopyFile(const std::string &srcFile, const std::string &dstFile);

    static int RemoveAllFilesOfDirectory(const std::string &dir, bool isNeedRemoveDir = true);

    static std::string GenerateIdentifierId(const std::string &storeId,
        const std::string &appId, const std::string &userId);

    static void SetDatabaseIds(KvDBProperties &properties, const std::string &appId, const std::string &userId,
        const std::string &storeId);
};

// Define short macro substitute for original long expression for convenience of using
#define VEC_TO_STR(x) DBCommon::VectorToHexString(x).c_str()
} // namespace DistributedDB

#endif // DISTRIBUTEDDB_COMMON_H
