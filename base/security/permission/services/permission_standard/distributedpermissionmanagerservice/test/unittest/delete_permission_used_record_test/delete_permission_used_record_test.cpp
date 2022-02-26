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

#include <thread>
#include <functional>

#include "gtest/gtest.h"
#include "distributed_permission_manager_service.h"
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Security::Permission;

namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
}  // namespace OHOS

class DeletePermissionUsedRecordTest : public testing::Test {
public:
    std::vector<GenericValues> visitorValues;
    std::vector<GenericValues> recordValues;
    DistributedPermissionManagerService managerService;
    void InitVisitorData();
    void InitRecordData(int64_t timestamp);
    static void SetUpTestCase()
    {
        OHOS::sptr<OHOS::IRemoteObject> bundleObject = NULL;
        OHOS::sptr<OHOS::IRemoteObject> permissionObject = new PermissionManagerService();
        auto sysMgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sysMgr == NULL) {
            GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
            return;
        }

        sysMgr->AddSystemAbility(Constant::ServiceId::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
        sysMgr->AddSystemAbility(Constant::ServiceId::SUBSYS_SECURITY_PERMISSION_SYS_SERVICE_ID, permissionObject);
    }

    static void TearDownTestCase()
    {
        cout << "TearDownTestCase()" << endl;
    }
    void SetUp()
    {
        GenericValues null;
        DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_VISITOR, null);
        DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_RECORD, null);
        cout << "SetUp() is running" << endl;
    }
    void TearDown()
    {
        cout << "TearDown()" << endl;
    }
};

void AddPermissionUsedRecordTest::InitVisitorData()
{
    std::vector<GenericValues> visitor;
    GenericValues genericVisitor;

    genericVisitor.Put(FIELD_DEVICE_ID, "device_0");
    genericVisitor.Put(FIELD_DEVICE_NAME, "device_name_0");
    genericVisitor.Put(FIELD_BUNDLE_USER_ID, 0);
    genericVisitor.Put(FIELD_BUNDLE_NAME, "bundleName");
    genericVisitor.Put(FIELD_BUNDLE_LABEL, "bundleInfo.label");
    visitor.emplace_back(genericVisitor);

    GenericValues genericVisitor1;

    genericVisitor1.Put(FIELD_DEVICE_ID, "device_1");
    genericVisitor1.Put(FIELD_DEVICE_NAME, "device_name_1");
    genericVisitor1.Put(FIELD_BUNDLE_USER_ID, 0);
    genericVisitor1.Put(FIELD_BUNDLE_NAME, "bundleName1");
    genericVisitor1.Put(FIELD_BUNDLE_LABEL, "bundleInfo.label1");
    visitor.emplace_back(genericVisitor1);
    DataStorage::GetRealDataStorage().Add(DataStorage::PERMISSION_VISITOR, visitor);
}

void AddPermissionUsedRecordTest::InitRecordData(int64_t timestamp)
{
    int opCode = 4;
    std::vector<GenericValues> visitor;
    DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_VISITOR, visitor);
    int visitorId = 0;
    if (visitor.size() > 0) {
        visitorId = visitor[0].GetInt(FIELD_ID);
    }

    std::vector<GenericValues> record;
    GenericValues genericRecordFore;

    genericRecordFore.Put(FIELD_TIMESTAMP, timestamp);
    genericRecordFore.Put(FIELD_VISITOR_ID, visitorId);
    genericRecordFore.Put(FIELD_OP_CODE, opCode);
    genericRecordFore.Put(FIELD_IS_FOREGROUND, 1);
    genericRecordFore.Put(FIELD_ACCESS_COUNT, 1);
    genericRecordFore.Put(FIELD_REJECT_COUNT, 0);
    GenericValues genericRecordBack;

    genericRecordBack.Put(FIELD_TIMESTAMP, timestamp);
    genericRecordBack.Put(FIELD_VISITOR_ID, visitorId);
    genericRecordBack.Put(FIELD_OP_CODE, opCode);
    genericRecordBack.Put(FIELD_IS_FOREGROUND, 0);
    genericRecordBack.Put(FIELD_ACCESS_COUNT, 1);
    genericRecordBack.Put(FIELD_REJECT_COUNT, 0);
    record.emplace_back(genericRecordFore);
    record.emplace_back(genericRecordBack);
    DataStorage::GetRealDataStorage().Add(DataStorage::PERMISSION_RECORD, record);
}

/**
 * @tc.number: DeletePermissionUsedRecord_0100
 * @tc.name: test delete permission used record by uid
 */
HWTEST_F(AddPermissionUsedRecordTest, DeletePermissionUsedRecord_0100, Function | MediumTest | Level1)
{
    InitVisitorData();
    InitRecordData(TimeUtil::GetTimestamp());

    visitorValues.clear();
    recordValues.clear();
    int visitorSize = 0;
    int recordSize = 0;
    int visitorResult = DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_VISITOR, visitorValues);
    int recordResult = DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_RECORD, recordValues);
    EXPECT_EQ(visitorResult, 0);
    EXPECT_EQ(recordResult, 0);
    visitorSize = visitorValues.size();
    recordSize = recordValues.size();
    cout << "visitorSize = " << visitorSize;
    cout << "recordSize = " << recordSize;
    int32_t uid = 20;
    managerService.DeletePermissionUsedRecords(uid);
    visitorValues.clear();
    recordValues.clear();
    visitorResult = DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_VISITOR, visitorValues);
    recordResult = DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_RECORD, recordValues);
    EXPECT_EQ(visitorResult, 0);
    EXPECT_EQ(recordResult, 0);
    EXPECT_EQ(visitorSize - 1, (int)visitorValues.size());
    EXPECT_EQ(recordSize - 2, (int)recordValues.size());
}
