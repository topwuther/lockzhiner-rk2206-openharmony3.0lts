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
#include "app_id_info.h"

using namespace testing::ext;
using namespace OHOS::Security::Permission;
using namespace std;

class AppIdInfoTest : public testing::Test {
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
};

HWTEST_F(AppIdInfoTest, Test01, Function | MediumTest | Level1)
{
    AppIdInfo pidInvalidAppIdInfo;
    pidInvalidAppIdInfo.pid = -1;
    pidInvalidAppIdInfo.uid = 1;
    pidInvalidAppIdInfo.deviceId = "1001";
    EXPECT_FALSE(pidInvalidAppIdInfo.Valid());
}

HWTEST_F(AppIdInfoTest, Test02, Function | MediumTest | Level1)
{
    AppIdInfo uidInvalidAppIdInfo;
    uidInvalidAppIdInfo.pid = 1;
    uidInvalidAppIdInfo.uid = -1;
    uidInvalidAppIdInfo.deviceId = "1001";
    EXPECT_FALSE(uidInvalidAppIdInfo.Valid());
}
HWTEST_F(AppIdInfoTest, Test03, Function | MediumTest | Level1)
{
    AppIdInfo deviceIdInvalidAppIdInfo;
    deviceIdInvalidAppIdInfo.pid = 1;
    deviceIdInvalidAppIdInfo.uid = 1;
    deviceIdInvalidAppIdInfo.deviceId = "12345678123456781234567812345678123456781234567812345678123456789";
    EXPECT_FALSE(deviceIdInvalidAppIdInfo.Valid());
}
HWTEST_F(AppIdInfoTest, Test04, Function | MediumTest | Level1)
{
    AppIdInfo deviceIdInvalidAppIdInfo;
    deviceIdInvalidAppIdInfo.pid = 1;
    deviceIdInvalidAppIdInfo.uid = 1;
    deviceIdInvalidAppIdInfo.deviceId = "123";
    EXPECT_TRUE(deviceIdInvalidAppIdInfo.Valid());
}  // namespace Permission
