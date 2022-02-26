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

#include "db_constant.h"
#include "db_common.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    KvStoreNbDelegate::Option g_nbOption;

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));

    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    std::string g_storeId;
    std::string g_identifier;
    vector<string> g_singleVerFileNames;
    vector<string> g_commitLogFileNames;
    vector<string> g_metaStorageFileNames;
    vector<string> g_multiVerDataFileNames;
    vector<string> g_ValueStorageFileNames;

    void GetRealFileUrl()
    {
        std::string origIdentifier = USER_ID + "-" + APP_ID + "-" + g_storeId;
        std::string hashIdentifier = DBCommon::TransferHashString(origIdentifier);
        g_identifier = DBCommon::TransferStringToHex(hashIdentifier);

        g_singleVerFileNames = {
            g_testDir + "/" + g_identifier + "/single_ver/main/gen_natural_store.db",
            g_testDir + "/" + g_identifier + "/single_ver/main/gen_natural_store.db-shm",
            g_testDir + "/" + g_identifier + "/single_ver/main/gen_natural_store.db-wal"};
        g_commitLogFileNames = {
            g_testDir + "/" + g_identifier + "/multi_ver/commit_logs.db",
            g_testDir + "/" + g_identifier + "/multi_ver/commit_logs.db-shm",
            g_testDir + "/" + g_identifier + "/multi_ver/commit_logs.db-wal"};
        g_metaStorageFileNames = {
            g_testDir + "/" + g_identifier + "/multi_ver/meta_storage.db",
            g_testDir + "/" + g_identifier + "/multi_ver/meta_storage.db-shm",
            g_testDir + "/" + g_identifier + "/multi_ver/meta_storage.db-wal"};
        g_multiVerDataFileNames = {
            g_testDir + "/" + g_identifier + "/multi_ver/multi_ver_data.db",
            g_testDir + "/" + g_identifier + "/multi_ver/multi_ver_data.db-shm",
            g_testDir + "/" + g_identifier + "/multi_ver/multi_ver_data.db-wal"};
        g_ValueStorageFileNames = {
            g_testDir + "/" + g_identifier + "/multi_ver/value_storage.db",
            g_testDir + "/" + g_identifier + "/multi_ver/value_storage.db-shm",
            g_testDir + "/" + g_identifier + "/multi_ver/value_storage.db-wal"};
    }

    vector<string> GetMultiVerFilelist()
    {
        vector<string> multiFileNames;
        for (const auto &iter : g_commitLogFileNames) {
            multiFileNames.push_back(iter);
        }
        for (const auto &iter : g_metaStorageFileNames) {
            multiFileNames.push_back(iter);
        }
        for (const auto &iter : g_multiVerDataFileNames) {
            multiFileNames.push_back(iter);
        }
        for (const auto &iter : g_ValueStorageFileNames) {
            multiFileNames.push_back(iter);
        }
        return multiFileNames;
    }
}

class DistributedDBInterfacesSpaceManagementTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesSpaceManagementTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesSpaceManagementTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesSpaceManagementTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesSpaceManagementTest::TearDown(void) {}

// use another way calculate small file size(2G)
static uint64_t CheckRealFileSize(const vector<string> &fileNames)
{
    int size = 0;
    for (const auto &file : fileNames) {
        FILE *fileHandle = nullptr;
        fileHandle = fopen(file.c_str(), "rb");
        if (fileHandle == nullptr) {
            LOGE("Open file[%s] fail", file.c_str());
            continue;
        }
        (void)fseek(fileHandle, 0, SEEK_END);
        size += ftell(fileHandle);
        LOGD("CheckRealFileSize:FileName[%s],size[%lld]", file.c_str(), ftell(fileHandle));
        (void)fclose(fileHandle);
    }
    return size;
}

/**
  * @tc.name: GetKvStoreDiskSize001
  * @tc.desc: ROM space occupied by applications in the distributed database can be calculated.
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesSpaceManagementTest, GetKvStoreDiskSize001, TestSize.Level1)
{
    g_storeId = "distributed_GetKvStoreDiskSize_001";
    GetRealFileUrl();

    g_mgr.GetKvStore(g_storeId, g_nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps: step1/2. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step1/2. Return right size and ok.
     */
    uint64_t localDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, localDbSize), OK);
    EXPECT_EQ(CheckRealFileSize(g_singleVerFileNames), localDbSize);

    /**
     * @tc.steps: step3. Reopen Db.
     */
    g_mgr.GetKvStore(g_storeId, g_nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step4. Put some Key Value to change Db size.
     */
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, DBConstant::MAX_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, DBConstant::MAX_VALUE_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);

    /**
     * @tc.steps: step5/6. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step5/6. Return right size and ok.
     */
    localDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, localDbSize), OK);
    EXPECT_EQ(CheckRealFileSize(g_singleVerFileNames), localDbSize);

    /**
     * @tc.steps: step7. Close and Delete Db.
     * @tc.expected: step7. Successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(g_storeId), OK);
}

/**
  * @tc.name: GetKvStoreDiskSize002
  * @tc.desc: Obtain the size of the opened database.
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesSpaceManagementTest, GetKvStoreDiskSize002, TestSize.Level1)
{
    g_storeId = "distributed_GetKvStoreDiskSize_002";
    GetRealFileUrl();

    KvStoreDelegate::Option option;
    g_mgr.GetKvStore(g_storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    g_mgr.GetKvStore(g_storeId, g_nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step1/2. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step1/2. Return right size and ok.
     */
    uint64_t singleAndMultiDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleAndMultiDbSize), OK);
    uint64_t dbSizeForCheck = CheckRealFileSize(g_singleVerFileNames) + CheckRealFileSize(GetMultiVerFilelist());
    EXPECT_EQ(dbSizeForCheck, singleAndMultiDbSize);

    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, DBConstant::MAX_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, DBConstant::MAX_VALUE_SIZE);

    EXPECT_EQ(g_kvNbDelegatePtr->Put(key, value), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps: step3/4. Reopen Db and Put some Key Value to change Db size.
     */
    g_mgr.GetKvStore(g_storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(key, value), OK);

    /**
     * @tc.steps: step5/6. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step5/6. Return right size and ok.
     */
    singleAndMultiDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleAndMultiDbSize), OK);
    ASSERT_TRUE(dbSizeForCheck != singleAndMultiDbSize);
    dbSizeForCheck = CheckRealFileSize(g_singleVerFileNames) + CheckRealFileSize(GetMultiVerFilelist());
    EXPECT_EQ(dbSizeForCheck, singleAndMultiDbSize);
    LOGE("single:%lld,mul:%lld", CheckRealFileSize(g_singleVerFileNames), CheckRealFileSize(GetMultiVerFilelist()));

    /**
     * @tc.steps: step7. Close and Delete Db.
     * @tc.expected: step7. Successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(g_storeId), OK);
}

// The file will be deleted after the test, it only for test, no security impact on permissions
static void CreateFile(const std::string &fileUrl, uint64_t fileSize)
{
    ofstream mcfile;
    mcfile.open(fileUrl);
    if (!mcfile.is_open()) {
        return;
    }
    std::string fileContext;
    fileContext.resize(fileSize, 'X');
    mcfile << fileContext;
    mcfile.close();
    return;
}

static void DeleteFile(const std::string &fileUrl)
{
    std::ifstream walFile(fileUrl);
    if (walFile) {
        int result = remove(fileUrl.c_str());
        if (result < 0) {
            LOGE("failed to delete the file[%s]:%d", fileUrl.c_str(), errno);
        }
    }
    return;
}

/**
  * @tc.name: GetKvStoreDiskSize003
  * @tc.desc: Verification exception parameters
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesSpaceManagementTest, GetKvStoreDiskSize003, TestSize.Level1)
{
    g_storeId = "distributed_GetKvStoreDiskSize_003";
    GetRealFileUrl();
    KvStoreNbDelegate::Option nbOption;
    g_mgr.GetKvStore(g_storeId, nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    KvStoreDelegate::Option option;
    g_mgr.GetKvStore(g_storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step1. Use an anomalous length of storeId by GetKvStoreDiskSize to get size.
     * @tc.expected: step1. Return 0 size and INVALID_ARGS.
     */
    uint64_t dbSize = 0;
    std::string exceptStoreId;
    exceptStoreId.clear();
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(exceptStoreId, dbSize), INVALID_ARGS);
    EXPECT_EQ(dbSize, 0ull);

    exceptStoreId.resize(129, 'X');
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(exceptStoreId, dbSize), INVALID_ARGS);
    EXPECT_EQ(dbSize, 0ull);

    /**
     * @tc.steps: step2. Use a valid but not exist storeId to GetKvStoreDiskSize.
     * @tc.expected: step2. Return 0 size and NOT_FOUND.
     */
    exceptStoreId.resize(128, 'X');
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(exceptStoreId, dbSize), NOT_FOUND);
    EXPECT_EQ(dbSize, 0ull);

    /**
     * @tc.steps: step3/4. Use right storeId to GetKvStoreDiskSize.
     * @tc.expected: step3/4. Return right size and OK.
     */
    uint64_t singleAndMultiDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleAndMultiDbSize), OK);
    uint64_t dbSizeForCheck = CheckRealFileSize(g_singleVerFileNames) + CheckRealFileSize(GetMultiVerFilelist());
    EXPECT_EQ(dbSizeForCheck, singleAndMultiDbSize);

    /**
     * @tc.steps: step5. Create irrelevant files.
     */
    CreateFile(g_testDir + "/" + g_storeId + "/" + DBConstant::MULTI_SUB_DIR + "/test.txt", 1024 * 1024);

    /**
     * @tc.steps: step6/7/8. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step6/7/8. Return right size and ok.
     */
    singleAndMultiDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleAndMultiDbSize), OK);
    EXPECT_EQ(dbSizeForCheck, singleAndMultiDbSize);

    DeleteFile(g_testDir + "/" + g_storeId + "/" + DBConstant::MULTI_SUB_DIR + "/test.txt");
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(g_storeId), OK);
}

/**
  * @tc.name: GetKvStoreDiskSize004
  * @tc.desc: Calculate memory database size
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesSpaceManagementTest, GetKvStoreDiskSize004, TestSize.Level1)
{
    g_storeId = "distributed_GetKvStoreDiskSize_004";
    GetRealFileUrl();

    KvStoreNbDelegate::Option nbOption;
    g_mgr.GetKvStore(g_storeId, nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    uint64_t singleVerRealSize = CheckRealFileSize(g_singleVerFileNames);

    /**
     * @tc.steps: step1/2. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step1/2. Return right size and ok.
     */
    uint64_t singleVerDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleVerDbSize), OK);
    EXPECT_EQ(singleVerDbSize, singleVerRealSize);

    /**
     * @tc.steps: step3. Use the same storeId create memoryDb.
     */
    nbOption = {true, true};
    g_mgr.GetKvStore(g_storeId, nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step4/5. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step4/5. Return 0 size and ok.
     */
    singleVerDbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleVerDbSize), OK);
    EXPECT_EQ(singleVerDbSize, 0ull);

    /**
     * @tc.steps: step6. Close memoryDb.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps: step7. Get Db size by GetKvStoreDiskSize.
     * @tc.expected: step7. Return right size and ok.
     */
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(g_storeId, singleVerDbSize), OK);
    EXPECT_EQ(singleVerDbSize, singleVerRealSize);

    EXPECT_EQ(g_mgr.DeleteKvStore(g_storeId), OK);
}

/**
  * @tc.name: DeleteDbByStoreId001
  * @tc.desc: Delete database by storeId.
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesSpaceManagementTest, DeleteDbByStoreId001, TestSize.Level1)
{
    std::string storeId1 = "distributed_DeleteDbByStoreId001";
    KvStoreNbDelegate::Option nbOption;
    g_mgr.GetKvStore(storeId1, nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    KvStoreDelegate::Option option;
    g_mgr.GetKvStore(storeId1, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    option.localOnly = true;
    g_mgr.GetKvStore(storeId1, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    std::string storeId2 = "distributed_DeleteDbByStoreId002";

    g_mgr.GetKvStore(storeId2, nbOption, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    option.localOnly = false;
    g_mgr.GetKvStore(storeId2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    option.localOnly = true;
    g_mgr.GetKvStore(storeId2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    uint64_t store1DbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(storeId1, store1DbSize), OK);
    EXPECT_NE(store1DbSize, 0ull);
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(storeId2, store1DbSize), OK);
    EXPECT_NE(store1DbSize, 0ull);

    /**
     * @tc.steps: step1. Delete database by storeId 1.
     */
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId1), OK);

    /**
     * @tc.steps: step2. Use storeId 1 to get Db size by GetKvStoreDiskSize.
     * @tc.expected: step2. Return 0 size and ok.
     */
    store1DbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(storeId1, store1DbSize), NOT_FOUND);
    EXPECT_EQ(store1DbSize, 0ull);

    /**
     * @tc.steps: step3. Use storeId 2 to get Db size by GetKvStoreDiskSize.
     * @tc.expected: step3. Return right size and ok.
     */
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(storeId2, store1DbSize), OK);
    EXPECT_NE(store1DbSize, 0ull);
}

/**
  * @tc.name: DeleteDbByStoreId002
  * @tc.desc: Delete database by not exist storeId.
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesSpaceManagementTest, DeleteDbByStoreId002, TestSize.Level1)
{
    std::string storeId1 = "distributed_DeleteDbByStoreId001";

    uint64_t store1DbSize = 0;
    EXPECT_EQ(g_mgr.GetKvStoreDiskSize(storeId1, store1DbSize), NOT_FOUND);
    EXPECT_EQ(store1DbSize, 0ull);

    /**
     * @tc.steps: step1. Delete database by not exist storeId 1.
     * @tc.expected: step3. Return NOT_FOUND.
     */
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId1), NOT_FOUND);
}
