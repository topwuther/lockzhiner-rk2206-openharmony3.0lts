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
#include <fstream>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"
#include "process_communicator_test_stub.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbKvBackup {
static std::condition_variable g_kvBackupVar;
class DistributeddbKvBackupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

KvStoreDelegate *g_kvBackupDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_manager = nullptr;
DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;
DistributedDB::CipherPassword g_filePasswd1;
DistributedDB::CipherPassword g_filePasswd2;
void DistributeddbKvBackupTest::SetUpTestCase(void)
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

void DistributeddbKvBackupTest::TearDownTestCase(void)
{
}

void DistributeddbKvBackupTest::SetUp(void)
{
    const std::string exportPath = DIRECTOR + "export";
    RemoveDatabaseDirectory(exportPath);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    KvOption option = g_kvOption;
    g_kvBackupDelegate = DistributedTestTools::GetDelegateSuccess(g_manager, g_kvdbParameter1, option);
    ASSERT_TRUE(g_manager != nullptr && g_kvBackupDelegate != nullptr);
}

void DistributeddbKvBackupTest::TearDown(void)
{
    MST_LOG("TearDown after case.");
    EXPECT_EQ(g_manager->CloseKvStore(g_kvBackupDelegate), OK);
    g_kvBackupDelegate = nullptr;
    DBStatus status = g_manager->DeleteKvStore(STORE_ID_1);
    EXPECT_EQ(status, OK) << "fail to delete exist kvdb";
    delete g_manager;
    g_manager = nullptr;
}

/*
 * @tc.name: ExportTest 001
 * @tc.desc: test checking in parameter of filepath of export interface .
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call export interface and the dir of filepath is nonexistent.
     * @tc.expected: step1. call failed and return INVALID_ARGS.
     */
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    const std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), INVALID_ARGS);

    /**
     * @tc.steps: step2. call export interface, the filepath is absolute path and legal then check file number.
     * @tc.expected: step2. call successfully and the file number is 1.
     */
    SetDir(exportPath);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);
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
    const std::string filePath2 = noRightPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath2, NULL_PASSWD), NO_PERMISSION);
    RemoveDatabaseDirectory(noRightPath);
#endif
    /**
     * @tc.steps: step4. call export interface, the filepath is relative path and legal then check file number.
     * @tc.expected: step4. call successfully and the file number is 1.
     */
    const std::string exportPath2 = "../export";
    SetDir(exportPath2);
    const std::string filePath3 = exportPath2 + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath3, NULL_PASSWD), OK);
    CheckFileNumber(exportPath2, fileCount);
    EXPECT_EQ(fileCount, 1);
    RemoveDir(exportPath2);
}

/*
 * @tc.name: ExportTest 002
 * @tc.desc: Verify that the export interface will return FILE_ALREADY_EXISTED if the export file has been exist.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. call export interface and the dir of filepath is legal then check file number.
     * @tc.expected: step1. call successfully and the file number of filepath is 1.
     */
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    const std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1); // the number of files is 1.

    /**
     * @tc.steps: step2. call export interface and the directory of file is existent.
     * @tc.expected: step2. call failed and return FILE_ALREADY_EXISTED.
     */
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), FILE_ALREADY_EXISTED);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1); // the number of files is 1.
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest003
 * @tc.desc: Verify that call export will be block when there is uncommit or rollback transaction.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. start transaction and call export interface to export data.
     * @tc.expected: step1. start transaction successfully call export is blocked.
     */
    EXPECT_EQ(g_kvBackupDelegate->StartTransaction(), OK);
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB1.bin";
    DBStatus status = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);

    /**
     * @tc.steps: step2. start thread to commit transaction and check the number od files in the exportion directory.
     * @tc.expected: step2. commit successfully and the number of files is 1.
     */
    bool exportFlag = false;
    thread subThread1([&]() {
        DBStatus status = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);
        EXPECT_EQ(status, OK);
        exportFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread1.detach();
    EXPECT_EQ(g_kvBackupDelegate->Commit(), OK);
    std::mutex count;
    {
        std::unique_lock<std::mutex> lck(count);
        g_kvBackupVar.wait(lck, [&]{return exportFlag;});
    }
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1); // the number of files is 1.

    /**
     * @tc.steps: step3. start transaction and call export interface to export data.
     * @tc.expected: step3. start transaction successfully call export is blocked.
     */
    exportFlag = false;
    EXPECT_EQ(g_kvBackupDelegate->StartTransaction(), OK);
    filePath = exportPath + "/bkpDB2.bin";

    /**
     * @tc.steps: step3. start thread to rollback transaction and check the number od files in the exportion directory.
     * @tc.expected: step3. rollback successfully and the number of files is 2.
     */
    thread subThread2([&]() {
        status = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);
        EXPECT_EQ(status, OK);
        exportFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread2.detach();
    EXPECT_EQ(g_kvBackupDelegate->Rollback(), OK);
    {
        std::unique_lock<std::mutex> lck(count);
        g_kvBackupVar.wait(lck, [&]{return exportFlag;});
    }
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 2); // the number of files is 2.
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 004
 * @tc.desc: Verify that the export interface will return busy when the import hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest004, TestSize.Level2)
{
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    DBStatus status = DistributedTestTools::PutBatch(*g_kvBackupDelegate, entriesBatch);
    ASSERT_TRUE(status == DBStatus::OK);
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);

    /**
     * @tc.steps: step1. call import interface to import the file that prepared in advance.
     * @tc.expected: step1. call successfully.
     */
    bool importFlag = false;
    thread subThread([&]() {
        EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), OK);
        importFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call export interface when step1 is running.
     * @tc.expected: step2. return busy if step1 hasn't completed, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_TWO_HUNDREDS_MS));
    filePath = exportPath + "/bkpDB1.bin";
    status = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);
    EXPECT_TRUE(status == BUSY || status == OK);
    CheckFileNumber(exportPath, fileCount);
    if (status == BUSY) {
        EXPECT_EQ(fileCount, 1); // the number of file is 1(Export is busy).
    } else {
        EXPECT_EQ(fileCount, 2); // the number of file is 2(Export is OK).
    }

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_kvBackupVar.wait(lck, [&]{return importFlag;});
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 005
 * @tc.desc: Verify that the export interface will return busy when the rekey hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest005, TestSize.Level2)
{
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    DBStatus status = DistributedTestTools::PutBatch(*g_kvBackupDelegate, entriesBatch);
    ASSERT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step1. call import interface to import the file that prepared in advance.
     * @tc.expected: step1. call successfully.
     */
    bool rekeyFlag = false;
    thread subThread([&]() {
        DBStatus rekeyStatus = g_kvBackupDelegate->Rekey(g_passwd1);
        EXPECT_EQ(rekeyStatus, OK);
        rekeyFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call export interface when step1 is running.
     * @tc.expected: step2. return busy if step1 hasn't completed, else return OK.
     */
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    const std::string filePath = exportPath + "/bkpDB.bin";
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_TWO_HUNDREDS_MS));
    status = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);
    EXPECT_TRUE(status == BUSY || status == OK);
    CheckFileNumber(exportPath, fileCount);
    if (status == BUSY) {
        EXPECT_EQ(fileCount, 0); // 0 file if export failed.
    } else {
        EXPECT_EQ(fileCount, 1); // 1 file if export success.
    }

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_kvBackupVar.wait(lck, [&]{return rekeyFlag;});
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 006
 * @tc.desc: Verify that the export interface will execute blockly when last export hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest006, TestSize.Level2)
{
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    DBStatus status = DistributedTestTools::PutBatch(*g_kvBackupDelegate, entriesBatch);
    ASSERT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step1. call import interface to import the file that prepared in advance.
     * @tc.expected: step1. call successfully.
     */
    bool exportFlag = false;
    thread subThread([&]() {
        DBStatus exportStatus = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);
        EXPECT_EQ(exportStatus, OK);
        exportFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call export interface when step1 is running.
     * @tc.expected: step2. return OK.
     */
    std::string filePath2 = exportPath + "/bkpDB1.bin";
    std::this_thread::sleep_for(std::chrono::microseconds(MILLSECONDES_PER_SECOND));
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath2, NULL_PASSWD), OK);

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_kvBackupVar.wait(lck, [&]{return exportFlag;});
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 2); // the file number is 2.
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 007
 * @tc.desc: Verify that the put/delete operation will execute blockly when export hasn't completed.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest007, TestSize.Level2)
{
    vector<Entry> entriesPut;
    entriesPut.push_back(ENTRY_1);
    entriesPut.push_back(ENTRY_2);
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    ASSERT_TRUE(DistributedTestTools::PutBatch(*g_kvBackupDelegate, entriesBatch) == OK);

    /**
     * @tc.steps: step1. call export interface in subthread.
     * @tc.expected: step1. call successfully.
     */
    bool exportFlag = false;
    thread subThread([&]() {
        DBStatus exportStatus = g_kvBackupDelegate->Export(filePath, NULL_PASSWD);
        EXPECT_EQ(exportStatus, OK);
        exportFlag = true;
        g_kvBackupVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call Get,GetLocal,GetEntries to query data from db.
     * @tc.expected: step2. call successfully.
     */
    Value valueResult = DistributedTestTools::Get(*g_kvBackupDelegate, allKeys[0]);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, entriesBatch[0].value));
    vector<Entry> entries = DistributedTestTools::GetEntries(*g_kvBackupDelegate, KEY_EMPTY);
    ASSERT_EQ(entries.size(), entriesBatch.size());

    /**
     * @tc.steps: step3. call put/delete/putlocal/deletelocal interface when step1 is running.
     * @tc.expected: step3. return ok.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvBackupDelegate, entriesBatch[0].key, entriesBatch[0].value);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(DistributedTestTools::PutBatch(*g_kvBackupDelegate, entriesPut), OK);
    EXPECT_EQ(DistributedTestTools::Delete(*g_kvBackupDelegate, KEY_1), OK);
    EXPECT_EQ(DistributedTestTools::DeleteBatch(*g_kvBackupDelegate, allKeys), OK);

    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_kvBackupVar.wait(lck, [&]{return exportFlag;});
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 1);
    RemoveDir(exportPath);
}

/*
 * @tc.name: ExportTest 008
 * @tc.desc: Verify that passwd parameter of export interface decide the file exported is encrypted or not.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ExportTest008, TestSize.Level1)
{
    int fileCount = 0;
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB1.bin";

    /**
     * @tc.steps: step1. call export interface in subthread.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);
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
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, password), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 2); // the number of file is 2.

    /**
     * @tc.steps: step3. call export interface with the passwd = password(128B).
     * @tc.expected: step3. return OK.
     */
    filePath = exportPath + "/bkpDB4.bin";
    passwordVector.assign(BATCH_RECORDS, 'b'); // 1 Byte of passwd.
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, password), OK);
    CheckFileNumber(exportPath, fileCount);
    EXPECT_EQ(fileCount, 3); // the number of file is 3.
    RemoveDir(exportPath);
}

/*
 * @tc.name: ImportTest 001
 * @tc.desc: Verify that call import bkpfile with right password: none <-> none, password1 <-> password1.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. put (k1, v1) to device and update it to (k1, v2) after export the backup file.
     * @tc.expected: step1. put and update successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_1), OK);
    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB1.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_2), OK);

    /**
     * @tc.steps: step2. call import interface with password f1.
     * @tc.expected: step2. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, g_filePasswd1), INVALID_FILE);

    /**
     * @tc.steps: step3. call import interface with empty password and get the value of k1.
     * @tc.expected: step3. call import interface successfully and the value of k1 is v1
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), OK);
    Value valueGot = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueGot, VALUE_1));

    /**
     * @tc.steps: step4. put (k1, v2) to device for there is already encrypted backup file to update it to (k1, v1).
     * @tc.expected: step4. put and update successfully.
     */
    filePath = importPath + "/bkpDB2.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step5. call import interface with empty password.
     * @tc.expected: step5. call failed and return INVALID_FILE.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), INVALID_FILE);

    /**
     * @tc.steps: step6. call import interface with db password p1.
     * @tc.expected: step6. call failed and return INVALID_FILE
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, g_passwd1), INVALID_FILE);

    /**
     * @tc.steps: step7. call import interface with passwd = password(129B).
     * @tc.expected: step7. call failed and return INVALID_ARGS
     */
    vector<uint8_t> passwordVector(PASSWD_BYTE, 'a');
    CipherPassword invalidPassword;
    EXPECT_EQ(invalidPassword.SetValue(passwordVector.data(), passwordVector.size()),
        CipherPassword::ErrorCode::OVERSIZE);

    /**
     * @tc.steps: step8. call import interface with password f1 and get the value of k1.
     * @tc.expected: step8. call import interface successfully and the value of k1 is v1
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, g_filePasswd1), OK);
    valueGot.clear();
    valueGot = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueGot, VALUE_1));
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 002
 * @tc.desc: Verify that can't import when the file is not exist or the file path is wrong.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest002, TestSize.Level1)
{
    std::string importPath1 = DIRECTOR + "export";
    SetDir(importPath1);
    std::string filePath = importPath1 + "/bkpDB1.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. import the backup file 1 in the noexsit path.
     * @tc.expected: step1. call failed and return INVALID_FILE.
     */
    std::string importPath2 = DIRECTOR + "noexsit";
    filePath = importPath2 + "/bkpDB1.bin";
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), INVALID_ARGS);

    /**
     * @tc.steps: step2. import the no exist backup file 2 in the noexsit path.
     * @tc.expected: step2. call failed and return INVALID_FILE
     */
    filePath = importPath1 + "/bkpDB2.bin";
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), INVALID_FILE);
    RemoveDir(importPath1);
}

/*
 * @tc.name: ImportTest 003
 * @tc.desc: Verify that can't import when the file is not a right DB file or the DB file is damaged.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest003, TestSize.Level1)
{
    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. import a.txt in the right path.
     * @tc.expected: step1. call failed and return INVALID_FILE.
     */
    filePath = importPath + "/a.txt";
    ofstream createFile(filePath);
    if (createFile) {
        createFile << '1' << endl;
        createFile.close();
    }
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), INVALID_FILE);

    /**
     * @tc.steps: step2. write some data to DB file1.
     * @tc.expected: step2. write successfully
     */
    filePath = importPath + "/bkpDB.bin";
    ofstream damageFile(filePath, ios::out | ios::binary);
    ASSERT_TRUE(damageFile.is_open());
    damageFile.write(reinterpret_cast<char *>(&filePath), filePath.size());
    damageFile.close();

    /**
     * @tc.steps: step3. import DB file with empty password.
     * @tc.expected: step3. import failed and returned INVALID_FILE
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), INVALID_FILE);
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 005
 * @tc.desc: Verify that import will return busy when there are many delegate of the same db.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest005, TestSize.Level1)
{
    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB1.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, g_filePasswd1), OK);

    /**
     * @tc.steps: step1. open the other delegate of the same kvstore.
     * @tc.expected: step1. open successfully.
     */
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvOption option = g_kvOption;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter1, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    /**
     * @tc.steps: step2. import the backup file of the same kvstore of g_kvdbParameter1.
     * @tc.expected: step2. import failed and returned BUSY.
     */
    EXPECT_EQ(delegate2->Import(filePath, g_filePasswd1), BUSY);

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    delete manager2;
    manager2 = nullptr;
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 006
 * @tc.desc: Verify that if the DB register observer or snapshot, it can't import DB backup file.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest006, TestSize.Level1)
{
    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB1.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, g_filePasswd1), OK);

    /**
     * @tc.steps: step1. register the observer of DB of the same kvstore.
     * @tc.expected: step1. create successfully.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_kvBackupDelegate, &observer);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. import the backup file of the DB.
     * @tc.expected: step2. import failed and returned BUSY.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, g_filePasswd1), BUSY);
    /**
     * @tc.steps: step3. unregister the observer and register the snap shot.
     * @tc.expected: step3. unregister observer and register the snap shot successfully.
     */
    status = DistributedTestTools::UnRegisterObserver(g_kvBackupDelegate, &observer);
    EXPECT_EQ(status, OK);
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_kvBackupDelegate, &observer);
    EXPECT_NE(snapShot, nullptr);
    /**
     * @tc.steps: step4. import the backup file of the DB second time.
     * @tc.expected: step4. import failed and returned BUSY.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, g_filePasswd1), BUSY);
    EXPECT_EQ(g_kvBackupDelegate->ReleaseKvStoreSnapshot(snapShot), OK);

    snapShot = nullptr;
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 007
 * @tc.desc: Verify that it will return busy if execute CRUD during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest007, TestSize.Level2)
{
    vector<Entry> entries;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entries, allKeys, TEN_RECORDS, KEY_SIX_BYTE, ONE_M_LONG_STRING);
    for (auto iter = entries.begin(); iter != entries.end(); iter++) {
        EXPECT_EQ(g_kvBackupDelegate->Put(iter->key, iter->value), OK);
    }

    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB1.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start the sub thread to import a very big backup DB file.
     * @tc.expected: step1. import successfully if step1 running before step2.
     */
    bool importFlag = false;
    thread subThread([&]() {
        DBStatus importStatus = g_kvBackupDelegate->Import(filePath, NULL_PASSWD);
        EXPECT_TRUE(importStatus == OK || importStatus == BUSY);
        importFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. get kvstore again during the sub thread is importing backup file.
     * @tc.expected: step2. get failed and return BUSY if step1 is running, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LONG_TIME));
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig(KV_CONFIG), OK);
    DelegateKvMgrCallback getDelegateCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &getDelegateCallback, std::placeholders::_1, std::placeholders::_2);
    KvStoreDelegate::Option option = DistributedTestTools::TransferKvOptionType(g_kvOption);
    manager->GetKvStore(g_kvdbParameter1.storeId, option, function);
    DBStatus status = getDelegateCallback.GetStatus();
    EXPECT_TRUE(status == BUSY || status == OK);
    /**
     * @tc.steps: step3. CRUD on DB during the sub thread is importing backup file.
     * @tc.expected: step3. put failed and return BUSY.
     */
    status = g_kvBackupDelegate->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(status == BUSY || status == OK);
    status = g_kvBackupDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == BUSY || status == OK);
    status = g_kvBackupDelegate->Delete(KEY_1);
    EXPECT_TRUE(status == BUSY || status == OK);

    std::mutex reGetKvStoreMtx;
    std::unique_lock<std::mutex> lck(reGetKvStoreMtx);
    g_kvBackupVar.wait(lck, [&]{return importFlag;});
    delete manager;
    manager = nullptr;
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 008
 * @tc.desc: Verify that it will return busy if it rekey during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest008, TestSize.Level2)
{
    vector<Entry> entriesRekey;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesRekey, allKeys, TEN_RECORDS, KEY_SIX_BYTE, ONE_M_LONG_STRING);
    for (auto iter = entriesRekey.begin(); iter != entriesRekey.end(); iter++) {
        EXPECT_EQ(g_kvBackupDelegate->Put(iter->key, iter->value), OK);
    }

    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start the sub thread to import a very big backup DB file.
     * @tc.expected: step1. import successfully.
     */
    bool importFlag = false;
    thread subThread([&]() {
        EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), OK);
        importFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. rekey kvstore use p1 during the sub thread is importing backup file.
     * @tc.expected: step2. rekey failed and return BUSY if step1 hasn't completed, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_TWO_HUNDREDS_MS));
    DBStatus status = g_kvBackupDelegate->Rekey(g_passwd1);
    EXPECT_TRUE(status == BUSY || status == OK);

    std::mutex reGetKvStoreMtx;
    std::unique_lock<std::mutex> lck(reGetKvStoreMtx);
    g_kvBackupVar.wait(lck, [&]{return importFlag;});
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 009
 * @tc.desc: Verify that it will return busy if it import during it is rekeying.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest009, TestSize.Level2)
{
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvOption option = g_createKvDiskUnencrypted;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);

    vector<Entry> entriesRekey;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesRekey, allKeys, TEN_RECORDS, KEY_SIX_BYTE, ONE_M_LONG_STRING);
    for (auto iter = entriesRekey.begin(); iter != entriesRekey.end(); iter++) {
        EXPECT_EQ(delegate2->Put(iter->key, iter->value), OK);
    }

    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    delegate2->Export(filePath, NULL_PASSWD);

    /**
     * @tc.steps: step1. start the sub thread to import a very big backup DB file.
     * @tc.expected: step1. import successfully.
     */
    bool rekeyFlag = false;
    thread subThread([&]() {
        EXPECT_EQ(delegate2->Rekey(g_passwd1), OK);
        rekeyFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. rekey kvstore use p1 during the sub thread is importing backup file.
     * @tc.expected: step2. rekey failed and return BUSY if step1 hasn't completed, else return OK.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_TWO_HUNDREDS_MS));
    DBStatus status = delegate2->Import(filePath, NULL_PASSWD);
    EXPECT_TRUE(status == BUSY || status == OK);

    std::mutex rekeyMtx;
    std::unique_lock<std::mutex> lck(rekeyMtx);
    g_kvBackupVar.wait(lck, [&]{return rekeyFlag;});

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
    RemoveDir(importPath);
}

/*
 * @tc.name: ImportTest 010
 * @tc.desc: Verify that it will return busy if it import again during it is importing.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest010, TestSize.Level2)
{
    vector<Entry> entriesRekey2;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesRekey2, allKeys, TEN_RECORDS, KEY_SIX_BYTE, ONE_M_LONG_STRING);
    for (auto iter = entriesRekey2.begin(); iter != entriesRekey2.end(); iter++) {
        EXPECT_EQ(g_kvBackupDelegate->Put(iter->key, iter->value), OK);
    }

    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start the sub thread to import a very big backup DB file.
     * @tc.expected: step1. import successfully.
     */
    bool importFlag = false;
    thread subThread([&]() {
        EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), OK);
        importFlag = true;
        g_kvBackupVar.notify_one();
    });
    subThread.detach();
    /**
     * @tc.steps: step2. import the backup file again during the sub thread is importing.
     * @tc.expected: step2. call import interface failed and return BUSY.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(MILLSECONDES_PER_SECOND));
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath, NULL_PASSWD), OK);

    std::mutex reImportMtx;
    std::unique_lock<std::mutex> lck(reImportMtx);
    g_kvBackupVar.wait(lck, [&]{return importFlag;});
    RemoveDir(importPath);
}

void KvSubImportThread(int index, std::string importPath)
{
    vector<Entry> tenEntries;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(tenEntries, allKeys, TEN_RECORDS, KEY_SIX_BYTE, FOUR_M_LONG_STRING);
    /**
     * @tc.steps: step1. every thread create one DB put 10 entries into the DB.
     * @tc.expected: step1. put successfully.
     */
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvOption option[] = {g_createKvDiskUnencrypted, g_createKvDiskUnencrypted, g_createKvDiskUnencrypted,
        g_createKvDiskEncrypted, g_createKvDiskEncrypted};
    KvDBParameters parameter[] = {g_kvdbParameter2, g_kvdbParameter3, g_kvdbParameter4,
        g_kvdbParameter5, g_kvdbParameter6};
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, parameter[index], option[index]);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    for (auto iter = tenEntries.begin(); iter != tenEntries.end(); iter++) {
        EXPECT_EQ(delegate2->Put(iter->key, iter->value), OK);
    }

    /**
     * @tc.steps: step2. every thread export DB to backup file.
     * @tc.expected: step2. export successfully.
     */
    DistributedDB::CipherPassword passwd[] = {NULL_PASSWD, NULL_PASSWD, NULL_PASSWD, g_filePasswd1, g_filePasswd2};
    const std::string backupFile[] = {"/bkpDB1.bin", "/bkpDB2.bin", "/bkpDB3.bin", "/bkpDB4.bin", "/bkpDB5.bin"};
    std::string filePath[] = {(importPath + backupFile[INDEX_ZEROTH]), (importPath + backupFile[INDEX_FIRST]),
        (importPath + backupFile[INDEX_SECOND]), (importPath + backupFile[INDEX_THIRD]),
        (importPath + backupFile[INDEX_FORTH])};
    delegate2->Export(filePath[index], passwd[index]);

    /**
     * @tc.steps: step3. every thread import backup file to DB.
     * @tc.expected: step3. import successfully.
     */
    EXPECT_EQ(delegate2->Import(filePath[index], passwd[index]), OK);
    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    std::string storeId[] = {STORE_ID_2, STORE_ID_3, STORE_ID_4, STORE_ID_5, STORE_ID_6};
    EXPECT_EQ(manager2->DeleteKvStore(storeId[index]), OK);
    delete manager2;
    manager2 = nullptr;
}

/*
 * @tc.name: ImportTest 011
 * @tc.desc: Verify that different DB export and import in the same time don't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, ImportTest011, TestSize.Level3)
{
    std::vector<std::thread> threads;
    const std::string importPath = DIRECTOR + "export";
    SetDir(importPath);
    for (int index = 0; index < FIVE_TIMES; ++index) {
        threads.push_back(std::thread(KvSubImportThread, index, std::ref(importPath)));
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
HWTEST_F(DistributeddbKvBackupTest, Exchange001, TestSize.Level2)
{
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath1 = exportPath + "/bkpDB1.bin";
    std::string filePath2 = exportPath + "/bkpDB2.bin";
    /**
     * @tc.steps: step1. export data as "bkpDB1.bin" with passwd g_filePasswd1 and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step2. import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step2. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step3. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step3. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step4. rekey db1 with g_passwd1, and then import "bkpDB1.bin" and "bkpDB2.bin" with p1.
     * @tc.expected: step4. rekey succeeded and import failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Rekey(g_passwd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step5. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step5. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step6. import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step6. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    RemoveDir(exportPath);
}

/*
 * @tc.name: Exchange 002
 * @tc.desc: whether current db is encrypted or not, import file need to use exported password(or NULL_PASSWD).
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, Exchange002, TestSize.Level2)
{
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath1 = exportPath + "/bkpDB1.bin";
    std::string filePath2 = exportPath + "/bkpDB2.bin";
    /**
     * @tc.steps: step1. rekey db1 with g_passwd1,
     *  export data as "bkpDB1.bin" with passwd g_filePasswd1 and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Rekey(g_passwd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step2. import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step2. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step3. import "bkpDB1.bin" and "bkpDB2.bin" with g_passwd1.
     * @tc.expected: step3. rekey succeeded and import failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step4. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step4. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step5. rekey db1 with NULL_PASSWD,
     *  import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step5. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_kvBackupDelegate->Rekey(NULL_PASSWD), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step6. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step6. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    RemoveDir(exportPath);
}

/*
 * @tc.name: Exchange 003
 * @tc.desc: whether current db is encrypted or not, import file need to use exported password(or NULL_PASSWD).
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvBackupTest, Exchange003, TestSize.Level2)
{
    const std::string exportPath = DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath1 = exportPath + "/bkpDB1.bin";
    std::string filePath2 = exportPath + "/bkpDB2.bin";
    /**
     * @tc.steps: step1. rekey db1 with g_passwd1,
     *  export data as "bkpDB1.bin" with passwd g_filePasswd1 and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step1. call successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Rekey(g_passwd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Export(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step2. rekey db1 with g_passwd2,
     *  import "bkpDB1.bin" and "bkpDB2.bin" with NULL_PASSWD.
     * @tc.expected: step2. import "bkpDB1.bin" failed and import "bkpDB2.bin" succeeded.
     */
    EXPECT_EQ(g_kvBackupDelegate->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, NULL_PASSWD), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, NULL_PASSWD), OK);
    /**
     * @tc.steps: step3. Put (k1,v1), close db, open db with g_passwd2 and delete (k1,v1)
     * @tc.expected: step3. operate successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ((g_manager->CloseKvStore(g_kvBackupDelegate)), OK);
    g_kvBackupDelegate = nullptr;
    delete g_manager;
    g_manager = nullptr;
    KvOption kvEncrypted(true, false, true, DistributedDB::CipherType::DEFAULT,
        DistributedDBDataGenerator::PASSWD_VECTOR_2);
    g_kvBackupDelegate = DistributedTestTools::GetDelegateSuccess(g_manager, g_kvdbParameter1, kvEncrypted);
    ASSERT_TRUE(g_manager != nullptr && g_kvBackupDelegate != nullptr);
    EXPECT_EQ(g_kvBackupDelegate->Delete(KEY_1), OK);
    /**
     * @tc.steps: step4. import "bkpDB1.bin" and "bkpDB2.bin" with g_passwd1.
     * @tc.expected: step4. import failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_passwd1), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_passwd1), INVALID_FILE);
    /**
     * @tc.steps: step5. import "bkpDB1.bin" and "bkpDB2.bin" with g_passwd2.
     * @tc.expected: step5. import failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_passwd2), INVALID_FILE);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_passwd2), INVALID_FILE);
    /**
     * @tc.steps: step6. import "bkpDB1.bin" and "bkpDB2.bin" with g_filePasswd1.
     * @tc.expected: step6. import "bkpDB1.bin" succeeded and import "bkpDB2.bin" failed.
     */
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath1, g_filePasswd1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Import(filePath2, g_filePasswd1), INVALID_FILE);
    /**
     * @tc.steps: step7. Put (k2,v2), close db, open db with g_passwd2 and delete (k2,v2)
     * @tc.expected: step7. operate successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_2, VALUE_2), OK);
    EXPECT_EQ((g_manager->CloseKvStore(g_kvBackupDelegate)), OK);
    g_kvBackupDelegate = nullptr;
    delete g_manager;
    g_manager = nullptr;
    g_kvBackupDelegate = DistributedTestTools::GetDelegateSuccess(g_manager, g_kvdbParameter1, kvEncrypted);
    ASSERT_TRUE(g_manager != nullptr && g_kvBackupDelegate != nullptr);
    EXPECT_EQ(g_kvBackupDelegate->Delete(KEY_2), OK);
    RemoveDir(exportPath);
}
}