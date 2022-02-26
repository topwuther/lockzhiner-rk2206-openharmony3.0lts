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

#include "distributeddb_data_generate_unit_test.h"
#include "kv_store_nb_conflict_data.h"
#include "kv_store_nb_delegate_impl.h"
#include "sqlite_single_ver_natural_store.h"
#include "sqlite_single_ver_natural_store_connection.h"
#include "time_helper.h"
#include "kvdb_conflict_entry.h"
#include "db_errno.h"
#include "db_common.h"
#include "db_constant.h"
#include "platform_specific.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    const string STORE_ID = STORE_ID_SYNC;
    std::string g_identifier;
    const int CONFLICT_ALL = 15;
    const int TIME_LAG = 100;
    const int DATA_TIME_LAG = 1000;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    SQLiteSingleVerNaturalStore *g_store = nullptr;

    const Value DEFT_VALUE;
    DistributedDB::SQLiteSingleVerNaturalStoreConnection *g_connection = nullptr;
    const auto OLD_VALUE_TYPE = KvStoreNbConflictData::ValueType::OLD_VALUE;
    const auto NEW_VALUE_TYPE = KvStoreNbConflictData::ValueType::NEW_VALUE;
    std::list<KvDBConflictEntry> g_conflictDataList;
    std::vector<KvDBConflictEntry> g_conflictDataConnect;
    struct SingleVerConflictData {
        KvStoreNbConflictType type = CONFLICT_NATIVE_ALL;
        Key key;
        Value oldValue;
        Value newValue;
        bool oldIsDeleted = false;
        bool newIsDeleted = false;
        bool oldIsNative = false;
        bool newIsNative = false;
        int getoldValueErrCode = 0;
        int getNewValueErrCode = 0;
        bool operator==(const SingleVerConflictData &comparedData) const
        {
            if (this->type == comparedData.type &&
                this->key == comparedData.key &&
                this->oldValue == comparedData.oldValue &&
                this->newValue == comparedData.newValue &&
                this->oldIsDeleted == comparedData.oldIsDeleted &&
                this->newIsDeleted == comparedData.newIsDeleted &&
                this->oldIsNative == comparedData.oldIsNative &&
                this->newIsNative == comparedData.newIsNative &&
                this->getoldValueErrCode == comparedData.getoldValueErrCode &&
                this->getNewValueErrCode == comparedData.getNewValueErrCode) {
                return true;
            }
            LOGD("type = %d, ctype = %d", this->type, comparedData.type);
            DBCommon::PrintHexVector(this->key, __LINE__, "key");
            DBCommon::PrintHexVector(comparedData.key, __LINE__, "ckey");
            DBCommon::PrintHexVector(this->oldValue, __LINE__, "value");
            DBCommon::PrintHexVector(comparedData.oldValue, __LINE__, "oldValue");
            DBCommon::PrintHexVector(this->newValue, __LINE__, "oldvalue");
            DBCommon::PrintHexVector(comparedData.newValue, __LINE__, "newValue");

            LOGD("oldIsDeleted = %d, coldIsDeleted = %d", this->oldIsDeleted, comparedData.oldIsDeleted);
            LOGD("newIsDeleted = %d, cnewIsDeleted = %d", this->newIsDeleted, comparedData.newIsDeleted);
            LOGD("oldIsNative = %d, coldIsNative = %d", this->oldIsNative, comparedData.oldIsNative);
            LOGD("newIsNative = %d, cnewIsNative = %d", this->newIsNative, comparedData.newIsNative);
            LOGD("getoldValueErrCode = %d, cgetoldValueErrCode = %d", this->getoldValueErrCode,
                comparedData.getoldValueErrCode);
            LOGD("getNewValueErrCode = %d, cgetNewValueErrCode = %d", this->getNewValueErrCode,
                comparedData.getNewValueErrCode);

            return false;
        }
    };
    std::vector<SingleVerConflictData> g_conflictData;

    void NotifierConnectCallback(const KvDBCommitNotifyData &data)
    {
        int errCode;
        g_conflictDataList = data.GetCommitConflicts(errCode);
        for (const auto &element : g_conflictDataList) {
            g_conflictDataConnect.push_back(element);
        }
    }

    void NotifierCallback(const KvStoreNbConflictData &data)
    {
        Key key;
        Value oldValue;
        Value newValue;
        data.GetKey(key);
        data.GetValue(OLD_VALUE_TYPE, oldValue);
        LOGD("Get new value status:%d", data.GetValue(NEW_VALUE_TYPE, newValue));
        LOGD("Type:%d", data.GetType());
        DBCommon::PrintHexVector(oldValue, __LINE__);
        DBCommon::PrintHexVector(newValue, __LINE__);
        LOGD("Type:IsDeleted %d vs %d, IsNative %d vs %d", data.IsDeleted(OLD_VALUE_TYPE),
            data.IsDeleted(NEW_VALUE_TYPE), data.IsNative(OLD_VALUE_TYPE), data.IsNative(NEW_VALUE_TYPE));
        g_conflictData.push_back({data.GetType(), key, oldValue, newValue, data.IsDeleted(OLD_VALUE_TYPE),
            data.IsDeleted(NEW_VALUE_TYPE), data.IsNative(OLD_VALUE_TYPE), data.IsNative(NEW_VALUE_TYPE),
            data.GetValue(OLD_VALUE_TYPE, oldValue), data.GetValue(NEW_VALUE_TYPE, newValue)});
    }

    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));
}

class DistributedDBStorageRegisterConflictTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageRegisterConflictTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    std::string origIdentifier = USER_ID + "-" + APP_ID + "-" + STORE_ID;
    std::string identifier = DBCommon::TransferHashString(origIdentifier);
    g_identifier = DBCommon::TransferStringToHex(identifier);

    string dir = g_testDir + "/" + g_identifier + "/" + DBConstant::SINGLE_SUB_DIR;
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }
}

void DistributedDBStorageRegisterConflictTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir + "/" + g_identifier + "/" +
        DBConstant::SINGLE_SUB_DIR) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBStorageRegisterConflictTest::SetUp(void)
{
    /*
     * Here, we create STORE_ID before test,
     * and it will be closed in TearDown().
     */
    KvStoreNbDelegate::Option option = {true};
    g_mgr.GetKvStore(STORE_ID, option, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);

    KvDBProperties property;
    property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    property.SetStringProp(KvDBProperties::STORE_ID, STORE_ID);
    property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, g_identifier);
    property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    g_store = new (std::nothrow) SQLiteSingleVerNaturalStore;
    ASSERT_NE(g_store, nullptr);
    ASSERT_EQ(g_store->Open(property), E_OK);

    int erroCode = E_OK;
    g_connection = static_cast<SQLiteSingleVerNaturalStoreConnection *>(g_store->GetDBConnection(erroCode));
    ASSERT_NE(g_connection, nullptr);
    g_store->DecObjRef(g_store);
    EXPECT_EQ(erroCode, E_OK);

    g_conflictData.clear();
    g_conflictDataConnect.clear();
}

void DistributedDBStorageRegisterConflictTest::TearDown(void)
{
    if (g_connection != nullptr) {
        g_connection->Close();
    }

    g_store = nullptr;

    if (g_kvNbDelegatePtr != nullptr) {
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;
        EXPECT_TRUE(g_mgr.DeleteKvStore(STORE_ID) == OK);
    }
}

static bool CheckNewConflictData(KvDBConflictEntry &notifyData, KvDBConflictEntry &expectNotifyData)
{
    if (notifyData.newData.value != expectNotifyData.newData.value) {
        LOGD("New Data value ERROR! Actual vs Expected");
        DBCommon::PrintHexVector(notifyData.newData.value);
        DBCommon::PrintHexVector(expectNotifyData.newData.value);
        return false;
    }

    if (notifyData.oldData.isDeleted != expectNotifyData.oldData.isDeleted) {
        LOGD("Old Data IsDeleted ERROR! Actual %d vs Expected %d", notifyData.oldData.isDeleted,
            expectNotifyData.oldData.isDeleted);
        return false;
    }
    if (notifyData.oldData.isLocal != expectNotifyData.oldData.isLocal) {
        LOGD("Old Data IsLocal ERROR! Actual %d vs Expected %d",
            notifyData.oldData.isLocal, expectNotifyData.oldData.isLocal);
        return false;
    }

    if (notifyData.oldData.value != expectNotifyData.oldData.value) {
        LOGD("Old Data value ERROR! Actualvs Expected");
        DBCommon::PrintHexVector(notifyData.oldData.value);
        DBCommon::PrintHexVector(expectNotifyData.oldData.value);
        return false;
    }

    return true;
}

static bool CheckOldConflictData(KvDBConflictEntry &notifyData, KvDBConflictEntry &expectNotifyData)
{
    if (notifyData.type != expectNotifyData.type) {
        LOGD("Conflict Type ERROR! Actual %d vs Expected %d", notifyData.type, expectNotifyData.type);
        return false;
    }

    if (notifyData.key != expectNotifyData.key) {
        LOGD("key not match");
        return false;
    }

    if (notifyData.newData.isDeleted != expectNotifyData.newData.isDeleted) {
        LOGD("New Data IsDeleted ERROR! Actual %d vs Expected %d", notifyData.newData.isDeleted,
            expectNotifyData.newData.isDeleted);
        return false;
    }

    if (notifyData.newData.isLocal != expectNotifyData.newData.isLocal) {
        LOGD("New Data IsLocal ERROR! Actual %d vs Expected %d", notifyData.newData.isLocal,
            expectNotifyData.newData.isLocal);
        return false;
    }

    return true;
}

static void SyncPutConflictData(int deltaTime)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    TimeStamp timeEnd = TimeHelper::GetSysCurrentTime();
    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, timeEnd, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_EQ(g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback), E_OK);
    EXPECT_EQ(g_conflictDataConnect.empty(), true);

    vect.clear();
    vect.push_back({KEY_1, VALUE_2, timeEnd + deltaTime, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceC"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_EQ(g_conflictDataConnect.empty(), false);

    KvDBConflictEntry expectNotifyData1 = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ONLY, KEY_1,
        {VALUE_2, false, false}, {VALUE_1, false, true}};
    KvDBConflictEntry expectNotifyData2 = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ONLY, KEY_1,
        {VALUE_1, false, true}, {VALUE_2, false, false}};
    if (deltaTime > 0) {
        EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData2), true);
        EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData2), true);
    } else {
        EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData1), true);
        EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData1), true);
    }
}

static void SyncDeleteConflictData(const int deltaTime)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    TimeStamp time = TimeHelper::GetSysCurrentTime();

    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, time, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_EQ(g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback), E_OK);
    EXPECT_EQ(g_conflictDataConnect.empty(), true);

    vect.clear();
    std::vector<uint8_t> hashKey;
    DistributedDBToolsUnitTest::CalcHash(KEY_1, hashKey);
    vect.push_back({hashKey, DEFT_VALUE, time + deltaTime, 1, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceC"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
}

static void SyncPutFromDiffDevConflictData(const int deltaTime)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    TimeStamp time = TimeHelper::GetSysCurrentTime();

    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, time, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback);

    vect.clear();
    vect.push_back({KEY_1, VALUE_2, time + deltaTime, 0, DBCommon::TransferHashString("deviceC")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceC"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
}

static void SyncDeleteFromDiffDevConflictData(const int deltaTime)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    TimeStamp time = TimeHelper::GetSysCurrentTime();

    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, time, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));

    ASSERT_EQ(g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback), E_OK);
    EXPECT_EQ(g_conflictDataConnect.empty(), true);

    vect.clear();
    std::vector<uint8_t> hashKey;
    DistributedDBToolsUnitTest::CalcHash(KEY_1, hashKey);
    vect.push_back({hashKey, DEFT_VALUE, time + deltaTime, 1, DBCommon::TransferHashString("deviceC")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceC"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
}

/**
  * @tc.name: ConflictNotificationTest001
  * @tc.desc: Put a non-conflict key and expect no conflict being triggered.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Setup conflict notifier.
     * @tc.expected: step1. Expect setup success
     */
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);
    /**
     * @tc.steps:step2. Put a key into the database.
     * @tc.expected: step2. Return no conflict.
     */
    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_EQ(g_conflictData.empty(), true);
}

/**
  * @tc.name: ConflictNotificationTest002
  * @tc.desc: Put a native conflict key and expect native conflict being triggered.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest002, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Put a kv data into database with KEY_1, VALUE_1 and setup conflict notifier.
     * @tc.expected: step1/2. setup success.
     */
    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);
    /**
     * @tc.steps:step3. Put another kv data into database with the same key KEY_1.
     * @tc.expected: step3. Expect to trigger a conflict. Return a SingleVerConflictData with {
     * CONFLICT_NATIVE_ALL, KEY_1, VALUE_1, VALUE_2, false, false, true, true}
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_FALSE(g_conflictData.empty());
    SingleVerConflictData expectNotifyData = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
    KEY_1, VALUE_1, VALUE_2, false, false, true, true};
    EXPECT_EQ(g_conflictData.front() == expectNotifyData, true);
}

/**
  * @tc.name: ConflictNotificationTest003
  * @tc.desc: Put a data then delete it. Expect native conflict being triggered.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest003, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Put a kv data into database with KEY_1, VALUE_1 and setup conflict notifier.
     * @tc.expected: step1/2. setup success.
     */
    g_kvNbDelegatePtr->Put(KEY_1, VALUE_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);
    /**
     * @tc.steps:step3. Delete KEY_1.
     * @tc.expected: step3. Expect Delete action triggers a conflict. Return a SingleVerConflictData with {
     * KvStoreNbConflictType::CONFLICT_NATIVE_ALL, KEY_1, VALUE_1, DEFT_VALUE, false, true, true, true, OK, ERROR};
     */
    g_kvNbDelegatePtr->Delete(KEY_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_EQ(g_conflictData.empty(), false);
    SingleVerConflictData expectNotifyData = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
    KEY_1, VALUE_1, DEFT_VALUE, false, true, true, true, OK, DB_ERROR};
    EXPECT_EQ(g_conflictData.front() == expectNotifyData, true);
}

/**
  * @tc.name: ConflictNotificationTest004
  * @tc.desc: Sync a data then put a data with the same key. Expect native conflict being triggered.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest004, TestSize.Level1)
{
    TimeStamp time = TimeHelper::GetSysCurrentTime();
    /**
     * @tc.steps:step1/2. Sync a kv data into database with KEY_1, VALUE_1 and setup conflict notifier.
     * @tc.expected: step1/2. setup conflict notifier success.
     */
    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, time, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);
    /**
     * @tc.steps:step3. Put a kv data with the same key but different value KEY_1, VALUE_2.
     * @tc.expected: step3. Expect Put action triggers a conflict, which is of type SingleVerConflictData,
     * {KvStoreNbConflictType::CONFLICT_NATIVE_ALL, KEY_1, VALUE_1, VALUE_2, false, false, true, true, OK, OK};
     */
    g_kvNbDelegatePtr->Put(KEY_1, VALUE_2);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_EQ(g_conflictData.empty(), false);
    SingleVerConflictData expectNotifyData = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
        KEY_1, VALUE_1, VALUE_2, false, false, true, true, OK, OK};
    EXPECT_EQ(g_conflictData.front() == expectNotifyData, true);
}

/**
  * @tc.name: ConflictNotificationTest005
  * @tc.desc: Get a Sync data then delete it. Expect to see native conflict.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest005, TestSize.Level1)
{
    TimeStamp time = TimeHelper::GetSysCurrentTime();
    /**
     * @tc.steps:step1/2. Sync a kv data into database with KEY_1, VALUE_1 and setup conflict notifier.
     * @tc.expected: step1/2. setup conflict notifier success.
     */
    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, time, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    /**
     * @tc.steps:step3. Delete the synchronized data.
     * @tc.expected: step3. Expect Delete action triggers a conflict, which is of type SingleVerConflictData,
     * {KvStoreNbConflictType::CONFLICT_NATIVE_ALL, KEY_1, VALUE_1, DEFT_VALUE, false, true, true, true, OK, ERROR};
     */
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);
    g_kvNbDelegatePtr->Delete(KEY_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    SingleVerConflictData expectNotifyData = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
    KEY_1, VALUE_1, DEFT_VALUE, false, true, true, true, OK, DB_ERROR};
    EXPECT_EQ(g_conflictData.front() == expectNotifyData, true);
}

/**
  * @tc.name: ConflictNotificationTest006
  * @tc.desc: Get a sync data without local key that conflicts with it. Expect to see no conflict.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest006, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database with KEY_1, VALUE_1 and setup conflict notifier.
     * @tc.expected: step1/2. setup conflict notifier success and no conflict being triggered.
     */
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);
    TimeStamp time = TimeHelper::GetSysCurrentTime();
    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, time, 1, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_EQ(g_conflictData.empty(), true);
}

/**
  * @tc.name: ConflictNotificationTest007
  * @tc.desc: Sync-sync data conflict. Expect to see foreign conflict and the winner has larger time tag.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest007, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another with the same key and same origin, with a time
     * lag DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect to see conflict with Foreign key only conflict:
     * {CONFLICT_FOREIGN_KEY_ONLY, KEY_1.key, {VALUE_1, false, true}, {VALUE_2, false, false}}
     */
    SyncPutConflictData(DATA_TIME_LAG);
}

/**
  * @tc.name: ConflictNotificationTest008
  * @tc.desc: Sync-sync data conflict. Expect to see foreign conflict and the winner has larger time tag.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest008, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another with the same key and same origin, with
     * time advanced DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect to see conflict:
     * {CONFLICT_FOREIGN_KEY_ONLY, KEY_1.key, {VALUE_1, false, true}, {VALUE_2, false, false}}
     */
    SyncPutConflictData(-DATA_TIME_LAG);
}

/**
  * @tc.name: ConflictNotificationTest009
  * @tc.desc: Sync a data to the device. Sync another data with the same key and the time tag +DATA_TIME_LAG us.
  *           Expect to see native conflict and the first data being deleted.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest009, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another deleted data (null data) with the same key
     * and same origin, with a time lag DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect the deleted data triggers conflict:
     * {CONFLICT_FOREIGN_KEY_ONLY, KEY_1.key, {VALUE_1, false, true}, {DEFT_VALUE.value, true, false}}
     */
    SyncDeleteConflictData(DATA_TIME_LAG);
    ASSERT_FALSE(g_conflictDataConnect.empty());
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ONLY, KEY_1,
        {VALUE_1, false, true}, {DEFT_VALUE, true, false}};
    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

/**
  * @tc.name: ConflictNotificationTest010
  * @tc.desc: Sync a data to the device. Sync another data with the same key and the time tag -DATA_TIME_LAG us.
  *           Expect to see native conflict and the second data being deleted.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest010, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another deleted data (null data) with the same key
     * and same origin, with a time lag DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect the deleted data triggers conflict:
     * {CONFLICT_FOREIGN_KEY_ONLY, KEY_1.key, {DEFT_VALUE.value, true, false}, {VALUE_1, false, true}}
     */
    SyncDeleteConflictData(-DATA_TIME_LAG);
    EXPECT_EQ(g_conflictDataConnect.empty(), false);
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ONLY, KEY_1,
        {DEFT_VALUE, true, false}, {VALUE_1, false, true}};
    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

/**
  * @tc.name: ConflictNotificationTest011
  * @tc.desc: Sync-sync multi-origin conflict.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest011, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another with the same key
     * but origin differs from the previous, with a time lag DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect the Put Action triggers conflict:
     * {CONFLICT_FOREIGN_KEY_ORIG, KEY_1.key, {VALUE_1, false, true}, {VALUE_2, false, false}}
     */
    SyncPutFromDiffDevConflictData(DATA_TIME_LAG);
    EXPECT_EQ(g_conflictDataConnect.empty(), false);
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ORIG, KEY_1,
        {VALUE_1, false, true}, {VALUE_2, false, false}};
    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

/**
  * @tc.name: ConflictNotificationTest012
  * @tc.desc: Sync-sync multi-origin conflict.
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest012, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another with the same key
     * but origin differs from the previous, with a time advanced DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect the Put Action triggers conflict:
     * {CONFLICT_FOREIGN_KEY_ORIG, KEY_1.key, {VALUE_1, false, true}, {VALUE_2, false, false}}
     */
    SyncPutFromDiffDevConflictData(-DATA_TIME_LAG);
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ORIG, KEY_1,
        {VALUE_2, false, false}, {VALUE_1, false, true}};
    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

/**
  * @tc.name: ConflictNotificationTest013
  * @tc.desc: Sync-sync multi-origin conflict with deleted data
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest013, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another with the same key
     * but origin differs from the previous, with a time lag DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect the deleted data triggers conflict:
     * {CONFLICT_FOREIGN_KEY_ORIG, KEY_1.key, {VALUE_1, false, true}, {VALUE_2, false, false}}
     */
    SyncDeleteFromDiffDevConflictData(DATA_TIME_LAG);
    EXPECT_EQ(g_conflictDataConnect.empty(), false);
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ORIG, KEY_1,
        {VALUE_1, false, true}, {DEFT_VALUE, true, false}};
    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

/**
  * @tc.name: ConflictNotificationTest014
  * @tc.desc: Sync-sync multi-origin conflict with deleted data
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest014, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Sync a kv data into database, and sync another with the same key
     * but origin differs from the previous, with a time advanced DATA_TIME_LAG us.
     * @tc.expected: step1/2. Expect the deleted data triggers conflict:
     * {CONFLICT_FOREIGN_KEY_ORIG, KEY_1.key, {VALUE_1, false, true}, {VALUE_2, false, false}}
     */
    SyncDeleteFromDiffDevConflictData(-DATA_TIME_LAG);
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ORIG, KEY_1,
        {DEFT_VALUE, true, false}, {VALUE_1, false, true}};
    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

/**
  * @tc.name: ConflictNotificationTest015
  * @tc.desc: put same record for conflict notification function
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest015, TestSize.Level1)
{
    TimeStamp timeEnd = TimeHelper::GetSysCurrentTime();

    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, timeEnd, 0, "deviceB", 0, "deviceB"});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_EQ(g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback), E_OK);
    EXPECT_EQ(g_conflictDataConnect.empty(), true);

    vect.clear();
    vect.push_back({KEY_1, VALUE_1, timeEnd + DATA_TIME_LAG, 0, "deviceB", 0, "deviceB"});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_EQ(g_conflictDataConnect.empty(), true);
}

/**
  * @tc.name: ConflictNotificationTest016
  * @tc.desc: put record for conflict notification function
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest016, TestSize.Level1)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    TimeStamp timeEnd = TimeHelper::GetSysCurrentTime();

    std::vector<DataItem> vect;
    vect.push_back({KEY_1, VALUE_1, timeEnd, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    ASSERT_EQ(g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback), E_OK);
    EXPECT_EQ(g_conflictDataConnect.empty(), true);

    vect.clear();
    vect.push_back({KEY_1, VALUE_2, timeEnd, 0, DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));

    ASSERT_EQ(g_conflictDataConnect.empty(), false);
    KvDBConflictEntry expectNotifyData = {KvStoreNbConflictType::CONFLICT_FOREIGN_KEY_ONLY, KEY_1,
        {VALUE_1, false, true}, {VALUE_2, false, false}};

    EXPECT_EQ(CheckOldConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
    EXPECT_EQ(CheckNewConflictData(g_conflictDataConnect.front(), expectNotifyData), true);
}

namespace {
    void GetNewConflictStore()
    {
        if (g_connection != nullptr) {
            g_connection->Close();
        }
        if (g_kvNbDelegatePtr != nullptr) {
            EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
            g_kvNbDelegatePtr = nullptr;
            EXPECT_TRUE(g_mgr.DeleteKvStore(STORE_ID) == OK);
        }
        KvDBProperties property;
        property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
        property.SetStringProp(KvDBProperties::STORE_ID, STORE_ID);
        property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, g_identifier);
        property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
        property.SetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, DENY_OTHER_DEV_AMEND_CUR_DEV_DATA);
        g_store = new (std::nothrow) SQLiteSingleVerNaturalStore;
        ASSERT_NE(g_store, nullptr);
        ASSERT_EQ(g_store->Open(property), E_OK);

        int erroCode = E_OK;
        g_connection = static_cast<SQLiteSingleVerNaturalStoreConnection *>(g_store->GetDBConnection(erroCode));
        ASSERT_NE(g_connection, nullptr);
        g_store->DecObjRef(g_store);
        EXPECT_EQ(erroCode, E_OK);
    }
}

/**
  * @tc.name: ConflictNotificationTest017
  * @tc.desc: put record for conflict notification function
  * @tc.type: FUNC
  * @tc.require: AR000CQS3U
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageRegisterConflictTest, ConflictNotificationTest017, TestSize.Level1)
{
    GetNewConflictStore();
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    std::vector<DataItem> vect;
    g_connection->Put(option, KEY_1, VALUE_1);
    ASSERT_EQ(g_connection->SetConflictNotifier(CONFLICT_ALL, NotifierConnectCallback), E_OK);

    TimeStamp timeEnd = TimeHelper::GetSysCurrentTime();
    static const uint32_t addTimestamp = 10000;
    vect.push_back({KEY_1, VALUE_2, timeEnd + addTimestamp, 0, "", timeEnd + addTimestamp,
        DBCommon::TransferHashString("deviceB")});
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(g_store, vect, "deviceB"), E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
    EXPECT_EQ(g_conflictDataConnect.empty(), true);
    Value readValue;
    EXPECT_EQ(g_connection->Get(option, KEY_1, readValue), E_OK);
    EXPECT_EQ(VALUE_1, readValue);
}