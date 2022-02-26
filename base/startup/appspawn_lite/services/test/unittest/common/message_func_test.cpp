/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <vector>
#include "gtest/gtest.h"
#include "appspawn_message.h"

using namespace testing::ext;

namespace OHOS {
const unsigned int MALLOC_TEST_LENGTH = 10;
const long NANOSECONDS_PER_SECOND = 1000000000;
const int TEST_UID = 999;
const int TEST_GID = 888;
std::vector<std::string> g_badStrings;
std::vector<std::string> g_goodStrings;

class StartupAppspawnUTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // empty
        g_badStrings.push_back(std::string(""));
        // not json
        g_badStrings.push_back(std::string("abcdefghijklmnopqrstuvwxyz"));
        g_badStrings.push_back(std::string("0123456789876543210"));
        g_badStrings.push_back(std::string("xxxx"));
        g_badStrings.push_back(std::string("xxxxssssssssssssssssssss"));
        g_badStrings.push_back(std::string("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""));
        g_badStrings.push_back(std::string("............................................."));
        g_badStrings.push_back(std::string("....%%%....^..***@##.../*--++......$$&&....."));
        // looks like json but format error
        g_badStrings.push_back(std::string("{bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName:\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV,\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID:\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1,\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID:10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID:10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability:[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]"));
        g_badStrings.push_back(std::string("\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\"\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\"\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\"\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\"\"uID\":10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\"10,\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10\"gID\":10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\"10,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\"[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0]},"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0],}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1\",\"uID\":10,\"gID\":10,\"capability\":[0,]}"));
        // json format correct but fields missing
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000}"));
        // field value invalid
        g_badStrings.push_back(std::string("{\"bundleName\":\"\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1test1\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"012345978901234597890123459789\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":-1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":-1000,\"capability\":[-7]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":1}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0, 3, -9]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[99999999]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":1234,\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":\"1000\",\"gID\":1000,\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":\"1000\",\"capability\":[0]}"));
        g_badStrings.push_back(std::string("{\"bundleName\":\"nameV\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":\"0\"}"));
        g_badStrings.push_back(std::string("{\"bundleName\": 250,\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));

        g_goodStrings.push_back(std::string("{\"bundleName\":\"testvalid1\",\"identityID\":\"1234\",\"uID\":1000,\"gID\":1000,\"capability\":[0]}"));
        g_goodStrings.push_back(std::string("{\"bundleName\":\"testvalid2\",\"identityID\":\"5678\",\"uID\":1001,\"gID\":1001,\"capability\":[3,5]}"));
        g_goodStrings.push_back(std::string("{\"bundleName\":\"testvalid3\",\"identityID\":\"91011\",\"uID\":1002,\"gID\":1002,\"capability\":[8,9]}"));
        g_goodStrings.push_back(std::string("{\"bundleName\":\"testvalid3\",\"identityID\":\"999\",\"uID\":1002,\"gID\":1002,\"capability\":[]}"));
        g_goodStrings.push_back(std::string("{\"bundleName\":\"testvalid3\",\"identityID\":\"3\",\"uID\":1002,\"gID\":1002,\"capability\":[1,2]}"));
        printf("[----------] StartupAppspawnUTest, message func test setup.\n");
    }

    static void TearDownTestCase()
    {
        g_badStrings.clear();
        g_goodStrings.clear();
        printf("[----------] StartupAppspawnUTest, message func test teardown.\n");
    }
    void SetUp() {}
    void TearDown() {}
};

/*
 ** @tc.name: msgFuncFreeTest_001
 ** @tc.desc: free message struct function, nullptr test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733K
 ** @tc.author: ZAL
 **/
HWTEST_F(StartupAppspawnUTest, msgFuncFreeTest_001, TestSize.Level1)
{
    // do not crash here
    FreeMessageSt(nullptr);

    MessageSt msgSt = {0};
    FreeMessageSt(&msgSt);

    EXPECT_EQ(msgSt.bundleName, nullptr);
    EXPECT_EQ(msgSt.identityID, nullptr);
    EXPECT_EQ(msgSt.caps, nullptr);
    EXPECT_EQ(msgSt.uID, -1);
    EXPECT_EQ(msgSt.gID, -1);
    EXPECT_EQ(msgSt.capsCnt, 0);
}

/*
 ** @tc.name: msgFuncFreeTest_002
 ** @tc.desc: free message struct function, function test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733K
 ** @tc.author: ZAL
 **/
HWTEST_F(StartupAppspawnUTest, msgFuncFreeTest_002, TestSize.Level1)
{
    MessageSt msgSt = {0};

    msgSt.capsCnt = MALLOC_TEST_LENGTH;
    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.capsCnt, 0);

    msgSt.uID = TEST_UID;
    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.uID, -1);

    msgSt.gID = TEST_GID;
    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.gID, -1);

    msgSt.bundleName = (char*)malloc(MALLOC_TEST_LENGTH);
    EXPECT_TRUE(msgSt.bundleName != nullptr);
    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.bundleName, nullptr);

    msgSt.identityID = (char*)malloc(MALLOC_TEST_LENGTH);
    EXPECT_TRUE(msgSt.identityID != nullptr);
    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.identityID, nullptr);

    msgSt.caps = (unsigned int*)malloc(MALLOC_TEST_LENGTH * sizeof(unsigned int));
    EXPECT_TRUE(msgSt.caps != nullptr);
    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.caps, nullptr);

    // full test
    msgSt.bundleName = (char*)malloc(MALLOC_TEST_LENGTH);
    msgSt.identityID = (char*)malloc(MALLOC_TEST_LENGTH);
    msgSt.caps = (unsigned int*)malloc(MALLOC_TEST_LENGTH * sizeof(unsigned int));
    EXPECT_TRUE(msgSt.bundleName != nullptr);
    EXPECT_TRUE(msgSt.identityID != nullptr);
    EXPECT_TRUE(msgSt.caps != nullptr);

    msgSt.capsCnt = MALLOC_TEST_LENGTH;
    msgSt.uID = TEST_UID;
    msgSt.gID = TEST_GID;

    FreeMessageSt(&msgSt);
    EXPECT_EQ(msgSt.bundleName, nullptr);
    EXPECT_EQ(msgSt.identityID, nullptr);
    EXPECT_EQ(msgSt.caps, nullptr);
    EXPECT_EQ(msgSt.capsCnt, 0);
    EXPECT_EQ(msgSt.uID, -1);
    EXPECT_EQ(msgSt.gID, -1);
};

static void GetCurrentTime(struct timespec* tmCur)
{
    if (tmCur == NULL) {
        return;
    }
    if (clock_gettime(CLOCK_REALTIME, tmCur) != 0) {
        printf("[----------] StartupAppspawnUTest, get time failed! err %d.\n", errno);
    }
}

/*
 ** @tc.name: msgFuncSplitTest_001
 ** @tc.desc: split message function, bad strings test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733K
 ** @tc.author: ZAL
 **/
HWTEST_F(StartupAppspawnUTest, msgFuncSplitTest_001, TestSize.Level1)
{
    MessageSt msgSt = {0};
    EXPECT_NE(SplitMessage(nullptr, 0, nullptr), 0);
    EXPECT_NE(SplitMessage(nullptr, 0, &msgSt), 0);

    std::string testMsg = "xxxxxxxx";
    EXPECT_NE(SplitMessage(testMsg.c_str(), testMsg.length(), nullptr), 0);

    struct timespec tmStart = {0};
    GetCurrentTime(&tmStart);

    for (size_t i = 0; i < g_badStrings.size(); ++i) {
        int ret = SplitMessage(g_badStrings[i].c_str(), g_badStrings[i].length(), &msgSt);
        EXPECT_NE(ret, 0);
        if (ret == 0) {
            printf("[----------] StartupAppspawnUTest, msgFuncSplitTest_001 i = %u.\n", i);
            FreeMessageSt(&msgSt);
        }
    }

    struct timespec tmEnd = {0};
    GetCurrentTime(&tmEnd);
    long timeUsed = (tmEnd.tv_sec - tmStart.tv_sec) * NANOSECONDS_PER_SECOND + (tmEnd.tv_nsec - tmStart.tv_nsec);
    printf("[----------] StartupAppspawnUTest, msgFuncSplitTest_001, total time %ld ns, strCnt %u.\n",\
        timeUsed, g_badStrings.size());
}

/*
 ** @tc.name: msgFuncSplitTest_002
 ** @tc.desc: split message function, good strings test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733K
 ** @tc.author: ZAL
 **/
HWTEST_F(StartupAppspawnUTest, msgFuncSplitTest_002, TestSize.Level1)
{
    MessageSt msgSt = {0};

    struct timespec tmStart = {0};
    GetCurrentTime(&tmStart);

    for (size_t i = 0; i < g_goodStrings.size(); ++i) {
        int ret = SplitMessage(g_goodStrings[i].c_str(), g_goodStrings[i].length(), &msgSt);
        EXPECT_EQ(ret, 0);
        if (ret != 0) {
            printf("[----------] StartupAppspawnUTest, msgFuncSplitTest_002 i = %u.\n", i);
        } else {
            FreeMessageSt(&msgSt);
        }
    }

    struct timespec tmEnd = {0};
    GetCurrentTime(&tmEnd);

    long timeUsed = (tmEnd.tv_sec - tmStart.tv_sec) * NANOSECONDS_PER_SECOND + (tmEnd.tv_nsec - tmStart.tv_nsec);
    printf("[----------] StartupAppspawnUTest, msgFuncSplitTest_002, total time %ld ns, strCnt %u.\n", \
        timeUsed, g_goodStrings.size());

    // parse one good string and check all results
    std::string validStr =
        "{\"bundleName\":\"validName\",\"identityID\":\"135\",\"uID\":999,\"gID\":888,\"capability\":[0, 1, 5]}";
    int ret = SplitMessage(validStr.c_str(), validStr.length(), &msgSt);
    EXPECT_EQ(ret, 0);

    std::vector<unsigned int> caps;
    caps.push_back(0);    // 0, test capability
    caps.push_back(1);    // 1, test capability
    caps.push_back(5);    // 5, test capability

    EXPECT_NE(msgSt.bundleName, nullptr);
    EXPECT_NE(msgSt.identityID, nullptr);
    EXPECT_EQ(strcmp("validName", msgSt.bundleName), 0);
    EXPECT_EQ(strcmp("135", msgSt.identityID), 0);
    EXPECT_EQ(TEST_UID, msgSt.uID);
    EXPECT_EQ(TEST_GID, msgSt.gID);
    EXPECT_EQ(caps.size(), msgSt.capsCnt);
    EXPECT_NE(msgSt.caps, nullptr);
    for (size_t i = 0; i < caps.size(); ++i) {
        EXPECT_EQ(caps[i], msgSt.caps[i]);
    }
    FreeMessageSt(&msgSt);
}
}  // namespace OHOS
