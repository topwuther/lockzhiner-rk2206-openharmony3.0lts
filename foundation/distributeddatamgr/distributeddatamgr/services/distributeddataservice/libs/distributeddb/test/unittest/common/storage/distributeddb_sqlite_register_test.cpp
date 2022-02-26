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

#include <gtest/gtest.h>

#include "sqlite_import.h"
#include "platform_specific.h"
#include "distributeddb_tools_unit_test.h"
#include "sqlite_local_kvdb_connection.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    char *g_errMsg = nullptr;
    sqlite3 *g_sqliteDb = nullptr;

    const char *DB_NAME = "test.db";
    const char *SQL_HASH = "SELECT CALC_HASH_KEY(KEY) FROM ADDRESS_TEST";
#ifndef OMIT_JSON
    const char *SQL_JSON_RIGHT = "SELECT * FROM ADDRESS_TEST \
        WHERE JSON_EXTRACT_BY_PATH(VALUE, '$.population', 0) > 800000";
    const char *SQL_JSON_WRONG_PATH = "SELECT * FROM ADDRESS_TEST \
        WHERE JSON_EXTRACT_BY_PATH(VALUE, '.populationWrong', 0) > 800000";
    const char *SQL_JSON_WRONG_ARGS = "SELECT * FROM ADDRESS_TEST \
        WHERE JSON_EXTRACT_BY_PATH(VALUE, '$.population') > 800000";
#endif
    const char *SQL_CREATE_TABLE = "CREATE TABLE IF NOT EXISTS ADDRESS_TEST("  \
        "KEY    BLOB    NOT NULL    PRIMARY KEY,"  \
        "VALUE  BLOB    NOT NULL);";

    const char *SQL_INSERT = "INSERT INTO ADDRESS_TEST (KEY, VALUE)"  \
        "VALUES ('1', '{\"province\":\"hunan\", \"city\":\"shaoyang\", \"population\":1000000}');" \
        "INSERT INTO ADDRESS_TEST (KEY, VALUE)"  \
        "VALUES ('12', '{\"province\":\"jiangsu\", \"city\":\"nanjing\", \"population\":3500000}');" \
        "INSERT INTO ADDRESS_TEST (KEY, VALUE)"  \
        "VALUES ('123', '{\"province\":\"guangdong\", \"city\":\"shenzhen\", \"population\":700000}');";

#ifndef OMIT_JSON
    int Callback(void *data, int argc, char **argv, char **azColName)
    {
        for (int i = 0; i < argc; i++) {
            LOGI("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        return 0;
    }
#endif
}

class DistributedDBSqliteRegisterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBSqliteRegisterTest::SetUpTestCase(void)
{
    DistributedDB::OS::RemoveFile(DB_NAME);
    int errCode = sqlite3_open(DB_NAME, &g_sqliteDb);
    ASSERT_EQ(errCode, SQLITE_OK);
    errCode = SQLiteUtils::RegisterJsonFunctions(g_sqliteDb);
    ASSERT_EQ(errCode, SQLITE_OK);

    errCode = sqlite3_exec(g_sqliteDb, SQL_CREATE_TABLE, nullptr, nullptr, &g_errMsg);
    if (errCode != SQLITE_OK) {
        if (g_errMsg != nullptr) {
            LOGE("SQL error: %s\n", g_errMsg);
            sqlite3_free(g_errMsg);
            g_errMsg = nullptr;
        }
    }
    ASSERT_EQ(errCode, SQLITE_OK);

    errCode = sqlite3_exec(g_sqliteDb, SQL_INSERT, nullptr, nullptr, &g_errMsg);
    if (errCode != SQLITE_OK) {
        if (g_errMsg != nullptr) {
            LOGE("SQL error: %s\n", g_errMsg);
            sqlite3_free(g_errMsg);
            g_errMsg = nullptr;
        }
    }
    ASSERT_EQ(errCode, SQLITE_OK);
}

void DistributedDBSqliteRegisterTest::TearDownTestCase(void)
{
    if (g_sqliteDb != nullptr) {
        sqlite3_close(g_sqliteDb);
    }
    if (g_errMsg != nullptr) {
        sqlite3_free(g_errMsg);
        g_errMsg = nullptr;
    }
}

void DistributedDBSqliteRegisterTest::SetUp()
{
}

void DistributedDBSqliteRegisterTest::TearDown()
{
}
#ifndef OMIT_JSON
/**
  * @tc.name: JsonExtract001
  * @tc.desc: test json_extract_by_path function in sqlite
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBSqliteRegisterTest, JsonExtract001, TestSize.Level1)
{
    ASSERT_NE(g_sqliteDb, nullptr);
    int errCode = sqlite3_exec(g_sqliteDb, SQL_JSON_RIGHT, Callback, nullptr, &g_errMsg);
    if (errCode != SQLITE_OK) {
        if (g_errMsg != nullptr) {
            LOGE("SQL error: %s\n", g_errMsg);
            sqlite3_free(g_errMsg);
            g_errMsg = nullptr;
        }
    }
    ASSERT_EQ(errCode, SQLITE_OK);
}

/**
  * @tc.name: JsonExtract002
  * @tc.desc: test json_extract_by_path function in sqlite
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBSqliteRegisterTest, JsonExtract002, TestSize.Level1)
{
    ASSERT_NE(g_sqliteDb, nullptr);
    if (g_sqliteDb == nullptr) {
        LOGE("Sqlite DB not exists.");
        return;
    }
    int errCode = sqlite3_exec(g_sqliteDb, SQL_JSON_WRONG_PATH, Callback, nullptr, &g_errMsg);
    if (errCode != SQLITE_OK) {
        if (g_errMsg != nullptr) {
            LOGE("SQL error: %s\n", g_errMsg);
            sqlite3_free(g_errMsg);
            g_errMsg = nullptr;
        }
    }
    ASSERT_TRUE(errCode != SQLITE_OK);
}

/**
  * @tc.name: JsonExtract003
  * @tc.desc: test json_extract_by_path function in sqlite
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBSqliteRegisterTest, JsonExtract003, TestSize.Level1)
{
    ASSERT_NE(g_sqliteDb, nullptr);
    int errCode = sqlite3_exec(g_sqliteDb, SQL_JSON_WRONG_ARGS, Callback, nullptr, &g_errMsg);
    if (errCode != SQLITE_OK) {
        if (g_errMsg != nullptr) {
            LOGE("SQL error: %s\n", g_errMsg);
            sqlite3_free(g_errMsg);
            g_errMsg = nullptr;
        }
    }
    ASSERT_NE(errCode, SQLITE_OK);
}
#endif
/**
  * @tc.name: CalcHashValue001
  * @tc.desc: test calc_hash_key function in sqlite
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBSqliteRegisterTest, CalcHashValue001, TestSize.Level1)
{
    ASSERT_NE(g_sqliteDb, nullptr);
    int errCode = sqlite3_exec(g_sqliteDb, SQL_HASH, nullptr, nullptr, &g_errMsg);
    if (errCode != SQLITE_OK) {
        if (g_errMsg != nullptr) {
            LOGE("SQL error: %s\n", g_errMsg);
            sqlite3_free(g_errMsg);
            g_errMsg = nullptr;
        }
    }
    ASSERT_EQ(errCode, SQLITE_OK);
}
