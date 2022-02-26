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
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>

#include "types.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributeddb_nb_test_tools.h"
#include "distributeddb_data_generator.h"
#include "process_communicator_test_stub.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbBackup {
KvStoreNbDelegate *g_nbBackupDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;
static std::condition_variable g_backupVar;

DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;
DistributedDB::CipherPassword g_filePasswd1;
DistributedDB::CipherPassword g_filePasswd2;

class DistributeddbNbBackupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbBackupTest::SetUpTestCase(void)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    manager->SetProcessLabel("MST", "GetDevicesID");
    manager->SetProcessCommunicator(std::make_shared<ProcessCommunicatorTestStub>());
    delete manager;
    manager = nullptr;
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    (void)g_passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
    (void)g_filePasswd1.SetValue(FILE_PASSWD_VECTOR_1.data(), FILE_PASSWD_VECTOR_1.size());
    (void)g_filePasswd2.SetValue(FILE_PASSWD_VECTOR_2.data(), FILE_PASSWD_VECTOR_2.size());
}

void DistributeddbNbBackupTest::TearDownTestCase(void)
{
}

void DistributeddbNbBackupTest::SetUp(void)
{
    const std::string exportPath = NB_DIRECTOR + "export";
    RemoveDatabaseDirectory(exportPath);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_nbBackupDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_manager != nullptr && g_nbBackupDelegate != nullptr);
}

void DistributeddbNbBackupTest::TearDown(void)
{
    MST_LOG("TearDown after case.");
    ASSERT_NE(g_manager, nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(g_manager, g_nbBackupDelegate, STORE_ID_1, g_option.isMemoryDb));
}

/*
 * @tc.name: ExportTest001
 * @tc.desc: test checking in parameter of filepath of export interface.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call export interface and the dir of filepath is nonexistent.
     * @tc.expected: step1. call failed and return INVALID_ARGS.
     */
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    const std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), INVALID_ARGS);

    /**
     * @tc.steps: step2. call export interface, the filepath is absolute path and legal then check file number.
     * @tc.expected: step2. call successfully and the file number is 1.
     */
    SetDir(exportPath);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);
    RemoveDir(exportPath);
    RemoveDatabaseDirectory(exportPath);

    /**
     * @tc.steps: step3. call export interface to export data and the dir of filepath is no r,w right.
     * @tc.expected: step3. call failed return INVALID_ARGS.
     */
#ifdef RUNNING_ON_SIMULATED_ENV
    const std::string noRightPath = "../noright";
    const int authRight = 0111;
    SetDir(noRightPath, authRight);
    std::string filePath2 = noRightPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath2, NULL_PASSWD), NO_PERMISSION);
    RemoveDatabaseDirectory(noRightPath);
#endif
    /**
     * @tc.steps: step4. call export interface, the filepath is relative path and legal then check file number.
     * @tc.expected: step4. call successfully and the file number is 1.
     */
    const std::string exportPath2 = "../export";
    SetDir(exportPath2);
    const std::string filePath3 = exportPath2 + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath3, NULL_PASSWD), OK);
    CheckFileNumber(exportPath2, fileCount);
    EXPECT_EQ(fileCount, 1);
    RemoveDir(exportPath2);
}

/*
 * @tc.name: ExportTest 002
 * @tc.desc: Verify that the export interface will return FILE_ALREADY_EXISTED if the export file has been exist and
 * the memory db doesn't support export.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call export interface and the dir of filepath is legal then check file number.
     * @tc.expected: step1. call successfully and the file number of filepath is 1.
     */
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, NULL_PASSWD) == OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);

    /**
     * @tc.steps: step2. call export interface and the directory of file is existent.
     * @tc.expected: step2. call failed and return FILE_ALREADY_EXISTED.
     */
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, NULL_PASSWD) == FILE_ALREADY_EXISTED);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1); // the number of files is 1.

    /**
     * @tc.steps: step3. create memory db and call export interface.
     * @tc.expected: step3. call failed and return NOT_SUPPORT.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option;
    option.isMemoryDb = true;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    filePath = exportPath + "/bkpDB1.bin";
    EXPECT_TRUE(delegate->Export(filePath, NULL_PASSWD) == NOT_SUPPORT);
    EXPECT_EQ((manager->CloseKvStore(delegate)), OK);

    /**
     * @tc.steps: step4. call export interface when there is unclosed resultset and check the number of files.
     * @tc.expected: step4. call successfully and the number of files is 2.
     */
    KvStoreResultSet *resultSet;
    EXPECT_EQ(g_nbBackupDelegate->GetEntries(KEY_EMPTY, resultSet), OK);
    filePath = exportPath + "/bkpDB2.bin";
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, NULL_PASSWD) == OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 2); // the number of files is 2.
    EXPECT_EQ(g_nbBackupDelegate->CloseResultSet(resultSet), OK);
    RemoveDir(exportPath);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ExportTest 003
 * @tc.desc: Verify that the export interface will return busy when the import hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest003, TestSize.Level2)
{
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, NULL_PASSWD) == OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);

    /**
     * @tc.steps: step1. call import interface to import the file that prepared in advance.
     * @tc.expected: step1. call successfully.
     */
    thread subThread([&]() {
        EXPECT_EQ(g_nbBackupDelegate->Import(filePath, NULL_PASSWD), OK);
        g_backupVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call export interface when step1 is running.
     * @tc.expected: step2. return busy if step1 hasn't completed, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_TWO_HUNDREDS_MS));
    filePath = exportPath + "/bkpDB1.bin";
    DBStatus status = g_nbBackupDelegate->Export(filePath, NULL_PASSWD);
    EXPECT_TRUE(status == BUSY || status == OK);
    CheckFileNumber(exportPath, fileCount);
    if (status == BUSY) {
        EXPECT_EQ(fileCount, 1);
    } else {
        EXPECT_EQ(fileCount, 2); // if export success, the number of file is 2.
    }

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_backupVar.wait(lck);
    RemoveDir(exportPath);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: ExportTest 004
 * @tc.desc: Verify that the export interface will return busy when the rekey hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest004, TestSize.Level3)
{
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, NB_PREDATA_NUM, ONE_K_LONG_STRING, TWO_M_LONG_STRING);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }
    /**
     * @tc.steps: step1. call import interface to import the file that prepared in advance.
     * @tc.expected: step1. call successfully if step1 running before step2, else return busy.
     */
    thread subThread([&]() {
        DBStatus rekeyStatus = g_nbBackupDelegate->Rekey(g_passwd1);
        EXPECT_TRUE(rekeyStatus == OK || rekeyStatus == BUSY);
        MST_LOG("The rekeyStatus is %d", rekeyStatus);
        g_backupVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call export interface when step1 is running.
     * @tc.expected: step2. return busy if step1 is running, else return OK.
     */
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    const std::string filePath = exportPath + "/bkpDB.bin";
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LONG_TIME));
    DBStatus status = g_nbBackupDelegate->Export(filePath, NULL_PASSWD);
    EXPECT_TRUE(status == BUSY || status == OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_TRUE(fileCount == 0 || fileCount == 1);

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_backupVar.wait(lck);
    RemoveDir(exportPath);
}
#endif

/*
 * @tc.name: ExportTest 005
 * @tc.desc: Verify that the export interface will execute blockly when last export hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest005, TestSize.Level2)
{
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, FOUR_M_LONG_STRING);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }
    /**
     * @tc.steps: step1. call import interface to import the file that prepared in advance.
     * @tc.expected: step1. call successfully.
     */
    bool exportFlag = false;
    thread subThread([&]() {
        std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LONG_TIME));
        DBStatus exportStatus = g_nbBackupDelegate->Export(filePath, NULL_PASSWD);
        EXPECT_EQ(exportStatus, OK);
        exportFlag = true;
        g_backupVar.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call export interface when step1 is running.
     * @tc.expected: step2. it can be called after last export finished.
     */
    std::string filePath2 = exportPath + "/bkpDB1.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath2, NULL_PASSWD), OK);

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_backupVar.wait(lck, [&]{return exportFlag;});
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 2); // the export file number is 2.
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 006
 * @tc.desc: Verify that the put/delete operation will execute blockly when export hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest006, TestSize.Level3)
{
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, FIFTY_RECORDS, ONE_K_LONG_STRING, TWO_M_LONG_STRING);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
        EXPECT_TRUE(g_nbBackupDelegate->PutLocal(iter->key, iter->value) == OK);
    }
    /**
     * @tc.steps: step1. call export interface in subthread.
     * @tc.expected: step1. call successfully.
     */
    bool exportFlag = false;
    thread subThread([&]() {
        DBStatus exportStatus = g_nbBackupDelegate->Export(filePath, NULL_PASSWD);
        EXPECT_EQ(exportStatus, OK);
        exportFlag = true;
        g_backupVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call Get,GetLocal,GetEntries to query data from db.
     * @tc.expected: step2. call successfully(if export continued over 30s, put will return busy).
     */
    Value valueResult;
    EXPECT_TRUE(g_nbBackupDelegate->Get(allKeys[0], valueResult) == OK);
    EXPECT_EQ(valueResult, entriesBatch[0].value);
    EXPECT_TRUE(g_nbBackupDelegate->GetLocal(allKeys[1], valueResult) == OK);
    EXPECT_EQ(valueResult, entriesBatch[1].value);
    std::vector<Entry> entries;
    EXPECT_TRUE(g_nbBackupDelegate->GetEntries(KEY_EMPTY, entries) == OK);
    EXPECT_EQ(entries.size(), entriesBatch.size());
    /**
     * @tc.steps: step3. call put/delete/putlocal/deletelocal interface when step1 is running.
     * @tc.expected: step3. return ok, if step1 need more than 30s to finish, it will return busy.
     */
    DBStatus status = g_nbBackupDelegate->Put(entriesBatch[0].key, entriesBatch[0].value);
    EXPECT_TRUE((status == OK) || (status == BUSY));
    status = g_nbBackupDelegate->PutLocal(entriesBatch[1].key, entriesBatch[1].value);
    EXPECT_TRUE((status == OK) || (status == BUSY));
    status = g_nbBackupDelegate->Delete(allKeys[0]);
    EXPECT_TRUE((status == OK) || (status == BUSY));

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_backupVar.wait(lck, [&]{return exportFlag;});
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 007
 * @tc.desc: Verify that passwd parameter of export interface decide the file exported is encrypted or not.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ExportTest007, TestSize.Level1)
{
    int fileCount = 0;
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB1.bin";

    /**
     * @tc.steps: step1. call export interface with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);

    /**
     * @tc.steps: step2. call export interface with the passwd = password(129B).
     * @tc.expected: step2. return INVALID_ARGS.
     */
    filePath = exportPath + "/bkpDB2.bin";
    vector<uint8_t> passwordVector(PASSWD_BYTE, 'a');
    CipherPassword password;
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OVERSIZE);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);

    /**
     * @tc.steps: step3. call export interface with the passwd = password(1B).
     * @tc.expected: step3. return OK.
     */
    filePath = exportPath + "/bkpDB3.bin";
    passwordVector.assign(1, 'b'); // 1 Byte of passwd.
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, password), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 2); // the number of file is 2.

    /**
     * @tc.steps: step3. call export interface with the passwd = password(128B).
     * @tc.expected: step3. return OK.
     */
    filePath = exportPath + "/bkpDB4.bin";
    passwordVector.assign(BATCH_RECORDS, 'b'); // 1 Byte of passwd.
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, password), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 3); // the number of file is 3.
    RemoveDir(exportPath);
}

/*
 * @tc.name: ImportTest 001
 * @tc.desc: Verify that it can only import success when the pass word is rightly match(exist and rightly or not).
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device put (k1, v2) to DB and then update it to (k1, v1).
     * @tc.expected: step1. put successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_1), OK);
    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath1 = importPath + "/bkpDB1.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath1, NULL_PASSWD), OK);
    std::string filePath2 = importPath + "/bkpDB2.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath2, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_2), OK);

    /**
     * @tc.steps: step2. import backup file bkpDB1.bin with passwd f1.
     * @tc.expected: step2. import failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_filePasswd1), INVALID_FILE);

    /**
     * @tc.steps: step3. import backup file bkpDB1.bin with empty passwd and get the value of key1.
     * @tc.expected: step3. import successfully, and the value of key1 is value1.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), OK);
    Value valueResult;
    g_nbBackupDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(valueResult, VALUE_1);

    /**
     * @tc.steps: step4. update the entry (k1, v1) to (k1, v2).
     * @tc.expected: step4. update success.
     */
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step5. import backup file bkpDB2.bin with empty passwd.
     * @tc.expected: step5. import failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), INVALID_FILE);
    /**
     * @tc.steps: step6. import backup file bkpDB2.bin with DB passwd p1.
     * @tc.expected: step6. import failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step7. import backup file bkpDB2.bin with passwd = password(129B).
     * @tc.expected: step7. import failed and return INVALID_FILE.
     */
    vector<uint8_t> passwordVector(PASSWD_BYTE, 'a');
    CipherPassword password;
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OVERSIZE);
    /**
     * @tc.steps: step8. import backup file bkpDB2.bin with DB passwd f1 and get the value of k1.
     * @tc.expected: step8. import success and return INVALID_FILE.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_filePasswd1), OK);
    g_nbBackupDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(valueResult, VALUE_1);
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 002
 * @tc.desc: Verify that memory DB can't support to import.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest002, TestSize.Level0)
{
    std::string importPath1 = NB_DIRECTOR + "export";
    SetDir(importPath1);
    std::string filePath = importPath1 + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. create memory DB and import the backup file.
     * @tc.expected: step1. create success but import failed and return NOT_SUPPORT.
     */
    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    Option option;
    option.isMemoryDb = true;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    EXPECT_EQ(delegate2->Import(filePath, NULL_PASSWD), NOT_SUPPORT);
    /**
     * @tc.steps: step2. the disk DB import backup file bkpDB1.bin from noexsit path with empty passwd.
     * @tc.expected: step2. import failed and returned INVALID_FILE.
     */
    std::string importPath2 = NB_DIRECTOR + "/noexsit";
    std::string filePath2 = importPath2 + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), INVALID_ARGS);

    /**
     * @tc.steps: step3. the disk DB import noexsit backup file bkpDB1.bin from noexsit path with empty passwd..
     * @tc.expected: step3. import failed and returned INVALID_FILE.
     */
    std::string filePath3 = importPath1 + "/bkpDB2.bin";
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath3, NULL_PASSWD), INVALID_FILE);

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    delete manager2;
    manager2 = nullptr;
    RemoveDir(importPath1);
}

/*
 * @tc.name: ImportTest 003
 * @tc.desc: Verify that it can't import the not DB file and damaged file.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest003, TestSize.Level0)
{
    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. import a.txt in the right path.
     * @tc.expected: step1. call failed and return INVALID_FILE.
     */
    std::string filePath1 = importPath + "/a.txt";
    ofstream createFile(filePath1);
    if (createFile) {
        createFile << '1' << endl;
        createFile.close();
    }
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);

    /**
     * @tc.steps: step2. write some data to DB file1.
     * @tc.expected: step2. write successfully
     */
    ofstream damageFile(filePath, ios::out | ios::binary);
    ASSERT_TRUE(damageFile.is_open());
    damageFile.write(reinterpret_cast<char *>(&filePath), filePath.size());
    damageFile.close();

    /**
     * @tc.steps: step3. import DB file with empty password.
     * @tc.expected: step3. import failed and returned INVALID_FILE
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath, NULL_PASSWD), INVALID_FILE);
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 004
 * @tc.desc: Verify that multi-DB can't import nb-DB backup file, and nb-DB can't import multi-DB backup file too.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest004, TestSize.Level1)
{
    KvStoreDelegate *multiEncryptDelegate = nullptr;
    KvStoreDelegateManager *multiEncryptManager = nullptr;

    KvOption option = g_kvOption;
    multiEncryptDelegate = DistributedTestTools::GetDelegateSuccess(multiEncryptManager, g_kvdbParameter2, option);
    ASSERT_TRUE(multiEncryptManager != nullptr && multiEncryptDelegate != nullptr);
    const std::string multiImportPath = DIRECTOR + "export";
    SetDir(multiImportPath);
    std::string multiFilePath1 = multiImportPath + "/bkpDB1.bin";
    multiEncryptDelegate->Export(multiFilePath1, g_filePasswd1);

    KvStoreDelegate *localDelegate = nullptr;
    KvStoreDelegateManager *localManager = nullptr;
    option.localOnly = true;
    localDelegate = DistributedTestTools::GetDelegateSuccess(localManager, g_kvdbParameter3, option);
    ASSERT_TRUE(localManager != nullptr && localDelegate != nullptr);

    std::string multiFilePath2 = multiImportPath + "/bkpDB2.bin";
    localDelegate->Export(multiFilePath2, g_filePasswd1);

    const std::string nbImportPath = NB_DIRECTOR + "export";
    SetDir(nbImportPath);
    std::string nbFilePath = nbImportPath + "/bkpDB1.bin";
    g_nbBackupDelegate->Export(nbFilePath, g_filePasswd1);
    /**
     * @tc.steps: step1. the multi DB1 import the nb DB backup file with the password f1.
     * @tc.expected: step1. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(multiEncryptDelegate->Import(nbFilePath, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step2. the multi DB1 import backup file of local db with the password f1.
     * @tc.expected: step2. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(multiEncryptDelegate->Import(multiFilePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step3. the nb DB import the multi DB backup file with the password f1.
     * @tc.expected: step3. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(multiFilePath1, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step4. the nb DB import the noexsit nb DB backup file with the password f1.
     * @tc.expected: step4. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(multiFilePath2, g_filePasswd1), INVALID_FILE);

    /**
     * @tc.steps: step5. Local DB import nb-backup file with the password f1.
     * @tc.expected: step5. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(localDelegate->Import(nbFilePath, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step6. Local DB import multi-backup file with the password f1.
     * @tc.expected: step6. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(localDelegate->Import(multiFilePath1, g_filePasswd1), INVALID_FILE);
    EXPECT_EQ(multiEncryptManager->CloseKvStore(multiEncryptDelegate), OK);
    multiEncryptDelegate = nullptr;
    EXPECT_EQ(multiEncryptManager->DeleteKvStore(STORE_ID_2), OK);
    delete multiEncryptManager;
    multiEncryptManager = nullptr;
    EXPECT_EQ(localManager->CloseKvStore(localDelegate), OK);
    localDelegate = nullptr;
    EXPECT_EQ(localManager->DeleteKvStore(STORE_ID_3), OK);
    delete localManager;
    localManager = nullptr;
    RemoveDir(multiImportPath);
    RemoveDir(nbImportPath);
}

/*
 * @tc.name: ImportTest 006
 * @tc.desc: Verify that many kvstore of one DB or the DB registered observer can't import the backup file.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest006, TestSize.Level0)
{
    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, g_filePasswd1), OK);

    /**
     * @tc.steps: step1. create another delegate of the same storeId.
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter1, g_option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);

    /**
     * @tc.steps: step2. use the second delegate to import the backup file.
     * @tc.expected: step2. import failed and return busy
     */
    EXPECT_EQ(delegate2->Import(filePath, g_filePasswd1), BUSY);

    EXPECT_TRUE(manager2->CloseKvStore(delegate2) == OK);
    delegate2 = nullptr;
    delete manager2;
    manager2 = nullptr;
    /**
     * @tc.steps: step3. delegate1 register observer.
     * @tc.expected: step3. register success
     */
    KvStoreObserverImpl observerSync;
    EXPECT_EQ(g_nbBackupDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observerSync), OK);
    /**
     * @tc.steps: step4. delegate1 import the backup file with the password f1.
     * @tc.expected: step4. register success
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath, g_filePasswd1), BUSY);
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 007
 * @tc.desc: Verify that the DB registered conflict notifier or hadn't resultSet can't import the backup file.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest007, TestSize.Level0)
{
    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, g_filePasswd1), OK);

    /**
     * @tc.steps: step1. register conflict notifier.
     * @tc.expected: step1. register successfully.
     */
    ConflictNbCallback callback;
    std::vector<ConflictData> conflictData;
    auto notifier = bind(&ConflictNbCallback::NotifyCallBack, &callback, placeholders::_1, &conflictData);
    EXPECT_EQ(g_nbBackupDelegate->SetConflictNotifier(CONFLICT_NATIVE_ALL, notifier), OK);
    /**
     * @tc.steps: step2. import the backup file with the password f1.
     * @tc.expected: step2. import failed and returned BUSY.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath, g_filePasswd1), BUSY);
    EXPECT_EQ(g_nbBackupDelegate->SetConflictNotifier(CONFLICT_NATIVE_ALL, nullptr), OK);

    /**
     * @tc.steps: step3. call GetEntries to get resultSet.
     * @tc.expected: step3. get successfully.
     */
    KvStoreResultSet *resultSet;
    EXPECT_EQ(g_nbBackupDelegate->GetEntries(KEY_K, resultSet), OK);
    /**
     * @tc.steps: step4. import the backup file with the password f1.
     * @tc.expected: step4. import failed and returned BUSY.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath, g_filePasswd1), BUSY);
    EXPECT_EQ(g_nbBackupDelegate->CloseResultSet(resultSet), OK);

    RemoveDir(importPath);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: ImportTest 008
 * @tc.desc: Verify that it will return busy when GetKvStore again or crud during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest008, TestSize.Level3)
{
    vector<Entry> entries;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entries, allKeys, FIFTY_RECORDS, KEY_SIX_BYTE, TWO_M_LONG_STRING);
    for (auto iter = entries.begin(); iter != entries.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }

    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start subthread to import the backup file with empty password.
     * @tc.expected: step1. start successfully if step1 running before step2, else return BUSY.
     */
    thread subThread([&]() {
        DBStatus importStatus = g_nbBackupDelegate->Import(filePath, NULL_PASSWD);
        EXPECT_TRUE(importStatus == OK || importStatus == BUSY);
        g_backupVar.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. GetKvStore again during the subthread is importing.
     * @tc.expected: step2. GetKvStore failed and returned BUSY if step1 is running, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LONG_TIME));
    KvStoreDelegateManager *manager = nullptr;
    DBStatus statusReturn;
    DistributedDBNbTestTools::GetNbDelegateStatus(manager, statusReturn, g_dbParameter1, g_option);
    EXPECT_TRUE(statusReturn == BUSY || statusReturn == OK);
    /**
     * @tc.steps: step3. put (k1, v1) during the subthread is importing.
     * @tc.expected: step3. put failed and returned BUSY if step1 is running, else return OK.
     */
    statusReturn = g_nbBackupDelegate->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(statusReturn == BUSY || statusReturn == OK);
    std::mutex rekeyMtx;
    std::unique_lock<std::mutex> lck(rekeyMtx);
    g_backupVar.wait(lck);
    RemoveDir(importPath);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: ImportTest 009
 * @tc.desc: Verify that it will return busy when rekey during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest009, TestSize.Level3)
{
    vector<Entry> entriesRekey;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesRekey, allKeys, FIFTY_RECORDS, KEY_SIX_BYTE, FOUR_M_LONG_STRING);
    for (auto iter = entriesRekey.begin(); iter != entriesRekey.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }

    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start subthread to import the backup file with empty password.
     * @tc.expected: step1. start successfully if step1 running before step2, else return BUSY.
     */
    thread subThread([&]() {
        DBStatus importStatus = g_nbBackupDelegate->Import(filePath, NULL_PASSWD);
        EXPECT_TRUE(importStatus == OK || importStatus == BUSY);
        g_backupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. rekey  during the subthread is importing.
     * @tc.expected: step2. rekey failed and returned BUSY if step1 is running, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LAST_SYNC));
    DBStatus status = g_nbBackupDelegate->Rekey(g_passwd1);
    EXPECT_TRUE(status == BUSY || status == OK);

    std::mutex rekeyMtx;
    std::unique_lock<std::mutex> lck(rekeyMtx);
    g_backupVar.wait(lck);
    RemoveDir(importPath);
}
#endif

/*
 * @tc.name: ImportTest 010
 * @tc.desc: Verify that it will return busy when rekey during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest010, TestSize.Level3)
{
    vector<Entry> entriesRekey2;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesRekey2, allKeys, FIFTY_RECORDS, KEY_SIX_BYTE, TWO_M_LONG_STRING);
    for (auto iter = entriesRekey2.begin(); iter != entriesRekey2.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }

    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start subthread to rekey DB with password 1.
     * @tc.expected: step1. start successfully.
     */
    bool rekeyFlag = false;
    thread subThread([&]() {
        EXPECT_EQ(g_nbBackupDelegate->Rekey(g_passwd1), OK);
        rekeyFlag = true;
        g_backupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. import the backup file during the subthread is rekeying.
     * @tc.expected: step2. import failed and returned BUSY.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LAST_SYNC));
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath, NULL_PASSWD), BUSY);

    std::mutex rekeyMtx;
    std::unique_lock<std::mutex> lck(rekeyMtx);
    g_backupVar.wait(lck, [&]{return rekeyFlag;});
    RemoveDir(importPath);
}
#endif

/*
 * @tc.name: ImportTest 011
 * @tc.desc: Verify that it will allow to import backup file again during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest011, TestSize.Level3)
{
    vector<Entry> entriesReImport;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesReImport, allKeys, TEN_RECORDS, KEY_SIX_BYTE, FOUR_M_LONG_STRING);
    for (auto iter = entriesReImport.begin(); iter != entriesReImport.end(); iter++) {
        EXPECT_TRUE(g_nbBackupDelegate->Put(iter->key, iter->value) == OK);
    }

    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    g_nbBackupDelegate->Export(filePath, NULL_PASSWD);

    /**
     * @tc.steps: step1. start subthread to import the backup file.
     * @tc.expected: step1. start successfully.
     */
    bool importFlag = false;
    thread subThread([&]() {
        EXPECT_EQ(g_nbBackupDelegate->Import(filePath, NULL_PASSWD), OK);
        importFlag = true;
        g_backupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. import the backup file during the subthread is importing with empty password.
     * @tc.expected: step2. import successfully and return OK.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath, NULL_PASSWD), OK);

    std::mutex rekeyMtx;
    std::unique_lock<std::mutex> lck(rekeyMtx);
    g_backupVar.wait(lck, [&]{return importFlag;});

    RemoveDir(importPath);
}

void NbSubImportThread(int index, std::string importPath)
{
    vector<Entry> tenEntries;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(tenEntries, allKeys, FIVE_RECORDS, KEY_SIX_BYTE, FOUR_M_LONG_STRING);
    /**
     * @tc.steps: step1. every thread create one DB put 10 entries into the DB.
     * @tc.expected: step1. put successfully.
     */
    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    DBParameters parameter[] = {g_dbParameter2, g_dbParameter3, g_dbParameter4, g_dbParameter5, g_dbParameter6};
    Option option[] = {g_createDiskUnencrypted, g_createDiskUnencrypted, g_createDiskUnencrypted,
        g_createDiskEncrypted, g_createDiskEncrypted};
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, parameter[index], option[index]);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);

    for (auto iter = tenEntries.begin(); iter != tenEntries.end(); iter++) {
        EXPECT_TRUE(delegate2->Put(iter->key, iter->value) == OK);
    }

    /**
     * @tc.steps: step2. every thread export DB to backup file.
     * @tc.expected: step2. export successfully.
     */
    DistributedDB::CipherPassword passwd[] = {NULL_PASSWD, NULL_PASSWD, NULL_PASSWD, g_filePasswd1, g_filePasswd2};

    const std::string backupFile[] = {"/bkpDB1.bin", "/bkpDB2.bin", "/bkpDB3.bin", "/bkpDB4.bin", "/bkpDB5.bin"};
    std::string nbFilePath[] = {(importPath + backupFile[INDEX_ZEROTH]), (importPath + backupFile[INDEX_FIRST]),
        (importPath + backupFile[INDEX_SECOND]), (importPath + backupFile[INDEX_THIRD]),
        (importPath + backupFile[INDEX_FORTH])};
    delegate2->Export(nbFilePath[index], passwd[index]);

    /**
     * @tc.steps: step3. every thread import backup file to DB.
     * @tc.expected: step3. import successfully.
     */
    EXPECT_EQ(delegate2->Import(nbFilePath[index], passwd[index]), OK);
    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    std::string storeId[] = {STORE_ID_2, STORE_ID_3, STORE_ID_4, STORE_ID_5, STORE_ID_6};
    EXPECT_TRUE(manager2->DeleteKvStore(storeId[index]) == OK);
    delete manager2;
    manager2 = nullptr;
}

/*
 * @tc.name: ImportTest 012
 * @tc.desc: Verify that different DB export and import in the same time won't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, ImportTest012, TestSize.Level3)
{
    std::vector<std::thread> threads;
    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    for (int index = 0; index < FIVE_TIMES; ++index) {
        threads.push_back(std::thread(NbSubImportThread, index, std::ref(importPath)));
    }

    for (auto& th : threads) {
        th.join();
    }
    RemoveDir(importPath);
}

/*
 * @tc.name: Exchange 001
 * @tc.desc: whether current db is encrypted or not, import file need to use exported password(or NULL_PASSWD).
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, Exchange001, TestSize.Level1)
{
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath1 = exportPath + "/bkpDB1.bin";
    std::string filePath2 = exportPath + "/bkpDB2.bin";
    /**
     * @tc.steps: step1. export data as "bkpDB1.bin" with passwd g_filePasswd1 and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step2. import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step2. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step3. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step3. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step4. rekey db1 with g_passwd1, and then import "bkpDB1.bin" and "bkpDB2.bin" with p1.
     * @tc.expected: step4. rekey succeeded and import failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Rekey(g_passwd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step5. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step5. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step6. import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step6. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    RemoveDir(exportPath);
}

/*
 * @tc.name: Exchange 002
 * @tc.desc: whether current db is encrypted or not, import file need to use exported password(or NULL_PASSWD).
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, Exchange002, TestSize.Level1)
{
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath1 = exportPath + "/bkpDB1.bin";
    std::string filePath2 = exportPath + "/bkpDB2.bin";
    /**
     * @tc.steps: step1. rekey db1 with g_passwd1,
     *  export data as "bkpDB1.bin" with passwd g_filePasswd1 and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Rekey(g_passwd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step2. import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step2. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step3. import "bkpDB1.bin" and "bkpDB2.bin" with g_passwd1.
     * @tc.expected: step3. rekey succeeded and import failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step4. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step4. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step5. rekey db1 with NULL_PASSWD,
     *  import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step5. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_nbBackupDelegate->Rekey(NULL_PASSWD), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step6. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step6. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    RemoveDir(exportPath);
}

/*
 * @tc.name: Exchange 003
 * @tc.desc: whether current db is encrypted or not, import file need to use exported password(or NULL_PASSWD).
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, Exchange003, TestSize.Level1)
{
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath1 = exportPath + "/bkpDB1.bin";
    std::string filePath2 = exportPath + "/bkpDB2.bin";
    /**
     * @tc.steps: step1. rekey db1 with g_passwd1,
     *  export data as "bkpDB1.bin" with passwd g_filePasswd1 and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Rekey(g_passwd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Export(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step2. rekey db1 with g_passwd2,
     *  import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step2. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_nbBackupDelegate->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step3. Put (k1,v1), close db, open db with g_passwd2 and delete (k1,v1)
     * @tc.expected: step3. operate successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ((g_manager->CloseKvStore(g_nbBackupDelegate)), OK);
    g_nbBackupDelegate = nullptr;
    delete g_manager;
    g_manager = nullptr;
    Option encrypted(true, false, true, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_2);
    g_nbBackupDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, encrypted);
    ASSERT_TRUE(g_manager != nullptr && g_nbBackupDelegate != nullptr);
    EXPECT_EQ(g_nbBackupDelegate->Delete(KEY_1), OK);
    /**
     * @tc.steps: step4. import "bkpDB1.bin" and "bkpDB2.bin" with g_passwd1.
     * @tc.expected: step4. import failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step5. import "bkpDB1.bin" and "bkpDB2.bin" with g_passwd2.
     * @tc.expected: step5. import failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_passwd2), INVALID_FILE);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_passwd2), INVALID_FILE);
    /**
     * @tc.steps: step6. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step6. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_nbBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step7. Put (k2,v2), close db, open db with g_passwd2 and delete (k2,v2)
     * @tc.expected: step7. operate successfully.
     */
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_2, VALUE_2), OK);
    EXPECT_EQ((g_manager->CloseKvStore(g_nbBackupDelegate)), OK);
    g_nbBackupDelegate = nullptr;
    delete g_manager;
    g_manager = nullptr;
    g_nbBackupDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, encrypted);
    ASSERT_TRUE(g_manager != nullptr && g_nbBackupDelegate != nullptr);
    EXPECT_EQ(g_nbBackupDelegate->Delete(KEY_2), OK);
    RemoveDir(exportPath);
}

/*
 * @tc.name: CorruptionHandler 001
 * @tc.desc: Verify that the Corruption Handler can be set only one, if set many times, the last one is valid.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, CorruptionHandler001, TestSize.Level1)
{
    EXPECT_TRUE(g_nbBackupDelegate->Put(KEY_1, VALUE_1) == OK);
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, NULL_PASSWD) == OK);
    EXPECT_TRUE(g_nbBackupDelegate->Delete(KEY_1) == OK);

    /**
     * @tc.steps: step1. device A call SetKvStoreCorruptionHandler.
     * @tc.expected: step1. call successfully.
     */
    KvStoreNbCorruptInfo corruptInfo;
    KvStoreNbCorruptInfo corruptInfoNew;
    auto notifier = bind(&KvStoreNbCorruptInfo::CorruptCallBack, &corruptInfo,
        placeholders::_1, placeholders::_2, placeholders::_3);
    g_manager->SetKvStoreCorruptionHandler(notifier);
    /**
     * @tc.steps: step2. device A call SetKvStoreCorruptionHandler the second time.
     * @tc.expected: step2. call successfully.
     */
    CallBackParam pathResult = {filePath, true};
    auto notifierNew = bind(&KvStoreNbCorruptInfo::CorruptNewCallBack, &corruptInfoNew,
        placeholders::_1, placeholders::_2, placeholders::_3, std::ref(g_manager), pathResult);
    g_manager->SetKvStoreCorruptionHandler(notifierNew);
    EXPECT_TRUE(pathResult.result);
    /**
     * @tc.steps: step3. make file corrupted and call GetKvStore to trigger callback function.
     * @tc.expected: step3. call successfully.
     */
    EXPECT_EQ((g_manager->CloseKvStore(g_nbBackupDelegate)), OK);
    g_nbBackupDelegate = nullptr;
    std::string dbFliePath = DistributedDBNbTestTools::GetKvNbStoreDirectory(g_dbParameter1);
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath));
    DBStatus status;
    KvStoreDelegateManager *manager = nullptr;
    EXPECT_EQ(DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter1, g_option), nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);
    /**
     * @tc.steps: step4. open db and Get(k1) to verify importing in callback is successful.
     * @tc.expected: step4. Get(k1)=v1.
     */
    g_nbBackupDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_manager != nullptr && g_nbBackupDelegate != nullptr);
    Value valueResult;
    EXPECT_TRUE(g_nbBackupDelegate->Get(KEY_1, valueResult) == OK);
    EXPECT_EQ(valueResult, VALUE_1);
    g_manager->SetKvStoreCorruptionHandler(nullptr);
    RemoveDir(exportPath);
}

/*
 * @tc.name: CorruptionHandler 002
 * @tc.desc: Verify that can import db files and can't export in the Corruption Handler callback when db flies
 * are corrupted in using.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, CorruptionHandler002, TestSize.Level2)
{
    EXPECT_TRUE(g_nbBackupDelegate->Put(KEY_1, VALUE_1) == OK);
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, g_filePasswd1) == OK);

    /**
     * @tc.steps: step1. device A call SetKvStoreCorruptionHandler to register CorruptCallBackOfImport.
     * @tc.expected: step1. call successfully.
     */
    KvStoreNbCorruptInfo corruptInfo;
    CallBackParam pathResult = {filePath, true};
    auto notifier = bind(&KvStoreNbCorruptInfo::CorruptCallBackOfImport, &corruptInfo,
        placeholders::_1, placeholders::_2, placeholders::_3, g_nbBackupDelegate, pathResult);
    g_manager->SetKvStoreCorruptionHandler(notifier);
    EXPECT_TRUE(pathResult.result);
    /**
     * @tc.steps: step2. device A modify the db files to trigger the CorruptCallBackOfImport and check.
     * @tc.expected: step2. operate successfully and import the recover db files in callback successfully.
     */
    EXPECT_TRUE(g_nbBackupDelegate->Delete(KEY_1) == OK);
    std::string dbFliePath = DistributedDBNbTestTools::GetKvNbStoreDirectory(g_dbParameter1);
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath));
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath + "-wal"));
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_1), INVALID_PASSWD_OR_CORRUPTED_DB);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND)); // wait for callback complete for 1 second.
    Value valueResult;
    EXPECT_TRUE(g_nbBackupDelegate->Get(KEY_1, valueResult) == OK);
    EXPECT_EQ(valueResult, VALUE_1);

    /**
     * @tc.steps: step3. device A call SetKvStoreCorruptionHandler again to register CorruptCallBackOfExport.
     * @tc.expected: step3. call successfully.
     */
    filePath = exportPath + "/bkpDB1.bin";
    auto notifierNew = bind(&KvStoreNbCorruptInfo::CorruptCallBackOfExport, &corruptInfo,
        placeholders::_1, placeholders::_2, placeholders::_3, g_nbBackupDelegate, pathResult);
    g_manager->SetKvStoreCorruptionHandler(notifierNew);
    EXPECT_TRUE(pathResult.result);

    /**
     * @tc.steps: step4. device A modify the db files to trigger the CorruptCallBackOfExport and check.
     * @tc.expected: step4. operate successfully and export the recover db data in callback failed.
     */
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath));
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_1), INVALID_PASSWD_OR_CORRUPTED_DB);
    g_manager->SetKvStoreCorruptionHandler(nullptr);
    RemoveDir(exportPath);
}

/*
 * @tc.name: CorruptionHandler 003
 * @tc.desc: Verify that if call SetKvStoreCorruptionHandler(nullptr) will unregister the Corruption Handler.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBackupTest, CorruptionHandler003, TestSize.Level2)
{
    EXPECT_TRUE(g_nbBackupDelegate->Put(KEY_1, VALUE_1) == OK);
    const std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_TRUE(g_nbBackupDelegate->Export(filePath, g_filePasswd1) == OK);

    /**
     * @tc.steps: step1. device A's db1 call SetKvStoreCorruptionHandler to register CorruptCallBack.
     * @tc.expected: step1. call successfully.
     */
    KvStoreNbCorruptInfo corruptInfo;
    auto notifier = bind(&KvStoreNbCorruptInfo::CorruptCallBack, &corruptInfo,
        placeholders::_1, placeholders::_2, placeholders::_3);
    g_manager->SetKvStoreCorruptionHandler(notifier);

    /**
     * @tc.steps: step2. device A's db2 call SetKvStoreCorruptionHandler to register CorruptCallBackOfImport.
     * @tc.expected: step2. call successfully.
     */
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *delegate = nullptr;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    KvStoreNbCorruptInfo corruptInfoNew;
    CallBackParam pathResult = {filePath, true};
    auto notifierNew = bind(&KvStoreNbCorruptInfo::CorruptCallBackOfImport, &corruptInfoNew,
        placeholders::_1, placeholders::_2, placeholders::_3, g_nbBackupDelegate, pathResult);
    manager->SetKvStoreCorruptionHandler(notifierNew);
    EXPECT_TRUE(pathResult.result);

    /**
     * @tc.steps: step3. device A modify the db files to trigger the CorruptCallBackOfImport and check.
     * @tc.expected: step3. operate successfully and import the recover db files in callback successfully.
     */
    std::string dbFliePath = DistributedDBNbTestTools::GetKvNbStoreDirectory(g_dbParameter1);
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath));
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath + "-wal"));
    EXPECT_EQ(g_nbBackupDelegate->Put(KEY_1, VALUE_1), INVALID_PASSWD_OR_CORRUPTED_DB);
    std::this_thread::sleep_for(std::chrono::seconds(TWO_SECONDS)); // wait for callback complete for 1 second.
    Value valueResult;
    EXPECT_EQ(g_nbBackupDelegate->Get(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);

    /**
     * @tc.steps: step4. device A call SetKvStoreCorruptionHandler again to register nullptr.
     * @tc.expected: step4. call successfully.
     */
    manager->SetKvStoreCorruptionHandler(nullptr);

    /**
     * @tc.steps: step5. device A modify the db files to trigger the CorruptCallBack and check.
     * @tc.expected: step5. put return error but trigger the CorruptCallBack failed.
     */
    EXPECT_TRUE(delegate->Put(KEY_1, VALUE_1) == OK);
    dbFliePath = DistributedDBNbTestTools::GetKvNbStoreDirectory(g_dbParameter2);
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath));
    EXPECT_TRUE(DistributedDBNbTestTools::ModifyDatabaseFile(dbFliePath + "-wal"));
    EXPECT_EQ(delegate->Put(KEY_1, VALUE_1), INVALID_PASSWD_OR_CORRUPTED_DB);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND)); // wait for callback complete for 1 second.
    EXPECT_EQ(delegate->Get(KEY_1, valueResult), INVALID_PASSWD_OR_CORRUPTED_DB);

    EXPECT_EQ((manager->CloseKvStore(delegate)), OK);
    delegate = nullptr;
    manager->DeleteKvStore(STORE_ID_2);
    delete manager;
    manager = nullptr;
    RemoveDir(exportPath);
}
}