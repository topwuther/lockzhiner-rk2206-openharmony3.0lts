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

#include "cust_utils_test.h"

#include <gtest/gtest.h>

#include "cust_utils.h"

using namespace testing::ext;

namespace OHOS {
class CustUtilsTest : public testing::Test {
};

bool TestGetCfgFile(const char *testPathSuffix)
{
    CfgFiles *cfgFiles = GetCfgFiles(testPathSuffix, CUST_TYPE_CONFIG);
    bool flag = false;
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        char *filePath = cfgFiles->paths[i];
        if (filePath && *filePath != '\0') {
            std::cout << "filePath: " << filePath << std::endl;
            flag = true;
        }
    }
    char buf[MAX_PATH_LEN];
    char *filePath = GetOneCfgFile(testPathSuffix, CUST_TYPE_CONFIG, buf, MAX_PATH_LEN);
    if (filePath && *filePath != '\0') {
        std::cout << "one filePath: " << filePath << std::endl;
        flag = flag && true;
    }
    return flag;
}

/**
 * @tc.name: CustUtilsFuncTest001
 * @tc.desc: Test GetOneCfgFile & GetCfgFiles function, none file case.
 * @tc.type: FUNC
 */
HWTEST_F(CustUtilsTest, CustUtilsFuncTest001, TestSize.Level1)
{
    const char *testPathSuffix = "none.xml";
    EXPECT_FALSE(TestGetCfgFile(testPathSuffix));
}

/**
 * @tc.name: CustUtilsFuncTest002
 * @tc.desc: Test GetOneCfgFile & GetCfgFiles function, system file case.
 * @tc.type: FUNC
 */
HWTEST_F(CustUtilsTest, CustUtilsFuncTest002, TestSize.Level1)
{
    const char *testPathSuffix = "system.xml";
    EXPECT_TRUE(TestGetCfgFile(testPathSuffix));
}

/**
 * @tc.name: CustUtilsFuncTest003
 * @tc.desc: Test GetOneCfgFile & GetCfgFiles function, user file case.
 * @tc.type: FUNC
 */
HWTEST_F(CustUtilsTest, CustUtilsFuncTest003, TestSize.Level1)
{
    const char *testPathSuffix = "user.xml";
    EXPECT_TRUE(TestGetCfgFile(testPathSuffix));
}

/**
 * @tc.name: CustUtilsFuncTest004
 * @tc.desc: Test GetOneCfgFile & GetCfgFiles function, both files case.
 * @tc.type: FUNC
 */
HWTEST_F(CustUtilsTest, CustUtilsFuncTest004, TestSize.Level1)
{
    const char *testPathSuffix = "both.xml";
    EXPECT_TRUE(TestGetCfgFile(testPathSuffix));
}

/**
 * @tc.name: CustUtilsFuncTest005
 * @tc.desc: Test struct CfgDir *GetCfgDirListType(int type) function.
 * @tc.type: FUNC
 */
HWTEST_F(CustUtilsTest, CustUtilsFuncTest005, TestSize.Level1)
{
    CfgDir *cfgDir = GetCfgDirListType(CUST_TYPE_RFU);
    EXPECT_TRUE(cfgDir != NULL);
    bool flag = false;
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        char *filePath = cfgDir->paths[i];
        if (filePath && *filePath != '\0') {
            std::cout << "filePath: " << filePath << std::endl;
            flag = true;
        }
    }
    EXPECT_TRUE(flag);
}

/**
 * @tc.name: CustUtilsFuncTest006
 * @tc.desc: Test struct CfgDir *GetCfgDirList(void) function.
 * @tc.type: FUNC
 */
HWTEST_F(CustUtilsTest, CustUtilsFuncTest006, TestSize.Level1)
{
    CfgDir *cfgDir = GetCfgDirList();
    EXPECT_TRUE(cfgDir != NULL);
    bool flag = false;
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        char *filePath = cfgDir->paths[i];
        if (filePath && *filePath != '\0') {
            std::cout << "filePath: " << filePath << std::endl;
            flag = true;
        }
    }
    EXPECT_TRUE(flag);
}
} // namespace OHOS
