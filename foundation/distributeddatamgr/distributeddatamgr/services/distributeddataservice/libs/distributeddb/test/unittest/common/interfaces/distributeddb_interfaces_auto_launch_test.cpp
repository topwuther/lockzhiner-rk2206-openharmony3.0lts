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

#include <thread>
#include <chrono>

#include "db_errno.h"
#include "runtime_context.h"
#include "db_common.h"
#include "db_constant.h"
#include "kvdb_manager.h"
#include "kvdb_pragma.h"
#include "kv_store_delegate_manager.h"
#include "distributeddb_tools_unit_test.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    // define some variables to init a KvStoreDelegateManager object.
    const std::string APP_ID1 = "app1";
    const std::string USER_ID1 = "user1";
    const Key KEY_1 = {'K', '1'};
    KvStoreDelegateManager g_mgr(APP_ID1, USER_ID1);
    std::string g_testDir;

    constexpr int MAX_AUTO_LAUNCH_NUM = 8;
    constexpr uint32_t AUTO_LAUNCH_CYCLE_TIME = 6000;
    constexpr uint32_t AUTO_LAUNCH_CHECK_TIME = (AUTO_LAUNCH_CYCLE_TIME / 2) + 500; // 500ms more than half.
    constexpr int WAIT_FOR_RESPONSE_TIME = 200;
    // define the g_kvDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvStoreStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvStore = nullptr;
    auto g_kvNbDelegateCallback = std::bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        std::placeholders::_1, std::placeholders::_2, std::ref(g_kvStoreStatus), std::ref(g_kvStore));

    const std::string SCHEMA_DEFINE1 = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name2\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\"]}";
    const std::string SCHEMA_DEFINE2 = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name3\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\"]}";
    class StoreCommunicatorAggregator : public ICommunicatorAggregator {
    public:
        // Return 0 as success. Return negative as error
        int Initialize(IAdapter *inAdapter) override
        {
            return E_OK;
        }

        void Finalize() override
        {}

        // If not success, return nullptr and set outErrorNo
        ICommunicator *AllocCommunicator(uint64_t commLabel, int &outErrorNo) override
        {
            outErrorNo = -E_OUT_OF_MEMORY;
            return nullptr;
        }
        ICommunicator *AllocCommunicator(const LabelType &commLabel, int &outErrorNo) override
        {
            outErrorNo = -E_OUT_OF_MEMORY;
            return nullptr;
        }

        void ReleaseCommunicator(ICommunicator *inCommunicator) override
        {}

        int RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack, const Finalizer &inOper) override
        {
            lackCallback_ = onCommLack;
            return E_OK;
        }
        int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override
        {
            return E_OK;
        }

        void PutCommLackInfo(const std::string &identifier) const
        {
            if (lackCallback_) {
                std::vector<uint8_t> vect(identifier.begin(), identifier.end());
                lackCallback_(vect);
            }
        }
    private:
        CommunicatorLackCallback lackCallback_;
    };

    struct AutoLaunchNotifyInfo {
        void Reset()
        {
            triggerTimes = 0;
        }
        int triggerTimes = 0;
        std::string userId;
        std::string appId;
        std::string storeId;
        AutoLaunchStatus status = WRITE_CLOSED;
    };
    AutoLaunchNotifyInfo g_autoLaunchNotifyInfo;

    void AutoLaunchNotifierCallback(AutoLaunchNotifyInfo &info, const std::string &userId, const std::string &appId,
        const std::string &storeId, AutoLaunchStatus status)
    {
        info.triggerTimes++;
        info.userId = userId;
        info.appId = appId;
        info.storeId = storeId;
        info.status = status;
    }

    auto g_autoLaunchNotifyFunc = std::bind(&AutoLaunchNotifierCallback, std::ref(g_autoLaunchNotifyInfo),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    StoreCommunicatorAggregator *g_aggregator = nullptr;
}

class DistributedDBInterfacesAutoLaunchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesAutoLaunchTest::SetUpTestCase(void)
{
    LOGI("Start test interface auto launch test");
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    KvStoreConfig config;
    config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(config);
    g_aggregator = new (std::nothrow) StoreCommunicatorAggregator;
    ASSERT_NE(g_aggregator, nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_aggregator);
}

void DistributedDBInterfacesAutoLaunchTest::TearDownTestCase(void)
{
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesAutoLaunchTest::SetUp(void)
{
    g_kvStoreStatus = INVALID_ARGS;
    g_kvStore = nullptr;
}

void DistributedDBInterfacesAutoLaunchTest::TearDown(void)
{
    g_autoLaunchNotifyInfo.Reset();
}
#if !defined(OMIT_ENCRYPT) && !defined(OMIT_JSON)
/**
  * @tc.name: EnableKvStoreAutoLaunch001
  * @tc.desc: Enable the kvstore with the diff parameters.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, EnableKvStoreAutoLaunch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create the kv store with passwd and no schema.
     * @tc.expected: step1. Returns a non-null kvstore.
     */
    CipherPassword passwd;
    std::vector<uint8_t> passwdVect = {'p', 's', 'd', '1'};
    passwd.SetValue(passwdVect.data(), passwdVect.size());
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, passwd, SCHEMA_DEFINE1, false};
    std::string storeId = "test1";
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvStoreStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvStore), OK);

    /**
     * @tc.steps: step2. Enable the kv store with different password.
     * @tc.expected: step2. Returns INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    passwdVect = {'p', 's', 'd', '2'};
    CipherPassword passwdOther;
    passwdOther.SetValue(passwdVect.data(), passwdVect.size());
    AutoLaunchOption launchOption = {true, true, CipherType::DEFAULT, passwdOther, "", false, g_testDir, nullptr};
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId,
        launchOption, nullptr);
    EXPECT_NE(status, OK);

    /**
     * @tc.steps: step3. Enable the kv store with different schema.
     * @tc.expected: step3. Returns not OK.
     */
    launchOption.passwd = passwd;
    launchOption.schema = SCHEMA_DEFINE2;
    status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId,
        launchOption, nullptr);
    EXPECT_NE(status, OK);

    /**
     * @tc.steps: step4. Enable the kv store with correct parameter.
     * @tc.expected: step4. Returns OK.
     */
    launchOption.passwd = passwd;
    launchOption.schema = SCHEMA_DEFINE1;
    status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId,
        launchOption, nullptr);
    EXPECT_EQ(status, OK);
    KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId);
    g_mgr.DeleteKvStore(storeId);
}
#endif
/**
  * @tc.name: EnableKvStoreAutoLaunch002
  * @tc.desc: Enable the kv store auto launch for the change of createIfNecessary.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, EnableKvStoreAutoLaunch002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Enable the kv store with createIfNecessary is false.
     * @tc.expected: step1. Returns not OK.
     */
    CipherPassword passwd;
    std::string storeId = "test2";
    AutoLaunchOption launchOption = {false, false, CipherType::DEFAULT, passwd, "", false, g_testDir, nullptr};
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId,
        launchOption, nullptr);
    EXPECT_NE(status, OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), NOT_FOUND);

    /**
     * @tc.steps: step2. Enable the kv store with createIfNecessary is true.
     * @tc.expected: step2. Returns OK.
     */
    launchOption.createIfNecessary = true;
    status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId,
        launchOption, nullptr);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
}

namespace {
IKvDB *GetKvDB(const std::string &storeId)
{
    KvDBProperties prop;
    prop.SetStringProp(KvDBProperties::USER_ID, USER_ID1);
    prop.SetStringProp(KvDBProperties::APP_ID, APP_ID1);
    prop.SetStringProp(KvDBProperties::STORE_ID, storeId);
    std::string identifier = DBCommon::TransferHashString(USER_ID1 + "-" + APP_ID1 + "-" + storeId);

    prop.SetStringProp(KvDBProperties::IDENTIFIER_DATA, identifier);
    std::string identifierDir = DBCommon::TransferStringToHex(identifier);
    prop.SetStringProp(KvDBProperties::IDENTIFIER_DIR, identifierDir);
    prop.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    prop.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    prop.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    int errCode = E_OK;
    return KvDBManager::OpenDatabase(prop, errCode);
}

void PutSyncData(const std::string &storeId, const Key &key, const Value &value, bool isCover)
{
    auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(GetKvDB(storeId));
    ASSERT_NE(kvStore, nullptr);
    int errCode;
    auto *connection = kvStore->GetDBConnection(errCode);
    ASSERT_NE(connection, nullptr);
    if (kvStore != nullptr) {
        std::vector<DataItem> vect;
        TimeStamp time = 100; // initial valid timestamp.
        kvStore->GetMaxTimeStamp(time);
        if (isCover) {
            time += 10; // add the diff for 10.
        } else {
            time -= 10; // add the diff for -10.
        }
        vect.push_back({key, value, time, 0, DBCommon::TransferHashString("deviceB")});
        EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(kvStore, vect, "deviceB"), E_OK);
    }
    RefObject::DecObjRef(kvStore);
    connection->Close();
    connection = nullptr;
}

void GetSyncData(const std::string &storeId)
{
    auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(GetKvDB(storeId));
    ASSERT_NE(kvStore, nullptr);
    int errCode;
    auto *connection = kvStore->GetDBConnection(errCode);
    ASSERT_NE(connection, nullptr);

    std::vector<SingleVerKvEntry *> entries;
    ContinueToken token = nullptr;
    DataSizeSpecInfo syncDataSizeInfo = {DBConstant::MAX_VALUE_SIZE, DBConstant::MAX_HPMODE_PACK_ITEM_SIZE};
    kvStore->GetSyncData(0, UINT64_MAX / 2, entries, token, syncDataSizeInfo); // half of the max timestamp.
    for (auto &item : entries) {
        kvStore->ReleaseKvEntry(item);
        item = nullptr;
    }
    if (token != nullptr) {
        kvStore->ReleaseContinueToken(token);
    }

    RefObject::DecObjRef(kvStore);
    connection->Close();
    connection = nullptr;
}

void PrePutDataIntoDatabase(const std::string &storeId)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvStoreStatus == OK);

    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    EXPECT_EQ(g_kvStore->Put(KEY_1, value), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvStore), OK);
}

void TriggerAutoLaunch(const std::string &storeId, bool isWriteCovered)
{
    /**
     * @tc.steps: step1. Enable the auto launch of the database.
     * @tc.expected: step1. Returns OK.
     */
    PrePutDataIntoDatabase(storeId);
    CipherPassword passwd;
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_NE(observer, nullptr);

    AutoLaunchOption launchOption = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, observer};
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId, launchOption,
        g_autoLaunchNotifyFunc);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. Trigger the auto launch of the database.
     */
    std::string identifier = DBCommon::TransferHashString(USER_ID1 + "-" + APP_ID1 + "-" + storeId);
    g_aggregator->PutCommLackInfo(identifier);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    PutSyncData(storeId, KEY_1, value, isWriteCovered);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    /**
     * @tc.steps: step3. Check the notifier and the observer.
     */
    if (!isWriteCovered) {
        EXPECT_EQ(g_autoLaunchNotifyInfo.triggerTimes, 0);
    } else {
        EXPECT_GT(g_autoLaunchNotifyInfo.triggerTimes, 0);
        EXPECT_EQ(g_autoLaunchNotifyInfo.status, WRITE_OPENED);
        EXPECT_GT(observer->GetCallCount(), 0UL);
    }

    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    delete observer;
    observer = nullptr;
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
}
}

/**
  * @tc.name: EnableKvStoreAutoLaunch003
  * @tc.desc: test the data change and the notifier of the auto open for no data changed.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, EnableKvStoreAutoLaunch003, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Enable the auto launch of the database.
     * @tc.steps: step2. Trigger the auto launch of the database.
     * @tc.steps: step3. Put the data which would be dispatched into the database by sync.
     * @tc.steps: step4. Check the notifier and the observer change.
     * @tc.expected: step1. Returns OK.
     * @tc.expected: step4. The notifier and the observer wouldn't be triggered.
     */
    TriggerAutoLaunch("test3", false);
}

/**
  * @tc.name: EnableKvStoreAutoLaunch004
  * @tc.desc: test the data change and the notifier of the auto open for data changed.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, EnableKvStoreAutoLaunch004, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Enable the auto launch of the database.
     * @tc.steps: step2. Trigger the auto launch of the database.
     * @tc.steps: step3. Put the data which would overwrite into the database by sync.
     * @tc.steps: step4. Check the notifier and the observer change.
     * @tc.expected: step1. Returns OK.
     * @tc.expected: step4. The notifier and the observer would be triggered.
     */
    TriggerAutoLaunch("test4", true);
}

/**
  * @tc.name: EnableKvStoreAutoLaunch005
  * @tc.desc: Test enable the same database twice.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, EnableKvStoreAutoLaunch005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Enable the kv store auto launch.
     * @tc.expected: step1. Returns OK.
     */
    std::string storeId = "test5";
    CipherPassword passwd;
    AutoLaunchOption launchOption = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, nullptr};
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId, launchOption,
        nullptr);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. Ee-enable the kv store auto launch.
     * @tc.expected: step2. Returns not OK.
     */
    status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId, launchOption,
        nullptr);
    EXPECT_NE(status, OK);
    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
}

/**
  * @tc.name: EnableKvStoreAutoLaunch005
  * @tc.desc: test the over limits for the enable list.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, EnableKvStoreAutoLaunch006, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Enable the 8 kv store auto launch.
     * @tc.expected: step1. Returns OK.
     */
    CipherPassword passwd;
    AutoLaunchOption launchOption = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, nullptr};
    for (int i = 0; i < MAX_AUTO_LAUNCH_NUM; i++) {
        std::string storeId = "store_" + std::to_string(i + 1);
        DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId,
            launchOption, nullptr);
        EXPECT_EQ(status, OK);
    }

    /**
     * @tc.steps: step2. Enable the 9th kv store auto launch.
     * @tc.expected: step2. Returns OK.
     */
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, "store_9",
        launchOption, nullptr);
    EXPECT_EQ(status, OVER_MAX_LIMITS);

    /**
     * @tc.steps: step3. Disable the 1th kv store auto launch.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, "store_1"), OK);
    /**
     * @tc.steps: step4. Enable the 9th kv store auto launch.
     * @tc.expected: step4. Returns OK.
     */
    status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, "store_9",
        launchOption, nullptr);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step5. Disable all the kv stores auto launched.
     * @tc.expected: step5. Returns OK.
     */
    for (int i = 1; i <= MAX_AUTO_LAUNCH_NUM; i++) {
        std::string storeId = "store_" + std::to_string(i + 1);
        EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
    }
    /**
     * @tc.steps: step6. Disable the kv stores which is not enabled.
     * @tc.expected: step6. Returns NOT_FOUND.
     */
    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, "store_1"), NOT_FOUND);
}

namespace {
void SetAutoLaunchLifeCycleTime(const std::string &storeId, uint32_t time)
{
    LOGI("SetAutoLifeTime:%u", time);
    auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(GetKvDB(storeId));
    ASSERT_NE(kvStore, nullptr);
    int errCode;
    auto *connection = kvStore->GetDBConnection(errCode);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->Pragma(PRAGMA_SET_AUTO_LIFE_CYCLE, static_cast<PragmaData>(&time)), E_OK);
    RefObject::DecObjRef(kvStore);
    connection->Close();
    connection = nullptr;
}
}
/**
  * @tc.name: EnableKvStoreAutoLaunch007
  * @tc.desc: test the over limits for the enable list.
  * @tc.type: FUNC
  * @tc.require: AR000DR9KU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, DisableKvStoreAutoLaunch001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Enable the auto launch for 'test7'.
     * @tc.expected: step1. Returns OK.
     */
    CipherPassword passwd;
    AutoLaunchOption launchOption = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, nullptr};
    std::string storeId = "test7";
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId, launchOption,
        g_autoLaunchNotifyFunc);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. Disable the auto launch for 'test7'.
     * @tc.expected: step2. Returns OK.
     */
    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    /**
     * @tc.steps: step3. Trigger the auto launch and check the status of the database.
     * @tc.expected: step3. The database was not auto launched.
     */
    std::string identifier = DBCommon::TransferHashString(USER_ID1 + "-" + APP_ID1 + "-" + storeId);
    g_aggregator->PutCommLackInfo(identifier);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    SetAutoLaunchLifeCycleTime(storeId, AUTO_LAUNCH_CYCLE_TIME);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    PutSyncData(storeId, KEY_1, value, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    EXPECT_EQ(g_autoLaunchNotifyInfo.triggerTimes, 0);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
}

/**
  * @tc.name: AutoLaunchLifeCycle001
  * @tc.desc: test the auto closed for the database auto launched by the msg.
  * @tc.type: FUNC
  * @tc.require: AR000E8S2T
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, AutoLaunchLifeCycle001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Enable the auto launch for 'test8'.
     * @tc.expected: step1. Returns OK.
     */
    CipherPassword passwd;
    AutoLaunchOption launchOption = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, nullptr};
    std::string storeId = "test8";
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId, launchOption,
        g_autoLaunchNotifyFunc);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. Trigger the auto launch.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    std::string identifier = DBCommon::TransferHashString(USER_ID1 + "-" + APP_ID1 + "-" + storeId);
    g_aggregator->PutCommLackInfo(identifier);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    SetAutoLaunchLifeCycleTime(storeId, AUTO_LAUNCH_CYCLE_TIME);
    /**
     * @tc.steps: step3. Put data into the database by sync.
     */
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    PutSyncData(storeId, KEY_1, value, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(AUTO_LAUNCH_CHECK_TIME));
    /**
     * @tc.steps: step4. Check the notifier.
     * @tc.expected: step4. notifier is triggered for the opened change.
     */
    EXPECT_GT(g_autoLaunchNotifyInfo.triggerTimes, 0);
    EXPECT_NE(g_mgr.DeleteKvStore(storeId), OK);
    g_autoLaunchNotifyInfo.Reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(AUTO_LAUNCH_CHECK_TIME));
    /**
     * @tc.steps: step5. Check the notifier for waiting for more than the life time of the auto launched database.
     * @tc.expected: step5. notifier is triggered for the closed change.
     */
    EXPECT_GT(g_autoLaunchNotifyInfo.triggerTimes, 0);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);

    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
}

namespace {
void DelayAutoLaunchCycle(const std::string &storeId, bool isWrite)
{
    CipherPassword passwd;
    AutoLaunchOption launchOption = {true, false, CipherType::DEFAULT, passwd, "", false, g_testDir, nullptr};
    /**
     * @tc.steps: step1. Enable the auto launch for 'test8'.
     * @tc.expected: step1. Returns OK.
     */
    DBStatus status = KvStoreDelegateManager::EnableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId, launchOption,
        nullptr);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. Trigger the auto launch.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    std::string identifier = DBCommon::TransferHashString(USER_ID1 + "-" + APP_ID1 + "-" + storeId);
    g_aggregator->PutCommLackInfo(identifier);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RESPONSE_TIME));
    SetAutoLaunchLifeCycleTime(storeId, AUTO_LAUNCH_CYCLE_TIME);

    /**
     * @tc.steps: step3. Write/Read the data into/from the database by sync.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(AUTO_LAUNCH_CHECK_TIME));
    EXPECT_NE(g_mgr.DeleteKvStore(storeId), OK);
    if (isWrite) {
        Value value;
        DistributedDBToolsUnitTest::GetRandomKeyValue(value);
        PutSyncData(storeId, KEY_1, value, true);
    } else {
        GetSyncData(storeId);
    }

    /**
     * @tc.steps: step5. Check the status of the auto launched database.
     * @tc.expected: step5. the life cycle of the auto launched database is prolonged by the sync operation.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(AUTO_LAUNCH_CHECK_TIME));
    EXPECT_NE(g_mgr.DeleteKvStore(storeId), OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(AUTO_LAUNCH_CHECK_TIME));
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);

    EXPECT_EQ(KvStoreDelegateManager::DisableKvStoreAutoLaunch(USER_ID1, APP_ID1, storeId), OK);
}
}

/**
  * @tc.name: AutoLaunchLifeCycle002
  * @tc.desc: test the over limits for the enable list.
  * @tc.type: FUNC
  * @tc.require: AR000E8S2T
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, AutoLaunchLifeCycle002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Enable the auto launch for 'test_9'.
     * @tc.steps: step2. Trigger the auto launch.
     * @tc.steps: step3. Trigger the sync writing operation.
     * @tc.steps: step4. Check the status of the auto launched database.
     * @tc.expected: step1. Returns OK.
     * @tc.expected: step4. The life cycle is prolonged for the writing operation.
     */
    DelayAutoLaunchCycle("test_9", true);
}

/**
  * @tc.name: AutoLaunchLifeCycle003
  * @tc.desc: test the life cycle of the auto launched database in the sync reading scene.
  * @tc.type: FUNC
  * @tc.require: AR000E8S2T
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesAutoLaunchTest, AutoLaunchLifeCycle003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Enable the auto launch for 'test_10'.
     * @tc.steps: step2. Trigger the auto launch.
     * @tc.steps: step3. Trigger the sync reading operation.
     * @tc.steps: step4. Check the status of the auto launched database.
     * @tc.expected: step1. Returns OK.
     * @tc.expected: step4. The life cycle is prolonged for the reading operation.
     */
    DelayAutoLaunchCycle("test_10", false);
}
