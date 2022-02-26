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

using namespace testing::ext;
using namespace OHOS::Security::Permission;
using namespace std;

class AppIdInfoHelperTest : public testing::Test {
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

HWTEST_F(AppIdInfoHelperTest, Test01, Function | MediumTest | Level1)
{
    pid_t pid = 1;
    uid_t uid = 2;
    string j = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid);
    string expect_j = "{\"pid\":1,\"uid\":2}";
    EXPECT_STREQ(expect_j.c_str(), j.c_str());
}

HWTEST_F(AppIdInfoHelperTest, Test02, Function | MediumTest | Level1)
{
    pid_t pid = 1;
    uid_t uid = 2;
    string deviceId = "1001";
    string j = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid, deviceId);
    string expect_j = "{\"deviceID\":\"1001\",\"pid\":1,\"uid\":2}";
    EXPECT_STREQ(expect_j.c_str(), j.c_str());
}
HWTEST_F(AppIdInfoHelperTest, Test03, Function | MediumTest | Level1)
{
    pid_t pid = 1;
    uid_t uid = 2;
    string deviceId = " ";
    string j = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid, deviceId);
    string expect_j = "{\"deviceID\":\" \",\"pid\":1,\"uid\":2}";
    EXPECT_STREQ(expect_j.c_str(), j.c_str());
}
HWTEST_F(AppIdInfoHelperTest, Test04, Function | MediumTest | Level1)
{
    pid_t pid = 1;
    uid_t uid = 2;
    string deviceId = "1001";
    string bundleName = "theme";
    string j = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid, deviceId, bundleName);
    string expect_j = "{\"bundleName\":\"theme\",\"deviceID\":\"1001\",\"pid\":1,\"uid\":2}";
    EXPECT_STREQ(expect_j.c_str(), j.c_str());
}
HWTEST_F(AppIdInfoHelperTest, Test05, Function | MediumTest | Level1)
{
    pid_t pid = 1;
    uid_t uid = 2;
    string deviceId = " ";
    string bundleName = "theme";
    string j = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid, deviceId, bundleName);
    string expect_j = "{\"bundleName\":\"theme\",\"deviceID\":\" \",\"pid\":1,\"uid\":2}";
    EXPECT_STREQ(expect_j.c_str(), j.c_str());
}
HWTEST_F(AppIdInfoHelperTest, Test06, Function | MediumTest | Level1)
{
    pid_t pid = 1;
    uid_t uid = 2;
    string deviceId = "1001";
    string bundleName = " ";
    string j = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid, deviceId, bundleName);
    string expect_j = "{\"bundleName\":\" \",\"deviceID\":\"1001\",\"pid\":1,\"uid\":2}";
    EXPECT_STREQ(expect_j.c_str(), j.c_str());
}
HWTEST_F(AppIdInfoHelperTest, Test07, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"deviceID\":\"1001\",\"pid\":-1,\"uid\":2";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_FALSE(ret);
}
HWTEST_F(AppIdInfoHelperTest, Test08, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"deviceID\":\"1001\",\"pid\":1,\"uid\":2}";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_TRUE(ret);
}
HWTEST_F(AppIdInfoHelperTest, Test09, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"deviceID\":\"1001\",\"pid\":1,\"uid\":-2}";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_FALSE(ret);
}
HWTEST_F(AppIdInfoHelperTest, Test10, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"deviceID\":\" \",\"pid\":1,\"uid\":2}";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_TRUE(ret);
}
HWTEST_F(AppIdInfoHelperTest, Test11, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"pid\":1,\"uid\":2}";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_TRUE(ret);
}
HWTEST_F(AppIdInfoHelperTest, Test12, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"deviceID\":\"1001\",\"uid\":2}";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_FALSE(ret);
}
HWTEST_F(AppIdInfoHelperTest, Test13, Function | MediumTest | Level1)
{
    string appIdInfo = "{\"deviceID\":\"1001\",\"pid\":1}";
    AppIdInfo appIdInfoObj;
    bool ret = DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj);
    EXPECT_FALSE(ret);
}