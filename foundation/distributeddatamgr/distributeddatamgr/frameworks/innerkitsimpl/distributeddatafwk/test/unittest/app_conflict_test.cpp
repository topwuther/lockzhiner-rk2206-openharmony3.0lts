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

#define LOG_TAG "AppConflictTest"

#include "app_distributed_kv_data_manager.h"
#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <unistd.h>
#include "app_kvstore.h"
#include "app_types.h"
#include "log_print.h"

using namespace testing::ext;
using namespace OHOS::AppDistributedKv;

class AppConflictTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<AppDistributedKvDataManager> manager;
    static std::unique_ptr<AppKvStore> appKvStorePtr; // declare kvstore instance.
    static Status statusGetKvStore;
};

std::shared_ptr<AppDistributedKvDataManager> AppConflictTest::manager;
std::unique_ptr<AppKvStore> AppConflictTest::appKvStorePtr = nullptr;
Status AppConflictTest::statusGetKvStore = Status::ERROR;
static const auto OLD_VALUE_TYPE = AppKvStoreConflictData::ConflictValueType::OLD_VALUE;
static const auto NEW_VALUE_TYPE = AppKvStoreConflictData::ConflictValueType::NEW_VALUE;
static const int TIME_SLEEP = 100000;

struct ConflictDataTest {
    AppKvStoreConflictPolicyType type = AppKvStoreConflictPolicyType::CONFLICT_NATIVE_ALL;
    Key key;
    Value oldValue;
    Value newValue;
    bool oldIsDeleted = false;
    bool newIsDeleted = false;
    bool oldIsNative = false;
    bool newIsNative = false;
    bool operator==(const ConflictDataTest &comparedConflictDataTest) const
    {
        if (this->type == comparedConflictDataTest.type &&
            this->key == comparedConflictDataTest.key &&
            this->oldValue == comparedConflictDataTest.oldValue &&
            this->newValue == comparedConflictDataTest.newValue &&
            this->oldIsDeleted == comparedConflictDataTest.oldIsDeleted &&
            this->newIsDeleted == comparedConflictDataTest.newIsDeleted &&
            this->oldIsNative == comparedConflictDataTest.oldIsNative &&
            this->newIsNative == comparedConflictDataTest.newIsNative) {
            return true;
        }
        return false;
    }
};
static std::vector<ConflictDataTest> g_conflictData;

static void ConflictCallback(const AppKvStoreConflictData &data)
{
    ZLOGD("start.");
    Key key;
    Value oldValue;
    Value newValue;

    data.GetKey(key);
    data.GetValue(OLD_VALUE_TYPE, oldValue);
    data.GetValue(NEW_VALUE_TYPE, newValue);
    bool oldIsDeleted = data.IsDeleted(OLD_VALUE_TYPE);
    bool newIsDeleted = data.IsDeleted(NEW_VALUE_TYPE);
    bool oldIsNative = data.IsNative(OLD_VALUE_TYPE);
    bool newIsNative = data.IsNative(NEW_VALUE_TYPE);
    g_conflictData.push_back({data.GetType(), key, oldValue, newValue, oldIsDeleted, newIsDeleted,
                              oldIsNative, newIsNative});

    ZLOGD("Get key: %s", key.ToString().c_str());
    ZLOGD("Get old value: %s", oldValue.ToString().c_str());
    ZLOGD("Get oldIsDeleted: %d", oldIsDeleted);
    ZLOGD("Get newIsDeleted: %d", newIsDeleted);
    ZLOGD("Get oldIsNative: %d", oldIsNative);
    ZLOGD("Get newIsNative: %d", newIsNative);
}

void AppConflictTest::SetUpTestCase(void)
{
}

void AppConflictTest::TearDownTestCase(void)
{
}

void AppConflictTest::SetUp(void)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;  // not supported yet.
    options.persistant = true;  // not supported yet.

    std::string appId = "odmf";  // define app name.
    std::string storeId = "conflictdb";  // define kvstore(database) name.
    std::string dataDir = "data/misc_ce/0/odmf_test";

    manager = AppDistributedKvDataManager::GetInstance(appId, dataDir);
    // [create and] open and initialize kvstore instance.
    statusGetKvStore = manager->GetKvStore(options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
        appKvStorePtr = std::move(appKvStore);
    });

    g_conflictData.clear();
}

void AppConflictTest::TearDown(void)
{
    Status statusRet = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(statusRet, Status::SUCCESS);
    statusRet = manager->DeleteKvStore("conflictdb");
    EXPECT_EQ(statusRet, Status::SUCCESS);
}

/**
  * @tc.name: AppConflict001
  * @tc.desc: set conflict resolution policy and no conflict
  * @tc.type: FUNC
  * @tc.require: AR000CQDUG
  * @tc.author: liuyuhui
  */
HWTEST_F(AppConflictTest, AppConflict001, TestSize.Level1)
{
    ZLOGD("AppConflict001");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, appKvStorePtr) << "appKvStorePtr is nullptr";

    Status status = appKvStorePtr->SetConflictResolutionPolicy(AppKvStoreConflictPolicyType::CONFLICT_NATIVE_ALL,
                                                               ConflictCallback);
    EXPECT_EQ(Status::SUCCESS, status) << "SetConflictResolutionPolicy return wrong status";
    WriteOptions localWrite;
    localWrite.local = false;
    status = appKvStorePtr->Put(localWrite, "Id1", "conflict");
    EXPECT_EQ(Status::SUCCESS, status) << "Put return wrong status";
    usleep(TIME_SLEEP);
    EXPECT_EQ(g_conflictData.empty(), true);
}

/**
  * @tc.name: AppConflict002
  * @tc.desc: set conflict resolution policy and exist conflict -put the same key
  * @tc.type: FUNC
  * @tc.require: AR000CQDUG
  * @tc.author: liuyuhui
  */
HWTEST_F(AppConflictTest, AppConflict002, TestSize.Level1)
{
    ZLOGD("AppConflict002");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, appKvStorePtr) << "appKvStorePtr is nullptr";

    WriteOptions localWrite;
    localWrite.local = false;
    Status status = appKvStorePtr->Put(localWrite, "Id2", "conflict");
    EXPECT_EQ(Status::SUCCESS, status) << "Put return wrong status";
    usleep(TIME_SLEEP);

    status = appKvStorePtr->SetConflictResolutionPolicy(AppKvStoreConflictPolicyType::CONFLICT_NATIVE_ALL,
                                                        ConflictCallback);
    EXPECT_EQ(Status::SUCCESS, status) << "SetConflictResolutionPolicy return wrong status";
    status = appKvStorePtr->Put(localWrite, "Id2", "conflict_modify");
    EXPECT_EQ(Status::SUCCESS, status) << "Put return wrong status";
    usleep(TIME_SLEEP);
    EXPECT_EQ(g_conflictData.empty(), false);
    ConflictDataTest expectData = {AppKvStoreConflictPolicyType::CONFLICT_NATIVE_ALL,
                                   "Id2", "conflict", "conflict_modify", false, false, true, true};
    EXPECT_EQ(g_conflictData.front() == expectData, true);
}

/**
  * @tc.name: AppConflict003
  * @tc.desc: set conflict resolution policy and exist conflict -put and delete the same key
  * @tc.type: FUNC
  * @tc.require: AR000CQDUG
  * @tc.author: liuyuhui
  */
HWTEST_F(AppConflictTest, AppConflict003, TestSize.Level1)
{
    ZLOGD("app_conflict_003 start");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, appKvStorePtr) << "appKvStorePtr is nullptr";

    WriteOptions localWrite;
    localWrite.local = false;
    Status status = appKvStorePtr->Put(localWrite, "Id3", "conflict");
    EXPECT_EQ(Status::SUCCESS, status) << "Put return wrong status";
    usleep(TIME_SLEEP);

    status = appKvStorePtr->SetConflictResolutionPolicy(AppKvStoreConflictPolicyType::CONFLICT_NATIVE_ALL,
                                                        ConflictCallback);
    EXPECT_EQ(Status::SUCCESS, status) << "SetConflictResolutionPolicy return wrong status";
    status = appKvStorePtr->Delete(localWrite, "Id3");
    EXPECT_EQ(Status::SUCCESS, status) << "Delete return wrong status";
    usleep(TIME_SLEEP);
    EXPECT_EQ(g_conflictData.empty(), false);
    Value newValue;
    ConflictDataTest expectData = {AppKvStoreConflictPolicyType::CONFLICT_NATIVE_ALL,
                                   "Id3", "conflict", newValue, false, true, true, true};
    EXPECT_EQ(g_conflictData.front() == expectData, true);
    ZLOGD("app_conflict_003 end");
}
