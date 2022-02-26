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
#ifndef OMIT_ENCRYPT
#include <gtest/gtest.h>
#include <fstream>

#include "distributeddb_data_generate_unit_test.h"
#include "platform_specific.h"
#include "process_communicator_test_stub.h"
#include "process_system_api_adapter_impl.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;
    std::string g_exportFileDir;
    std::vector<std::string> g_junkFilesList;

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    KvStoreNbDelegate *g_kvNbDelegatePtrWithoutPasswd = nullptr;
    KvStoreDelegate *g_kvDelegatePtrWithoutPasswd = nullptr;
    KvStoreDelegate::Option g_option;
    const size_t MAX_PASSWD_SIZE = 128;

    // define the g_valueCallback, used to query a value object data from the kvdb.
    DBStatus g_valueStatus = INVALID_ARGS;
    Value g_value;

    CipherPassword g_passwd1;
    CipherPassword g_passwd2;
    CipherPassword g_passwd3;
    CipherPassword g_passwd4;
    // the type of g_valueCallback is function<void(DBStatus, Value)>
    auto g_valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(g_valueStatus), std::ref(g_value));

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));

    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    void RemoveJunkFile(const std::vector<std::string> &fileList)
    {
        for (auto &junkFile : fileList) {
            std::ifstream file(junkFile);
            if (file) {
                file.close();
                int result = remove(junkFile.c_str());
                if (result < 0) {
                    LOGE("failed to delete the db file:%d", errno);
                }
            }
        }
        return;
    }
}

class DistributedDBInterfacesImportAndExportTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesImportAndExportTest::SetUpTestCase(void)
{
    g_mgr.SetProcessLabel("6666", "8888");
    g_mgr.SetProcessCommunicator(std::make_shared<ProcessCommunicatorTestStub>());
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    g_exportFileDir = g_testDir + "/ExportDir";
    OS::MakeDBDirectory(g_exportFileDir);
    vector<uint8_t> passwdBuffer1(5, 1);
    int errCode = g_passwd1.SetValue(passwdBuffer1.data(), passwdBuffer1.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    vector<uint8_t> passwdBuffer2(5, 2);
    errCode = g_passwd2.SetValue(passwdBuffer2.data(), passwdBuffer2.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    vector<uint8_t> passwdBuffer3(5, 3);
    errCode = g_passwd3.SetValue(passwdBuffer3.data(), passwdBuffer3.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    vector<uint8_t> passwdBuffer4(5, 4);
    errCode = g_passwd4.SetValue(passwdBuffer4.data(), passwdBuffer4.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
}

void DistributedDBInterfacesImportAndExportTest::TearDownTestCase(void)
{
    OS::RemoveDBDirectory(g_exportFileDir);
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesImportAndExportTest::SetUp(void)
{
    g_junkFilesList.clear();
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesImportAndExportTest::TearDown(void)
{
    RemoveJunkFile(g_junkFilesList);
}

/**
  * @tc.name: NormalExport001
  * @tc.desc: The data of the current version of the board is exported and the package file is single.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, NormalExport001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Pre-create folder dir
     */
    std::string singleExportFileName = g_exportFileDir + "/singleNormalExport001.$$";
    std::string singleStoreId = "distributed_ExportSingle_001";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2. Specify the path to export the non-encrypted board database.
     * @tc.expected: step2. Returns OK
     */
    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);

    std::string mulitExportFileName = g_exportFileDir + "/mulitNormalExport001.$$";
    std::string multiStoreId = "distributed_ExportMulit_001";
    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step3. Specify the path to export the multi-version non-encrypted database.
     * @tc.expected: step3. Returns OK
     */
    EXPECT_EQ(g_kvDelegatePtr->Export(mulitExportFileName, passwd), OK);

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);
    g_junkFilesList.push_back(mulitExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
}

/**
  * @tc.name: UndisturbedlSingleExport001
  * @tc.desc: Check that the export action is an independent transaction.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, UndisturbedlSingleExport001, TestSize.Level1)
{
    std::string singleStoreId = "distributed_ExportSingle_002";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step1. Three known data records are preset in the board database.
     */
    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);
    g_kvNbDelegatePtr->Put(KEY_2, VALUE_2);
    g_kvNbDelegatePtr->Put(KEY_3, VALUE_3);

    /**
     * @tc.steps: step2. Execute the export action.
     */
    std::string singleExportFileName = g_exportFileDir + "/UndisturbedlSingleExport001.$$";
    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step3. Insert multiple new data records into the database.
     */
    g_kvNbDelegatePtr->Put(KEY_4, VALUE_4);
    g_kvNbDelegatePtr->Put(KEY_5, VALUE_5);

    /**
     * @tc.steps: step4.  Import backup data.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step5. Check whether the imported data is the preset content in step 1.
     * @tc.expected: step5. Three preset data records are found.
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_3, readValue), OK);
    EXPECT_EQ(readValue, VALUE_3);

    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_4, readValue), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_5, readValue), NOT_FOUND);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
    g_junkFilesList.push_back(singleExportFileName);
}

static void GetSnapshotUnitTest(KvStoreDelegate *&kvDelegatePtr, KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus snapshotDelegateStatus = INVALID_ARGS;
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(snapshotDelegateStatus), std::ref(snapshotDelegatePtr));

    kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallback);
    EXPECT_TRUE(snapshotDelegateStatus == OK);
    ASSERT_TRUE(snapshotDelegatePtr != nullptr);
}

/**
  * @tc.name: UndisturbedlMultiExport001
  * @tc.desc: Check that the export action is an independent transaction.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, UndisturbedlMultiExport001, TestSize.Level1)
{
    std::string multiStoreId = "distributed_Exportmulit_001";
    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step1. Three known data records are preset in the board database.
     */
    g_kvDelegatePtr->Put(KEY_1, VALUE_1);
    g_kvDelegatePtr->Put(KEY_2, VALUE_2);
    g_kvDelegatePtr->Put(KEY_3, VALUE_3);

    /**
     * @tc.steps: step2. Execute the export action.
     */
    std::string mulitExportFileName = g_exportFileDir + "/UndisturbedlMultiExport001.$$";
    CipherPassword passwd;
    EXPECT_EQ(g_kvDelegatePtr->Export(mulitExportFileName, passwd), OK);

    /**
     * @tc.steps: step3. Insert multiple new data records into the database.
     */
    g_kvDelegatePtr->Put(KEY_4, VALUE_4);
    g_kvDelegatePtr->Put(KEY_5, VALUE_5);

    /**
     * @tc.steps: step4.  Import backup data.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Import(mulitExportFileName, passwd), OK);

    KvStoreSnapshotDelegate *snapshotDelegatePtr = nullptr;
    GetSnapshotUnitTest(g_kvDelegatePtr, snapshotDelegatePtr);

    /**
     * @tc.steps: step5. Check whether the imported data is the preset content in step 1.
     * @tc.expected: step5. Three preset data records are found.
     */
    snapshotDelegatePtr->Get(KEY_1, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(g_value, VALUE_1);
    snapshotDelegatePtr->Get(KEY_2, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(g_value, VALUE_2);
    snapshotDelegatePtr->Get(KEY_3, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(g_value, VALUE_3);

    snapshotDelegatePtr->Get(KEY_4, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);
    snapshotDelegatePtr->Get(KEY_5, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);

    if (g_kvDelegatePtr != nullptr && snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotDelegatePtr) == OK);
        snapshotDelegatePtr = nullptr;
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
    g_junkFilesList.push_back(mulitExportFileName);
}

/**
  * @tc.name: ExportParameterCheck001
  * @tc.desc: Check the verification of abnormal interface parameters.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, ExportParameterCheck001, TestSize.Level1)
{
    std::string singleStoreId = "distributed_ExportSingle_003";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);

    /**
     * @tc.steps: step1. The filePath path does not exist.
     * @tc.expected: step1. Return INVALID_ARGS.
     */
    std::string invalidFileName = g_exportFileDir + "/jadaksdjadkjsa/" + "/ExportParameterCheck001.$$";
    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(invalidFileName, passwd), INVALID_ARGS);

    /**
     * @tc.steps: step2. Password length MAX_PASSWD_SIZE + 1
     * @tc.expected: step2. Return INVALID_ARGS.
     */
    vector<uint8_t> passwdBuffer(MAX_PASSWD_SIZE + 1, MAX_PASSWD_SIZE);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OVERSIZE);
    /**
     * @tc.steps: step3. Password length MAX_PASSWD_SIZE
     * @tc.expected: step3. Return OK.
     */
    passwdBuffer.resize(MAX_PASSWD_SIZE, MAX_PASSWD_SIZE);
    errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    std::string singleExportFileName = g_exportFileDir + "/ExportParameterCheck001.$$";
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);
    // Check export FILE_ALREADY_EXISTED
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), FILE_ALREADY_EXISTED);

    /**
     * @tc.steps: step4. Delete the database.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);

    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step5. Use the password to import the file again,
     * @tc.expected: step5. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);
    Value readValue;
    g_kvNbDelegatePtr->Get(KEY_1, readValue);
    EXPECT_EQ(readValue, VALUE_1);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
    g_junkFilesList.push_back(singleExportFileName);
}

/**
  * @tc.name: ExportParameterCheck002
  * @tc.desc: Check the verification of abnormal interface parameters.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, ExportParameterCheck002, TestSize.Level1)
{
    std::string multiStoreId = "distributed_ExportMulti_003";
    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    g_kvDelegatePtr->Put(KEY_1, VALUE_1);

    /**
     * @tc.steps: step1. The filePath path does not exist.
     * @tc.expected: step1. Return INVALID_ARGS.
     */
    std::string invalidExportFileName = g_exportFileDir + "/jadaksdjadkjsa/" + "/ExportParameterCheck002.$$";
    CipherPassword passwd;
    EXPECT_EQ(g_kvDelegatePtr->Export(invalidExportFileName, passwd), INVALID_ARGS);

    /**
     * @tc.steps: step2. Password length MAX_PASSWD_SIZE + 1
     * @tc.expected: step2. Return INVALID_ARGS.
     */
    vector<uint8_t> passwdBuffer(MAX_PASSWD_SIZE + 1, MAX_PASSWD_SIZE);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OVERSIZE);
    /**
     * @tc.steps: step3. Password length MAX_PASSWD_SIZE
     * @tc.expected: step3. Return OK.
     */
    passwdBuffer.resize(MAX_PASSWD_SIZE, MAX_PASSWD_SIZE);
    errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    std::string multiExportFileName = g_exportFileDir + "/ExportParameterCheck002.$$";
    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileName, passwd), OK);
    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileName, passwd), FILE_ALREADY_EXISTED); // Check export INVALID_FILE

    /**
     * @tc.steps: step4. Delete the database.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);

    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step5. Use the password to import the file again,
     * @tc.expected: step5. Return OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, passwd), OK);

    KvStoreSnapshotDelegate *snapshotDelegatePtr = nullptr;
    GetSnapshotUnitTest(g_kvDelegatePtr, snapshotDelegatePtr);

    snapshotDelegatePtr->Get(KEY_1, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(g_value, VALUE_1);

    if (g_kvDelegatePtr != nullptr && snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotDelegatePtr) == OK);
        snapshotDelegatePtr = nullptr;
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
    g_junkFilesList.push_back(multiExportFileName);
}

/**
  * @tc.name: NormalImport001
  * @tc.desc: Normal import capability for single version, parameter verification capability
  * @tc.type: FUNC
  * @tc.require: AR000D487A
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, NormalImport001, TestSize.Level1)
{
    std::string singleExportFileName = g_exportFileDir + "/NormalImport001.$$";
    std::string singleStoreId = "distributed_Importmulti_001";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);

    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step1. Import the invalid path.
     * @tc.expected: step1. Return INVALID_ARGS.
     */
    std::string invalidPath = g_exportFileDir + "sdad" + "/NormalImport001.$$";
    EXPECT_EQ(g_kvNbDelegatePtr->Import(invalidPath, passwd), INVALID_ARGS);

    /**
     * @tc.steps: step2. Import an authorized path with an incorrect password.
     * @tc.expected: step2. Return INVALID_FILE.
     */
    vector<uint8_t> passwdBuffer(MAX_PASSWD_SIZE, MAX_PASSWD_SIZE);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), INVALID_FILE);

    /**
     * @tc.steps: step3. Import a permission path without a password.
     * @tc.expected: step3. Return OK.
     */
    errCode = passwd.Clear();
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step4. Check whether the data is the same as the backup database.
     * @tc.expected: step4. Same database data.
     */
    Value readValue;
    g_kvNbDelegatePtr->Get(KEY_1, readValue);
    EXPECT_EQ(readValue, VALUE_1);

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
}

/**
  * @tc.name: NormalImport001
  * @tc.desc: Normal import capability for multi version, parameter verification capability
  * @tc.type: FUNC
  * @tc.require: AR000D487A
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, NormalImport002, TestSize.Level1)
{
    std::string multiExportFileName = g_exportFileDir + "/NormalImport002.$$";
    std::string multiStoreId = "distributed_ImportSingle_002";
    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    g_kvDelegatePtr->Put(KEY_1, VALUE_1);

    CipherPassword passwd;
    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileName, passwd), OK);

    /**
     * @tc.steps: step1. Import the invalid path.
     * @tc.expected: step1. Return INVALID_ARGS.
     */
    std::string invalidPath = g_exportFileDir + "sdad" + "/NormalImport002.$$";
    EXPECT_EQ(g_kvDelegatePtr->Import(invalidPath, passwd), INVALID_ARGS);

    /**
     * @tc.steps: step2. Import an authorized path with an incorrect password.
     * @tc.expected: step2. Return INVALID_FILE.
     */
    vector<uint8_t> passwdBuffer(MAX_PASSWD_SIZE, MAX_PASSWD_SIZE);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, passwd), INVALID_FILE);

    g_kvDelegatePtr->Delete(KEY_1);
    /**
     * @tc.steps: step3. Import a permission path without a password.
     * @tc.expected: step3. Return OK.
     */
    errCode = passwd.Clear();
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, passwd), OK);

    KvStoreSnapshotDelegate *snapshotDelegatePtr = nullptr;
    GetSnapshotUnitTest(g_kvDelegatePtr, snapshotDelegatePtr);

    /**
     * @tc.steps: step4. Check whether the data is the same as the backup database.
     * @tc.expected: step4. Same database data.
     */
    snapshotDelegatePtr->Get(KEY_1, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(g_value, VALUE_1);

    if (g_kvDelegatePtr != nullptr && snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotDelegatePtr) == OK);
        snapshotDelegatePtr = nullptr;
    }

    // clear resource
    g_junkFilesList.push_back(multiExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
}

/**
  * @tc.name: ExceptionFileImport001
  * @tc.desc: Normal import capability for single version, parameter verification capability
  * @tc.type: FUNC
  * @tc.require: AR000D487A
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, ExceptionFileImport001, TestSize.Level1)
{
    std::string singleExportFileName = g_exportFileDir + "/ExceptionFileImport001.$$";
    std::string singleStoreId = "distributed_ImportExceptionsigle_001";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    g_kvNbDelegatePtr->Put(KEY_2, VALUE_2);

    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step1. Repeat import backup file to same database.
     * @tc.expected: step1. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step2. Change the name of file1 to file2.
     */
    std::string newSingleExportFileName = g_exportFileDir + "/newExceptionFileImport001.$$";
    EXPECT_EQ(rename(singleExportFileName.c_str(), newSingleExportFileName.c_str()), 0);

    /**
     * @tc.steps: step3. Import file1 into the database.
     * @tc.expected: step3. Return INVALID_FILE.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), INVALID_FILE);

    /**
     * @tc.steps: step4. Import file2 into the database.
     * @tc.expected: step4. Return INVALID_FILE.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(newSingleExportFileName, passwd), OK);

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);
    g_junkFilesList.push_back(newSingleExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
}

/**
  * @tc.name: ExceptionFileImport002
  * @tc.desc: Normal import capability for multi version, parameter verification capability
  * @tc.type: FUNC
  * @tc.require: AR000D487A
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, ExceptionFileImport002, TestSize.Level1)
{
    std::string multiExportFileName = g_exportFileDir + "/ExceptionFileImport002.$$";
    std::string multiStoreId = "distributed_ImportExceptionMulti_001";
    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    g_kvDelegatePtr->Put(KEY_1, VALUE_1);

    CipherPassword passwd;
    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileName, passwd), OK);

    /**
     * @tc.steps: step1. Import the backup file that has been tampered with to the multi-version database.
     * @tc.expected: step1. Return INVALID_FILE.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::ModifyDatabaseFile(multiExportFileName), 0);
    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, passwd), INVALID_FILE);

    // clear resource
    g_junkFilesList.push_back(multiExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
}

/**
  * @tc.name: ExceptionFileImport003
  * @tc.desc: The data of the current version of the board is exported and the package file is single.
  * @tc.type: FUNC
  * @tc.require: AR000D487A
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, ExceptionFileImport003, TestSize.Level1)
{
    std::string singleExportFileName = g_exportFileDir + "/singleExceptionFileImport003.$$";
    std::string singleStoreId = "distributed_ExportSingle_001";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);

    std::string mulitExportFileName = g_exportFileDir + "/mulitExceptionFileImport003.$$";
    std::string multiStoreId = "distributed_ExportMulit_001";
    g_mgr.GetKvStore(multiStoreId, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvDelegatePtr->Export(mulitExportFileName, passwd), OK);

    /**
     * @tc.steps: step1. Use the single ver import interface. The file path is a multi-version backup file.
     * @tc.expected: step1. Return INVALID_FILE.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(mulitExportFileName, passwd), INVALID_FILE);

    /**
     * @tc.steps: step2.  Use the single ver import interface. The file path is a single-version backup file.
     * @tc.expected: step2. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);

    /**
     * @tc.steps: step3.  Use the multi-version import interface. The file path is a single-version backup file.
     * @tc.expected: step3. Return INVALID_FILE.
     */
    EXPECT_EQ(g_kvDelegatePtr->Import(singleExportFileName, passwd), INVALID_FILE);

    /**
     * @tc.steps: step4.  Use the multi-version import interface. The file path is a multi-version backup file.
     * @tc.expected: step4. Return INVALID_FILE.
     */
    EXPECT_EQ(g_kvDelegatePtr->Import(mulitExportFileName, passwd), OK);

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);
    g_junkFilesList.push_back(mulitExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
}

/**
  * @tc.name: ExceptionFileImport004
  * @tc.desc: The data of the current version of the board is exported and the package file is single.
  * @tc.type: FUNC
  * @tc.require: AR000D487A
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, ExceptionFileImport004, TestSize.Level1)
{
    std::string singleExportFileName = g_exportFileDir + "/singleExceptionFileImport004.$$";
    std::string singleStoreId = "distributed_ExportSingle_004";
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, g_passwd2), OK);

    std::string mulitExportFileName = g_exportFileDir + "/mulitExceptionFileImport004.$$";
    std::string multiStoreId = "distributed_ExportMulit_004";

    KvStoreDelegate::Option multiOption = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(multiStoreId, multiOption, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);
    EXPECT_EQ(g_kvDelegatePtr->Export(mulitExportFileName, g_passwd2), OK);

    /**
     * @tc.steps: step1. Use the diff passwd, try to import database.
     */
    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), INVALID_FILE);
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, g_passwd2), OK);

    EXPECT_EQ(g_kvDelegatePtr->Import(mulitExportFileName, passwd), INVALID_FILE);
    EXPECT_EQ(g_kvDelegatePtr->Import(mulitExportFileName, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_kvDelegatePtr->Import(mulitExportFileName, g_passwd2), OK);

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);
    g_junkFilesList.push_back(mulitExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
}

static void TryDbForPasswordIndependence001()
{
    std::string singleStoreIdNoPasswd = "distributed_ExportSingle_005";
    std::string singleStoreId = "distributed_ExportSingle_006";

    /**
     * @tc.steps: step4. Run the p3 command to open the database db1.
     * @tc.expected: step4. Return ERROR.
     */
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(singleStoreIdNoPasswd, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_NE(g_kvDelegateStatus, OK);

    /**
     * @tc.steps: step5. Run the p4 command to open the database db2.
     * @tc.expected: step5. Return ERROR.
     */
    option = {true, false, true, CipherType::DEFAULT, g_passwd4};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    ASSERT_TRUE(g_kvDelegateStatus != OK);

    /**
     * @tc.steps: step6. Open the db1 directly.
     * @tc.expected: step6. Return OK.
     */
    option = {true, false, false, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(singleStoreIdNoPasswd, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);
    g_kvNbDelegatePtrWithoutPasswd = g_kvNbDelegatePtr;

    /**
     * @tc.steps: step7. Open the db1 directly
     * @tc.expected: step7. Return ERROR.
     */
    option = {true, false, false, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);
    ASSERT_TRUE(g_kvDelegateStatus != OK);

    /**
     * @tc.steps: step8. Run the p2 command to open the db2 file.
     * @tc.expected: step8. Return ERROR.
     */
    option = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);
}

/**
  * @tc.name: PasswordIndependence001
  * @tc.desc: The data of the current version of the board is exported and the package file is single.
  * @tc.type: FUNC
  * @tc.require: AR000D487B
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, PasswordIndependence001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Back up a single database db1 No password backup password p3
     */
    std::string singleExportFileNameNoPasswd = g_exportFileDir + "/singleNoPasswdIndependence001.$$";
    std::string singleStoreIdNoPasswd = "distributed_ExportSingle_005";
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(singleStoreIdNoPasswd, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    g_kvNbDelegatePtrWithoutPasswd = g_kvNbDelegatePtr;

    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileNameNoPasswd, g_passwd3), OK);

    /**
     * @tc.steps: step2. Back up the database of the single version db2 Password p2 Backup file password p4
     */
    std::string singleExportFileName = g_exportFileDir + "/singleIndependence001.$$";
    std::string singleStoreId = "distributed_ExportSingle_006";
    option = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, g_passwd4), OK);

    /**
     * @tc.steps: step3. Recover the backup file.
     */
    EXPECT_EQ(g_kvNbDelegatePtrWithoutPasswd->Import(singleExportFileNameNoPasswd, g_passwd3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, g_passwd4), OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrWithoutPasswd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    (void)TryDbForPasswordIndependence001();

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);
    g_junkFilesList.push_back(singleExportFileNameNoPasswd);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrWithoutPasswd), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreIdNoPasswd), OK);
}

static void TryDbForPasswordIndependence002()
{
    std::string multiStoreIdNoPasswd = "distributed_ExportMulti_007";
    std::string multiStoreId = "distributed_ExportMulti_008";

    KvStoreDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(multiStoreIdNoPasswd, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    ASSERT_TRUE(g_kvDelegateStatus != OK);

    option = {true, false, true, CipherType::DEFAULT, g_passwd4};
    g_mgr.GetKvStore(multiStoreId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    ASSERT_TRUE(g_kvDelegateStatus != OK);

    option = {true, false, false, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(multiStoreIdNoPasswd, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);
    g_kvDelegatePtrWithoutPasswd = g_kvDelegatePtr;

    option = {true, false, false, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(multiStoreId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    ASSERT_TRUE(g_kvDelegateStatus != OK);

    option = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(multiStoreId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);
}

/**
  * @tc.name: PasswordIndependence002
  * @tc.desc: The data of the current version of the board is exported and the package file is single.
  * @tc.type: FUNC
  * @tc.require: AR000D487B
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, PasswordIndependence002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Back up a single database db1 No password backup password p3
     */
    std::string multiExportFileNameNoPasswd = g_exportFileDir + "/multiNoPasswdIndependence001.$$";
    std::string multiStoreIdNoPasswd = "distributed_ExportMulti_007";
    KvStoreDelegate::Option option;
    g_mgr.GetKvStore(multiStoreIdNoPasswd, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    g_kvDelegatePtrWithoutPasswd = g_kvDelegatePtr;

    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileNameNoPasswd, g_passwd3), OK);

    /**
     * @tc.steps: step2. Back up the database of the single version db2 Password p2 Backup file password p4
     */
    std::string multiExportFileName = g_exportFileDir + "/multiIndependence001.$$";
    std::string multiStoreId = "distributed_ExportMulti_008";
    option = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(multiStoreId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileName, g_passwd4), OK);

    /**
     * @tc.steps: step3. Recover the backup file.
     */
    EXPECT_EQ(g_kvDelegatePtrWithoutPasswd->Import(multiExportFileNameNoPasswd, g_passwd3), OK);
    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, g_passwd4), OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtrWithoutPasswd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    /**
     * @tc.steps: step4. Try diff passwd.
     */
    (void)TryDbForPasswordIndependence002();

    // clear resource
    g_junkFilesList.push_back(multiExportFileName);
    g_junkFilesList.push_back(multiExportFileNameNoPasswd);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtrWithoutPasswd), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreIdNoPasswd), OK);
}

/**
  * @tc.name: PasswordIndependence002
  * @tc.desc: The data of the current version of the board is exported and the package file is single.
  * @tc.type: FUNC
  * @tc.require: AR000D487B
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, PasswordIndependence003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Back up the (passwd1) encryption single-version (passwd2) database.
     */
    std::string singleExportFileName = g_exportFileDir + "/singleIndependence003.$$";
    std::string singleStoreId = "distributed_ExportSingle_009";
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, g_passwd1), OK);

    /**
     * @tc.steps: step2. Rekey The password by passwd3
     */
    g_kvNbDelegatePtr->Rekey(g_passwd3);

    /**
     * @tc.steps: step3. Import the database using passwd3.
     * @tc.expected: step3. Return INVALID_FILE.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, g_passwd3), INVALID_FILE);

    /**
     * @tc.steps: step4. Import the database using passwd1.
     * @tc.expected: step4. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, g_passwd1), OK);

    /**
     * @tc.steps: step5. Repeat step 1 - 4.
     */
    std::string multiExportFileName = g_exportFileDir + "/multiIndependence003.$$";
    std::string multiStoreId = "distributed_ExportMulti_010";
    KvStoreDelegate::Option multiOption = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(multiStoreId, multiOption, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvDelegatePtr->Export(multiExportFileName, g_passwd1), OK);
    remove(singleExportFileName.c_str());

    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, g_passwd3), INVALID_FILE);
    EXPECT_EQ(g_kvDelegatePtr->Import(multiExportFileName, g_passwd1), OK);

    // clear resource
    g_junkFilesList.push_back(multiExportFileName);
    g_junkFilesList.push_back(singleExportFileName);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(multiStoreId), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
}

/**
  * @tc.name: SeparaDbExportAndImport
  * @tc.desc: Import and export after Separate database.
  * @tc.type: FUNC
  * @tc.require: AR000D487B
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, SeparaDbExportAndImport, TestSize.Level1)
{
    std::shared_ptr<ProcessSystemApiAdapterImpl> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(adapter);

    std::string singleExportFileName = g_exportFileDir + "/SeparaDbExportAndImport.$$";
    std::string singleStoreId = "distributed_ExportSingle_010";
    KvStoreNbDelegate::Option option = {true, false, false};
    SecurityOption secOption{SecurityLabel::S3, SecurityFlag::SECE};
    option.secOption = secOption;

    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);

    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);

    CipherPassword passwd;
    EXPECT_EQ(g_kvNbDelegatePtr->Export(singleExportFileName, passwd), OK);

    g_kvNbDelegatePtr->Put(KEY_2, VALUE_2);

    EXPECT_EQ(g_kvNbDelegatePtr->Import(singleExportFileName, passwd), OK);
    Value valueRead;
    g_kvNbDelegatePtr->Get(KEY_2, valueRead);
    EXPECT_EQ(valueRead, Value());
    g_kvNbDelegatePtr->Get(KEY_1, valueRead);
    EXPECT_EQ(valueRead, VALUE_1);
    g_kvNbDelegatePtr->Put(KEY_3, VALUE_3);

    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd1), OK);
    g_kvNbDelegatePtr->Get(KEY_3, valueRead);
    EXPECT_EQ(valueRead, VALUE_3);

    // clear resource
    g_junkFilesList.push_back(singleExportFileName);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr == nullptr);

    option.passwd = g_passwd1;
    option.isEncryptedDb = true;
    g_mgr.GetKvStore(singleStoreId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_kvDelegateStatus, OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(singleStoreId), OK);
}

/**
  * @tc.name: SeparaDbExportAndImport
  * @tc.desc: Import and export after Separate database.
  * @tc.type: FUNC
  * @tc.require: AR000D487B
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesImportAndExportTest, SeparaDbNoPasswdRekey, TestSize.Level1)
{
    std::shared_ptr<ProcessSystemApiAdapterImpl> adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(adapter);

    KvStoreNbDelegate::Option option = {true, false, true};
    SecurityOption secOption{SecurityLabel::S3, SecurityFlag::SECE};
    option.secOption = secOption;
    option.passwd = g_passwd1;
    g_mgr.GetKvStore("SeparaDbNoPasswdRekey", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);

    EXPECT_EQ(g_kvDelegateStatus, OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    option.passwd = g_passwd2;
    g_mgr.GetKvStore("SeparaDbNoPasswdRekey", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("SeparaDbNoPasswdRekey"), OK);
}
#endif
