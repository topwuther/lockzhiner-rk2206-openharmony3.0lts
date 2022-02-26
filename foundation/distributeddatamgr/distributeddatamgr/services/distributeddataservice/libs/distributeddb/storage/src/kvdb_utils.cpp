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
#include "kvdb_utils.h"

#include "platform_specific.h"
#include "log_print.h"
#include "db_constant.h"
#include "db_errno.h"
#include "db_common.h"
#include "sqlite_utils.h"

namespace DistributedDB {
void KvDBUtils::GetStoreDirectory(std::string &directory, const std::string &identifierName)
{
    if (!directory.empty() && directory.back() != '/') {
        directory += "/";
    }
    directory += identifierName;
    return;
}

int KvDBUtils::RemoveKvDB(const std::string &dirAll, const std::string &dirStoreOnly, const std::string &dbName)
{
    int errCodeAll = KvDBUtils::RemoveKvDB(dirAll, dbName);
    if (errCodeAll != -E_NOT_FOUND && errCodeAll != E_OK) {
        return errCodeAll;
    }
    int errCodeOnlyStore = KvDBUtils::RemoveKvDB(dirStoreOnly, dbName);
    if (errCodeOnlyStore != -E_NOT_FOUND && errCodeOnlyStore != E_OK) {
        return errCodeOnlyStore;
    }
    if ((errCodeAll == -E_NOT_FOUND) && (errCodeOnlyStore == -E_NOT_FOUND)) {
        return -E_NOT_FOUND;
    }
    return E_OK;
}

int KvDBUtils::RemoveKvDB(const std::string &dir, const std::string &dbName)
{
    std::string dbFileName = dir;
    GetStoreDirectory(dbFileName, dbName);
    dbFileName += DBConstant::SQLITE_DB_EXTENSION;
    int errCode = E_OK;
    if (OS::CheckPathExistence(dbFileName)) {
        errCode = DBCommon::RemoveAllFilesOfDirectory(dir, true);
        if (errCode != E_OK) {
            LOGE("Failed to delete the db file! errno[%d], errCode[%d]", errno, errCode);
            return -E_REMOVE_FILE;
        }
    } else {
        errCode = -E_NOT_FOUND;
        LOGD("Db file not existed! errCode[%d]", errCode);
    }
    return errCode;
}

int KvDBUtils::GetKvDbSize(const std::string &dirAll, const std::string &dirStoreOnly,
    const std::string &dbName, uint64_t &size)
{
    int errCodeAll = SQLiteUtils::GetDbSize(dirAll, dbName, size);
    if (errCodeAll != -E_NOT_FOUND && errCodeAll != E_OK) {
        return errCodeAll;
    }

    int errCodeOnlyStore = SQLiteUtils::GetDbSize(dirStoreOnly, dbName, size);
    if (errCodeOnlyStore != -E_NOT_FOUND && errCodeOnlyStore != E_OK) {
        return errCodeOnlyStore;
    }
    if ((errCodeAll == -E_NOT_FOUND) && (errCodeOnlyStore == -E_NOT_FOUND)) {
        return -E_NOT_FOUND;
    }
    return E_OK;
}
} // namespace DistributedDB