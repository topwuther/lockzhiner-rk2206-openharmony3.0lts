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

#include "distributeddb_tools_unit_test.h"
#include "securec.h"
#include "ikvdb_factory.h"
#include "default_factory.h"
#include "db_errno.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    IKvDBCommitStorage::Property g_prop;
    IKvDBCommitStorage *g_commitStorage = nullptr;
    bool g_createFactory = false;
    Version g_defaultCommitVer1 = 1;
    Version g_defaultCommitVer2 = 2;
    Version g_defaultCommitVer3 = 3;
    Version g_defaultCommitVer4 = 4;
    Version g_defaultCommitVer5 = 5;
    Version g_defaultCommitVer6 = 6;
    Version g_defaultCommitVer7 = 7;
    Version g_defaultCommitVer8 = 8;
    Version g_defaultCommitVer9 = 9;
    Version g_defaultCommitVer10 = 10;
    Version g_defaultCommitVer11 = 11;
    Version g_defaultCommitVer12 = 12;
    string g_defaultCommitID0 = "";
    string g_defaultCommitID1 = "commit_ID_1";
    string g_defaultCommitID2 = "commit_ID_2";
    string g_defaultCommitID3 = "commit_ID_3";
    string g_defaultCommitID4 = "commit_ID_4";
    string g_defaultCommitID5 = "commit_ID_5";
    string g_defaultCommitID6 = "commit_ID_6";
    string g_defaultCommitID7 = "commit_ID_7";
    string g_defaultCommitID8 = "commit_ID_8";
    string g_defaultCommitID9 = "commit_ID_9";
    string g_defaultCommitID10 = "commit_ID_10";
    string g_defaultCommitID11 = "commit_ID_11";
    string g_defaultCommitID12 = "commit_ID_12";
    string g_defaultCommitID13 = "commit_ID_13";
    string g_defaultCommitID14 = "commit_ID_14";
    DeviceID g_localDevice = "local";
    DeviceID g_remoteDeviceA = "remote_device_A";
    DeviceID g_remoteDeviceB = "remote_device_B";
    DeviceID g_remoteDeviceC = "remote_device_C";
    DeviceID g_remoteDeviceD = "remote_device_D";
    const TimeStamp TIME_STAMP1 = 100;
    const TimeStamp TIME_STAMP2 = 200;
    const TimeStamp TIME_STAMP3 = 300;
    const TimeStamp TIME_STAMP4 = 400;
    const TimeStamp TIME_STAMP5 = 500;
    const TimeStamp TIME_STAMP6 = 600;
    const TimeStamp TIME_STAMP7 = 700;
    const TimeStamp TIME_STAMP8 = 800;
    const TimeStamp TIME_STAMP9 = 900;
    const TimeStamp TIME_STAMP10 = 1000;
    const TimeStamp TIME_STAMP11 = 1100;
    const TimeStamp TIME_STAMP12 = 1200;

    struct CommitInfo {
        Version version;
        string commitID;
        string leftCommitID;
        string rightCommitID;
        TimeStamp timestamp;
        bool localFlag;
        DeviceID deviceInfo;
    };

    string TransCommitIDToStr(const CommitID &inputCommitID)
    {
        string commitIDStr = "";
        if (inputCommitID.size() != 0) {
            commitIDStr.resize(inputCommitID.size());
            commitIDStr.assign(inputCommitID.begin(), inputCommitID.end());
        }
        return commitIDStr;
    }

    void CompareCommitWithExpectation(const IKvDBCommit *commit, const CommitInfo &commitInfo)
    {
        Version versionInfo = commit->GetCommitVersion();
        ASSERT_EQ(versionInfo, commitInfo.version);
        CommitID commitID = commit->GetCommitId();
        string commitIDStr = TransCommitIDToStr(commitID);
        ASSERT_STREQ(commitIDStr.c_str(), commitInfo.commitID.c_str());
        CommitID leftCommitID = commit->GetLeftParentId();
        string leftCommitIDStr = TransCommitIDToStr(leftCommitID);
        ASSERT_STREQ(leftCommitIDStr.c_str(), commitInfo.leftCommitID.c_str());
        CommitID rightCommitID = commit->GetRightParentId();
        string rightCommitIDStr = TransCommitIDToStr(rightCommitID);
        ASSERT_STREQ(rightCommitIDStr.c_str(), commitInfo.rightCommitID.c_str());
        TimeStamp timestamp = commit->GetTimestamp();
        ASSERT_EQ(timestamp, commitInfo.timestamp);
        bool localFlag = commit->GetLocalFlag();
        ASSERT_EQ(localFlag, commitInfo.localFlag);
        DeviceID deviceInfo = commit->GetDeviceInfo();
        ASSERT_EQ(deviceInfo == commitInfo.deviceInfo, true);
    }

    void TestLatestCommitOfDevice(const std::map<DeviceID, IKvDBCommit*> &latestCommits,
        const DeviceID &deviceInfo, const CommitInfo &expectCommitInfo)
    {
        auto latestCommit = latestCommits.find(deviceInfo);
        ASSERT_EQ(latestCommit != latestCommits.end(), true);
        CompareCommitWithExpectation(latestCommit->second, expectCommitInfo);
    }

    CommitID TransStrToCommitID(const string &commitIDStr)
    {
        CommitID commitID;
        commitID.resize(commitIDStr.size());
        if (commitIDStr.size() != 0) {
            commitID.assign(commitIDStr.begin(), commitIDStr.end());
        }
        return commitID;
    }

    void InsertCommitToCommitStorage(const CommitInfo &commitInfo, int expectedResult)
    {
        int result;
        IKvDBCommit *commit = g_commitStorage->AllocCommit(result);
        ASSERT_EQ(result, E_OK);
        ASSERT_NE(commit, nullptr);
        if (result != E_OK || commit == nullptr) {
            return;
        }
        CommitID commitID = TransStrToCommitID(commitInfo.commitID);
        CommitID leftCommitID = TransStrToCommitID(commitInfo.leftCommitID);
        CommitID rightCommitID = TransStrToCommitID(commitInfo.rightCommitID);
        commit->SetCommitVersion(commitInfo.version);
        commit->SetLeftParentId(leftCommitID);
        commit->SetRightParentId(rightCommitID);
        commit->SetCommitId(commitID);
        commit->SetTimestamp(commitInfo.timestamp);
        commit->SetLocalFlag(commitInfo.localFlag);
        commit->SetDeviceInfo(commitInfo.deviceInfo);
        result = g_commitStorage->AddCommit(*commit, false);
        ASSERT_EQ(result, expectedResult);
        g_commitStorage->ReleaseCommit(commit);
        commit = nullptr;
    }

    void DeleteCommit(const string &inputCommitID, int expectedResult)
    {
        CommitID commitID = TransStrToCommitID(inputCommitID);
        int result = g_commitStorage->RemoveCommit(commitID);
        ASSERT_EQ(result, expectedResult);
    }

    void TestCommit(const CommitInfo &commitInfo, int expectedResult)
    {
        int result;
        CommitID inputCommitID = TransStrToCommitID(commitInfo.commitID);
        IKvDBCommit *commit = g_commitStorage->GetCommit(inputCommitID, result);
        ASSERT_EQ(result, expectedResult);
        if (expectedResult == E_OK) {
            ASSERT_NE(commit, nullptr);
        } else {
            ASSERT_EQ(commit, nullptr);
        }
        if (result != E_OK || commit == nullptr) {
            return;
        }
        CompareCommitWithExpectation(commit, commitInfo);
        g_commitStorage->ReleaseCommit(commit);
        commit = nullptr;
    }

    void SetCommitStorageHeader(const string &inputHeader, int expectedResult)
    {
        CommitID header = TransStrToCommitID(inputHeader);
        int result = g_commitStorage->SetHeader(header);
        ASSERT_EQ(result, expectedResult);
    }

    void TestCommitStorageHeader(const string &expectedHeader)
    {
        int errCode = E_OK;
        CommitID header = g_commitStorage->GetHeader(errCode);
        string headerStr = TransCommitIDToStr(header);
        ASSERT_STREQ(headerStr.c_str(), expectedHeader.c_str());
    }

    /*
    * commit tree is as below:
    * L A B C D
    * 1 2 4 8 d
    * |/|/| |
    * 3 / | 9
    * |X| |/|
    * 5 6 a e
    * |/|/
    * 7 b
    * |/
    * c
    */
    void PrepareCommitTree()
    {
        CommitInfo commitInfo1 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
            TIME_STAMP1, true, g_localDevice};
        CommitInfo commitInfo2 = {g_defaultCommitVer2, g_defaultCommitID2, g_defaultCommitID0, g_defaultCommitID0,
            TIME_STAMP2, false, g_remoteDeviceA};
        CommitInfo commitInfo3 = {g_defaultCommitVer3, g_defaultCommitID3, g_defaultCommitID1, g_defaultCommitID2,
            TIME_STAMP3, true, g_localDevice};
        CommitInfo commitInfo4 = {g_defaultCommitVer4, g_defaultCommitID4, g_defaultCommitID0, g_defaultCommitID0,
            TIME_STAMP4, false, g_remoteDeviceB};
        CommitInfo commitInfo5 = {g_defaultCommitVer5, g_defaultCommitID5, g_defaultCommitID3, g_defaultCommitID4,
            TIME_STAMP5, true, g_localDevice};
        CommitInfo commitInfo6 = {g_defaultCommitVer6, g_defaultCommitID6, g_defaultCommitID2, g_defaultCommitID3,
            TIME_STAMP6, false, g_remoteDeviceA};
        CommitInfo commitInfo7 = {g_defaultCommitVer7, g_defaultCommitID7, g_defaultCommitID5, g_defaultCommitID6,
            TIME_STAMP7, true, g_localDevice};
        CommitInfo commitInfo8 = {g_defaultCommitVer8, g_defaultCommitID8, g_defaultCommitID0, g_defaultCommitID0,
            TIME_STAMP8, false, g_remoteDeviceC};
        CommitInfo commitInfo9 = {g_defaultCommitVer9, g_defaultCommitID9, g_defaultCommitID8, g_defaultCommitID0,
            TIME_STAMP9, false, g_remoteDeviceC};
        CommitInfo commitInfo10 = {g_defaultCommitVer10, g_defaultCommitID10, g_defaultCommitID4, g_defaultCommitID9,
            TIME_STAMP10, false, g_remoteDeviceB};
        CommitInfo commitInfo11 = {g_defaultCommitVer11, g_defaultCommitID11, g_defaultCommitID6, g_defaultCommitID10,
            TIME_STAMP11, false, g_remoteDeviceA};
        CommitInfo commitInfo12 = {g_defaultCommitVer12, g_defaultCommitID12, g_defaultCommitID7, g_defaultCommitID11,
            TIME_STAMP12, true, g_localDevice};
        InsertCommitToCommitStorage(commitInfo1, E_OK);
        InsertCommitToCommitStorage(commitInfo2, E_OK);
        InsertCommitToCommitStorage(commitInfo3, E_OK);
        InsertCommitToCommitStorage(commitInfo4, E_OK);
        InsertCommitToCommitStorage(commitInfo5, E_OK);
        InsertCommitToCommitStorage(commitInfo6, E_OK);
        InsertCommitToCommitStorage(commitInfo7, E_OK);
        InsertCommitToCommitStorage(commitInfo8, E_OK);
        InsertCommitToCommitStorage(commitInfo9, E_OK);
        InsertCommitToCommitStorage(commitInfo10, E_OK);
        InsertCommitToCommitStorage(commitInfo11, E_OK);
        InsertCommitToCommitStorage(commitInfo12, E_OK);
        SetCommitStorageHeader(g_defaultCommitID12, E_OK);
    }
}

class DistributedDBStorageCommitStorageTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageCommitStorageTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_prop.path);
    g_prop.isNeedCreate = true;
    if (IKvDBFactory::GetCurrent() == nullptr) {
        IKvDBFactory *factory = new (std::nothrow) DefaultFactory();
        ASSERT_NE(factory, nullptr);
        if (factory == nullptr) {
            LOGE("failed to new DefaultFactory!");
            return;
        }
        IKvDBFactory::Register(factory);
        g_createFactory = true;
    }
}

void DistributedDBStorageCommitStorageTest::TearDownTestCase(void)
{
    if (g_createFactory) {
        if (IKvDBFactory::GetCurrent() != nullptr) {
            delete IKvDBFactory::GetCurrent();
            IKvDBFactory::Register(nullptr);
        }
    }
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_prop.path) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBStorageCommitStorageTest::SetUp(void)
{
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    ASSERT_NE(factory, nullptr);
    if (factory == nullptr) {
        LOGE("failed to get DefaultFactory!");
        return;
    }
    int result;
    g_commitStorage = factory->CreateMultiVerCommitStorage(result);
    ASSERT_EQ(result, E_OK);
    ASSERT_NE(g_commitStorage, nullptr);
    if (g_commitStorage == nullptr) {
        return;
    }

    int errCode = g_commitStorage->Open(g_prop);
    ASSERT_EQ(errCode, E_OK);
    if (errCode != E_OK) {
        delete g_commitStorage;
        g_commitStorage = nullptr;
        return;
    }
}

void DistributedDBStorageCommitStorageTest::TearDown(void)
{
    if (g_commitStorage != nullptr) {
        (void)g_commitStorage->Remove(g_prop);
        delete g_commitStorage;
        g_commitStorage = nullptr;
    }
}

/**
  * @tc.name: MultiVerCommitStorage001
  * @tc.desc: Open a commit storage when it has been opened.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage001, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Open commit log database
     * @tc.expected: step1/2. Return OK.
     */
    int result = g_commitStorage->Open(g_prop);
    ASSERT_EQ(result, E_OK);
    return;
}

/**
  * @tc.name: MultiVerCommitStorage002
  * @tc.desc: Remove a commit storage database, then try to add, delete and query commit, set and get header.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage002, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Remove commit log database
     * @tc.expected: step1/2. Return OK.
     */
    int result = g_commitStorage->Remove(g_prop);
    ASSERT_EQ(result, E_OK);
    if (result != E_OK) {
        return;
    }
    CommitInfo commitInfo = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step3/4. Insert recording commit log database
     * @tc.expected: step3/4. Return E_INVALID_DB.
     */
    InsertCommitToCommitStorage(commitInfo, -E_INVALID_DB);

    /**
     * @tc.steps:step5/6. Delete commit History to commit log database
     * @tc.expected: step5/6. Return E_INVALID_DB.
     */
    DeleteCommit(g_defaultCommitID1, -E_INVALID_DB);

    /**
     * @tc.steps:step7/8. Cheeck commit History to commit log database
     * @tc.expected: step7/8. Return E_INVALID_DB.
     */
    TestCommit(commitInfo, -E_INVALID_DB);

    /**
     * @tc.steps:step9/10. Cheeck change commit header
     * @tc.expected: step7/10. Return E_INVALID_DB.
     */
    SetCommitStorageHeader(g_defaultCommitID1, -E_INVALID_DB);

    /**
     * @tc.steps:step11/12. Cheeck query commit header
     * @tc.expected: step11/12. Return failed.
     */
    TestCommitStorageHeader(g_defaultCommitID0);
}

/**
  * @tc.name: MultiVerCommitStorage003
  * @tc.desc: Insert a commit to commit storage, and get it.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage003, TestSize.Level1)
{
    CommitInfo commitInfo = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert recording commit log database
     * @tc.expected: step1. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo, E_OK);

    /**
     * @tc.steps:step2. Cheeck commit History to commit log database
     * @tc.expected: step2. Return E_OK.
     */
    TestCommit(commitInfo, E_OK);
    return;
}

/**
  * @tc.name: MultiVerCommitStorage004
  * @tc.desc: Set header of commit storage, and get it.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage004, TestSize.Level1)
{
    CommitInfo commitInfo = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert recording commit log database
     * @tc.expected: step1. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo, E_OK);

    /**
     * @tc.steps:step2. Cheeck change commit header
     * @tc.expected: step2. Return E_OK.
     */
    SetCommitStorageHeader(g_defaultCommitID1, E_OK);

    /**
     * @tc.steps:step3. Cheeck query commit header
     * @tc.expected: step3. Return success.
     */
    TestCommitStorageHeader(g_defaultCommitID1);
    return;
}

/**
  * @tc.name: MultiVerCommitStorage005
  * @tc.desc: Delete the header commit, test if it can be get, and get the new header.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage005, TestSize.Level1)
{
    CommitInfo commitInfo1 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert recording commitInfo1 commit log database
     * @tc.expected: step1. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo1, E_OK);
    CommitInfo commitInfo2 = {g_defaultCommitVer2, g_defaultCommitID2, g_defaultCommitID1, g_defaultCommitID0,
        TIME_STAMP2, true, g_localDevice};

    /**
     * @tc.steps:step2. Insert recording commitInfo2 commit log database
     * @tc.expected: step2. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo2, E_OK);

    /**
     * @tc.steps:step3. Cheeck change commit header
     * @tc.expected: step3. Return E_OK.
     */
    SetCommitStorageHeader(g_defaultCommitID2, E_OK);

    /**
     * @tc.steps:step4. Delete commit History to commit log database
     * @tc.expected: step4. Return E_INVALID_DB.
     */
    DeleteCommit(g_defaultCommitID2, E_OK);

    /**
     * @tc.steps:step5. Cheeck query commit header
     * @tc.expected: step5. Return success.
     */
    TestCommitStorageHeader(g_defaultCommitID1);

    /**
     * @tc.steps:step6. Cheeck commit History to commit log database
     * @tc.expected: step6. Return E_OK.
     */
    TestCommit(commitInfo1, E_OK);

    /**
     * @tc.steps:step7. Cheeck commit History to commit log database
     * @tc.expected: step7. Return E_OK.
     */
    TestCommit(commitInfo2, -E_NOT_FOUND);
    return;
}

/**
  * @tc.name: MultiVerCommitStorage006
  * @tc.desc: Add commit with empty commit ID, and it will not be added.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage006, TestSize.Level1)
{
    CommitInfo commitInfo = {g_defaultCommitVer1, g_defaultCommitID0, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1/2. Insert commit ID is null to commit log database
     * @tc.expected: step1/2. Return E_UNEXPECTED_DATA.
     */
    InsertCommitToCommitStorage(commitInfo, -E_UNEXPECTED_DATA);
}

/**
  * @tc.name: MultiVerCommitStorage008
  * @tc.desc: Add commit with the same commit ID as its left parent, and it will not be added.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage008, TestSize.Level1)
{
    CommitInfo commitInfo1 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert a recording to commit log database
     * @tc.expected: step1. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo1, E_OK);
    CommitInfo commitInfo2 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID1, g_defaultCommitID0,
        TIME_STAMP2, true, g_localDevice};

    /**
     * @tc.steps:step2. Add commit with the same commit ID as its right parent
     * @tc.expected: step2. Return E_UNEXPECTED_DATA.
     */
    InsertCommitToCommitStorage(commitInfo2, -E_UNEXPECTED_DATA);

    /**
     * @tc.steps:step3. Cheeck commit History to commit log database
     * @tc.expected: step3. Return E_OK.
     */
    TestCommit(commitInfo1, E_OK);
}

/**
  * @tc.name: MultiVerCommitStorage009
  * @tc.desc: Add commit with the same commit ID as its right parent, and it will not be added.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage009, TestSize.Level1)
{
    CommitInfo commitInfo1 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert a recording to commit log database
     * @tc.expected: step1. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo1, E_OK);
    CommitInfo commitInfo2 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID1,
        TIME_STAMP2, true, g_localDevice};

    /**
     * @tc.steps:step2. Add commit with the same commit ID as its right parent
     * @tc.expected: step2. Return E_UNEXPECTED_DATA.
     */
    InsertCommitToCommitStorage(commitInfo2, -E_UNEXPECTED_DATA);

    /**
     * @tc.steps:step3. Cheeck commit History to commit log database
     * @tc.expected: step3. Return E_OK.
     */
    TestCommit(commitInfo1, E_OK);
}

/**
  * @tc.name: MultiVerCommitStorage010
  * @tc.desc: Add commit whose left parent and right parent is the same, and it will not be added.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage010, TestSize.Level1)
{
    CommitInfo commitInfo1 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert a recording to commit log database
     * @tc.expected: step1. Return E_OK.
     */
    InsertCommitToCommitStorage(commitInfo1, E_OK);
    CommitInfo commitInfo2 = {g_defaultCommitVer1, g_defaultCommitID2, g_defaultCommitID1, g_defaultCommitID1,
        TIME_STAMP2, true, g_localDevice};

    /**
     * @tc.steps:step2. Add commit whose left parent and right parent is the same
     * @tc.expected: step2. Return E_UNEXPECTED_DATA.
     */
    InsertCommitToCommitStorage(commitInfo2, -E_UNEXPECTED_DATA);

    /**
     * @tc.steps:step3. Cheeck commit History of commitInfo1 to commit log database
     * @tc.expected: step3. Return E_OK.
     */
    TestCommit(commitInfo1, E_OK);

    /**
     * @tc.steps:step3. Cheeck commit History of commitInfo2 to commit log database
     * @tc.expected: step3. Return E_NOT_FOUND.
     */
    TestCommit(commitInfo2, -E_NOT_FOUND);
}

/**
  * @tc.name: MultiVerCommitStorage011
  * @tc.desc: Add commit with a non exist left parent, and it will not be added.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage011, TestSize.Level1)
{
    CommitInfo commitInfo = {g_defaultCommitVer1, g_defaultCommitID2, g_defaultCommitID1, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Add commit with a non exist left parent
     * @tc.expected: step1. Return E_NOT_FOUND.
     */
    InsertCommitToCommitStorage(commitInfo, -E_NOT_FOUND);

    /**
     * @tc.steps:step2. Cheeck commit History to commit log database
     * @tc.expected: step2. Return E_NOT_FOUND.
     */
    TestCommit(commitInfo, -E_NOT_FOUND);
}

/**
  * @tc.name: MultiVerCommitStorage012
  * @tc.desc: Add commit with a non exist right parent, and it will not be added.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage012, TestSize.Level1)
{
    CommitInfo commitInfo = {g_defaultCommitVer1, g_defaultCommitID2, g_defaultCommitID0, g_defaultCommitID1,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Add commit with a non exist right parent
     * @tc.expected: step1. Return E_NOT_FOUND.
     */
    InsertCommitToCommitStorage(commitInfo, -E_NOT_FOUND);

    /**
     * @tc.steps:step2. Cheeck commit History to commit log database
     * @tc.expected: step2. Return E_NOT_FOUND.
     */
    TestCommit(commitInfo, -E_NOT_FOUND);
}

/**
  * @tc.name: MultiVerCommitStorage013
  * @tc.desc: Delete a commit which is not header, and it will not be deleted.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage013, TestSize.Level1)
{
    CommitInfo commitInfo1 = {g_defaultCommitVer1, g_defaultCommitID1, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP1, true, g_localDevice};

    /**
     * @tc.steps:step1. Insert a recording to commit log database
     */
    InsertCommitToCommitStorage(commitInfo1, E_OK);
    CommitInfo commitInfo2 = {g_defaultCommitVer2, g_defaultCommitID2, g_defaultCommitID1, g_defaultCommitID0,
        TIME_STAMP2, true, g_localDevice};

    /**
     * @tc.steps:step2. Insert a left parent to commit log database
     */
    InsertCommitToCommitStorage(commitInfo2, E_OK);

    /**
     * @tc.steps:step3. Set g_defaultCommitID2 is parent to commit log database
     */
    SetCommitStorageHeader(g_defaultCommitID2, E_OK);

    /**
     * @tc.steps:step4. Delete g_defaultCommitID1
     * @tc.expected: step4. Return E_UNEXPECTED_DATA.
     */
    DeleteCommit(g_defaultCommitID1, -E_UNEXPECTED_DATA);

    /**
     * @tc.steps:step5. Cheeck commit header is same as g_defaultCommitID2
     */
    TestCommitStorageHeader(g_defaultCommitID2);

    TestCommit(commitInfo1, E_OK);
    TestCommit(commitInfo2, E_OK);
    return;
}

/**
  * @tc.name: MultiVerCommitStorage014
  * @tc.desc: Set unexist commit to header, and it will not success.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage014, TestSize.Level1)
{
    SetCommitStorageHeader(g_defaultCommitID2, -E_NOT_FOUND);
    TestCommitStorageHeader(g_defaultCommitID0);
}

/**
  * @tc.name: MultiVerCommitStorage015
  * @tc.desc: SDetermine whether commit exists
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage015, TestSize.Level1)
{
    PrepareCommitTree();
    int errCode = E_OK;
    EXPECT_EQ(g_commitStorage->CommitExist(TransStrToCommitID(g_defaultCommitID7), errCode), true);
    EXPECT_EQ(g_commitStorage->CommitExist(TransStrToCommitID(g_defaultCommitID13), errCode), false);
}

/**
  * @tc.name: MultiVerCommitStorage016
  * @tc.desc: Get latest commit of each device from commit storage
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage016, TestSize.Level1)
{
    PrepareCommitTree();
    std::map<DeviceID, IKvDBCommit*> latestCommits;
    int result = g_commitStorage->GetLatestCommits(latestCommits);
    EXPECT_EQ(result, E_OK);
    CommitInfo commitInfoLocal = {g_defaultCommitVer12, g_defaultCommitID12, g_defaultCommitID7,
        g_defaultCommitID11, TIME_STAMP12, true, g_localDevice};
    CommitInfo commitInfoA = {g_defaultCommitVer11, g_defaultCommitID11, g_defaultCommitID6,
        g_defaultCommitID10, TIME_STAMP11, false, g_remoteDeviceA};
    CommitInfo commitInfoB = {g_defaultCommitVer10, g_defaultCommitID10, g_defaultCommitID4,
        g_defaultCommitID9, TIME_STAMP10, false, g_remoteDeviceB};
    CommitInfo commitInfoC = {g_defaultCommitVer9, g_defaultCommitID9, g_defaultCommitID8,
        g_defaultCommitID0, TIME_STAMP9, false, g_remoteDeviceC};
    TestLatestCommitOfDevice(latestCommits, g_localDevice, commitInfoLocal);
    TestLatestCommitOfDevice(latestCommits, g_remoteDeviceA, commitInfoA);
    TestLatestCommitOfDevice(latestCommits, g_remoteDeviceB, commitInfoB);
    TestLatestCommitOfDevice(latestCommits, g_remoteDeviceC, commitInfoC);
    for (auto latestCommit : latestCommits) {
        g_commitStorage->ReleaseCommit(latestCommit.second);
        latestCommit.second = nullptr;
    }
}

/**
  * @tc.name: MultiVerCommitStorage017
  * @tc.desc: Get commit tree from commit storage by latest commits
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageCommitStorageTest, MultiVerCommitStorage017, TestSize.Level1)
{
    PrepareCommitTree();
    map<DeviceID, CommitID> latestCommits;
    latestCommits.insert(make_pair(g_localDevice, TransStrToCommitID(g_defaultCommitID3)));
    latestCommits.insert(make_pair(g_remoteDeviceA, TransStrToCommitID(g_defaultCommitID2)));
    latestCommits.insert(make_pair(g_remoteDeviceC, TransStrToCommitID(g_defaultCommitID14)));
    latestCommits.insert(make_pair(g_remoteDeviceD, TransStrToCommitID(g_defaultCommitID13)));
    list<IKvDBCommit *> commits;
    int result = g_commitStorage->GetCommitTree(latestCommits, commits);
    EXPECT_EQ(result, E_OK);
    vector<IKvDBCommit *> commitsVector(commits.begin(), commits.end());
    LOGD("Commits.size%zu", commits.size());
    ASSERT_GT(commits.size(), 6UL);
    CommitInfo commitInfo4 = {g_defaultCommitVer4, g_defaultCommitID4, g_defaultCommitID0, g_defaultCommitID0,
        TIME_STAMP4, false, g_remoteDeviceB};
    CommitInfo commitInfo5 = {g_defaultCommitVer5, g_defaultCommitID5, g_defaultCommitID3, g_defaultCommitID4,
        TIME_STAMP5, true, g_localDevice};
    CommitInfo commitInfo6 = {g_defaultCommitVer6, g_defaultCommitID6, g_defaultCommitID2, g_defaultCommitID3,
        TIME_STAMP6, false, g_remoteDeviceA};
    CommitInfo commitInfo7 = {g_defaultCommitVer7, g_defaultCommitID7, g_defaultCommitID5, g_defaultCommitID6,
        TIME_STAMP7, true, g_localDevice};
    CommitInfo commitInfo10 = {g_defaultCommitVer10, g_defaultCommitID10, g_defaultCommitID4, g_defaultCommitID9,
        TIME_STAMP10, false, g_remoteDeviceB};
    CommitInfo commitInfo11 = {g_defaultCommitVer11, g_defaultCommitID11, g_defaultCommitID6, g_defaultCommitID10,
        TIME_STAMP11, false, g_remoteDeviceA};
    CommitInfo commitInfo12 = {g_defaultCommitVer12, g_defaultCommitID12, g_defaultCommitID7, g_defaultCommitID11,
        TIME_STAMP12, true, g_localDevice};
    CompareCommitWithExpectation(commitsVector[0], commitInfo4);
    CompareCommitWithExpectation(commitsVector[1], commitInfo5);
    CompareCommitWithExpectation(commitsVector[2], commitInfo6);
    CompareCommitWithExpectation(commitsVector[3], commitInfo7);
    CompareCommitWithExpectation(commitsVector[4], commitInfo10);
    CompareCommitWithExpectation(commitsVector[5], commitInfo11);
    CompareCommitWithExpectation(commitsVector[6], commitInfo12);
    for (auto commit : commits) {
        g_commitStorage->ReleaseCommit(commit);
        commit = nullptr;
    }
}
