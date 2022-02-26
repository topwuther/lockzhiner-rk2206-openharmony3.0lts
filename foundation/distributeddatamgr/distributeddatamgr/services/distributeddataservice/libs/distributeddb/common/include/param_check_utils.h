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

#ifndef PARAM_CHECK_UTILS_H
#define PARAM_CHECK_UTILS_H

#include <string>

#include "db_types.h"
#include "auto_launch_export.h"
#include "schema_object.h"

namespace DistributedDB {
class ParamCheckUtils final {
public:

    static bool CheckDataDir(const std::string &dir, std::string &canonicalDir);

    // Check if the storeID is a safe arg.
    static bool IsStoreIdSafe(const std::string &storeId);

    // check appId, userId, storeId.
    static bool CheckStoreParameter(const std::string &storeId, const std::string &appId, const std::string &userId);

    // check encrypted args for KvStore.
    static bool CheckEncryptedParameter(CipherType cipher, const CipherPassword &passwd);

    static bool CheckConflictNotifierType(int conflictType);

    static bool CheckSecOption(const SecurityOption &secOption);

    static bool CheckObserver(const Key &key, unsigned int mode);

    static bool IsS3SECEOpt(const SecurityOption &secOpt);

    static int CheckAndTransferAutoLaunchParam(const AutoLaunchParam &param,
        SchemaObject &schemaObject, std::string &canonicalDir);
};
} // namespace DistributedDB

#endif // DISTRIBUTEDDB_PARAM_CHECK_UTILS_H
