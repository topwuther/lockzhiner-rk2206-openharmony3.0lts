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

#include <dirent.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <thread>
#include <vector>
#include "kvstore_account_observer.h"
#include "kvstore_observer_client.h"
#include "kvstore_data_service.h"
#include "kvstore_impl.h"
#include "refbase.h"
#include "types.h"
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "directory_ex.h"
#include "constant.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "common_event_define.h"
#include "common_event_manager.h"
#include "ohos/aafwk/content/intent.h"
#include "ohos_account_kits.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;
using namespace OHOS;
using namespace Notification;

static const int SYSTEM_USER_ID = 1000;

static const int WAIT_TIME_FOR_ACCOUNT_OPERATION = 2; // indicates the wait time in seconds

class DistributedDataAccountEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void ChangeUser(int uid);
    static void TearDownTestCase();
    static void HarmonyAccountLogin();
    static void HarmonyAccountLogout();
    static void HarmonyAccountDelete();
};

void DistributedDataAccountEventTest::SetUpTestCase()
{
    DistributedDataAccountEventTest::ChangeUser(SYSTEM_USER_ID);
}

void DistributedDataAccountEventTest::TearDownTestCase()
{
    DistributedDataAccountEventTest::HarmonyAccountDelete();
}

void DistributedDataAccountEventTest::HarmonyAccountLogin()
{
    sptr<AAFwk::Intent> intent = new AAFwk::Intent();
    intent->SetAction(CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    sptr<CommonEventData> event = new CommonEventData(intent);
    sptr<CommonEventPublishInfo> publishInfo = new CommonEventPublishInfo();
    auto err = CommonEventManager::GetInstance().PublishCommonEventData(event, publishInfo, nullptr);
    EXPECT_EQ(ERR_OK, err);
    sleep(WAIT_TIME_FOR_ACCOUNT_OPERATION);
}

void DistributedDataAccountEventTest::HarmonyAccountLogout()
{
    sptr<AAFwk::Intent> intent = new AAFwk::Intent();
    intent->SetAction(CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    sptr<CommonEventData> event = new CommonEventData(intent);
    sptr<CommonEventPublishInfo> publishInfo = new CommonEventPublishInfo();
    auto err = CommonEventManager::GetInstance().PublishCommonEventData(event, publishInfo, nullptr);
    EXPECT_EQ(ERR_OK, err);
    sleep(WAIT_TIME_FOR_ACCOUNT_OPERATION);
}

void DistributedDataAccountEventTest::HarmonyAccountDelete()
{
    sptr<AAFwk::Intent> intent = new AAFwk::Intent();
    intent->SetAction(CommonEventSupport::COMMON_EVENT_HWID_TOKEN_INVALID);
    sptr<CommonEventData> event = new CommonEventData(intent);
    sptr<CommonEventPublishInfo> publishInfo = new CommonEventPublishInfo();
    auto err = CommonEventManager::GetInstance().PublishCommonEventData(event, publishInfo, nullptr);
    EXPECT_EQ(ERR_OK, err);
    sleep(WAIT_TIME_FOR_ACCOUNT_OPERATION);
}

void DistributedDataAccountEventTest::ChangeUser(int uid)
{
    if (setgid(uid)) {
        std::cout << "error to set gid " << uid << "errno is " << errno << std::endl;
    }

    if (setuid(uid)) {
        std::cout << "error to set uid " << uid << "errno is " << errno << std::endl;
    }
}

/**
 * @tc.name: KvStore data storage path verify when get KvStore with default device account and harmony account logout.
 * @tc.desc: Verify that the KvStore data storage path is consistent with the path spliced by distributedDB interface.
 * with default device account and harmony account logout.
 * @tc.type: FUNC
 * @tc.require: SR000DOH0F AR000DPSE5
 * @tc.author: FengLin
 */
HWTEST_F(DistributedDataAccountEventTest, GetKvStore_DefaultDeviceAccount_001, TestSize.Level3)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    AppId appId;
    appId.appId = "com.ohos.distributeddata.accountmsttest";
    StoreId storeId;
    storeId.storeId = "AccountEventStore001";
    std::string hashedStoreId;

    // Step1. Splice kvStore data storage expected path by distributedDB hash interface and clear directory.
    DistributedDB::DBStatus dbStatus =
            DistributedDB::KvStoreDelegateManager::GetDatabaseDir(storeId.storeId, hashedStoreId);
    EXPECT_EQ(dbStatus, DistributedDB::OK) << "Get data directory name from DB failed.";
    std::string appDataStoragePath;
    appDataStoragePath = KvStoreAppManager::GetDataStoragePath("0", appId.appId, KvStoreAppManager::PATH_CE);
    const std::string kvStoreDataStorageExpectPath = Constant::Concatenate(
        { appDataStoragePath, "/", appId.appId, "/", hashedStoreId });
    DIR *dir = opendir(kvStoreDataStorageExpectPath.c_str());
    if (dir != nullptr) {
        ForceRemoveDirectory(kvStoreDataStorageExpectPath);
        closedir(dir);
    }
    dir = opendir(kvStoreDataStorageExpectPath.c_str());
    EXPECT_EQ(dir, nullptr) << "KvStore data storage directory was not cleared successfully.";

    // Step2. Get KvStore and check the created data storage path is consistent with the expected directory.
    KvStoreDataService kvStoreDataService;
    sptr<IKvStoreImpl> iKvStoreImplPtr;
    Status status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                       [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplPtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(iKvStoreImplPtr, nullptr) << "GetKvStore executed failed";
    dir = opendir(kvStoreDataStorageExpectPath.c_str());
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_NE(dir, nullptr) << "KvStore data storage directory created is not consistent with the expected.";
    closedir(dir);
}

/**
 * @tc.name: Re-operate exist KvStore successfully verify when harmony account login/logout.
 * @tc.desc: Verify that after harmony account login/logout, re-operate exist KvStore successfully.
 * @tc.type: FUNC
 * @tc.require: SR000DOH0F AR000DPSE7
 * @tc.author: FengLin
 */
HWTEST_F(DistributedDataAccountEventTest, GetKvStore_DefaultDeviceAccount_002, TestSize.Level3)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    AppId appId;
    appId.appId = "com.ohos.distributeddata.accountmsttest";
    StoreId storeId;
    storeId.storeId = "AccountEventStore002";
    std::string hashedStoreId;

    // Step1. Get KvStore with specified appId and storeId when harmony account logout.
    KvStoreDataService kvStoreDataService;
    sptr<IKvStoreImpl> iKvStoreImplPtr;
    Status status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                       [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplPtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(iKvStoreImplPtr, nullptr) << "GetKvStore executed failed";

    Key key = "Von";
    Value value = "Leon";
    status = iKvStoreImplPtr->Put(key, value);
    EXPECT_EQ(status, Status::SUCCESS) << "PUT string to KvStore return wrong status";

    // Step2. Get KvStoreSnapshot and verify get value from KvStore correctly.
    sptr<KvStoreObserverClient> kvStoreObserverClient =
        new KvStoreObserverClient(storeId, SubscribeType::SUBSCRIBE_TYPE_ALL, nullptr, KvStoreType::MULTI_VERSION);
    sptr<IKvStoreSnapshotImpl> snapshotProxyTmp;
    auto snapshotCallbackFunction = [&](Status statusTmp, sptr<IKvStoreSnapshotImpl> snapshotProxy) {
        status = statusTmp;
        snapshotProxyTmp = snapshotProxy;
    };
    iKvStoreImplPtr->GetKvStoreSnapshot(kvStoreObserverClient, snapshotCallbackFunction);
    Value getValue = "";
    snapshotProxyTmp->Get(key, getValue);
    EXPECT_EQ(getValue, value) << "Get string from KvStore not equal to PUT";

    sleep(WAIT_TIME_FOR_ACCOUNT_OPERATION);
    // Step3. Harmony account login
    DistributedDataAccountEventTest::HarmonyAccountLogin();

    // Step4. Verify get value from KvStore and put string to KvStore with last KvStoreSnapShot correctly after LOGIN.
    getValue.Clear();
    snapshotProxyTmp->Get(key, getValue);
    EXPECT_EQ(getValue, value) << "Get string from KvStore not equal to PUT after LOGIN";

    Key key2 = "Von2";
    Value value2 = "Leon2";
    status = iKvStoreImplPtr->Put(key2, value2);
    EXPECT_EQ(status, Status::SUCCESS) << "PUT string to KvStore return wrong status";

    sptr<KvStoreObserverClient> kvStoreObserverClient1 =
            new KvStoreObserverClient(storeId, SubscribeType::SUBSCRIBE_TYPE_ALL, nullptr, KvStoreType::MULTI_VERSION);
    sptr<IKvStoreSnapshotImpl> snapshotProxyTmp1;
    auto snapshotCallbackFunction1 = [&](Status statusTmp, sptr<IKvStoreSnapshotImpl> snapshotProxy) {
        status = statusTmp;
        snapshotProxyTmp1 = snapshotProxy;
    };
    iKvStoreImplPtr->GetKvStoreSnapshot(kvStoreObserverClient1, snapshotCallbackFunction1);

    getValue.Clear();
    snapshotProxyTmp1->Get(key2, getValue);
    EXPECT_EQ(getValue, value2) << "Get string from KvStore not equal to PUT after LOGIN";

    // Step6. Harmony account logout
    DistributedDataAccountEventTest::HarmonyAccountLogout();

    // Step7. Verify get value from KvStore and put string to KvStore with last KvStoreSnapShot correctly after LOGOUT.
    getValue.Clear();
    snapshotProxyTmp->Get(key, getValue);
    EXPECT_EQ(getValue, value) << ", Get string from KvStore not equal to PUT after LOGOUT";

    Key key3 = "Von3";
    Value value3 = "Leon3";
    status = iKvStoreImplPtr->Put(key3, value3);
    EXPECT_EQ(status, Status::SUCCESS) << "PUT string to KvStore return wrong status";

    sptr<KvStoreObserverClient> kvStoreObserverClient2 =
        new KvStoreObserverClient(storeId, SubscribeType::SUBSCRIBE_TYPE_ALL, nullptr, KvStoreType::MULTI_VERSION);
    sptr<IKvStoreSnapshotImpl> snapshotProxyTmp2;
    auto snapshotCallbackFunction2 = [&](Status statusTmp, sptr<IKvStoreSnapshotImpl> snapshotProxy) {
        status = statusTmp;
        snapshotProxyTmp2 = snapshotProxy;
    };
    iKvStoreImplPtr->GetKvStoreSnapshot(kvStoreObserverClient2, snapshotCallbackFunction2);
    getValue.Clear();
    snapshotProxyTmp2->Get(key3, getValue);
    EXPECT_EQ(getValue, value3) << "Get string from KvStore not equal to PUT after LOGOUT";

    iKvStoreImplPtr->ReleaseKvStoreSnapshot(snapshotProxyTmp);
    iKvStoreImplPtr->ReleaseKvStoreSnapshot(snapshotProxyTmp1);
    iKvStoreImplPtr->ReleaseKvStoreSnapshot(snapshotProxyTmp2);
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_EQ(status, Status::SUCCESS) << "CloseKvStore return wrong status after harmony account logout";

    // Step7. Verify that when harmony account logout and in the situation that the exist KvStore has been closed,
    // re-get exist KvStore successfully and the KvStoreImplPtr not equal with the one before harmony account logout.
    sptr<IKvStoreImpl> iKvStoreImplLogoutPtr;
    status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplLogoutPtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status after harmony account logout";
    EXPECT_NE(iKvStoreImplLogoutPtr, nullptr) << "GetKvStore executed failed after harmony account logout";
    EXPECT_NE(iKvStoreImplPtr, iKvStoreImplLogoutPtr) << "kvStoreImpl NE fail after harmony account logout";
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_EQ(status, Status::SUCCESS) << "CloseKvStore return wrong status after harmony account logout";
}

/**
 * @tc.name: KvStore data storage path verify when the ownership of distributed data set to ACCOUNT.
 * @tc.desc: Verify that in the situation that distributed data ownership set to ACCOUNT, get KvStore successfully
 * and verify that the KvStore data storage path is consistent with the path spliced by distributedDB interface.
 * @tc.type: FUNC
 * @tc.require: SR000DOH0F AR000DPTQ8
 * @tc.author: FengLin
 */
HWTEST_F(DistributedDataAccountEventTest, GetKvStore_DefaultDeviceAccount_003, TestSize.Level3)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    AppId appId;
    appId.appId = "com.ohos.distributeddata.accountmsttest";
    StoreId storeId;
    storeId.storeId = "AccountEventStore003";
    std::string hashedStoreId;
    KvStoreDataService kvStoreDataService;

    // Step1. Set distributed data ownership to ACCOUNT.
    options.dataOwnership = false;

    // Step2. Splice kvStore data storage expected path by distributedDB hash interface and clear directory.
    DistributedDB::DBStatus dbStatus = DistributedDB::KvStoreDelegateManager::GetDatabaseDir(
        storeId.storeId, appId.appId, AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId(), hashedStoreId);
    EXPECT_EQ(dbStatus, DistributedDB::OK) << "Get data directory name from DB failed.";

    std::string appDataStoragePath;
    appDataStoragePath = KvStoreAppManager::GetDataStoragePath("0", appId.appId, KvStoreAppManager::PATH_CE);
    const std::string kvStoreDataStorageExpectPath = Constant::Concatenate(
        { appDataStoragePath, "/", appId.appId, "/", hashedStoreId });
    DIR *dir = opendir(kvStoreDataStorageExpectPath.c_str());
    if (dir != nullptr) {
        ForceRemoveDirectory(kvStoreDataStorageExpectPath);
        closedir(dir);
    }
    dir = opendir(kvStoreDataStorageExpectPath.c_str());
    EXPECT_EQ(dir, nullptr) << "KvStore data storage directory was not cleared successfully.";

    // Step2. Get KvStore and check the created data storage path is consistent with the expected directory.
    sptr<IKvStoreImpl> iKvStoreImplPtr;
    Status status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                       [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplPtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(iKvStoreImplPtr, nullptr) << "GetKvStore executed failed";
    dir = opendir(kvStoreDataStorageExpectPath.c_str());
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_NE(dir, nullptr) << "KvStore data storage directory created is not consistent with the expected.";
    closedir(dir);
}

/**
 * @tc.name: System upgrade kvStore data migration verify when the data ownership changed from ACCUNT to DEVICE.
 * @tc.desc: Verify that in the situation that distributed data ownership set to ACCOUNT, get KvStore successfully
 * then set data ownership set to DEVICE, harmony account login, re-get kvStore successfully and verify that the
 * KvStore data storage path changed to the path spliced by distributedDB interface.
 * @tc.type: FUNC
 * @tc.require: SR000DOH0F AR000DPSDH
 * @tc.author: FengLin
 */
HWTEST_F(DistributedDataAccountEventTest, GetKvStore_DefaultDeviceAccount_004, TestSize.Level3)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    AppId appId;
    appId.appId = "com.ohos.distributeddata.accountmsttest";
    StoreId storeId;
    storeId.storeId = "AccountEventStore004";
    std::string hashedStoreId;
    KvStoreDataService kvStoreDataService;

    // Step1. Set distributed data ownership to ACCOUNT.
    options.dataOwnership = false;

    // Step2. Splice kvStore data storage expected path by distributedDB hash interface and clear directory.
    DistributedDB::DBStatus dbStatus = DistributedDB::KvStoreDelegateManager::GetDatabaseDir(
            storeId.storeId, appId.appId, AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId(), hashedStoreId);
    EXPECT_EQ(dbStatus, DistributedDB::OK) << "Get data directory name from DB failed.";

    std::string appDataStoragePath;
    appDataStoragePath = KvStoreAppManager::GetDataStoragePath("0", appId.appId, KvStoreAppManager::PATH_CE);
    std::string kvStoreDataStorageExpectPath = Constant::Concatenate(
        { appDataStoragePath, "/", appId.appId, "/", hashedStoreId });
    DIR *dir = opendir(kvStoreDataStorageExpectPath.c_str());
    if (dir != nullptr) {
        ForceRemoveDirectory(kvStoreDataStorageExpectPath);
        closedir(dir);
    }
    dir = opendir(kvStoreDataStorageExpectPath.c_str());
    EXPECT_EQ(dir, nullptr) << "KvStore data storage directory was not cleared successfully.";

    // Step2. Get KvStore.
    sptr<IKvStoreImpl> iKvStoreImplPtr;
    Status status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                       [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplPtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(iKvStoreImplPtr, nullptr) << "GetKvStore executed failed";
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_EQ(status, Status::SUCCESS) << "CloseKvStore return wrong status";

    // Step3. Set distributed data ownership to DEVICE.
    options.dataOwnership = true;

    // Step4. Harmony account login
    DistributedDataAccountEventTest::HarmonyAccountLogin();

    // Step5. Re-Get KvStore and verify the data storage path has been changed to only store id hashed path.
    sptr<IKvStoreImpl> iKvStoreImplRePtr;
    status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                       [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplRePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status when harmony account login";
    EXPECT_NE(iKvStoreImplRePtr, nullptr) << "GetKvStore executed failed when harmony account login";
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_EQ(status, Status::SUCCESS) << "CloseKvStore return wrong status when harmony account login";

    hashedStoreId.clear();
    dbStatus = DistributedDB::KvStoreDelegateManager::GetDatabaseDir(storeId.storeId, hashedStoreId);
    EXPECT_EQ(dbStatus, DistributedDB::OK) << "Get data directory name from DB failed.";
    kvStoreDataStorageExpectPath.clear();
    kvStoreDataStorageExpectPath = Constant::Concatenate({ appDataStoragePath, "/", appId.appId, "/", hashedStoreId });
    dir = opendir(kvStoreDataStorageExpectPath.c_str());
    EXPECT_NE(dir, nullptr) << "KvStore data storage directory was not changed correctly.";
    closedir(dir);
}

/**
 * @tc.name: APP get KvStore failed verify when DDS is processing harmony account login event.
 * @tc.desc: Verify that in the situation that DDS is processing harmony account login event,
 * APP will get KvStore failed.
 * @tc.type: FUNC
 * @tc.require: SR000DOH0F AR000DPSE6
 * @tc.author: FengLin
 */
HWTEST_F(DistributedDataAccountEventTest, GetKvStore_DefaultDeviceAccount_005, TestSize.Level3)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    AppId appId;
    appId.appId = "com.ohos.distributeddata.accountmsttest";
    StoreId storeId;
    storeId.storeId = "AccountEventStore005";
    std::string hashedStoreId;
    KvStoreDataService kvStoreDataService;

    // Step1. Harmony account login.
    DistributedDataAccountEventTest::HarmonyAccountLogin();

    // Step2. Set DDS status to processing harmony account login event.
    g_kvStoreAccountEventStatus = 1;

    // Step3. Get KvStore.
    sptr<IKvStoreImpl> iKvStoreImplPtr;
    Status status = kvStoreDataService.GetKvStore(options, appId, storeId,
                                       [&](sptr<IKvStoreImpl> kvStore) { iKvStoreImplPtr = std::move(kvStore); });
    // Caution: When the function opened, this verify should be EQ
    EXPECT_NE(status, Status::SYSTEM_ACCOUNT_EVENT_PROCESSING) << "GetKvStore return unexpectedstatus";
    EXPECT_NE(iKvStoreImplPtr, nullptr) << "GetKvStore executed with unexpectedresult";

    // Step4. Restore DDS status to finish harmony account login event process.
    g_kvStoreAccountEventStatus = 0;
}

/**
 * @tc.name: Re-get exist SingleKvStore successfully verify when harmony account login/logout.
 * @tc.desc: Verify that after harmony account login/logout, re-operate exist SingleKvStore successfully.
 * @tc.type: FUNC
 * @tc.require: SR000DOH0F AR000DPSE7
 * @tc.author: FengLin
 */
HWTEST_F(DistributedDataAccountEventTest, GetKvStore_DefaultDeviceAccount_006, TestSize.Level3)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;

    AppId appId;
    appId.appId = "com.ohos.distributeddata.accountmsttest";
    StoreId storeId;
    storeId.storeId = "AccountEventStore006";
    std::string hashedStoreId;

    // Step1. Get KvStore with specified appId and storeId when harmony account logout.
    KvStoreDataService kvStoreDataService;
    sptr<ISingleKvStore> iSingleKvStorePtr;
    Status status = kvStoreDataService.GetSingleKvStore(options, appId, storeId,
                    [&](sptr<ISingleKvStore> singleKvStore) { iSingleKvStorePtr = std::move(singleKvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetSingleKvStore return wrong status";
    EXPECT_NE(iSingleKvStorePtr, nullptr) << "GetSingleKvStore executed failed";

    Key key = "Von";
    Value value = "Leon";
    status = iSingleKvStorePtr->Put(key, value);
    EXPECT_EQ(status, Status::SUCCESS) << "PUT string to SingleKvStore return wrong status";
    Value getValue = "";
    iSingleKvStorePtr->Get(key, getValue);
    EXPECT_EQ(getValue, value) << "Get string from SingleKvStore not equal to PUT";

    // Step2. Get ResultSet and verify get entry from SingleKvStore correctly.
    sptr<IKvStoreResultSet> iKvStoreResultSet;
    auto resultSetCallbackFunction = [&](Status statusTmp, sptr<IKvStoreResultSet> iKvStoreResultSetTmp) {
        status = statusTmp;
        iKvStoreResultSet = iKvStoreResultSetTmp;
    };
    Key prefixKey = "Von";
    Entry entry;
    iSingleKvStorePtr->GetResultSet(prefixKey, resultSetCallbackFunction);
    EXPECT_EQ(status, Status::SUCCESS) << "Get resultset from SingleKvStore return wrong status";
    iKvStoreResultSet->MoveToNext();
    status = iKvStoreResultSet->GetEntry(entry);
    EXPECT_EQ(status, Status::SUCCESS) << "Get entry from ResultSet return wrong status";
    EXPECT_EQ(entry.key, key) << "Get entry key from SingleKvStore not equal to PUT";
    EXPECT_EQ(entry.value, value) << "Get entry value from SingleKvStore not equal to PUT";

    sleep(WAIT_TIME_FOR_ACCOUNT_OPERATION);
    // Step3. Harmony account login
    DistributedDataAccountEventTest::HarmonyAccountLogin();

    // Step4. Verify get value from SingleKvStore and put to SingleKvStore with last resultset correctly after LOGIN.
    getValue.Clear();
    iSingleKvStorePtr->Get(key, getValue);
    EXPECT_EQ(getValue, value) << "Get string from SingleKvStore not equal to PUT";

    Entry entry2;
    iKvStoreResultSet->GetEntry(entry2);
    EXPECT_EQ(entry2.key, key) << "Get entry key from SingleKvStore not equal to PUT";
    EXPECT_EQ(entry2.value, value) << "Get entry value from SingleKvStore not equal to PUT";

    Key key2 = "Von2";
    Value value2 = "Leon2";
    status = iSingleKvStorePtr->Put(key2, value2);
    EXPECT_EQ(status, Status::SUCCESS) << "PUT string to SingleKvStore return wrong status";
    getValue.Clear();
    iSingleKvStorePtr->Get(key2, getValue);
    EXPECT_EQ(getValue, value2) << "Get string from SingleKvStore not equal to PUT after LOGIN";

    Entry entry3;
    iSingleKvStorePtr->CloseResultSet(iKvStoreResultSet);
    iSingleKvStorePtr->GetResultSet(prefixKey, resultSetCallbackFunction);
    EXPECT_EQ(status, Status::SUCCESS) << "Get resultset from SingleKvStore return wrong status";
    iKvStoreResultSet->MoveToNext();
    status = iKvStoreResultSet->GetEntry(entry3);
    EXPECT_EQ(status, Status::SUCCESS) << "Get entry from SingleKvStore return wrong status";
    EXPECT_EQ(entry3.key, key) << "Get entry key from SingleKvStore not equal to PUT";
    EXPECT_EQ(entry3.value, value) << "Get entry value from SingleKvStore not equal to PUT";

    // Step6. Harmony account logout
    DistributedDataAccountEventTest::HarmonyAccountLogout();

    // Step7. Verify get value from SingleKvStore and put to SingleKvStore with last resultset correctly after LOGOUT.
    getValue.Clear();
    iSingleKvStorePtr->Get(key, getValue);
    EXPECT_EQ(getValue, value) << "Get string from SingleKvStore not equal to PUT";

    Entry entry4;
    iKvStoreResultSet->GetEntry(entry4);
    EXPECT_EQ(entry4.key, key) << "Get entry key from SingleKvStore not equal to PUT";
    EXPECT_EQ(entry4.value, value) << "Get entry value from SingleKvStore not equal to PUT";

    Key key3 = "Von3";
    Value value3 = "Leon3";
    status = iSingleKvStorePtr->Put(key3, value3);
    EXPECT_EQ(status, Status::SUCCESS) << "PUT string to SingleKvStore return wrong status";
    getValue.Clear();
    iSingleKvStorePtr->Get(key3, getValue);
    EXPECT_EQ(getValue, value3) << "Get string from SingleKvStore not equal to PUT after LOGIN";

    Entry entry5;
    iSingleKvStorePtr->GetResultSet(prefixKey, resultSetCallbackFunction);
    iKvStoreResultSet->MoveToNext();
    iKvStoreResultSet->GetEntry(entry5);
    EXPECT_EQ(entry5.key, key) << "Get entry key from SingleKvStore not equal to PUT";
    EXPECT_EQ(entry5.value, value) << "Get entry value from SingleKvStore not equal to PUT";

    iSingleKvStorePtr->CloseResultSet(iKvStoreResultSet);
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_EQ(status, Status::SUCCESS) << "CloseKvStore return wrong status after harmony account logout";

    // Step8. Verify that when harmony account logout and in the situation that the exist KvStore has been closed,
    // re-get exist KvStore successfully and the iSingleKvStorePtr not equal with the one before harmony account logout.
    sptr<ISingleKvStore> iSingleKvStoreLogoutPtr;
    status = kvStoreDataService.GetSingleKvStore(options, appId, storeId,
             [&](sptr<ISingleKvStore> singleKvStore) { iSingleKvStoreLogoutPtr = std::move(singleKvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetSingleKvStore return wrong status after harmony account logout";
    EXPECT_NE(iSingleKvStoreLogoutPtr, nullptr) << "GetSingleKvStore executed failed after harmony account logout";
    EXPECT_NE(iSingleKvStorePtr, iSingleKvStoreLogoutPtr) << "iSingleKvStorePtr NE fail after harmony account logout";
    status = kvStoreDataService.CloseKvStore(appId, storeId);
    EXPECT_EQ(status, Status::SUCCESS) << "CloseKvStore return wrong status after harmony account logout";
}