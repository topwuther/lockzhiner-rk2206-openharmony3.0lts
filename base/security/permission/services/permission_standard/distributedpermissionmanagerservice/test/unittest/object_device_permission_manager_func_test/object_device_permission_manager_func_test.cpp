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
#define private public
#include "distributed_permission_manager_service.h"
using namespace testing::ext;
namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
namespace Security {
namespace Permission {
namespace {}  // namespace
class ObjectDevicePermissionManagerFuncTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp()
    {
        service = DelayedSingleton<DistributedPermissionManagerService>::GetInstance();
        service->OnStart();
    }
    void TearDown()
    {
        service->OnStop();
    }
    std::shared_ptr<DistributedPermissionManagerService> service;
};
std::set<std::string> getRecoverPermissions(std::string deviceId, int32_t uid)
{
    return ObjectDevicePermissionRepository::GetInstance()
        .objectDevices_.at(deviceId)
        ->GetUidPermissions()
        .at(uid)
        ->GetGrabtedPermission();
}
void testRecoverFileNomal(std::string deviceId1, int32_t uid, std::string permission1, std::string permission2)
{
    std::set<std::string> recoverPermissions = getRecoverPermissions(deviceId1, uid);
    std::set<std::string>::iterator it;
    int32_t i = 0;
    for (auto it = recoverPermissions.begin(); it != recoverPermissions.end(); it++) {
        if (*it == permission1 || *it == permission2) {
            i++;
        }
    }
    EXPECT_TRUE(i == 2 && recoverPermissions.size() == 2);
    GTEST_LOG_(INFO) << "testRecoverFileNomal  is run";
}
/*
 * Feature: DPMS
 * Function: ObjectDevicePermissionManagerFuncTest
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_clear_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "func_test_clear_0100";
    std::string deviceId1 = "deviceId1";
    std::string deviceId2 = "deviceId2";
    std::string deviceId3 = "deviceId3";
    int32_t uid = 1;
    int32_t uid2 = 2;
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId1, uid);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId2, uid2);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId3, uid2);
    EXPECT_TRUE(ObjectDevicePermissionRepository::GetInstance().objectDevices_.size() == 3);
    ObjectDevicePermissionManager::GetInstance().Clear();
    EXPECT_TRUE(ObjectDevicePermissionRepository::GetInstance().objectDevices_.size() == 0);
    ObjectDevicePermissionRepository::GetInstance().RecoverFromFile();
    EXPECT_TRUE(ObjectDevicePermissionRepository::GetInstance().objectDevices_.size() == 0);
}
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_recover_from_file_0100, TestSize.Level1)
{
    std::string deviceId1 = "deviceId1";
    std::string deviceId2 = "deviceId2";
    std::string deviceId3 = "deviceId3";
    int32_t uid = 1;
    int32_t uid2 = 2;
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId1, uid);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId2, uid2);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId3, uid);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId3, uid2);
    std::set<std::string> permissions;
    std::string permission1(Constant::LOCATION);
    std::string permission2(Constant::CAMERA);
    permissions.insert(permission1);
    permissions.insert(permission2);
    std::set<std::string> permissions2;
    std::string permission3(Constant::LOCATION);
    std::string permission4(Constant::MICROPHONE);
    std::string permission5("permission5");
    permissions2.insert(permission2);
    permissions2.insert(permission3);
    permissions2.insert(permission4);
    permissions2.insert(permission5);
    ObjectDevicePermissionManager::GetInstance().ResetGrantSensitivePermission(deviceId1, uid, permissions);
    ObjectDevicePermissionManager::GetInstance().ResetGrantSensitivePermission(deviceId3, uid2, permissions2);
    ObjectDevicePermissionRepository::GetInstance().SaveToFile();
    ObjectDevicePermissionRepository::GetInstance().Clear();
    EXPECT_TRUE(ObjectDevicePermissionRepository::GetInstance().objectDevices_.size() == 0);
    ObjectDevicePermissionRepository::GetInstance().RecoverFromFile();
    EXPECT_TRUE(ObjectDevicePermissionRepository::GetInstance().objectDevices_.size() == 3);
    testRecoverFileNomal(deviceId1, uid, permission1, permission2);
    std::set<std::string> recoverPermissions2 = getRecoverPermissions(deviceId3, uid2);
    std::set<std::string>::iterator it2;
    int32_t j = 0;
    for (auto it2 = recoverPermissions2.begin(); it2 != recoverPermissions2.end(); it2++) {
        // permission5  is not SensitiveResource, save fail
        if (*it2 == permission2 || *it2 == permission3 || *it2 == permission4) {
            j++;
        }
    }
    EXPECT_TRUE(j == 3 && recoverPermissions2.size() == 3);
    // test multiple  uid
    std::map<int32_t, std::shared_ptr<ObjectUid>> multipleUidMap =
        ObjectDevicePermissionRepository::GetInstance().objectDevices_.at(deviceId3)->GetUidPermissions();
    std::map<int32_t, std::shared_ptr<ObjectUid>>::iterator it3;
    int32_t k = 0;
    for (auto it3 = multipleUidMap.begin(); it3 != multipleUidMap.end(); it3++) {
        // permission5  is not SensitiveResource, save fail
        if (it3->second->GetUid() == uid || it3->second->GetUid() == uid2) {
            k++;
        }
    }
    EXPECT_TRUE(k == 2 && multipleUidMap.size() == 2);
}
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_remove_notify_permission_monitor_userId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "func_test_remove_notify_permission_monitor_userId_0100";
    ObjectDevicePermissionManager::GetInstance().Clear();
    std::string deviceId1 = "deviceId1";
    std::string deviceId2 = "deviceId2";
    std::string deviceId3 = "deviceId3";
    int32_t uid = 100000;   // sys
    int32_t uid2 = 100001;  // del target
    int32_t uid3 = 102101;  // del target
    int32_t uid4 = 200001;
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId1, uid);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId1, uid2);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId1, uid3);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId1, uid4);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId2, uid2);
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId2, uid4);
    int32_t userId = 1;
    ObjectDevicePermissionManager::GetInstance().RemoveNotifyPermissionMonitorUserId(userId);
    std::map<int32_t, std::shared_ptr<ObjectUid>> multipleUidMap =
        ObjectDevicePermissionRepository::GetInstance().objectDevices_.at(deviceId1)->GetUidPermissions();
    std::map<int32_t, std::shared_ptr<ObjectUid>>::iterator it;
    int32_t i = 0;
    for (auto it = multipleUidMap.begin(); it != multipleUidMap.end(); it++) {
        if (it->second->GetUid() == uid || it->second->GetUid() == uid4) {
            i++;
        }
    }
    EXPECT_TRUE(i == 2 && multipleUidMap.size() == 2);

    std::map<int32_t, std::shared_ptr<ObjectUid>> multipleUidMap2 =
        ObjectDevicePermissionRepository::GetInstance().objectDevices_.at(deviceId2)->GetUidPermissions();
    std::map<int32_t, std::shared_ptr<ObjectUid>>::iterator it2;
    int32_t j = 0;
    for (auto it2 = multipleUidMap2.begin(); it2 != multipleUidMap2.end(); it2++) {
        if (it2->second->GetUid() == uid2 || it2->second->GetUid() == uid4) {
            j++;
        }
    }
    EXPECT_TRUE(j == 1 && multipleUidMap2.size() == 1);
}
// Check uid is negative, do remove operation instead.
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_notify_permission_changed_0100, TestSize.Level1)
{
    int32_t uid = -1;
    int result = ObjectDevicePermissionManager::GetInstance().NotifyPermissionChanged(uid);
    EXPECT_TRUE(result == Constant::SUCCESS);
}
// For special uid, do not need to notify.
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_notify_permission_changed_0200, TestSize.Level1)
{
    int32_t uid = 100000;
    int result = ObjectDevicePermissionManager::GetInstance().NotifyPermissionChanged(uid);
    EXPECT_TRUE(result == Constant::SUCCESS);
}
// uid < 0  RemoveNotifyPermissionMonitorUid
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_notify_permission_changed_0300, TestSize.Level1)
{
    int32_t uid = 102101;
    ObjectDevicePermissionManager::GetInstance().Clear();
    std::string deviceId = "deviceId1";
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId, uid);
    std::map<int32_t, std::shared_ptr<ObjectUid>> multipleUidMap1 =
        ObjectDevicePermissionRepository::GetInstance().objectDevices_.at(deviceId)->GetUidPermissions();
    int result = ObjectDevicePermissionManager::GetInstance().NotifyPermissionChanged(-uid);
    std::map<int32_t, std::shared_ptr<ObjectUid>> multipleUidMap =
        ObjectDevicePermissionRepository::GetInstance().objectDevices_.at(deviceId)->GetUidPermissions();
    EXPECT_TRUE(result == Constant::SUCCESS && multipleUidMap.size() == 0);
}
// toSyncDevices.empty()
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_notify_permission_changed_0400, TestSize.Level1)
{
    ObjectDevicePermissionManager::GetInstance().Clear();
    int32_t uid = 102101;
    int result = ObjectDevicePermissionManager::GetInstance().NotifyPermissionChanged(uid);
    EXPECT_TRUE(result == Constant::SUCCESS);
}
// normal success
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_notify_permission_changed_0500, TestSize.Level1)
{
    int32_t uid = 102101;
    ObjectDevicePermissionManager::GetInstance().Clear();
    std::string deviceId = "deviceId";
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId, uid);
    int result = ObjectDevicePermissionManager::GetInstance().NotifyPermissionChanged(uid);
    EXPECT_TRUE(result == Constant::SUCCESS);
}
// normal timeout
HWTEST_F(ObjectDevicePermissionManagerFuncTest, func_test_notify_permission_changed_0600, TestSize.Level1)
{
    int32_t uid = 102101;
    ObjectDevicePermissionManager::GetInstance().Clear();
    std::string deviceId = "deviceId1";
    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId, uid);
    int result = ObjectDevicePermissionManager::GetInstance().NotifyPermissionChanged(uid);
    EXPECT_TRUE(result == Constant::FAILURE);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS