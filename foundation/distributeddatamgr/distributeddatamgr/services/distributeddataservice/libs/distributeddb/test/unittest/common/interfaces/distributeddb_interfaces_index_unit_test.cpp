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

#ifndef OMIT_JSON
#include <cstdint>
#include <gtest/gtest.h>
#include "sqlite_import.h"
#include "distributeddb_tools_unit_test.h"
#include "query.h"
#include "db_common.h"
#include "db_constant.h"
#include "schema_utils.h"
#include "sqlite_local_kvdb_connection.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    // Directory and delegate related
    string g_testDir;
    KvStoreConfig g_config;
    const string USER_NAME = "TEST0";
    const string APP_NAME = "OHOS";
    KvStoreDelegateManager g_mgr(APP_NAME, USER_NAME);
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    DBStatus g_kvDelegateStatus2 = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr2 = nullptr;
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));
    auto g_kvNbDelegateCallback2 = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus2), std::ref(g_kvNbDelegatePtr2));

    string GetKvStoreDirectory(const string &storeId, int databaseType)
    {
        string identifier = USER_NAME + "-" + APP_NAME + "-" + storeId;
        string hashIdentifierName = DBCommon::TransferHashString(identifier);
        string identifierName = DBCommon::TransferStringToHex(hashIdentifierName);
        string filePath = g_testDir + "/" + identifierName + "/";
        if (databaseType == DBConstant::DB_TYPE_LOCAL) { // local
            filePath += (DBConstant::LOCAL_SUB_DIR + "/" + DBConstant::LOCAL_DATABASE_NAME +
                DBConstant::SQLITE_DB_EXTENSION);
        } else if (databaseType == DBConstant::DB_TYPE_SINGLE_VER) { // single ver
            filePath += (DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::MAINDB_DIR + "/" +
                DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION);
        } else if (databaseType == DBConstant::DB_TYPE_MULTI_VER) { // multi ver
            filePath += (DBConstant::MULTI_SUB_DIR + "/" + DBConstant::MULTI_VER_DATA_STORE +
                DBConstant::SQLITE_DB_EXTENSION);
        } else {
            filePath = "";
        }

        return filePath;
    }

    // Query database schema related info
    const string SQL_QUERY_INDEX = "SELECT COUNT(*) FROM sqlite_master where type = 'index' and name = ";
    int CallbackReturnCount(void *data, int argc, char **argv, char **azColName)
    {
        if (argc == 1) {
            int count = atoi(*(argv));
            if (data != nullptr) {
                int *mid = static_cast<int *>(data);
                *mid = count;
            }
        }
        return 0;
    }

    // Schema and value info related
    FieldName GenerateFieldName(uint32_t serial, uint32_t level, bool fullLength)
    {
        FieldName result = "Serial_";
        result += to_string(serial);
        result += "_Level_";
        result += to_string(level);
        if (fullLength) {
            while (result.size() < SCHEMA_FEILD_NAME_LENGTH_MAX) {
                result.push_back('_');
            }
        }
        return result;
    }

    FieldPath GenerateFieldPath(uint32_t totalLevel, uint32_t serial, bool fullLength)
    {
        FieldPath result;
        for (uint32_t level = 0; level < totalLevel; level++) {
            string fieldName = GenerateFieldName(serial, level, fullLength);
            result.push_back(fieldName);
        }
        return result;
    }

    string GenerateSchemaIndexArray(const vector<FieldPath> &indexAll)
    {
        string result = "[";
        for (auto &entry : indexAll) {
            result += "\"";
            result += SchemaUtils::FieldPathString(entry);
            result += "\",";
        }
        if (!indexAll.empty()) {
            result.pop_back();
        }
        result += "]";
        return result;
    }

    string GenerateEachSchemaDefine(const FieldPath &eachPath)
    {
        string result;
        for (auto iter = eachPath.rbegin(); iter != eachPath.rend(); iter++) {
            if (result.empty()) {
                result = string("\"") + *iter + "\":\"INTEGER\"";
            } else {
                result = string("\"") + *iter + "\":{" + result + "}";
            }
        }
        return result;
    }

    string GenerateSchemaString(const vector<FieldPath> &define, const vector<FieldPath> &index, int skipSize,
        bool hasIndex, bool hasSkipSize)
    {
        string result = "{\"SCHEMA_VERSION\":\"1.0\",\"SCHEMA_MODE\":\"STRICT\",\"SCHEMA_DEFINE\":{";
        for (auto &entry : define) {
            string defineStr = GenerateEachSchemaDefine(entry);
            result += defineStr;
            result += ",";
        }
        if (!define.empty()) {
            result.pop_back();
        }
        result += "}";
        if (hasIndex) {
            result += ",\"SCHEMA_INDEXES\":";
            result += GenerateSchemaIndexArray(index);
        }
        if (hasSkipSize) {
            result += ",\"SCHEMA_SKIPSIZE\":";
            result += to_string(skipSize);
        }
        result += "}";
        return result;
    }

    string GenerateValueItem(const FieldPath &eachPath, int intValue)
    {
        string result;
        for (auto iter = eachPath.rbegin(); iter != eachPath.rend(); iter++) {
            if (result.empty()) {
                result = string("\"") + *iter + "\":" + to_string(intValue);
            } else {
                result = string("\"") + *iter + "\":{" + result + "}";
            }
        }
        return result;
    }
    string GenerateValue(const vector<FieldPath> &define, uint32_t skipSize)
    {
        int intValue = 0;
        string result(skipSize, '*');
        result += "{";
        for (auto &entry : define) {
            string defineStr = GenerateValueItem(entry, intValue++);
            result += defineStr;
            result += ",";
        }
        if (!define.empty()) {
            result.pop_back();
        }
        result += "}";
        return result;
    }

    vector<FieldPath> g_pathGroup1;
    vector<FieldPath> g_pathGroup2;
    vector<string> g_pathStrGroup1;
    vector<string> g_pathStrGroup2;
    vector<FieldPath> g_definePath;
    string g_schemaString1;
    string g_schemaString2;
    string g_valueString1;
    string g_valueString2;

    void ResetGlobalVariable()
    {
        g_pathGroup1.clear();
        g_pathGroup2.clear();
        g_pathStrGroup1.clear();
        g_pathStrGroup2.clear();
        g_definePath.clear();
        g_schemaString1.clear();
        g_schemaString2.clear();
        g_valueString1.clear();
        g_valueString2.clear();
    }

    void PrepareCommonInfo(bool fullLength)
    {
        int serial = 0;
        for (uint32_t level = 1; level <= SCHEMA_FEILD_PATH_DEPTH_MAX; level++) {
            FieldPath path = GenerateFieldPath(level, serial, fullLength);
            string pathStr = SchemaUtils::FieldPathString(path);
            g_pathGroup1.push_back(path);
            g_pathStrGroup1.push_back(pathStr);
            serial++;
        }
        for (uint32_t level = 1; level <= SCHEMA_FEILD_PATH_DEPTH_MAX; level++) {
            FieldPath path = GenerateFieldPath(level, serial, fullLength);
            string pathStr = SchemaUtils::FieldPathString(path);
            g_pathGroup2.push_back(path);
            g_pathStrGroup2.push_back(pathStr);
            serial++;
        }
    }

    inline void CheckIndexFromDbFile(sqlite3 *db, const vector<string> &indexToCheck, int expectCount)
    {
        for (auto &str : indexToCheck) {
            string querySeq = SQL_QUERY_INDEX + "'" + str + "'";
            int count = -1;
            EXPECT_EQ(sqlite3_exec(db, querySeq.c_str(), CallbackReturnCount, &count, nullptr), SQLITE_OK);
            EXPECT_EQ(count, expectCount);
        }
    }
}

class DistributedDBInterfacesIndexUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
};

void DistributedDBInterfacesIndexUnitTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesIndexUnitTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("[TestSuiteTearDown] rm test db files error!");
    }
}

namespace {
    void PrepareInfoForCrudIndex001()
    {
        PrepareCommonInfo(false);
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_definePath.insert(g_definePath.end(), g_pathGroup2.begin(), g_pathGroup2.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_pathGroup1, 0, true, false);
        g_schemaString2 = GenerateSchemaString(g_definePath, g_definePath, 0, true, false);
        LOGI("[PrepareInfoForCrudIndex001] g_schemaString1=%s", g_schemaString1.c_str());
        LOGI("[PrepareInfoForCrudIndex001] g_schemaString2=%s", g_schemaString2.c_str());
    }
}
/**
  * @tc.name: CrudIndex001
  * @tc.desc: Test whether adding index is normal
  * @tc.type: FUNC
  * @tc.require: AR000DR9K8
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CrudIndex001, TestSize.Level1)
{
    PrepareInfoForCrudIndex001();
    sqlite3 *db = nullptr;
    string storeId = "CrudIndex001";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Specify the schema containing all levels of index to open the schema mode database.
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step2. Use the sql statement to get the count of the following index fields that will be added.
     * @tc.expected: step2. count == 0.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup2, 0);
    sqlite3_close(db);
    /**
     * @tc.steps:step3. Close the database.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    /**
     * @tc.steps:step4. The original schema adds the above index fields,
     *                  generates a new schema and opens the database with this schema.
     * @tc.expected: step4. return OK.
     */
    option.schema = g_schemaString2;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step5. Use the sql statement to get the count of the following index fields that are added.
     * @tc.expected: step5. count == 1.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup2, 1);
    sqlite3_close(db);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CrudIndex001"), OK);
    ResetGlobalVariable();
}

namespace {
    void PrepareInfoForCrudIndex002()
    {
        PrepareCommonInfo(false);
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_pathGroup1, 0, true, false);
        g_schemaString2 = GenerateSchemaString(g_definePath, vector<FieldPath>(), 0, true, false);
        LOGI("[PrepareInfoForCrudIndex002] g_schemaString1=%s", g_schemaString1.c_str());
        LOGI("[PrepareInfoForCrudIndex002] g_schemaString2=%s", g_schemaString2.c_str());
    }
}
/**
  * @tc.name: CrudIndex002
  * @tc.desc: Test whether deleting index is normal
  * @tc.type: FUNC
  * @tc.require: AR000DR9K8
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CrudIndex002, TestSize.Level1)
{
    PrepareInfoForCrudIndex002();
    sqlite3 *db = nullptr;
    string storeId = "CrudIndex002";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Specify the schema containing all levels of index to open the schema mode database.
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step2. Use the sql statement to get the count of the following index fields that will be deleted.
     * @tc.expected: step2. count == 1.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup1, 1);
    sqlite3_close(db);
    /**
     * @tc.steps:step3. Close the database.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    /**
     * @tc.steps:step4. The original schema delete the above index fields,
     *                  generates a new schema and opens the database with this schema.
     * @tc.expected: step4. return OK.
     */
    option.schema = g_schemaString2;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step5. Use the sql statement to get the count of the following index fields that are deleted.
     * @tc.expected: step5. count == 0.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup1, 0);
    sqlite3_close(db);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CrudIndex002"), OK);
    ResetGlobalVariable();
}

namespace {
    void PrepareInfoForCrudIndex003()
    {
        PrepareCommonInfo(false);
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_definePath.insert(g_definePath.end(), g_pathGroup2.begin(), g_pathGroup2.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_pathGroup1, 0, true, false);
        g_schemaString2 = GenerateSchemaString(g_definePath, g_pathGroup2, 0, true, false);
        LOGI("[PrepareInfoForCrudIndex003] g_schemaString1=%s", g_schemaString1.c_str());
        LOGI("[PrepareInfoForCrudIndex003] g_schemaString2=%s", g_schemaString2.c_str());
    }
}
/**
  * @tc.name: CrudIndex003
  * @tc.desc: Test whether updating index is normal
  * @tc.type: FUNC
  * @tc.require: AR000DR9K8
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CrudIndex003, TestSize.Level1)
{
    PrepareInfoForCrudIndex003();
    sqlite3 *db = nullptr;
    string storeId = "CrudIndex003";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Specify the schema containing all levels of index to open the schema mode database.
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step2. Use the sql statement to get the count of the following index fields that will be deleted.
     * @tc.expected: step2. count == 1.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup1, 1);
    /**
     * @tc.steps:step3. Use the sql statement to get the count of the following index fields that will be added.
     * @tc.expected: step3. count == 0.
     */
    CheckIndexFromDbFile(db, g_pathStrGroup2, 0);
    sqlite3_close(db);
    /**
     * @tc.steps:step3. Close the database.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    /**
     * @tc.steps:step4. The original schema update the above index fields,
     *                  generates a new schema and opens the database with this schema.
     * @tc.expected: step4. return OK.
     */
    option.schema = g_schemaString2;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step5. Use the sql statement to get the count of the following index fields that are deleted.
     * @tc.expected: step5. count == 0.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup1, 0);
    /**
     * @tc.steps:step5. Use the sql statement to get the count of the following index fields that are added.
     * @tc.expected: step5. count == 1.
     */
    CheckIndexFromDbFile(db, g_pathStrGroup2, 1);
    sqlite3_close(db);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CrudIndex003"), OK);
    ResetGlobalVariable();
}

namespace {
    void PrepareInfoForCreateIndex001()
    {
        PrepareCommonInfo(true);
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_definePath, 8, true, true); // skipsize 8 in schema
        g_valueString1 = GenerateValue(g_definePath, 8); // skipsize 8 in value
        g_valueString2 = GenerateValue(g_definePath, 10); // skipsize 10 in value
        LOGI("[PrepareInfoForCreateIndex001] g_schemaString1=%s", g_schemaString1.c_str());
        LOGI("[PrepareInfoForCreateIndex001] g_valueString1=%s", g_valueString1.c_str());
        LOGI("[PrepareInfoForCreateIndex001] g_valueString2=%s", g_valueString2.c_str());
    }
}
/**
  * @tc.name: CreateIndex001
  * @tc.desc: Test whether the index creation is normal
  * @tc.type: FUNC
  * @tc.require: AR000DR9K9
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CreateIndex001, TestSize.Level1)
{
    PrepareInfoForCreateIndex001();
    sqlite3 *db = nullptr;
    string storeId = "CreateIndex001";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Specify the schema containing all levels of index to open the schema mode database.
     *                  The four-level index has 64 bytes per field.
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step2. Use the sql statement to get each index count count from the schema table;
     * @tc.expected: step2. count == 1.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup1, 1);
    sqlite3_close(db);
    /**
     * @tc.steps:step3. Write a value with 8 prefix bytes and the json part strictly conforms
     *                  to the value of the schema. Call the query interface to query the inserted data.
     * @tc.expected: step3. The insertion is successful and the number of entries obtained by the query is 1.
     */
    Key key001{'1'};
    Value value001(g_valueString1.begin(), g_valueString1.end());
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key001, value001), OK);
    Query query = Query::Select().GreaterThanOrEqualTo(g_pathStrGroup1.front(), 0);
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(query, entries), OK);
    EXPECT_EQ(entries.size(), 1ul);
    /**
     * @tc.steps:step4. Write a value with 10 prefix bytes and the json part strictly conforms
     *                  to the value of the schema.
     * @tc.expected: step4. The insertion is failed.
     */
    Key key002{'2'};
    Value value002(g_valueString2.begin(), g_valueString2.end());
    EXPECT_TRUE(g_kvNbDelegatePtr->Put(key002, value002) != OK);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CreateIndex001"), OK);
    ResetGlobalVariable();
}

namespace {
    void PrepareInfoForCreateIndex002()
    {
        for (uint32_t serial = 0; serial < SCHEMA_INDEX_COUNT_MAX; serial++) {
            FieldPath path = GenerateFieldPath(SCHEMA_FEILD_PATH_DEPTH_MAX, serial, true);
            string pathStr = SchemaUtils::FieldPathString(path);
            g_pathGroup1.push_back(path);
            g_pathStrGroup1.push_back(pathStr);
        }
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_definePath, 0, true, false);
        LOGI("[PrepareInfoForCreateIndex002] g_schemaString1=%s", g_schemaString1.c_str());
    }
}
/**
  * @tc.name: CreateIndex002
  * @tc.desc: Test whether it is possible to insert 32 four-level indexes with each filed being 64.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K9
  * @tc.author: yiguang
  */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CreateIndex002, TestSize.Level1)
{
    PrepareInfoForCreateIndex002();
    sqlite3 *db = nullptr;
    string storeId = "CreateIndex002";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Specifies that a schema with 32 four-level indexes
     *                  with each filed being 64 opens the schema mode database
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step2. Use the sql statement to get each index count count from the schema table;
     * @tc.expected: step2. count == 1.
     */
    EXPECT_EQ(sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr), SQLITE_OK);
    CheckIndexFromDbFile(db, g_pathStrGroup1, 1);
    sqlite3_close(db);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CreateIndex002"), OK);
    ResetGlobalVariable();
}

namespace {
    void PrepareInfoForCheckSchemaSkipsize001()
    {
        PrepareCommonInfo(false);
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_pathGroup1, 0, true, false);
        g_valueString1 = GenerateValue(g_definePath, 0);
        g_valueString2 = GenerateValue(g_definePath, 8); // skipsize 8 in value
        LOGI("[PrepareInfoForCheckSchemaSkipsize001] g_schemaString1=%s", g_schemaString1.c_str());
        LOGI("[PrepareInfoForCheckSchemaSkipsize001] g_valueString1=%s", g_valueString1.c_str());
        LOGI("[PrepareInfoForCheckSchemaSkipsize001] g_valueString2=%s", g_valueString2.c_str());
    }
}
/**
 * @tc.name: Check schema skipsize 001
 * @tc.desc: When SCHEMA_SKIPSIZE is not defined, check if the default is 0
 * @tc.type: FUNC
 * @tc.require: AR000DR9K9
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CheckSchemaSkipsize001, TestSize.Level1)
{
    PrepareInfoForCheckSchemaSkipsize001();
    string storeId = "CheckSchemaSkipsize001";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Specify an undefined skipsize schema to open the schema database.
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    /**
     * @tc.steps:step2. Write a value without prefix and strictly in accordance with the schema.
     * @tc.expected: step2. return OK.
     */
    Key key001{'1'};
    Value value001(g_valueString1.begin(), g_valueString1.end());
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key001, value001), OK);
    /**
     * @tc.steps:step3. Write a value whose prefix is 8 and strictly in accordance with the schema.
     * @tc.expected: step3. return not OK.
     */
    Key key002{'2'};
    Value value002(g_valueString2.begin(), g_valueString2.end());
    EXPECT_TRUE(g_kvNbDelegatePtr->Put(key002, value002) != OK);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CheckSchemaSkipsize001"), OK);
    ResetGlobalVariable();
}

/**
 * @tc.name: Check schema skipsize 002
 * @tc.desc: SCHEMA_SKIPSIZE range is [0,4MB-2]
 * @tc.type: FUNC
 * @tc.require: AR000DR9K9
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CheckSchemaSkipsize002, TestSize.Level1)
{
    PrepareCommonInfo(false);
    string storeId = "CheckSchemaSkipsize002";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Set "SCHEMA_SKIPSIZE" in the schema as -1 to create the schema database.
     * @tc.expected: step1. return not OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(), -1, false, true); // skipsize -1 in schema
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);

    /**
     * @tc.steps:step2. Set "SCHEMA_SKIPSIZE" in the schema as 0 to create the schema database.
     * @tc.expected: step2. return not OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(), 0, false, true);
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CheckSchemaSkipsize002"), OK);

    /**
     * @tc.steps:step3. Set "SCHEMA_SKIPSIZE" in the schema as 8 to create the schema database.
     * @tc.expected: step3. return OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, g_pathGroup1, 8, true, true); // skipsize 8 in schema
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CheckSchemaSkipsize002"), OK);

    /**
     * @tc.steps:step4. Set "SCHEMA_SKIPSIZE" in the schema as 4MB-2 to create the schema database.
     * @tc.expected: step6. return OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(),
        4 * 1024 * 1024 - 2, false, true); // skipsize in schema, 4M - 2, 1024 is scale
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CheckSchemaSkipsize002"), OK);

    /**
     * @tc.steps:step5. Set SCHEMA_SKIPSIZE in the schema as 4MB-1 to create the schema database.
     * @tc.expected: step6. return not OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(),
        4 * 1024 * 1024 - 1, false, true); // skipsize in schema, 4M - 1, 1024 is scale
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);
    // Clear
    ResetGlobalVariable();
}

namespace {
    void PrepareInfoForCheckSchemaSkipsize003()
    {
        PrepareCommonInfo(false);
        g_definePath.insert(g_definePath.end(), g_pathGroup1.begin(), g_pathGroup1.end());
        g_schemaString1 = GenerateSchemaString(g_definePath, g_pathGroup1, 20, true, true); // skipsize 20 in schema
        g_valueString1 = GenerateValue(g_definePath, 19); // skipsize 19 in value
        g_valueString2 = GenerateValue(g_definePath, 20); // skipsize 20 in value
        LOGI("[PrepareInfoForCheckSchemaSkipsize003] g_schemaString1=%s", g_schemaString1.c_str());
        LOGI("[PrepareInfoForCheckSchemaSkipsize003] g_valueString1=%s", g_valueString1.c_str());
        LOGI("[PrepareInfoForCheckSchemaSkipsize003] g_valueString2=%s", g_valueString2.c_str());
    }
}
/**
 * @tc.name: Check schema skipsize 003
 * @tc.desc: When "SCHEMA_SKIPSIZE" is greater than or equal to the size of Value,
 *           the Value verification must fail
 * @tc.type: FUNC
 * @tc.require: AR000DR9K9
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, CheckSchemaSkipsize003, TestSize.Level1)
{
    PrepareInfoForCheckSchemaSkipsize003();
    string storeId = "CheckSchemaSkipsize003";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Set "SCHEMA_SKIPSIZE" in the schema as 20 to create the schema database.
     * @tc.expected: step1. return OK.
     */
    option.schema = g_schemaString1;
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);

    /**
     * @tc.steps:step5. Write a value whose prefix is 19 and strictly in accordance with the schema.
     * @tc.expected: step5. return OK.
     */
    Key key001{'1'};
    Value value001(g_valueString1.begin(), g_valueString1.end());
    EXPECT_TRUE(g_kvNbDelegatePtr->Put(key001, value001) != OK);
    /**
     * @tc.steps:step5. Write a value whose prefix is 20 and strictly in accordance with the schema.
     * @tc.expected: step5. return OK.
     */
    Key key002{'2'};
    Value value002(g_valueString2.begin(), g_valueString2.end());
    EXPECT_TRUE(g_kvNbDelegatePtr->Put(key002, value002) == OK);
    // Clear
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("CheckSchemaSkipsize003"), OK);
    ResetGlobalVariable();
}

/**
 * @tc.name: schema compare with skipsize 004
 * @tc.desc: When the SCHEMA_SKIPSIZE definitions of two Schemas are different,
 *           they will be regarded as inconsistent and incompatible
 * @tc.type: FUNC
 * @tc.require: AR000DR9K9
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBInterfacesIndexUnitTest, SchemaCompareSkipsize004, TestSize.Level1)
{
    PrepareCommonInfo(false);
    string storeId = "SchemaCompareSkipsize004";
    string filePath = GetKvStoreDirectory(storeId, DBConstant::DB_TYPE_SINGLE_VER);
    KvStoreNbDelegate::Option option = {true, false, false};
    /**
     * @tc.steps:step1. Set "SCHEMA_SKIPSIZE" in the schema as 0 to create the schema database.
     * @tc.expected: step1. return OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(), 0, false, true);
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);

    /**
     * @tc.steps:step2. Modify the schema, SCHEMA_SKIPSIZE in the new schema is not defined,
     *                  open the database repeatedly.
     * @tc.expected: step2. return OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(), 0, false, false);
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback2);
    ASSERT_TRUE(g_kvNbDelegatePtr2 != nullptr);
    EXPECT_EQ(g_kvDelegateStatus2, OK);

    /**
     * @tc.steps:step3. Close the database.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr2), OK);

    /**
     * @tc.steps:step4. SCHEMA_SKIPSIZE in the schema is not defined, reopen the database;
     * @tc.expected: step4. return OK.
     */
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);

    /**
     * @tc.steps:step5. Modify the schema, set SCHEMA_SKIPSIZE to 8 in the new schema,
     *                  and open the database repeatedly;
     * @tc.expected: step5. return OK.
     */
    option.schema = GenerateSchemaString(g_pathGroup1, vector<FieldPath>(), 8, false, true); // skipsize 8 in schema
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback2);
    EXPECT_TRUE(g_kvNbDelegatePtr2 == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus2 != OK);

    /**
     * @tc.steps:step6. Close the database.
     * @tc.expected: step6. return OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps:step4. Modify the schema, set SCHEMA_SKIPSIZE to 8 in the new schema, reopen the database;
     * @tc.expected: step4. return OK.
     */
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvNbDelegatePtr2 == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("SchemaCompareSkipsize004"), OK);
    ResetGlobalVariable();
}
#endif