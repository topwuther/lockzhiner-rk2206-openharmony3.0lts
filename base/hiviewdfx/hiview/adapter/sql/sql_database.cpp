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
#include "sql_database.h"

#include "logger.h"
#include "sql_operator.h"
#include "sql_wrapper.h"
using namespace std;

namespace sql {
DEFINE_LOG_TAG("HiView-SqlDatabase");

SqlDatabase::SqlDatabase(void)
{
    db = new Sqlite3Db();
    db->rawdb = nullptr;
    dbStatus = SQLITE_ERROR;
    Close();
}

SqlDatabase::~SqlDatabase(void)
{
    Close();
    delete db;
    db = nullptr;
}

const Sqlite3Db& SqlDatabase::GetHandle()
{
    return *db;
}

void SqlDatabase::Close()
{
    if (db->rawdb) {
        sqlite3_close(db->rawdb);
        db->rawdb = nullptr;
        errMsg.clear();
        dbStatus = SQLITE_ERROR;
    }
}

bool SqlDatabase::IsOpen()
{
    return (dbStatus == SQLITE_OK);
}

bool SqlDatabase::Open(std::string fileName)
{
    Close();
    sqlite3_config(SQLITE_CONFIG_SMALL_MALLOC, true); // set small malloc config
    sqlite3_soft_heap_limit64(100 * 1024); // set soft heap limit as 100 * 1024B = 100KB
    dbStatus = sqlite3_open(fileName.c_str(), &(db->rawdb));
    if (IsOpen()) {
        // set cache size as 10 pages
        int retCode = sqlite3_exec(db->rawdb, "PRAGMA cache_size=10;", nullptr, nullptr, nullptr);
        if (retCode != SQLITE_OK) {
            HIVIEW_LOGE("Failed(%d) to set cache size by %s", retCode, fileName.c_str());
        }

        // set normal synchronous
        retCode = sqlite3_exec(db->rawdb, "PRAGMA synchronous=1;", nullptr, nullptr, nullptr);
        if (retCode != SQLITE_OK) {
            HIVIEW_LOGE("Failed(%d) to set synchronous by %s", retCode, fileName.c_str());
        }
        return true;
    } else {
        errMsg = sqlite3_errmsg(db->rawdb);
        HIVIEW_LOGI("open db:%s failed, error msg is :%s", fileName.c_str(), errMsg.c_str());
        return false;
    }
}

bool SqlDatabase::BeginTransaction()
{
    SqlOperator sqlOperator(*db);
    return sqlOperator.ExecuteNoCallback("BEGIN TRANSACTION");
}

bool SqlDatabase::CommitTransaction()
{
    SqlOperator sqlOperator(*db);
    return sqlOperator.ExecuteNoCallback("COMMIT TRANSACTION");
}
};  // namespace sql
