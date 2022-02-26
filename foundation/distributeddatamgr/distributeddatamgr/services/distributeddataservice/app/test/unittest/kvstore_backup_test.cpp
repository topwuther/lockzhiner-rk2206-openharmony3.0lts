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
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>

#include "kvstore_impl.h"
#include "backup_handler.h"
#include "kv_scheduler.h"
#include "kvstore_data_service.h"
#include "kvstore_meta_manager.h"
#include "kvstore_utils.h"
#include "log_print.h"
#include "single_kvstore_impl.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;
using namespace OHOS;

class KvStoreBackupTest : public testing::Test {
public:
    static constexpr unsigned int DEFAULT_DIR_MODE = 0755;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KvStoreBackupTest::SetUpTestCase(void)
{}

void KvStoreBackupTest::TearDownTestCase(void)
{}

void KvStoreBackupTest::SetUp(void)
{
    const std::string backupDir = "/data/misc_ce/0/mdds/0/default/backup";

    unlink(backupDir.c_str());
    mkdir(backupDir.c_str(), KvStoreBackupTest::DEFAULT_DIR_MODE);
}

void KvStoreBackupTest::TearDown(void)
{}

/**
* @tc.name: KvStoreBackupTest001
* @tc.desc: set option backup
* @tc.type: FUNC
* @tc.require: SR000DR9J0 AR000DR9J1
* @tc.author: guodaoxin
*/
HWTEST_F(KvStoreBackupTest, KvStoreBackupTest001, TestSize.Level1)
{
    Options options = { .createIfMissing = true, .encrypt = false, .autoSync = true, .backup = true,
                        .kvStoreType = KvStoreType::SINGLE_VERSION, .dataOwnership = true };
    AppId appId = { "backup1" };
    StoreId storeId = { "store1" };
    KvStoreDataService kvDataService;
    kvDataService.DeleteKvStore(appId, storeId);
    sptr<ISingleKvStore> kvStorePtr;
    Status status = kvDataService.GetSingleKvStore(options, appId, storeId,
                                               [&](sptr<ISingleKvStore> kvStore) { kvStorePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "KvStoreBackupTest001 set backup true failed";
    kvDataService.CloseKvStore(appId, storeId);
}

/**
* @tc.name: KvStoreBackupTest002
* @tc.desc: kvstore backup test for single db
* @tc.type: FUNC
* @tc.require: SR000DR9J0 AR000DR9JM
* @tc.author: guodaoxin
*/
HWTEST_F(KvStoreBackupTest, KvStoreBackupTest002, TestSize.Level1)
{
    Options options = { .createIfMissing = true, .encrypt = false, .autoSync = true, .backup = true,
                       .kvStoreType = KvStoreType::SINGLE_VERSION, .dataOwnership = true };
    AppId appId = { "backup2" };
    StoreId storeId = { "store2" };

    KvStoreDataService kvDataService;
    kvDataService.DeleteKvStore(appId, storeId);
    sptr<ISingleKvStore> kvStorePtr;
    Status status = kvDataService.GetSingleKvStore(options, appId, storeId,
        [&](sptr<ISingleKvStore> kvStore) { kvStorePtr = std::move(kvStore);});

    EXPECT_EQ(status, Status::SUCCESS) << "KvStoreBackupTest002 set backup true failed";

    Key key1("test1_key");
    Value value1("test1_value");
    kvStorePtr->Put(key1, value1);
    Key key2("test2_key");
    Value value2("test2_value");
    kvStorePtr->Put(key2, value2);

    auto backupHandler = std::make_unique<BackupHandler>();
    auto trueAppId = KvStoreUtils::GetAppIdByBundleName(appId.appId);
    MetaData metaData{0};
    metaData.kvStoreMetaData.deviceAccountId = "0";
    metaData.kvStoreMetaData.userId = AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId();
    metaData.kvStoreMetaData.appId = trueAppId;
    metaData.kvStoreMetaData.storeId = storeId.storeId;
    metaData.kvStoreMetaData.isBackup = false;
    metaData.kvStoreType = KvStoreType::SINGLE_VERSION;

    backupHandler->SingleKvStoreBackup(metaData);

    kvStorePtr->Delete(key2);
    Value value22;
    kvStorePtr->Get(key2, value22);

    auto kptr = static_cast<SingleKvStoreImpl *>(kvStorePtr.GetRefPtr());
    kptr->Import(appId.appId);
    kvStorePtr->Get(key2, value22);

    kvDataService.CloseKvStore(appId, storeId);
}

/**
* @tc.name: KvStoreBackupTest003
* @tc.desc: kvstore backup test for multi db
* @tc.type: FUNC
* @tc.require: AR000DR9JM AR000D08K5
* @tc.author: guodaoxin
*/
HWTEST_F(KvStoreBackupTest, KvStoreBackupTest003, TestSize.Level1)
{
    Options options = { .createIfMissing = true, .encrypt = false, .autoSync = true, .backup = true,
            .kvStoreType = KvStoreType::SINGLE_VERSION, .dataOwnership = true };
    AppId appId = { "backup3" };
    StoreId storeId = { "store3" };

    KvStoreDataService kvDataService;
    kvDataService.DeleteKvStore(appId, storeId);
    sptr<IKvStoreImpl> kvStorePtr;
    Status status = kvDataService.GetKvStore(options, appId, storeId,
                                                   [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore);});

    EXPECT_EQ(status, Status::SUCCESS) << "KvStoreBackupTest003 set backup true failed";

    Key key1("test1_key");
    Value value1("test1_value");
    kvStorePtr->Put(key1, value1);
    Key key2("test2_key");
    Value value2("test2_value");
    kvStorePtr->Put(key2, value2);

    auto backupHandler = std::make_unique<BackupHandler>();
    auto trueAppId = KvStoreUtils::GetAppIdByBundleName(appId.appId);

    MetaData metaData{0};
    metaData.kvStoreMetaData.deviceAccountId = "0";
    metaData.kvStoreMetaData.userId = AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId();
    metaData.kvStoreMetaData.appId = trueAppId;
    metaData.kvStoreMetaData.storeId = storeId.storeId;
    metaData.kvStoreMetaData.isBackup = false;
    metaData.kvStoreType = KvStoreType::MULTI_VERSION;

    backupHandler->MultiKvStoreBackup(metaData);

    kvStorePtr->Delete(key2);

    auto kptr = static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr());
    kptr->Import(appId.appId);

    sptr<IKvStoreSnapshotImpl> kvStoreSnapshotPtr;
    kvStorePtr->GetKvStoreSnapshot(nullptr,
                                   [&](Status status, sptr<IKvStoreSnapshotImpl> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "KvStoreBackupTest003, kvStoreSnapshotPtr is nullptr";

    Value value22;
    kvStoreSnapshotPtr->Get(key2, value22);

    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
    kvDataService.CloseKvStore(appId, storeId);
}
