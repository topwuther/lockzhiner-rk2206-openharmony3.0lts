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

#include "kv_store_delegate_manager.h"
#include "kv_store_nb_delegate.h"
#include "distributeddb_tools_unit_test.h"
#include "sqlite_utils.h"
#include "sqlite_single_ver_natural_store.h"
#include "db_errno.h"
#include "log_print.h"
#include "db_common.h"
#include "db_constant.h"
#include "time_helper.h"
#include "platform_specific.h"
#include "iprocess_system_api_adapter.h"
#include "process_system_api_adapter_impl.h"
#include "runtime_context.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr("app0", "user0");
    enum ForkConcurrentStatus : int {
        NOT_RUN = 0,
        RUNNING,
        FINISHED,
    };
    string g_testDir;
    KvStoreConfig g_config;
    string g_identifier;
    string g_databaseName;
    string g_newDatabaseName;
    string g_localdatabaseName;
    Value g_origValue = {'c', 'e'};
    string g_flag = "2";
    CipherPassword g_passwd;
    int g_forkconcurrent = ForkConcurrentStatus::NOT_RUN;
    static std::shared_ptr<ProcessSystemApiAdapterImpl> g_adapter;
    string g_maindbPath;
    string g_metadbPath;
    string g_cachedbPath;
    DBStatus g_valueStatus = INVALID_ARGS;
    Value g_value;
    auto g_valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(g_valueStatus), std::ref(g_value));

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    const std::vector<std::string> ORIG_DATABASE_V1 = {
        "CREATE TABLE IF NOT EXISTS local_data(key BLOB PRIMARY KEY NOT NULL, value BLOB);",
        "CREATE TABLE IF NOT EXISTS sync_data(key BLOB NOT NULL, value BLOB, timestamp INT NOT NULL," \
            "flag INT NOT NULL, device BLOB, ori_device BLOB, hash_key BLOB PRIMARY KEY NOT NULL);",
        "CREATE TABLE IF NOT EXISTS meta_data(key BLOB PRIMARY KEY NOT NULL, value BLOB);",
        "CREATE INDEX IF NOT EXISTS key_index ON sync_data (key);",
        "CREATE INDEX IF NOT EXISTS time_index ON sync_data (timestamp);",
        "CREATE INDEX IF NOT EXISTS dev_index ON sync_data (device);",
        "PRAGMA user_version=101;"
    };

    const std::vector<std::string> ORIG_DATABASE_V2 = {
        "CREATE TABLE IF NOT EXISTS local_data(key BLOB PRIMARY KEY NOT NULL," \
            "value BLOB, timestamp INT, hash_key BLOB);",
        "CREATE TABLE IF NOT EXISTS sync_data(key BLOB NOT NULL, value BLOB, timestamp INT NOT NULL," \
            "flag INT NOT NULL, device BLOB, ori_device BLOB, hash_key BLOB PRIMARY KEY NOT NULL, w_timestamp INT);",
        "CREATE TABLE IF NOT EXISTS meta_data(key BLOB PRIMARY KEY NOT NULL, value BLOB);",
        "CREATE INDEX IF NOT EXISTS key_index ON sync_data (key, flag);",
        "CREATE INDEX IF NOT EXISTS time_index ON sync_data (timestamp);",
        "CREATE INDEX IF NOT EXISTS dev_index ON sync_data (device);",
        "CREATE INDEX IF NOT EXISTS local_hashkey_index ON local_data (hash_key);",
        "PRAGMA user_version=102;"
    };

    const std::vector<std::string> ORIG_DATABASE_V3 = {
        "CREATE TABLE IF NOT EXISTS local_data(key BLOB PRIMARY KEY NOT NULL, value BLOB, " \
            "timestamp INT, hash_key BLOB);",
        "CREATE TABLE IF NOT EXISTS sync_data(key BLOB NOT NULL, value BLOB, timestamp INT NOT NULL," \
            "flag INT NOT NULL, device BLOB, ori_device BLOB, hash_key BLOB PRIMARY KEY NOT NULL, w_timestamp INT);",
        "CREATE INDEX IF NOT EXISTS key_index ON sync_data (key, flag);",
        "CREATE INDEX IF NOT EXISTS time_index ON sync_data (timestamp);",
        "CREATE INDEX IF NOT EXISTS dev_index ON sync_data (device);",
        "CREATE INDEX IF NOT EXISTS local_hashkey_index ON local_data (hash_key);",
        "PRAGMA user_version=103;"
    };

    const std::vector<std::string> INSERT_DATA_V1 = {
        "INSERT INTO sync_data VALUES('ab', 'cd', 100, 2, '', '', 'efdef');" \
    };

    const std::string INSERT_LOCAL_DATA_V1 = {
        "INSERT INTO local_data VALUES(?, 'ce');"
    };

    const std::vector<std::string> INSERT_DATA_V2 = {
        "INSERT INTO sync_data VALUES('ab', 'cd', 100, " + g_flag + ", '', '', 'efdef', 100);"
    };

    const std::string INSERT_LOCAL_DATA_V2 = {
        "INSERT INTO local_data VALUES(?, 'ce',3169633545069981070,'efdef');"
    };

    const std::string INSERT_META_DATA_V2 = {
        "INSERT INTO meta_data VALUES('ab', 'ce');"
    };

    const std::string CHECK_V1_SYNC_UPGRADE =
        "SELECT w_timestamp, timestamp FROM sync_data;";

    const std::string CHECK_V2_SYNC_UPGRADE =
        "SELECT flag FROM sync_data;";

    const std::string CHECK_V1_LOCAL_UPGRADE =
        "SELECT timestamp, hash_key FROM local_data;";

    void KvStoreNbDelegateCallback(
        DBStatus statusSrc, KvStoreNbDelegate *kvStoreSrc, DBStatus &statusDst, KvStoreNbDelegate *&kvStoreDst)
    {
        statusDst = statusSrc;
        kvStoreDst = kvStoreSrc;
    }

    auto g_kvNbDelegateCallback = bind(&KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));

    void CreateDatabase(const std::vector<std::string> &insertSqls, const std::string &insertLocalDataSql,
        const OpenDbProperties &property)
    {
        sqlite3 *db = nullptr;
        EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
        ASSERT_NE(db, nullptr);

        for (const auto &item : insertSqls) {
            ASSERT_EQ(SQLiteUtils::ExecuteRawSQL(db, item), E_OK);
        }
        sqlite3_stmt *statement = nullptr;
        ASSERT_EQ(SQLiteUtils::GetStatement(db, insertLocalDataSql, statement), E_OK);
        ASSERT_NE(statement, nullptr);
        EXPECT_EQ(SQLiteUtils::BindBlobToStatement(statement, 1, g_origValue, false), E_OK);
        EXPECT_EQ(SQLiteUtils::StepWithRetry(statement, false), -SQLITE_DONE);
        EXPECT_EQ(sqlite3_finalize(statement), SQLITE_OK);

        (void)sqlite3_close_v2(db);
    }

    void CheckSyncDataV1ToV2(sqlite3 *db)
    {
        sqlite3_stmt *statement = nullptr;
        ASSERT_EQ(SQLiteUtils::GetStatement(db, CHECK_V1_SYNC_UPGRADE, statement), E_OK);
        ASSERT_NE(statement, nullptr);
        ASSERT_EQ(SQLiteUtils::StepWithRetry(statement), SQLiteUtils::MapSQLiteErrno(SQLITE_ROW));
        ASSERT_EQ(sqlite3_column_int64(statement, 0), sqlite3_column_int64(statement, 1));
        ASSERT_EQ(sqlite3_finalize(statement), SQLITE_OK);
    }

    void CheckSyncDataV2ToV3(sqlite3 *db)
    {
        sqlite3_stmt *statement = nullptr;
        ASSERT_EQ(SQLiteUtils::GetStatement(db, CHECK_V2_SYNC_UPGRADE, statement), E_OK);
        ASSERT_NE(statement, nullptr);
        ASSERT_EQ(SQLiteUtils::StepWithRetry(statement), SQLiteUtils::MapSQLiteErrno(SQLITE_ROW));
        long int targetFlagValue = sqlite3_column_int64(statement, 0);
        ASSERT_EQ(targetFlagValue, stol(g_flag));
        ASSERT_EQ(sqlite3_finalize(statement), SQLITE_OK);
    }

    void CheckLocalDataV1ToV2(sqlite3 *db)
    {
        sqlite3_stmt *statement = nullptr;
        ASSERT_EQ(SQLiteUtils::GetStatement(db, CHECK_V1_LOCAL_UPGRADE, statement), E_OK);
        ASSERT_NE(statement, nullptr);
        ASSERT_EQ(SQLiteUtils::StepWithRetry(statement), SQLiteUtils::MapSQLiteErrno(SQLITE_ROW));
        TimeStamp stamp = static_cast<uint64_t>(sqlite3_column_int64(statement, 0));
        EXPECT_NE(stamp, 0UL);

        Value readHashValue;
        Value calcValue;
        EXPECT_EQ(DBCommon::CalcValueHash(g_origValue, calcValue), E_OK);
        ASSERT_EQ(SQLiteUtils::GetColumnBlobValue(statement, 1, readHashValue), E_OK);
        EXPECT_EQ(readHashValue, calcValue);
        ASSERT_EQ(sqlite3_finalize(statement), SQLITE_OK);
    }

    void CheckDirectoryV2ToV3(bool expectedValue, bool expecteMetaDbExist)
    {
        std::string identifier = DBCommon::TransferStringToHex(g_identifier);
        std::string newDatabaseName = g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
            DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE + ".db";
        std::string newMetadatabaseName = g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
            DBConstant::METADB_DIR + "/" + DBConstant::SINGLE_VER_META_STORE + ".db";
        std::string newCacheDirectory = g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
            DBConstant::CACHEDB_DIR + "/";
        EXPECT_EQ(OS::CheckPathExistence(newDatabaseName), expectedValue);
        EXPECT_EQ(OS::CheckPathExistence(newMetadatabaseName), expecteMetaDbExist);
        EXPECT_EQ(OS::CheckPathExistence(newCacheDirectory), expectedValue);
    }

    void CheckVersionV3(sqlite3 *db)
    {
        int version = SOFTWARE_VERSION_BASE;
        SQLiteUtils::GetVersion(db, version);
        EXPECT_EQ(version, SINGLE_VER_STORE_VERSION_CURRENT);
    }

    void CheckSecOpt(const SecurityOption &currentSecOpt)
    {
        SecurityOption checkSecOpt;
        SecurityOption currentMetaSecOpt {SecurityLabel::S2, SecurityFlag::ECE};
        int errCode = RuntimeContext::GetInstance()->GetSecurityOption(g_maindbPath, checkSecOpt);
        EXPECT_TRUE(currentSecOpt == checkSecOpt);
        EXPECT_TRUE(errCode == E_OK);
        if (OS::CheckPathExistence(g_cachedbPath)) {
            errCode = RuntimeContext::GetInstance()->GetSecurityOption(g_cachedbPath, checkSecOpt);
            EXPECT_TRUE(currentSecOpt == checkSecOpt);
            EXPECT_TRUE(errCode == E_OK);
        }
        if (OS::CheckPathExistence(g_metadbPath)) {
            errCode = RuntimeContext::GetInstance()->GetSecurityOption(g_metadbPath, checkSecOpt);
            EXPECT_TRUE(currentMetaSecOpt == checkSecOpt);
            EXPECT_TRUE(errCode == E_OK);
        }
    }

    void GetKvStoreProcess(const KvStoreNbDelegate::Option &option, bool putCheck, bool secOptCheck,
        const SecurityOption &secopt)
    {
        Key keyTmp = {'1'};
        Value valueRead;
        Value value = {'7'};
        g_mgr.GetKvStore("TestUpgradeNb", option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        if (secOptCheck == true) {
            CheckSecOpt(secopt);
        }
        if (putCheck == true) {
            EXPECT_TRUE(g_kvNbDelegatePtr->Put(keyTmp, value) == OK);
            EXPECT_TRUE(g_kvNbDelegatePtr->Get(keyTmp, valueRead) == OK);
        }
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;
    }
}

class DistributedDBStorageSingleVerUpgradeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageSingleVerUpgradeTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    std::string oriIdentifier = "user0-app0-TestUpgradeNb";
    g_identifier = DBCommon::TransferHashString(oriIdentifier);
    std::string identifier = DBCommon::TransferStringToHex(g_identifier);
    g_databaseName = "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
        DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    g_newDatabaseName = "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::MAINDB_DIR + "/" +
        DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    g_localdatabaseName = "/" + identifier + "/" + DBConstant::LOCAL_SUB_DIR + "/" +
        DBConstant::LOCAL_DATABASE_NAME + DBConstant::SQLITE_DB_EXTENSION;
    const int passwdLen = 5;
    const int passwdVal = 1;
    vector<uint8_t> passwdBuffer1(passwdLen, passwdVal);
    int errCode = g_passwd.SetValue(passwdBuffer1.data(), passwdBuffer1.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    g_adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    g_maindbPath = g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::MAINDB_DIR +
        "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    g_metadbPath = g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::METADB_DIR +
        "/" + DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    g_cachedbPath = g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::CACHEDB_DIR +
        "/" + DBConstant::SINGLE_VER_CACHE_STORE + DBConstant::SQLITE_DB_EXTENSION;
}

void DistributedDBStorageSingleVerUpgradeTest::TearDownTestCase(void)
{
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

void DistributedDBStorageSingleVerUpgradeTest::SetUp(void)
{
    std::string identifier = DBCommon::TransferStringToHex(g_identifier);
    DBCommon::CreateDirectory(g_testDir + "/" + identifier);
    DBCommon::CreateDirectory(g_testDir + "/" + identifier + "/" + DBConstant::SINGLE_SUB_DIR);
    DBCommon::CreateDirectory(g_testDir + "/" + identifier + "/" + DBConstant::LOCAL_SUB_DIR);
}

void DistributedDBStorageSingleVerUpgradeTest::TearDown(void)
{
    while (g_forkconcurrent == ForkConcurrentStatus::RUNNING) {
        sleep(1);
    }
    g_adapter->ResetSecOptDic();
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

/**
  * @tc.name: UpgradeTest001
  * @tc.desc: Test the NbDelegate upgrade from the old version V1.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ7
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSingleVerUpgradeTest, UpgradeTest001, TestSize.Level2)
{
    /**
     * @tc.steps:step1. create old version V1 db.
     */
    std::string dbPath = g_testDir + g_databaseName;
    OpenDbProperties property = {dbPath, true, false, ORIG_DATABASE_V1};
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    SecurityOption secopt{SecurityLabel::S3, SecurityFlag::SECE};
    CreateDatabase(INSERT_DATA_V1, INSERT_LOCAL_DATA_V1, property);
    bool isDatabaseExists = OS::CheckPathExistence(dbPath);
    EXPECT_EQ(isDatabaseExists, true);
    /**
     * @tc.steps:step2. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.secOption = secopt;
    GetKvStoreProcess(option, true, true, SecurityOption());

    sqlite3 *db = nullptr;
    dbPath = g_testDir + g_newDatabaseName;
    property = {dbPath, true, false};
    EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
    ASSERT_NE(db, nullptr);
    /**
     * @tc.steps:step3. check result is ok.
     * @tc.expected: dir is ok,version is ok.
     */
    CheckLocalDataV1ToV2(db);
    CheckSyncDataV1ToV2(db);
    CheckDirectoryV2ToV3(true, false);
    CheckVersionV3(db);
    (void)sqlite3_close_v2(db);
    EXPECT_EQ(g_mgr.DeleteKvStore("TestUpgradeNb"), OK);
}

/**
  * @tc.name: UpgradeTest002
  * @tc.desc: Test the NbDelegate upgrade from the old version V2.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ7
  * @tc.author: zhuwentao
  */
HWTEST_F(DistributedDBStorageSingleVerUpgradeTest, UpgradeTest002, TestSize.Level2)
{
    /**
     * @tc.steps:step1. create old version V2 db.
     */
    std::string dbPath = g_testDir + g_databaseName;
    OpenDbProperties property = {dbPath, true, false, ORIG_DATABASE_V2};
    CreateDatabase(INSERT_DATA_V2, INSERT_LOCAL_DATA_V2, property);
    SecurityOption secopt{SecurityLabel::S3, SecurityFlag::SECE};
    bool isDatabaseExists = OS::CheckPathExistence(dbPath);
    EXPECT_EQ(isDatabaseExists, true);
    /**
     * @tc.steps:step2. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.secOption = secopt;
    GetKvStoreProcess(option, true, true, SecurityOption());

    sqlite3 *db = nullptr;
    dbPath = g_testDir + g_newDatabaseName;
    property = {dbPath, true, false};
    EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
    ASSERT_NE(db, nullptr);
    /**
     * @tc.steps:step3. check result is ok.
     * @tc.expected: dir is ok,version is ok.
     */
    CheckDirectoryV2ToV3(true, false);
    CheckVersionV3(db);
    (void)sqlite3_close_v2(db);
    GetKvStoreProcess(option, false, true, SecurityOption());
    EXPECT_EQ(g_mgr.DeleteKvStore("TestUpgradeNb"), OK);
}
#ifndef OMIT_JSON
/**
  * @tc.name: UpgradeTest003
  * @tc.desc: Test the NbDelegate upgrade from the old version V2 with schema.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ7
  * @tc.author: zhuwentao
  */
HWTEST_F(DistributedDBStorageSingleVerUpgradeTest, UpgradeTest003, TestSize.Level2)
{
    /**
     * @tc.steps:step1. create old version V2 db.
     */
    std::string dbPath = g_testDir + g_databaseName;
    OpenDbProperties property = {dbPath, true, false, ORIG_DATABASE_V2};
    std::string val = "{\"field_name1\":true, \"field_name2\":{\"field_name3\":1, \"field_name4\":1, \"field_name5\":1,\
        \"field_name6\":\"1\", \"field_name7\":null, \"field_name8\":null}}";
    std::string insertValueSql = "INSERT INTO sync_data VALUES('ab', '";
    insertValueSql += val;
    insertValueSql += "', 100, " + g_flag + ", '', '', 'efdef', 100);";
    CreateDatabase(std::vector<std::string> {insertValueSql}, INSERT_LOCAL_DATA_V2, property);
    SecurityOption secopt{SecurityLabel::S3, SecurityFlag::SECE};
    bool isDatabaseExists = OS::CheckPathExistence(dbPath);
    EXPECT_EQ(isDatabaseExists, true);
    /**
     * @tc.steps:step2. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.secOption = secopt;
    option.schema = "{\"SCHEMA_VERSION\":\"1.0\","
                    "\"SCHEMA_MODE\":\"STRICT\","
                    "\"SCHEMA_DEFINE\":{"
                        "\"field_name1\":\"BOOL\","
                        "\"field_name2\":{"
                            "\"field_name3\":\"INTEGER, NOT NULL\","
                            "\"field_name4\":\"LONG, DEFAULT 100\","
                            "\"field_name5\":\"DOUBLE, NOT NULL, DEFAULT 3.14\","
                            "\"field_name6\":\"STRING, NOT NULL, DEFAULT '3.1415'\","
                            "\"field_name7\":[],"
                            "\"field_name8\":{}"
                        "}"
                    "},"
                    "\"SCHEMA_INDEXES\":[\"$.field_name1\", \"$.field_name2.field_name6\"]}";
    GetKvStoreProcess(option, false, true, SecurityOption());

    sqlite3 *db = nullptr;
    dbPath = g_testDir + g_newDatabaseName;
    property = {dbPath, true, false};
    EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
    ASSERT_NE(db, nullptr);
    /**
     * @tc.steps:step3. check result is ok.
     * @tc.expected: dir is ok,version is ok.
     */
    CheckDirectoryV2ToV3(true, false);
    CheckVersionV3(db);
    (void)sqlite3_close_v2(db);

    GetKvStoreProcess(option, false, true, SecurityOption());
    EXPECT_EQ(g_mgr.DeleteKvStore("TestUpgradeNb"), OK);
}
#endif
/**
  * @tc.name: UpgradeTest004
  * @tc.desc: Test the NbDelegate upgrade from the old version V2 while secOption from NOT_SET to S3SECE.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ7
  * @tc.author: zhuwentao
  */
HWTEST_F(DistributedDBStorageSingleVerUpgradeTest, UpgradeTest004, TestSize.Level2)
{
    /**
     * @tc.steps:step1. create old version V2 db.
     */
    std::string dbPath = g_testDir + g_databaseName;
    OpenDbProperties property = {dbPath, true, false, ORIG_DATABASE_V2};
    CreateDatabase(INSERT_DATA_V2, INSERT_LOCAL_DATA_V2, property);
    SecurityOption secopt{SecurityLabel::S3, SecurityFlag::SECE};
    SecurityOption checkSecOpt;
    bool isDatabaseExists = OS::CheckPathExistence(dbPath);
    EXPECT_EQ(isDatabaseExists, true);
    /**
     * @tc.steps:step2. Get the nb delegate without secoption and Get the nb delegate again with secoption.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    GetKvStoreProcess(option, false, true, SecurityOption());
    RuntimeContext::GetInstance()->GetSecurityOption(g_maindbPath, checkSecOpt);
    EXPECT_TRUE(checkSecOpt.securityLabel == NOT_SET);

    option.secOption = secopt;
    GetKvStoreProcess(option, true, true, SecurityOption());

    sqlite3 *db = nullptr;
    dbPath = g_testDir + g_newDatabaseName;
    property = {dbPath, true, false};
    EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
    ASSERT_NE(db, nullptr);
    /**
     * @tc.steps:step3. check result is ok.
     * @tc.expected: dir is ok,version is ok.
     */
    CheckDirectoryV2ToV3(true, false);
    CheckVersionV3(db);
    (void)sqlite3_close_v2(db);

    GetKvStoreProcess(option, false, false, secopt);
    EXPECT_EQ(g_mgr.DeleteKvStore("TestUpgradeNb"), OK);
}

HWTEST_F(DistributedDBStorageSingleVerUpgradeTest, UpgradeTest005, TestSize.Level2)
{
    /**
     * @tc.steps:step1. create old version V2 db.
     */
    std::string dbPath = g_testDir + g_databaseName;
    OpenDbProperties property = {dbPath, true, false, ORIG_DATABASE_V2};
    CreateDatabase(INSERT_DATA_V2, INSERT_LOCAL_DATA_V2, property);
    SecurityOption secopt = {SecurityLabel::S2, SecurityFlag::ECE};
    bool isDatabaseExists = OS::CheckPathExistence(dbPath);
    EXPECT_EQ(isDatabaseExists, true);
    /**
     * @tc.steps:step2. Get the nb delegate while not sprite meta_db scene
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.secOption = secopt;
    GetKvStoreProcess(option, true, true, SecurityOption());

    sqlite3 *db = nullptr;
    dbPath = g_testDir + g_newDatabaseName;
    property = {dbPath, true, false};
    EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
    ASSERT_NE(db, nullptr);
    /**
     * @tc.steps:step3. check result is ok.
     * @tc.expected: dir is ok,version is ok.
     */
    CheckSyncDataV2ToV3(db);
    CheckDirectoryV2ToV3(true, false);
    CheckVersionV3(db);
    (void)sqlite3_close_v2(db);
    EXPECT_EQ(g_mgr.DeleteKvStore("TestUpgradeNb"), OK);
}

HWTEST_F(DistributedDBStorageSingleVerUpgradeTest, UpgradeTest006, TestSize.Level2)
{
    /**
     * @tc.steps:step1. create old version V2 db.
     */
    std::string dbPath = g_testDir + g_databaseName;
    OpenDbProperties property = {dbPath, true, false, ORIG_DATABASE_V2};
    CreateDatabase(INSERT_DATA_V2, INSERT_LOCAL_DATA_V2, property);
    SecurityOption secopt = {SecurityLabel::S3, SecurityFlag::ECE};
    bool isDatabaseExists = OS::CheckPathExistence(dbPath);
    EXPECT_EQ(isDatabaseExists, true);
    /**
     * @tc.steps:step2. Get the nb delegate while not sprite meta_db scene
     * @tc.expected: step2. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    option.secOption = secopt;
    GetKvStoreProcess(option, true, true, SecurityOption());

    sqlite3 *db = nullptr;
    dbPath = g_testDir + g_newDatabaseName;
    property = {dbPath, true, false};
    EXPECT_EQ(SQLiteUtils::OpenDatabase(property, db), E_OK);
    ASSERT_NE(db, nullptr);
    /**
     * @tc.steps:step3. check result is ok.
     * @tc.expected: dir is ok,version is ok.
     */
    CheckSyncDataV2ToV3(db);
    CheckDirectoryV2ToV3(true, false);
    CheckVersionV3(db);
    (void)sqlite3_close_v2(db);
    EXPECT_EQ(g_mgr.DeleteKvStore("TestUpgradeNb"), OK);
}
