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
#include <iostream>
#include "distributed_permission_kit.h"
#include "distributed_permission_manager_client.h"
#include "system_ability_definition.h"
#include "distributed_permission_death_recipient.h"
#include "permission_log.h"
#include "iservice_registry.h"
#include "ipc_skeleton.h"
#include "on_using_permission_reminder_stub.h"

using namespace testing::ext;
using namespace OHOS::Security::Permission;
using namespace std;

namespace OHOS {

pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";

}  // namespace OHOS

class CheckPermissionInnerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        cout << "SetUpTestCase()" << endl;
    }

    static void TearDownTestCase()
    {
        cout << "TearDownTestCase()" << endl;
    }
    void SetUp()
    {
        cout << "SetUp() is running" << endl;
    }
    void TearDown()
    {
        cout << "TearDown()" << endl;
    }

    int CheckProxy()
    {
        OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
            OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            return -1;
        }

        OHOS::sptr<OHOS::IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
            IDistributedPermission::SA_ID_DISTRIBUTED_PERMISSION_MANAGER_SERVICE);
        if (!remoteObject) {
            return -1;
        }

        OHOS::sptr<IDistributedPermission> distributedPermissionProxyTest;

        distributedPermissionProxyTest = OHOS::iface_cast<IDistributedPermission>(remoteObject);
        if ((!distributedPermissionProxyTest) || (!distributedPermissionProxyTest->AsObject())) {
            return -1;
        }

        OHOS::sptr<OHOS::IRemoteObject::DeathRecipient> recipientTest;
        recipientTest = new DistributedPermissionDeathRecipient();
        if (!recipientTest) {
            return -1;
        }
        return 0;
    }
};

HWTEST_F(CheckPermissionInnerTest, Test01, Function | MediumTest | Level1)
{
    int ret = -1;
    int exRet = CheckProxy();

    string permissionName = "ohos.permission.test";
    string appIdInfo = "{\"deviceID\":\"1001\",\"pid\":1,\"uid\":2}";

    ret = DistributedPermissionKit::CheckPermission(permissionName, appIdInfo);

    EXPECT_EQ(exRet, ret);
}