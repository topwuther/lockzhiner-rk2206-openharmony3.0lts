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

using namespace std;
using namespace testing::ext;
using namespace OHOS::Security::Permission;

namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
}  // namespace OHOS

class PermissionNameToOrFromOpCodeTest : public testing::Test {
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
    DistributedPermissionManagerService managerService;
};

/**
 * @tc.number: PermissionNameToOrFromOpCode_0100
 * @tc.name: test opCode translate into permissionName
 * @tc.desc: 1. opCode is not exist in translate map
 */
HWTEST_F(PermissionNameToOrFromOpCodeTest, PermissionNameToOrFromOpCode_0100, Function | MediumTest | Level1)
{
    int32_t opCode = 9985;
    std::string perName;
    bool flag = Constant::PermissionNameToOrFromOpCode(perName, opCode);
    EXPECT_FALSE(flag);
}
