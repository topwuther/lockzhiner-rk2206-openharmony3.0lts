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
#include <fstream>

#include <unistd.h>
#include "db_common.h"
#include "platform_specific.h"
#include "kvdb_manager.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "runtime_context.h"
#include "process_system_api_adapter_impl.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    enum {
        SCHEMA_TYPE1 = 1,
        SCHEMA_TYPE2
    };
    static int g_conflictCount = 0;
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;

    // define the g_kvDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));
#ifndef OMIT_JSON
    const int PASSWD_LEN = 10;
    const int PASSWD_VAL = 45;
    void GenerateValidSchemaString(std::string &string, int num = SCHEMA_TYPE1)
    {
        switch (num) {
            case SCHEMA_TYPE1:
                string = "{\"SCHEMA_VERSION\":\"1.0\","
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
                break;
            case SCHEMA_TYPE2:
                string = "{\"SCHEMA_VERSION\":\"1.0\","
                    "\"SCHEMA_MODE\":\"STRICT\","
                    "\"SCHEMA_DEFINE\":{"
                        "\"field_name1\":\"LONG, DEFAULT 100\","
                        "\"field_name2\":{"
                            "\"field_name3\":\"INTEGER, NOT NULL\","
                            "\"field_name4\":\"LONG, DEFAULT 100\","
                            "\"field_name5\":\"DOUBLE, NOT NULL, DEFAULT 3.14\","
                            "\"field_name6\":\"STRING, NOT NULL, DEFAULT '3.1415'\""
                        "}"
                    "},"
                    "\"SCHEMA_INDEXES\":[\"$.field_name1\", \"$.field_name2.field_name6\"]}";
                break;
            default:
                return;
        }
    }

    void GenerateInvalidSchemaString(std::string &string)
    {
        string = "123";
    }

    void GenerateEmptySchemaString(std::string &string)
    {
        string.clear();
    }

    int WriteValidDataIntoKvStore()
    {
        return OK;
    }

    void OpenOpenedKvstoreWithSchema(const std::string &storeId, bool isEncrypt)
    {
        /**
         * @tc.steps: step1. create a new db(non-memory, encrypt), with schema;
         * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
         */
        KvStoreNbDelegate::Option option = {true, false, isEncrypt};
        if (isEncrypt) {
            CipherPassword passwd;
            vector<uint8_t> passwdBuffer(PASSWD_LEN, PASSWD_VAL);
            passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
            option.passwd = passwd;
        }
        GenerateValidSchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        KvStoreNbDelegate *kvNbDelegatePtr1 = g_kvNbDelegatePtr;
        /**
         * @tc.steps: step2. open an opened db, with same schema;
         * @tc.expected: step2. Returns a non-null kvstore and error code is OK.
         */
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        KvStoreNbDelegate *kvNbDelegatePtr2 = g_kvNbDelegatePtr;
        /**
         * @tc.steps: step3. open an opened db, with valid but different schema;
         * @tc.expected: step3. Returns a null kvstore and error code is SCHEMA_MISMATCH.
         */
        GenerateValidSchemaString(option.schema, SCHEMA_TYPE2);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == SCHEMA_MISMATCH);

        /**
         * @tc.steps: step4. open an opened db, with invalid schema;
         * @tc.expected: step4. Returns a null kvstore and error code is INVALID_SCHEMA.
         */
        GenerateInvalidSchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);

        /**
         * @tc.steps: step5. open an opened db, with empty schema;
         * @tc.expected: step5. Returns a null kvstore and error code is INVALID_SCHEMA.
         */
        std::string emptySchema;
        option.schema = emptySchema;
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == SCHEMA_MISMATCH);

        EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr1), OK);
        EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr2), OK);
        g_kvNbDelegatePtr = nullptr;
        EXPECT_TRUE(g_mgr.DeleteKvStore(storeId) == OK);
    }

    void OpenClosedSchemaKvStore(const std::string &storeId, bool isEncrypt, std::string &inSchema)
    {
        /**
         * @tc.steps: step1. create a new db(non-memory), with input schema;
         * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
         */
        KvStoreNbDelegate::Option option = {true, false, isEncrypt};
        option.schema = inSchema;
        if (isEncrypt) {
            CipherPassword passwd;
            vector<uint8_t> passwdBuffer(PASSWD_LEN, PASSWD_VAL);
            passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
            option.passwd = passwd;
        }
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
        /**
         * @tc.steps: step2. close the created kvstore;
         * @tc.expected: step2. Return OK.
         */
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;

        /**
         * @tc.steps: step3. reopen the kvstore with same schema;
         * @tc.expected: step3. Return OK.
         */
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;

        /**
         * @tc.steps: step4. reopen the kvstore with valid schema, but the schema is not equal to inSchema;
         * @tc.expected: step4. Return a null kvstore and retCode is SCHEMA_MISMATCH.
         */
        GenerateValidSchemaString(option.schema, SCHEMA_TYPE2);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == SCHEMA_MISMATCH);
        /**
         * @tc.steps: step5. reopen the kvstore with invalid schema;
         * @tc.expected: step5. Return a null kvstore and retCode is INVALID_SCHEMA.
         */
        GenerateInvalidSchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);
        /**
         * @tc.steps: step6. reopen the kvstore with empty schema;
         * @tc.expected: step6. Return a read-only kvstore and retCode is READ_ONLY.
         */
        GenerateEmptySchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        // here should return READ_ONLY
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
        KvStoreNbDelegate *kvNbDelegatePtr1 = g_kvNbDelegatePtr;

        // Open another kvstore with empty schema
        GenerateEmptySchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        KvStoreNbDelegate *kvNbDelegatePtr2 = g_kvNbDelegatePtr;
        // here should return READ_ONLY
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);

        // Open another kvstore with origin schema
        option.schema = inSchema;
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == SCHEMA_MISMATCH);

        EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr1), OK);
        EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr2), OK);
        EXPECT_TRUE(g_mgr.DeleteKvStore(storeId) == OK);
    }

    void OpenClosedNormalKvStore(const std::string &storeId, bool isEncrypt)
    {
        /**
         * @tc.steps: step1. create a new db(non-memory), without schema;
         * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
         */
        KvStoreNbDelegate::Option option = {true, false, isEncrypt};
        if (isEncrypt) {
            CipherPassword passwd;
            vector<uint8_t> passwdBuffer(PASSWD_LEN, PASSWD_VAL);
            passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
            option.passwd = passwd;
        }
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
        /**
         * @tc.steps: step2. close the created kvstore;
         * @tc.expected: step2. Return OK.
         */
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;

        /**
         * @tc.steps: step3. reopen the kvstore with empty schema;
         * @tc.expected: step3. Return a kvstore and retCode is OK.
         */
        GenerateEmptySchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;
        /**
         * @tc.steps: step4. reopen the kvstore with valid schema;
         * @tc.expected: step4. Return OK.
         */
        GenerateValidSchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == OK);
        EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;

        /**
         * @tc.steps: step5. reopen the kvstore with invalid schema;
         * @tc.expected: step5. Return a null kvstore and retCode is SCHEMA_MISMATCH.
         */
        GenerateInvalidSchemaString(option.schema);
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
        EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);
        EXPECT_TRUE(g_mgr.DeleteKvStore(storeId) == OK);
    }
#endif
}

class DistributedDBInterfacesDatabaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesDatabaseTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

void DistributedDBInterfacesDatabaseTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

void DistributedDBInterfacesDatabaseTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesDatabaseTest::TearDown(void) {}

/**
  * @tc.name: GetKvStore001
  * @tc.desc: Get kv store through different parameters.
  * @tc.type: FUNC
  * @tc.require: AR000CQDV4 AR000CQS3P
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, GetKvStore001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter the normal storId, createIfNecessary(true) and isLocal(true).
     * @tc.steps: step2. Close the kvStore through the CloseKvStore interface of the delegate manager.
     * @tc.expected: step1. Returns a non-null kvstore.
     * @tc.expected: step2. Returns OK.
     */
    KvStoreDelegate::Option option = {true, true, false};
    g_mgr.GetKvStore("distributed_db_test1", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step3. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter the normal storId, createIfNecessary(true) and isLocal(false).
     * @tc.steps: step4. Close the kvStore through the CloseKvStore interface of the delegate manager.
     * @tc.expected: step3. Returns a non-null kvstore.
     * @tc.expected: step4. Returns OK.
     */
    option = {true, false, false};
    g_mgr.GetKvStore("distributed_db_test2", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step5. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter the normal storId, createIfNecessary(false) and isLocal(true).
     * @tc.expected: step5. Returns a non-null kvstore and error code is ERROR.
     */
    option = {false, true, false};
    g_mgr.GetKvStore("distributed_db_test3", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == DB_ERROR);

    /**
     * @tc.steps: step6. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter the normal storId, createIfNecessary(false) and isLocal(false).
     * @tc.expected: step6. Returns a non-null kvstore and error code is ERROR.
     */
    option = {false, false, false};
    g_mgr.GetKvStore("distributed_db_test4", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == DB_ERROR);

    /**
     * @tc.steps: step7. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  which is initialized with the empty appid.
     * @tc.expected: step7. Returns a non-null kvstore and error code is INVALID_ARGS.
     */
    KvStoreDelegateManager invalidMgrFirst("", USER_ID);
    invalidMgrFirst.SetKvStoreConfig(g_config);
    option = {true, true, false};
    invalidMgrFirst.GetKvStore("distributed_db_test5", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_ARGS);

    /**
     * @tc.steps: step8. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  which is initialized with the empty userid.
     * @tc.expected: step8. Returns a non-null kvstore and error code is INVALID_ARGS.
     */
    KvStoreDelegateManager invalidMgrSecond(APP_ID, "");
    invalidMgrSecond.SetKvStoreConfig(g_config);
    invalidMgrSecond.GetKvStore("distributed_db_test6", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_ARGS);

    /**
     * @tc.steps: step9. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter the empty storId, createIfNecessary(true) and isLocal(true).
     * @tc.expected: step9. Returns a non-null kvstore and error code is INVALID_ARGS.
     */
    g_mgr.GetKvStore("", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_ARGS);

    /**
     * @tc.steps: step10. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter the invalid storId, createIfNecessary(true) and isLocal(true).
     * @tc.expected: step10. Returns a non-null kvstore and error code is INVALID_ARGS.
     */
    g_mgr.GetKvStore("$@.test", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_ARGS);

    /**
     * @tc.steps: step11. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter: all alphabet string storId, createIfNecessary(true) and isLocal(true).
     * @tc.expected: step11. Returns a non-null kvstore and error code is OK.
     */
    g_mgr.GetKvStore("TEST", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step12. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter: digital string storId, createIfNecessary(true) and isLocal(true).
     * @tc.expected: step12. Returns a non-null kvstore and error code is OK.
     */
    g_mgr.GetKvStore("123", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step13. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parmater: digital and alphabet combined string storId, createIfNecessary(true) and isLocal(true).
     * @tc.expected: step13. Returns a non-null kvstore and error code is OK.
     */
    g_mgr.GetKvStore("TEST_test_123", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
}

/**
  * @tc.name: GetKvStore002
  * @tc.desc: Get kv store through different parameters for the same storeID.
  * @tc.type: FUNC
  * @tc.require: AR000CQDV5 AR000CQS3P
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, GetKvStore002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter createIfNecessary(true) and isLocal(true).
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     */
    CipherPassword passwd;
    KvStoreDelegate::Option option = {true, true, false};
    g_mgr.GetKvStore("distributed_getkvstore_002", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step2. Re-Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter createIfNecessary(true) and isLocal(false).
     * @tc.expected: step2. Returns a non-null kvstore and error code is OK.
     */
    option.localOnly = false;
    g_mgr.GetKvStore("distributed_getkvstore_002", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step3. Re-Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter createIfNecessary(false) and isLocal(true).
     * @tc.expected: step3. Returns a non-null kvstore and error code is OK.
     */
    option = {false, true, false};
    g_mgr.GetKvStore("distributed_getkvstore_002", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step4. Re-Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter createIfNecessary(false) and isLocal(false).
     * @tc.expected: step4. Returns a non-null kvstore and error code is OK.
     */
    option = {false, false, false};
    g_mgr.GetKvStore("distributed_getkvstore_002", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step5. Re-Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter createIfNecessary(false) and isLocal(false).
     * @tc.expected: step5. Returns a non-null kvstore and error code is OK.
     */
    option = {true, true, false};
    g_mgr.GetKvStore("distributed_getkvstore_002", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    string retStoreId = g_kvDelegatePtr->GetStoreId();
    EXPECT_TRUE(retStoreId.compare("distributed_getkvstore_002") == 0);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
}

/**
  * @tc.name: GetKvStore003
  * @tc.desc: Get kv store through different SecurityOption, abnormal or normal.
  * @tc.type: FUNC
  * @tc.require: AR000EV1G2
  * @tc.author: liuwenkai
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, GetKvStore003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter secOption(abnormal).
     * @tc.expected: step1. Returns a null kvstore and error code is not OK.
     */
    std::shared_ptr<IProcessSystemApiAdapter> g_adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    KvStoreNbDelegate::Option option = {true, false, false};
    int abnormalNum = -100;
    option.secOption.securityLabel = abnormalNum;
    option.secOption.securityFlag = abnormalNum;
    g_mgr.GetKvStore("distributed_getkvstore_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);

    /**
     * @tc.steps: step2. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter secOption(normal).
     * @tc.expected: step2. Returns a non-null kvstore and error code is OK.
     */
    option.secOption.securityLabel = S3;
    option.secOption.securityFlag = 0;
    g_mgr.GetKvStore("distributed_getkvstore_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *kvNbDelegatePtr1 = g_kvNbDelegatePtr;

    /**
     * @tc.steps: step3. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter secOption(normal but not same as last).
     * @tc.expected: step3. Returns a null kvstore and error code is not OK.
     */
    option.secOption.securityLabel = S3;
    option.secOption.securityFlag = 1;
    g_mgr.GetKvStore("distributed_getkvstore_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus != OK);

    /**
    * @tc.steps: step4. Obtain the kvStore through the GetKvStore interface of the delegate manager
    *  using the parameter secOption(normal and same as last).
    * @tc.expected: step4. Returns a non-null kvstore and error code is OK.
    */
    option.secOption.securityLabel = S3;
    option.secOption.securityFlag = 0;
    g_mgr.GetKvStore("distributed_getkvstore_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr1), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("distributed_getkvstore_003") == OK);
}

static void NotifierCallback(const KvStoreNbConflictData &data)
{
    LOGE("Trigger conflict callback!");
    g_conflictCount++;
}

/**
  * @tc.name: GetKvStore004
  * @tc.desc: Get kv store parameters with Observer and Notifier, then trigger callback.
  * @tc.type: FUNC
  * @tc.require: AR000EV1G2
  * @tc.author: liuwenkai
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, GetKvStore004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Obtain the kvStore through the GetKvStore interface of the delegate manager
     *  using the parameter observer, notifier, key.
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_NE(observer, nullptr);
    Key key;
    Value value1;
    Value value2;
    key.push_back(1);
    value1.push_back(1);
    value2.push_back(2);
    option.conflictType = CONFLICT_NATIVE_ALL;
    option.notifier = NotifierCallback;
    option.key = key;
    option.observer = observer;
    option.mode = OBSERVER_CHANGES_NATIVE;
    g_conflictCount = 0;
    int sleepTime = 100;
    g_mgr.GetKvStore("distributed_getkvstore_004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2. Put(k1,v1) to db and check the observer info.
     * @tc.expected: step2. Put successfully and trigger notifier callback.
     */
    EXPECT_TRUE(g_kvNbDelegatePtr->Put(key, value1) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    LOGI("observer count:%lu", observer->GetCallCount());
    EXPECT_TRUE(observer->GetCallCount() == 1);

    /**
     * @tc.steps: step3. put(k1,v2) to db and check the observer info.
     * @tc.expected: step3. put successfully and trigger conflict callback.
     */
    EXPECT_TRUE(g_kvNbDelegatePtr->Put(key, value2) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    LOGI("observer count:%lu", observer->GetCallCount());
    EXPECT_TRUE(observer->GetCallCount() == 2);
    LOGI("call conflictNotifier count:%d, 1 means trigger success.", g_conflictCount);
    EXPECT_EQ(g_conflictCount, 1);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    delete observer;
    observer = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("distributed_getkvstore_004") == OK);
}

/**
  * @tc.name: CloseKvStore001
  * @tc.desc: Test the CloseKvStore Interface and check whether the database file can be closed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDV6 AR000CQS3P
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, CloseKvStore001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Obtain the kvStore of the non-existed database through the GetKvStore interface of
     *  the delegate manager using the parameter createdIfNecessary(true)
     * @tc.steps: step2. Close the valid kvStore.
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     * @tc.expected: step2. Returns OK.
     */
    CipherPassword passwd;
    KvStoreDelegate::Option option = {true, true, false};
    g_mgr.GetKvStore("CloseKvStore_001", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step3. Obtain the kvStore of the existed database through the GetKvStore interface of
     *  the delegate manager using the parameter createIfNecessary(false)
     * @tc.steps: step4. Close the valid kvStore.
     * @tc.expected: step3. Returns a non-null kvstore and error code is OK.
     * @tc.expected: step4. Returns OK.
     */
    option = {false, true, false};
    g_mgr.GetKvStore("CloseKvStore_001", option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step5. Close the invalid kvStore which is nullptr.
     * @tc.expected: step5. Returns INVALID_ARGS.
     */
    KvStoreDelegate *storeDelegate = nullptr;
    EXPECT_EQ(g_mgr.CloseKvStore(storeDelegate), INVALID_ARGS);
}

/**
  * @tc.name: DeleteKvStore001
  * @tc.desc: Test the DeleteKvStore Interface and check whether the database files can be removed.
  * @tc.type: FUNC
  * @tc.require: AR000C2F0C AR000CQDV7
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, DeleteKvStore001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Obtain the kvStore through the GetKvStore interface of
     *  the delegate manager using the parameter createIfNecessary(true)
     * @tc.steps: step2. Close the kvStore.
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     * @tc.expected: step2. Returns OK.
     */
    CipherPassword passwd;
    KvStoreDelegate::Option option = {true, true, false};
    const std::string storeId("DeleteKvStore_001");
    g_mgr.GetKvStore(storeId, option, g_kvDelegateCallback);
    std::string origIdentifierName = USER_ID + "-" + APP_ID + "-" + storeId;
    std::string hashIdentifierName = DBCommon::TransferHashString(origIdentifierName);
    std::string identifierName = DBCommon::TransferStringToHex(hashIdentifierName);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step3. Check the database file
     * @tc.expected: step3. the database file are existed.
     */
    string dbFileName = g_testDir + "/" + identifierName + "/local/local.db";
    ifstream dbFile(dbFileName);
    EXPECT_TRUE(dbFile);
    dbFile.close();
    string walFileName = g_testDir + "/" + identifierName + "/local/local.db-wal";
    fstream walFile(walFileName, fstream::out);
    EXPECT_TRUE(walFile.is_open());
    walFile.close();
    string shmFileName = g_testDir + "/" + identifierName + "/local/local.db-shm";
    fstream shmFile(shmFileName, fstream::out);
    EXPECT_TRUE(shmFile.is_open());
    shmFile.close();

    std::string dataBaseDir = g_testDir + "/" + identifierName;
    EXPECT_GE(access(dataBaseDir.c_str(), F_OK), 0);

    /**
     * @tc.steps: step4. Delete the kvStore through the DeleteKvStore interface of
     *  the delegate manager
     * @tc.steps: step5. Check the database files and the storage paths.
     * @tc.expected: step4. Returns OK.
     * @tc.expected: step5. The database files  and the storage paths are not existed.
     */
    EXPECT_TRUE(g_mgr.DeleteKvStore(storeId) == OK);
    ifstream dbFileAfter(dbFileName);
    ifstream walFileAfter(walFileName);
    ifstream shmFileAfter(shmFileName);
    EXPECT_FALSE(dbFileAfter);
    EXPECT_FALSE(walFileAfter);
    EXPECT_FALSE(shmFileAfter);
    ASSERT_EQ(OS::CheckPathExistence(dataBaseDir), false);
    std::string storeIdOnlyIdentifier = DBCommon::TransferHashString(storeId);
    std::string storeIdOnlyIdentifierName = DBCommon::TransferStringToHex(storeIdOnlyIdentifier);
    std::string storeIdOnlyIdDataBaseDir = g_testDir + "/" + storeIdOnlyIdentifierName;
    ASSERT_EQ(OS::CheckPathExistence(storeIdOnlyIdDataBaseDir), false);

    /**
     * @tc.steps: step6. Re-Delete the kvStore through the DeleteKvStore interface of
     *  the delegate manager
     * @tc.expected: step6. Returns NOT_FOUND.
     */
    EXPECT_TRUE(g_mgr.DeleteKvStore(storeId) == NOT_FOUND);
}

/**
  * @tc.name: RepeatCloseKvStore001
  * @tc.desc: Close the kv store repeatedly and check the database.
  * @tc.type: FUNC
  * @tc.require: AR000C2F0C AR000CQDV7
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, RepeatCloseKvStore001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Obtain the kvStore through the GetKvStore interface of
     *  the delegate manager using the parameter createIfNecessary(true)
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     */
    CipherPassword passwd;
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("RepeatCloseKvStore_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    static const size_t totalSize = 50;

    /**
     * @tc.steps: step2. Put into the database some data.
     * @tc.expected: step2. Put returns OK.
     */
    std::vector<Key> keys;
    for (size_t i = 0; i < totalSize; i++) {
        Entry entry;
        DistributedDBToolsUnitTest::GetRandomKeyValue(entry.key, static_cast<uint32_t>(i + 1));
        DistributedDBToolsUnitTest::GetRandomKeyValue(entry.value);
        EXPECT_EQ(g_kvNbDelegatePtr->Put(entry.key, entry.value), OK);
        keys.push_back(entry.key);
    }

    /**
     * @tc.steps: step3. Delete the data from the database, and close the database, reopen the database and
     *  get the data.
     * @tc.expected: step3. Delete returns OK, Close returns OK and Get returns NOT_FOUND.
     */
    for (size_t i = 0; i < keys.size(); i++) {
        Value value;
        EXPECT_EQ(g_kvNbDelegatePtr->Delete(keys[i]), OK);
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keys[i], value), NOT_FOUND);
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_mgr.GetKvStore("RepeatCloseKvStore_001", option, g_kvNbDelegateCallback);
        EXPECT_EQ(g_kvNbDelegatePtr->Get(keys[i], value), NOT_FOUND);
    }
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    /**
     * @tc.steps: step4. Delete the kvstore created before.
     * @tc.expected: step4. Delete returns OK.
     */
    EXPECT_EQ(g_mgr.DeleteKvStore("RepeatCloseKvStore_001"), OK);
}
#ifndef OMIT_JSON
/**
  * @tc.name: CreatKvStoreWithSchema001
  * @tc.desc: Create non-memory KvStore with schema, check if create success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, CreatKvStoreWithSchema001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create a new db(non-memory, non-encrypt), with valid schema;
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    GenerateValidSchemaString(option.schema);
    g_mgr.GetKvStore("CreatKvStoreWithSchema_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("CreatKvStoreWithSchema_001") == OK);
    /**
     * @tc.steps: step2. create a new db(non-memory, non-encrypt), with invalid schema;
     * @tc.expected: step2. Returns null kvstore and error code is INVALID_SCHEMA.
     */
    option = {true, false, false};
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("CreatKvStoreWithSchema_001_invalid_schema", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);
    EXPECT_TRUE(g_mgr.DeleteKvStore("CreatKvStoreWithSchema_001_invalid_schema") == NOT_FOUND);
#ifndef OMIT_ENCRYPT
    /**
     * @tc.steps: step3. create a new db(non-memory, encrypt), with valid schema;
     * @tc.expected: step3. Returns a non-null kvstore and error code is OK.
     */
    CipherPassword passwd;
    vector<uint8_t> passwdBuffer(10, 45);
    passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    option = {true, false, true};
    GenerateValidSchemaString(option.schema);
    option.passwd = passwd;
    g_mgr.GetKvStore("CreatKvStoreWithSchema_001_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("CreatKvStoreWithSchema_001_002") == OK);

    /**
     * @tc.steps: step4. create a new db(non-memory, non-encrypt), with invalid schema;
     * @tc.expected: step2. Returns null kvstore and error code is INVALID_SCHEMA.
     */
    option = {true, false, false};
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("CreatKvStoreWithSchema_002_invalid_schema", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);
    EXPECT_TRUE(g_mgr.DeleteKvStore("CreatKvStoreWithSchema_002_invalid_schema") == NOT_FOUND);
#endif
}

/**
  * @tc.name: CreatKvStoreWithSchema002
  * @tc.desc: Create memory KvStore with schema, check if create success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, CreatKvStoreWithSchema002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create a new db(memory, non-encrypt), with valid schema;
     * @tc.expected: step1. Returns a null kvstore and error code is NOT_SUPPORT.
     */
    KvStoreNbDelegate::Option option = {true, true, false};
    GenerateValidSchemaString(option.schema);
    g_mgr.GetKvStore("CreatKvStoreWithSchema_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == NOT_SUPPORT);

    /**
     * @tc.steps: step2. create a new db(memory, non-encrypt), with invalid schema;
     * @tc.expected: step2. Returns null kvstore and error code is NOT_SUPPORT.
     */
    option = {true, true, false};
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("CreatKvStoreWithSchema_002_invalid", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == NOT_SUPPORT);
}
#ifndef OMIT_ENCRYPT
/**
  * @tc.name: OpenKvStoreWithSchema001
  * @tc.desc: open an opened kvstore(non-memory, no-schema) with schema, check if open success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithSchema001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create a new db(non-memory, non-encrypt), without schema;
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("OpenKvStoreWithSchema_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *kvNbDelegatePtr = g_kvNbDelegatePtr;
    /**
     * @tc.steps: step2. open the db with valid schema;
     * @tc.expected: step2. Returns a null kvstore and error code is SCHEMA_MISMATCH.
     */
    GenerateValidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. open the db with invalid schema;
     * @tc.expected: step3. Returns a null kvstore and error code is SCHEMA_MISMATCH.
     */
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);

    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("OpenKvStoreWithSchema_001") == OK);
    /**
     * @tc.steps: step4. create a new db(non-memory, encrypt), without schema;
     * @tc.expected: step4. Returns a non-null kvstore and error code is OK.
     */
    option = {true, false, true};
    CipherPassword passwd;
    vector<uint8_t> passwdBuffer(10, 45);
    passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    option.passwd = passwd;
    g_mgr.GetKvStore("OpenKvStoreWithSchema_001_encrypt", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    kvNbDelegatePtr = g_kvNbDelegatePtr;
    /**
     * @tc.steps: step5. open the db with valid schema;
     * @tc.expected: step5. Returns a null kvstore and error code is SCHEMA_MISMATCH.
     */
    GenerateValidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_001_encrypt", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == SCHEMA_MISMATCH);

    /**
     * @tc.steps: step6. open the db with invalid schema;
     * @tc.expected: step6. Returns a null kvstore and error code is SCHEMA_MISMATCH.
     */
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_001_encrypt", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == INVALID_SCHEMA);

    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(g_mgr.DeleteKvStore("OpenKvStoreWithSchema_001_encrypt") == OK);
}
#endif
/**
  * @tc.name: OpenKvStoreWithSchema002
  * @tc.desc: open an opened kvstore(non-memory, schema) with schema, check if open success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithSchema002, TestSize.Level1)
{
#ifndef OMIT_ENCRYPT
    /**
     * @tc.steps: step1. open an opened kvstore(non-memory, non-encrypt), with different schemas;
     */
    OpenOpenedKvstoreWithSchema("OpenKvStoreWithSchema_002", true);
#endif
    /**
     * @tc.steps: step2. open an opened kvstore(non-memory, encrypt), with different schemas;
     */
    OpenOpenedKvstoreWithSchema("OpenKvStoreWithSchema_002_encrypt", false);
}

/**
  * @tc.name: OpenKvStoreWithSchema003
  * @tc.desc: open an opened kvstore(memory) with different schemas, check if open success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithSchema003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create a new db(memory, non-encrypt), without schema;
     * @tc.expected: step1. Returns a non-null kvstore and error code is OK.
     */
    KvStoreNbDelegate::Option option = {true, true, false};
    g_mgr.GetKvStore("OpenKvStoreWithSchema_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *kvNbDelegatePtr1 = g_kvNbDelegatePtr;
    /**
     * @tc.steps: step2. open a new db(memory, non-encrypt), without schema;
     * @tc.expected: step2. Returns a non-null kvstore and error code is OK.
     */
    g_mgr.GetKvStore("OpenKvStoreWithSchema_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreNbDelegate *kvNbDelegatePtr2 = g_kvNbDelegatePtr;
    /**
     * @tc.steps: step3. open a new db(memory, non-encrypt), with valid schema;
     * @tc.expected: step3. Returns a null kvstore and error code is NOT_SUPPORT.
     */
    GenerateValidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == NOT_SUPPORT);

    /**
     * @tc.steps: step4. open a new db(memory, non-encrypt), with invalid schema;
     * @tc.expected: step4. Returns a null kvstore and error code is NOT_SUPPORT.
     */
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == NOT_SUPPORT);
    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr1), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr2), OK);
}

/**
  * @tc.name: OpenKvStoreWithSchema004
  * @tc.desc: open a totally closed schema-kvstore(non-memory) with different schemas, check if open success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithSchema004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open a new db(non-memory, non-encrypt), with different schemas;
     * @tc.expected: step1. Returns a null kvstore and error code is NOT_SUPPORT.
     */
    std::string schema;
    GenerateValidSchemaString(schema);
    OpenClosedSchemaKvStore("OpenKvStoreWithSchema_004", false, schema);
#ifndef OMIT_ENCRYPT
    /**
     * @tc.steps: step2. open a new db(non-memory, encrypt), with different schemas;
     * @tc.expected: step2. Returns a null kvstore and error code is NOT_SUPPORT.
     */
    OpenClosedSchemaKvStore("OpenKvStoreWithSchema_004_encrypt", true, schema);
#endif
}

/**
  * @tc.name: OpenKvStoreWithSchema005
  * @tc.desc: open a totally closed non-schema-kvstore(non-memory) with different schemas, check if open success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithSchema005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open a new db(non-memory, non-encrypt, non-schema), with different schemas;
     * @tc.expected: step1. Returns a different result.
     */
    OpenClosedNormalKvStore("OpenKvStoreWithSchema_005", false);
#ifndef OMIT_ENCRYPT
    /**
     * @tc.steps: step2. open a new db(non-memory, encrypt, non-schema), with different schemas;
     * @tc.expected: step2. Returns a different result.
     */
    OpenClosedNormalKvStore("OpenKvStoreWithSchema_005", true);
#endif
}

/**
  * @tc.name: OpenKvStoreWithSchema006
  * @tc.desc: open a memory non-schema-kvstore with different schemas, check if open success.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithSchema006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open a new db(memory, non-encrypt, non-schema), without schema;
     * @tc.expected: step1. Returns OK.
     */
    KvStoreNbDelegate::Option option = {true, true, false};
    g_mgr.GetKvStore("OpenKvStoreWithSchema_006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step2. close the kvstore;
     * @tc.expected: step2. Returns OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;

    /**
     * @tc.steps: step3. reopen the kvstore without schema;
     * @tc.expected: step3. Returns OK.
     */
    g_mgr.GetKvStore("OpenKvStoreWithSchema_006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(WriteValidDataIntoKvStore() == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;

    /**
     * @tc.steps: step4. reopen the kvstore with valid schema;
     * @tc.expected: step4. Returns OK.
     */
    GenerateValidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == NOT_SUPPORT);

    /**
     * @tc.steps: step4. reopen the kvstore with invalid schema;
     * @tc.expected: step4. Returns OK.
     */
    GenerateInvalidSchemaString(option.schema);
    g_mgr.GetKvStore("OpenKvStoreWithSchema_006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == NOT_SUPPORT);
}
#endif
/**
  * @tc.name: OpenKvStoreWithStoreOnly001
  * @tc.desc: open the kv store with the option that createDirByStoreIdOnly is true.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, OpenKvStoreWithStoreOnly001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open the kv store with the option that createDirByStoreIdOnly is true.
     * @tc.expected: step1. Returns OK.
     */
    KvStoreNbDelegate::Option option;
    option.createDirByStoreIdOnly = true;
    g_mgr.GetKvStore("StoreOnly001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    auto kvStorePtr = g_kvNbDelegatePtr;
    /**
     * @tc.steps: step2. open the same store with the option that createDirByStoreIdOnly is false.
     * @tc.expected: step2. Returns NOT OK.
     */
    option.createDirByStoreIdOnly = false;
    g_kvNbDelegatePtr = nullptr;
    g_mgr.GetKvStore("StoreOnly001", option, g_kvNbDelegateCallback);
    EXPECT_EQ(g_kvDelegateStatus, INVALID_ARGS);
    /**
     * @tc.steps: step3. close the kvstore and delete the kv store;
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(kvStorePtr), OK);
    kvStorePtr = nullptr;
    EXPECT_EQ(g_mgr.DeleteKvStore("StoreOnly001"), OK);
}

/**
  * @tc.name: GetDBWhileOpened001
  * @tc.desc: open the kv store with the option that createDirByStoreIdOnly is true.
  * @tc.type: FUNC
  * @tc.require: AR000E8S2V
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, GetDBWhileOpened001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the connection.
     * @tc.expected: step1. Returns OK.
     */
    KvDBProperties property;
    std::string storeId = "openTest";
    std::string origId = USER_ID + "-" + APP_ID + "-" + storeId;
    std::string identifier = DBCommon::TransferHashString(origId);
    std::string hexDir = DBCommon::TransferStringToHex(identifier);
    property.SetStringProp(KvDBProperties::IDENTIFIER_DATA, identifier);
    property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, hexDir);
    property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    property.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    property.SetBoolProp(KvDBProperties::MEMORY_MODE, false);
    property.SetBoolProp(KvDBProperties::ENCRYPTED_MODE, false);
    property.SetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, true);
    property.SetStringProp(KvDBProperties::APP_ID, APP_ID);
    property.SetStringProp(KvDBProperties::USER_ID, USER_ID);
    property.SetStringProp(KvDBProperties::APP_ID, storeId);

    int errCode = E_OK;
    auto connection1 = KvDBManager::GetDatabaseConnection(property, errCode, false);
    EXPECT_EQ(errCode, E_OK);
    /**
     * @tc.steps: step2. Get the connection with the para: isNeedIfOpened is false.
     * @tc.expected: step2. Returns -E_ALREADY_OPENED.
     */
    auto connection2 = KvDBManager::GetDatabaseConnection(property, errCode, false);
    EXPECT_EQ(errCode, -E_ALREADY_OPENED);
    EXPECT_EQ(connection2, nullptr);

    /**
     * @tc.steps: step3. Get the connection with the para: isNeedIfOpened is true.
     * @tc.expected: step3. Returns E_OK.
     */
    auto connection3 = KvDBManager::GetDatabaseConnection(property, errCode, true);
    EXPECT_EQ(errCode, OK);
    EXPECT_NE(connection3, nullptr);

    KvDBManager::ReleaseDatabaseConnection(connection1);
    KvDBManager::ReleaseDatabaseConnection(connection3);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
}
namespace {
    void OpenCloseDatabase(const std::string &storeId)
    {
        KvStoreNbDelegate::Option option;
        DBStatus status;
        KvStoreNbDelegate *delegate = nullptr;
        auto nbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
            placeholders::_1, placeholders::_2, std::ref(status), std::ref(delegate));
        int totalNum = 0;
        for (size_t i = 0; i < 100; i++) { // cycle 100 times.
            g_mgr.GetKvStore(storeId, option, nbDelegateCallback);
            if (delegate != nullptr) {
                totalNum++;
            }
            g_mgr.CloseKvStore(delegate);
            delegate = nullptr;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        LOGD("Succeed %d times", totalNum);
    }
}

/**
  * @tc.name: FreqOpenClose001
  * @tc.desc: Open and close the kv store concurrently.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K2
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, FreqOpenClose001, TestSize.Level2)
{
    std::string storeId = "FrqOpenClose001";
    std::thread t1(OpenCloseDatabase, storeId);
    std::thread t2(OpenCloseDatabase, storeId);
    std::thread t3(OpenCloseDatabase, storeId);
    std::thread t4(OpenCloseDatabase, storeId);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
}

/**
  * @tc.name: CheckKvStoreDir001
  * @tc.desc: Delete the kv store with the option that createDirByStoreIdOnly is true.
  * @tc.type: FUNC
  * @tc.require: AR000CQDV7
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDatabaseTest, CheckKvStoreDir001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open the kv store with the option that createDirByStoreIdOnly is true.
     * @tc.expected: step1. Returns OK.
     */
    KvStoreNbDelegate::Option option;
    option.createDirByStoreIdOnly = true;
    const std::string storeId("StoreOnly002");
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    std::string testSubDir;
    EXPECT_EQ(KvStoreDelegateManager::GetDatabaseDir(storeId, testSubDir), OK);
    std::string dataBaseDir = g_testDir + "/" + testSubDir;
    EXPECT_GE(access(dataBaseDir.c_str(), F_OK), 0);

    /**
     * @tc.steps: step2. delete the kv store, and check the directory.
     * @tc.expected: step2. the directory is removed.
     */
    g_mgr.CloseKvStore(g_kvNbDelegatePtr);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    LOGE("[%s]", dataBaseDir.c_str());
    ASSERT_EQ(OS::CheckPathExistence(dataBaseDir), false);
}
