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

#include "gtest/gtest.h"

#include <dirent.h>
#include <fstream>
#include <securec.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "bundle_manager.h"
#include "dmsfwk_interface.h"
#include "dmslite_permission.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
#ifndef WEARABLE_PRODUCT
const int32_t NON_EXISTENT_UID = 12345;
const char NATIVE_APPID_DIR[] = "/system/native_appid/";
const char FOUNDATION_APPID[] = "foundation_signature";
const char PREFIX[] = "uid_";
const char SUFFIX[] = "_appid";
const char LAUNCHER_BUNDLE_NAME[] = "com.huawei.launcher";
const char LAUNCHER_APPID[] = "com.huawei.launcher_BM70W1/aVSbkx+uI/WT/mO9NqmtEBx9esLAogYAid75/gTMpKWqrNUT5hS9Cj"
                              "Bq6kt1OcxgZzdCJ4HuVyS4dP8w=";
#endif
}

class PermissionTest : public testing::Test {
protected:
    static void SetUpTestCase() { }
    static void TearDownTestCase() { }
    virtual void SetUp() { }
    virtual void TearDown() { }
};

#ifndef WEARABLE_PRODUCT
/**
 * @tc.name: GetCallerBundleInfo_001
 * @tc.desc: GetCallerBundleInfo failed with null callerInfo or bundleInfo pointer
 * @tc.type: FUNC
 * @tc.require: AR000FU5M6
 */
HWTEST_F(PermissionTest, GetCallerBundleInfo_001, TestSize.Level1)
{
    CallerInfo callerInfo = {.uid = SHELL_UID};
    BundleInfo bundleInfo = {0};
    EXPECT_EQ(GetCallerBundleInfo(nullptr, &bundleInfo), DMS_EC_INVALID_PARAMETER);
    EXPECT_EQ(GetCallerBundleInfo(&callerInfo, nullptr), DMS_EC_INVALID_PARAMETER);
}

/**
 * @tc.name: GetCallerBundleInfo_002
 * @tc.desc: GetCallerBundleInfo failed with SHELL_UID=0/2, which is not configured with appId
 * @tc.type: FUNC
 * @tc.require: AR000FU5M6
 */
HWTEST_F(PermissionTest, GetCallerBundleInfo_002, TestSize.Level1)
{
    CallerInfo callerInfo = {.uid = SHELL_UID};
    BundleInfo bundleInfo = {0};
    EXPECT_EQ(GetCallerBundleInfo(&callerInfo, &bundleInfo), DMS_EC_FAILURE);
}

/**
 * @tc.name: GetCallerBundleInfo_003
 * @tc.desc: GetCallerBundleInfo successfully with FOUNDATION_UID=7, which has been configured with appId
 * @tc.type: FUNC
 * @tc.require: AR000FU5M6
 */
HWTEST_F(PermissionTest, GetCallerBundleInfo_003, TestSize.Level1)
{
    bool isDirExist = true;
    DIR *dir = opendir(NATIVE_APPID_DIR);
    if (dir == nullptr) {
        mode_t mode = 0700;
        EXPECT_EQ(mkdir(NATIVE_APPID_DIR, mode), 0);
        isDirExist = false;
    } else {
        closedir(dir);
    }
    CallerInfo callerInfo = {.uid = FOUNDATION_UID};
    stringstream filePath;
    filePath << NATIVE_APPID_DIR << PREFIX << callerInfo.uid << SUFFIX;
    fstream fs(filePath.str(), ios::out);
    EXPECT_TRUE(fs.is_open());
    fs << FOUNDATION_APPID;
    fs.close();
    BundleInfo bundleInfo = {0};
    EXPECT_EQ(GetCallerBundleInfo(&callerInfo, &bundleInfo), DMS_EC_SUCCESS);
    EXPECT_EQ(strcmp(bundleInfo.appId, FOUNDATION_APPID), 0);
    ClearBundleInfo(&bundleInfo);
    remove(filePath.str().c_str());
    if (!isDirExist) {
        rmdir(NATIVE_APPID_DIR);
    }
}

/**
 * @tc.name: GetCallerBundleInfo_004
 * @tc.desc: GetCallerBundleInfo failed with a non-existent uid
 * @tc.type: FUNC
 * @tc.require: AR000FU5M6
 */
HWTEST_F(PermissionTest, GetCallerBundleInfo_004, TestSize.Level1)
{
    CallerInfo callerInfo = {.uid = NON_EXISTENT_UID};
    BundleInfo bundleInfo = {0};
    EXPECT_EQ(GetCallerBundleInfo(&callerInfo, &bundleInfo), DMS_EC_FAILURE);
}

/**
 * @tc.name: GetCallerBundleInfo_005
 * @tc.desc: GetCallerBundleInfo successfully with com.huawei.launcher uid
 * @tc.type: FUNC
 * @tc.require: AR000FU5M6
 */
HWTEST_F(PermissionTest, GetCallerBundleInfo_005, TestSize.Level1)
{
    BundleInfo bundleInfo = {0};
    EXPECT_EQ(GetBundleInfo(LAUNCHER_BUNDLE_NAME, 0, &bundleInfo), EC_SUCCESS);
    CallerInfo callerInfo = {.uid = bundleInfo.uid};
    BundleInfo callerBundleInfo = {0};
    EXPECT_EQ(GetCallerBundleInfo(&callerInfo, &callerBundleInfo), DMS_EC_SUCCESS);
    EXPECT_EQ(strcmp(callerBundleInfo.appId, LAUNCHER_APPID), 0);
    ClearBundleInfo(&callerBundleInfo);
}
#endif
}
}
