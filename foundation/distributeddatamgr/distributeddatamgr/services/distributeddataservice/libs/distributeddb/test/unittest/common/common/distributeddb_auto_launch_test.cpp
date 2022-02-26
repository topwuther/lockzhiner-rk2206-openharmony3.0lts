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
#include "auto_launch.h"
#include "db_errno.h"
#include "log_print.h"
#include "db_common.h"
#include "kvdb_manager.h"
#include "distributeddb_tools_unit_test.h"
#include "vitural_communicator_aggregator.h"
#include "platform_specific.h"
#include "kv_store_nb_conflict_data.h"
#include "kvdb_pragma.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    const std::string APP_ID = "appId";
    const std::string USER_ID = "userId";
    const std::string STORE_ID_0 = "storeId0";
    const std::string STORE_ID_1 = "storeId1";
    const std::string STORE_ID_2 = "storeId2";
    const std::string STORE_ID_3 = "storeId3";
    const std::string STORE_ID_4 = "storeId4";
    const std::string STORE_ID_5 = "storeId5";
    const std::string STORE_ID_6 = "storeId6";
    const std::string STORE_ID_7 = "storeId7";
    const std::string STORE_ID_8 = "storeId8";
    string g_testDir;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    VirtualCommunicatorAggregator *g_communicatorAggregator = nullptr;

    const int TEST_ENABLE_CNT = 10; // 10 time
    const int TEST_ONLINE_CNT = 200; // 10 time
    const int WAIT_TIME = 1000; // 1000ms
    const int LIFE_CYCLE_TIME = 5000; // 5000ms
    const int WAIT_SHORT_TIME = 200; // 20ms
    const TimeStamp TIME_ADD = 1000; // not zero is ok
    const std::string REMOTE_DEVICE_ID = "remote_device";
    const std::string THIS_DEVICE = "real_device";

    const Key KEY1{'k', 'e', 'y', '1'};
    const Key KEY2{'k', 'e', 'y', '2'};
    const Value VALUE1{'v', 'a', 'l', 'u', 'e', '1'};
    const Value VALUE2{'v', 'a', 'l', 'u', 'e', '2'};
    KvDBProperties g_propA;
    KvDBProperties g_propB;
    KvDBProperties g_propC;
    KvDBProperties g_propD;
    KvDBProperties g_propE;
    KvDBProperties g_propF;
    KvDBProperties g_propG;
    KvDBProperties g_propH;
    KvDBProperties g_propI;
    std::string g_identifierA;
    std::string g_identifierB;
    std::string g_identifierC;
    std::string g_identifierD;
    std::string g_identifierE;
    std::string g_identifierF;
    std::string g_identifierG;
    std::string g_identifierH;
    std::string g_identifierI;
}

class DistributedDBAutoLaunchUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown() {};
};

void DistributedDBAutoLaunchUnitTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    string dir = g_testDir;
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(
        g_testDir + "/" + DBCommon::TransferStringToHex(g_identifierA) + "/single_ver") != 0) {
        LOGE("rm test db files error!");
    }
    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);
}

void DistributedDBAutoLaunchUnitTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(
        g_testDir + "/" + DBCommon::TransferStringToHex(g_identifierA) + "/single_ver") != 0) {
        LOGE("rm test db files error!");
    }
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
}

static void GetProperty(KvDBProperties &prop, std::string &identifier, std::string storeId)
{
    prop.SetStringProp(KvDBProperties::USER_ID, USER_ID);
    prop.SetStringProp(KvDBProperties::APP_ID, APP_ID);
    prop.SetStringProp(KvDBProperties::STORE_ID, storeId);
    identifier = DBCommon::TransferHashString(USER_ID + "-" + APP_ID + "-" + storeId);
    prop.SetStringProp(KvDBProperties::IDENTIFIER_DATA, identifier);
    std::string identifierDirA = DBCommon::TransferStringToHex(identifier);
    prop.SetStringProp(KvDBProperties::IDENTIFIER_DIR, identifierDirA);
    prop.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    prop.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    prop.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
}

void DistributedDBAutoLaunchUnitTest::SetUp(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(
        g_testDir + "/" + DBCommon::TransferStringToHex(g_identifierA) + "/single_ver") != 0) {
        LOGE("rm test db files error!");
    }
    GetProperty(g_propA, g_identifierA, STORE_ID_0);
    GetProperty(g_propB, g_identifierB, STORE_ID_1);
    GetProperty(g_propC, g_identifierC, STORE_ID_2);
    GetProperty(g_propD, g_identifierD, STORE_ID_3);
    GetProperty(g_propE, g_identifierE, STORE_ID_4);
    GetProperty(g_propF, g_identifierF, STORE_ID_5);
    GetProperty(g_propG, g_identifierG, STORE_ID_6);
    GetProperty(g_propH, g_identifierH, STORE_ID_7);
    GetProperty(g_propI, g_identifierI, STORE_ID_8);
}

static void PutSyncData(const KvDBProperties &prop, const Key &key, const Value &value)
{
    int errCode = E_OK;
    auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(KvDBManager::OpenDatabase(prop, errCode));
    ASSERT_NE(kvStore, nullptr);
    auto *connection = kvStore->GetDBConnection(errCode);
    ASSERT_NE(connection, nullptr);
    if (kvStore != nullptr) {
        std::vector<DataItem> vect;
        TimeStamp time;
        kvStore->GetMaxTimeStamp(time);
        time += TIME_ADD;
        LOGD("time:%lld", time);
        vect.push_back({key, value, time, 0, DBCommon::TransferHashString(REMOTE_DEVICE_ID)});
        EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(kvStore, vect, REMOTE_DEVICE_ID), E_OK);
    }
    RefObject::DecObjRef(kvStore);
    connection->Close();
    connection = nullptr;
}

static void SetLifeCycleTime(const KvDBProperties &prop)
{
    int errCode = E_OK;
    auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(KvDBManager::OpenDatabase(prop, errCode));
    ASSERT_NE(kvStore, nullptr);
    auto *connection = kvStore->GetDBConnection(errCode);
    ASSERT_NE(connection, nullptr);
    uint32_t time = LIFE_CYCLE_TIME;
    EXPECT_EQ(connection->Pragma(PRAGMA_SET_AUTO_LIFE_CYCLE, static_cast<PragmaData>(&time)), E_OK);
    RefObject::DecObjRef(kvStore);
    connection->Close();
    connection = nullptr;
}

/**
 * @tc.name: AutoLaunch001
 * @tc.desc: basic enable/disable func
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. right param A enable
     * @tc.expected: step1. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. wrong param B enable
     * @tc.expected: step2. failed.
     */
    g_propB.SetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode != E_OK);

    /**
     * @tc.steps: step3. right param C enable
     * @tc.expected: step3. success.
     */
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propC, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step4. param A disable
     * @tc.expected: step4. E_OK.
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step5. param B disable
     * @tc.expected: step5. -E_NOT_FOUND.
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
    EXPECT_TRUE(errCode == -E_NOT_FOUND);

    /**
     * @tc.steps: step6. param C disable
     * @tc.expected: step6. E_OK.
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierC);
    EXPECT_TRUE(errCode == E_OK);
}

/**
 * @tc.name: AutoLaunch002
 * @tc.desc: online callback
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch002, TestSize.Level3)
{
    std::mutex cvMutex;
    std::condition_variable cv;
    bool finished = false;
    std::map<const std::string, AutoLaunchStatus> statusMap;

    auto notifier = [&cvMutex, &cv, &finished, &statusMap] (const std::string &userId, const std::string &appId,
        const std::string &storeId, AutoLaunchStatus status) {
            LOGD("int AutoLaunch002 notifier status:%d", status);
            std::string identifier = DBCommon::TransferHashString(userId + "-" + appId + "-" + storeId);
            std::unique_lock<std::mutex> lock(cvMutex);
            statusMap[identifier] = status;
            LOGD("int AutoLaunch002 notifier statusMap.size():%d", statusMap.size());
            if (statusMap.size() == 2) { // A and B
                finished = true;
                cv.notify_one();
            }
        };
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps: step1. right param A B enable
     * @tc.expected: step1. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, notifier, observer, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, notifier, observer, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. RunOnConnectCallback
     * @tc.expected: step2. success.
     */
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));

    /**
     * @tc.steps: step3. PutSyncData
     * @tc.expected: step3. notifier WRITE_OPENED
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    PutSyncData(g_propB, KEY1, VALUE1);
    {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&finished] {return finished;});
        EXPECT_TRUE(statusMap[g_identifierA] == WRITE_OPENED);
        EXPECT_TRUE(statusMap[g_identifierB] == WRITE_OPENED);
        statusMap.clear();
        finished = false;
    }
    EXPECT_TRUE(observer->GetCallCount() == 2); // A and B
    delete observer;
    /**
     * @tc.steps: step4. param A B disable
     * @tc.expected: step4. notifier WRITE_CLOSED
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
    EXPECT_TRUE(errCode == E_OK);

    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&finished] {return finished;});
    EXPECT_TRUE(statusMap[g_identifierA] == WRITE_CLOSED);
    EXPECT_TRUE(statusMap[g_identifierB] == WRITE_CLOSED);
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, false);
}

/**
 * @tc.name: AutoLaunch003
 * @tc.desc: CommunicatorLackCallback
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch003, TestSize.Level3)
{
    std::mutex cvMutex;
    std::condition_variable cv;
    bool finished = false;
    std::map<const std::string, AutoLaunchStatus> statusMap;

    auto notifier = [&cvMutex, &cv, &finished, &statusMap] (const std::string &userId, const std::string &appId,
        const std::string &storeId, AutoLaunchStatus status) {
            LOGD("int AutoLaunch002 notifier status:%d", status);
            std::string identifier = DBCommon::TransferHashString(userId + "-" + appId + "-" + storeId);
            std::unique_lock<std::mutex> lock(cvMutex);
            statusMap[identifier] = status;
            LOGD("int AutoLaunch002 notifier statusMap.size():%d", statusMap.size());
            finished = true;
            cv.notify_one();
        };
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);

    /**
     * @tc.steps: step1. right param A B enable
     * @tc.expected: step1. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, notifier, observer, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, notifier, observer, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));

    /**
     * @tc.steps: step3. PutSyncData
     * @tc.expected: step3. notifier WRITE_OPENED
     */
    PutSyncData(g_propA, KEY2, VALUE2);
    {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&finished] {return finished;});
        EXPECT_TRUE(statusMap[g_identifierA] == WRITE_OPENED);
        statusMap.clear();
        finished = false;
    }
    EXPECT_TRUE(observer->GetCallCount() == 1); // only A
    delete observer;
    /**
     * @tc.steps: step4. param A B disable
     * @tc.expected: step4. notifier WRITE_CLOSED
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
    EXPECT_TRUE(errCode == E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
    EXPECT_TRUE(errCode == E_OK);

    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&finished] {return finished;});
    EXPECT_TRUE(statusMap[g_identifierA] == WRITE_CLOSED);
    EXPECT_TRUE(statusMap.size() == 1);
}

/**
 * @tc.name: AutoLaunch004
 * @tc.desc: basic enable/disable func
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. right param A~H enable
     * @tc.expected: step1. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propC, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propD, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propE, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propF, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propG, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propH, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. right param I enable
     * @tc.expected: step2. -E_MAX_LIMITS.
     */
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propI, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == -E_MAX_LIMITS);

    /**
     * @tc.steps: step3. param A disable
     * @tc.expected: step3. E_OK.
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step4. right param I enable
     * @tc.expected: step4. E_OK.
     */
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propI, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step6. param B~I disable
     * @tc.expected: step6. E_OK.
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierC);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierD);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierE);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierF);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierG);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierH);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierI);
    EXPECT_TRUE(errCode == E_OK);
}

/**
 * @tc.name: AutoLaunch005
 * @tc.desc: online device before enable
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch005, TestSize.Level3)
{
    std::mutex cvMutex;
    std::condition_variable cv;
    bool finished = false;
    std::map<const std::string, AutoLaunchStatus> statusMap;

    auto notifier = [&cvMutex, &cv, &finished, &statusMap] (const std::string &userId, const std::string &appId,
        const std::string &storeId, AutoLaunchStatus status) {
            LOGD("int AutoLaunch002 notifier status:%d", status);
            std::string identifier = DBCommon::TransferHashString(userId + "-" + appId + "-" + storeId);
            std::unique_lock<std::mutex> lock(cvMutex);
            statusMap[identifier] = status;
            LOGD("int AutoLaunch002 notifier statusMap.size():%d", statusMap.size());
            finished = true;
            cv.notify_one();
        };
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps: step1. RunOnConnectCallback
     * @tc.expected: step1. success.
     */
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));

    /**
     * @tc.steps: step2. right param A enable
     * @tc.expected: step2. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, notifier, observer, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step3. PutSyncData
     * @tc.expected: step3. notifier WRITE_OPENED
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&finished] {return finished;});
        EXPECT_TRUE(statusMap[g_identifierA] == WRITE_OPENED);
        statusMap.clear();
        finished = false;
    }
    EXPECT_TRUE(observer->GetCallCount() == 1); // only A
    /**
     * @tc.steps: step4. param A  disable
     * @tc.expected: step4. notifier WRITE_CLOSED
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
    EXPECT_TRUE(errCode == E_OK);

    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&finished] {return finished;});
    EXPECT_TRUE(statusMap[g_identifierA] == WRITE_CLOSED);
    delete observer;
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, false);
}

/**
 * @tc.name: AutoLaunch006
 * @tc.desc: online callback
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch006, TestSize.Level3)
{
    auto notifier = [] (const std::string &userId, const std::string &appId,
        const std::string &storeId, AutoLaunchStatus status) {
            LOGD("int AutoLaunch006 notifier status:%d", status);
        };
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    std::mutex cvLock;
    std::condition_variable cv;
    bool threadIsWorking = true;
    thread thread([&cvLock, &cv, &threadIsWorking](){
        LabelType label(g_identifierA.begin(), g_identifierA.end());
        for (int i = 0; i < TEST_ONLINE_CNT; i++) {
            g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, true);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_SHORT_TIME));
            g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, false);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_SHORT_TIME));
            g_communicatorAggregator->RunCommunicatorLackCallback(label);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_SHORT_TIME));
            LOGD("AutoLaunch006 thread i:%d", i);
        }
        std::unique_lock<std::mutex> lock(cvLock);
        threadIsWorking = false;
        cv.notify_one();
    });
    thread.detach();

    for (int i = 0; i < TEST_ENABLE_CNT; i++) {
        int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, notifier, observer, 0, nullptr);
        EXPECT_TRUE(errCode == E_OK);
        errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, notifier, observer, 0, nullptr);
        EXPECT_TRUE(errCode == E_OK);

        errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
        EXPECT_TRUE(errCode == E_OK);
        errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
        EXPECT_TRUE(errCode == E_OK);
        LOGD("AutoLaunch006 disable i:%d", i);
    }
    std::unique_lock<std::mutex> lock(cvLock);
    cv.wait(lock, [&threadIsWorking]{return threadIsWorking == false;});

    delete observer;
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, false);
}

namespace {
std::mutex g_cvMutex;
std::condition_variable g_cv;
bool g_finished = false;
std::map<const std::string, AutoLaunchStatus> g_statusMap;
void ConflictNotifierCallback(const KvStoreNbConflictData &data)
{
    LOGD("in ConflictNotifierCallback");
    Key key;
    Value oldValue;
    Value newValue;
    data.GetKey(key);
    data.GetValue(KvStoreNbConflictData::ValueType::OLD_VALUE, oldValue);
    data.GetValue(KvStoreNbConflictData::ValueType::NEW_VALUE, newValue);
    EXPECT_TRUE(key == KEY1);
    EXPECT_TRUE(oldValue == VALUE1);
    EXPECT_TRUE(newValue == VALUE2);
    g_finished = true;
    g_cv.notify_one();
}

void TestAutoLaunchNotifier(const std::string &userId, const std::string &appId, const std::string &storeId,
    AutoLaunchStatus status)
{
    LOGD("int AutoLaunchNotifier, status:%d", status);
    std::string identifier = DBCommon::TransferHashString(userId + "-" + appId + "-" + storeId);
    std::unique_lock<std::mutex> lock(g_cvMutex);
    g_statusMap[identifier] = status;
    g_finished = true;
    g_cv.notify_one();
};

bool AutoLaunchCallBack(const std::string &identifier, AutoLaunchParam &param, KvStoreObserverUnitTest *observer,
    bool ret)
{
    LOGD("int AutoLaunchCallBack");
    EXPECT_TRUE(identifier == g_identifierA);
    param.userId = USER_ID;
    param.appId = APP_ID;
    param.storeId = STORE_ID_0;
    CipherPassword passwd;
    param.option = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, observer,
        CONFLICT_FOREIGN_KEY_ONLY, ConflictNotifierCallback};
    param.notifier = TestAutoLaunchNotifier;
    return ret;
}

bool AutoLaunchCallBackBadParam(const std::string &identifier, AutoLaunchParam &param)
{
    LOGD("int AutoLaunchCallBack");
    EXPECT_TRUE(identifier == g_identifierA);
    param.notifier = TestAutoLaunchNotifier;
    return true;
}
}

/**
 * @tc.name: AutoLaunch007
 * @tc.desc: enhancement callback return true
 * @tc.type: FUNC
 * @tc.require: AR000EPARJ
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch007, TestSize.Level3)
{
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps: step1. SetAutoLaunchRequestCallback
     * @tc.expected: step1. success.
     */
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(
        std::bind(AutoLaunchCallBack, std::placeholders::_1, std::placeholders::_2, observer, true));
    /**
     * @tc.steps: step2. RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    /**
     * @tc.steps: step3. PutSyncData key1 value1
     * @tc.expected: step3. notifier WRITE_OPENED
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[g_identifierA] == WRITE_OPENED);
        g_statusMap.clear();
        g_finished = false;
    }
    EXPECT_TRUE(observer->GetCallCount() == 1); // only A
    /**
     * @tc.steps: step4. PutSyncData key1 value2
     * @tc.expected: step4. ConflictNotifierCallback
     */
    PutSyncData(g_propA, KEY1, VALUE2);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        g_finished = false;
    }
    /**
     * @tc.steps: step5. wait life cycle ,db close
     * @tc.expected: step5. notifier WRITE_CLOSED
     */
    SetLifeCycleTime(g_propA);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[g_identifierA] == WRITE_CLOSED);
        g_statusMap.clear();
        g_finished = false;
    }
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(nullptr);
    delete observer;
}

/**
 * @tc.name: AutoLaunch008
 * @tc.desc: enhancement callback return false
 * @tc.type: FUNC
 * @tc.require: AR000EPARJ
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch008, TestSize.Level3)
{
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps: step1. SetAutoLaunchRequestCallback
     * @tc.expected: step1. success.
     */
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(
        std::bind(AutoLaunchCallBack, std::placeholders::_1, std::placeholders::_2, observer, false));
    /**
     * @tc.steps: step2. RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    /**
     * @tc.steps: step3. PutSyncData key1 value1
     * @tc.expected: step3. db not open
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    PutSyncData(g_propA, KEY1, VALUE2);

    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    EXPECT_TRUE(observer->GetCallCount() == 0);
    EXPECT_TRUE(g_finished == false);
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(nullptr);
    delete observer;
}

/**
 * @tc.name: AutoLaunch009
 * @tc.desc: enhancement callback return bad param
 * @tc.type: FUNC
 * @tc.require: AR000EPARJ
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch009, TestSize.Level3)
{
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps: step1. SetAutoLaunchRequestCallback
     * @tc.expected: step1. success.
     */
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(AutoLaunchCallBackBadParam);
    /**
     * @tc.steps: step2. RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    /**
     * @tc.steps: step3. PutSyncData key1 value1
     * @tc.expected: step3. db not open, notify INVALID_PARAM
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    PutSyncData(g_propA, KEY1, VALUE2);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    EXPECT_TRUE(observer->GetCallCount() == 0);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[DBCommon::TransferHashString("--")] == INVALID_PARAM);
        g_statusMap.clear();
        g_finished = false;
    }
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(nullptr);
    delete observer;
}

/**
 * @tc.name: AutoLaunch010
 * @tc.desc: enhancement nullptr callback
 * @tc.type: FUNC
 * @tc.require: AR000EPARJ
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch010, TestSize.Level3)
{
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps: step1. SetAutoLaunchRequestCallback, then set nullptr
     * @tc.expected: step1. success.
     */
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(
        std::bind(AutoLaunchCallBack, std::placeholders::_1, std::placeholders::_2, observer, false));

    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(nullptr);
    /**
     * @tc.steps: step2. RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    /**
     * @tc.steps: step3. PutSyncData key1 value1
     * @tc.expected: step3. db not open
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    PutSyncData(g_propA, KEY1, VALUE2);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    EXPECT_TRUE(observer->GetCallCount() == 0);
    EXPECT_TRUE(g_finished == false);
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(nullptr);
    delete observer;
}

/**
 * @tc.name: AutoLaunch011
 * @tc.desc: enhancement GetKvStoreIdentifier
 * @tc.type: FUNC
 * @tc.require: AR000EPARJ
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch011, TestSize.Level3)
{
    EXPECT_EQ(KvStoreDelegateManager::GetKvStoreIdentifier("", APP_ID, STORE_ID_0), "");
    EXPECT_EQ(KvStoreDelegateManager::GetKvStoreIdentifier(
        USER_ID, APP_ID, STORE_ID_0), DBCommon::TransferHashString(USER_ID + "-" + APP_ID + "-" + STORE_ID_0));
}

/**
 * @tc.name: AutoLaunch012
 * @tc.desc: CommunicatorLackCallback
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch012, TestSize.Level3)
{
    /**
     * @tc.steps: step1. right param A B enable
     * @tc.expected: step1. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propA, TestAutoLaunchNotifier, nullptr,
        CONFLICT_FOREIGN_KEY_ONLY, ConflictNotifierCallback);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, nullptr, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    PutSyncData(g_propA, KEY1, VALUE1);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[g_identifierA] == WRITE_OPENED);
        g_statusMap.clear();
        g_finished = false;
    }
    /**
     * @tc.steps: step3. PutSyncData key1 value2
     * @tc.expected: step3. ConflictNotifierCallback
     */
    PutSyncData(g_propA, KEY1, VALUE2);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        g_finished = false;
    }
    /**
     * @tc.steps: step4. wait life cycle ,db close
     * @tc.expected: step4. notifier WRITE_CLOSED
     */
    SetLifeCycleTime(g_propA);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[g_identifierA] == WRITE_CLOSED);
        g_statusMap.clear();
        g_finished = false;
    }
    /**
     * @tc.steps: step5. param A B disable
     * @tc.expected: step5. OK
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierA);
    EXPECT_TRUE(errCode == E_OK);
}

/**
 * @tc.name: AutoLaunch013
 * @tc.desc: online callback
 * @tc.type: FUNC
 * @tc.require: AR000E8S2T
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBAutoLaunchUnitTest, AutoLaunch013, TestSize.Level3)
{
    auto notifier = [] (const std::string &userId, const std::string &appId,
        const std::string &storeId, AutoLaunchStatus status) {
            LOGD("int AutoLaunch013 notifier status:%d", status);
        };
    /**
     * @tc.steps: step1. right param b c enable, a SetAutoLaunchRequestCallback
     * @tc.expected: step1. success.
     */
    int errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propB, notifier, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(g_propC, notifier, nullptr, 0, nullptr);
    EXPECT_TRUE(errCode == E_OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(
        std::bind(AutoLaunchCallBack, std::placeholders::_1, std::placeholders::_2, observer, true));

    /**
     * @tc.steps: step2. RunOnConnectCallback RunCommunicatorLackCallback
     * @tc.expected: step2. success.
     */
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, true);
    LabelType label(g_identifierA.begin(), g_identifierA.end());
    g_communicatorAggregator->RunCommunicatorLackCallback(label);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));

    /**
     * @tc.steps: step3. PutSyncData
     * @tc.expected: step3. notifier WRITE_OPENED
     */
    PutSyncData(g_propA, KEY1, VALUE1);
    PutSyncData(g_propB, KEY1, VALUE1);
    PutSyncData(g_propC, KEY1, VALUE1);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[g_identifierA] == WRITE_OPENED);
        g_statusMap.clear();
        g_finished = false;
    }
    /**
     * @tc.steps: step4. PutSyncData key1 value2
     * @tc.expected: step4. ConflictNotifierCallback
     */
    PutSyncData(g_propA, KEY1, VALUE2);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        g_finished = false;
    }
    /**
     * @tc.steps: step5. wait life cycle ,db close
     * @tc.expected: step5. notifier WRITE_CLOSED
     */
    SetLifeCycleTime(g_propA);
    {
        std::unique_lock<std::mutex> lock(g_cvMutex);
        g_cv.wait(lock, [] {return g_finished;});
        EXPECT_TRUE(g_statusMap[g_identifierA] == WRITE_CLOSED);
        g_statusMap.clear();
        g_finished = false;
    }
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(nullptr);
    delete observer;
    /**
     * @tc.steps: step4. param A B disable
     * @tc.expected: step4. notifier WRITE_CLOSED
     */
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierB);
    EXPECT_TRUE(errCode == E_OK);
    errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(g_identifierC);
    EXPECT_TRUE(errCode == E_OK);
    g_communicatorAggregator->RunOnConnectCallback(REMOTE_DEVICE_ID, false);
}