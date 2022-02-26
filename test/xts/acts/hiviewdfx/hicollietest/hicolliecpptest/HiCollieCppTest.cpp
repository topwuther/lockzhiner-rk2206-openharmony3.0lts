/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <gtest/gtest.h>

#include <unistd.h>
#include <sys/wait.h>

#include "xcollie/xcollie.h"
#include "xcollie/xcollie_checker.h"
#include "xcollie/xcollie_define.h"
#include "ctime"
#include "file_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;

namespace {
int g_type = 1;
int g_blockTime;
int g_waitTime;
int g_setTimeout;
int g_updateTime = -1;
int g_cancelTimeout = -1;
}
class HiCollieCppTest : public testing::Test {
public:
    int callbackCnt_;
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    string ReadFileList(string basePath, string param);
    bool ReadHilogcatredirectFile(string path, string context);
    void XcollieTestInstance(string testItem);
    void DoXCollieTest();
    void TimeCallback(void *data);
    void SetCallbackCnt(int cnt);
    HiCollieCppTest();
    ~HiCollieCppTest();
private:
};
HiCollieCppTest::HiCollieCppTest():callbackCnt_(0)
{
}
HiCollieCppTest::~HiCollieCppTest()
{
}
void HiCollieCppTest::SetUp()
{
    std::cout << "SetUp" << std::endl;
}
void HiCollieCppTest::TearDown()
{
    std::cout << "TearDown" << std::endl;
    g_updateTime = -1;
    g_cancelTimeout = -1;
}
void HiCollieCppTest::SetUpTestCase()
{
    std::cout << "SetUpTestCase" << std::endl;
}
void HiCollieCppTest::TearDownTestCase()
{
    std::cout << "TearDownTestCase" << std::endl;
}
void HiCollieCppTest::TimeCallback(void *data)
{
    callbackCnt_++;
}
void HiCollieCppTest::SetCallbackCnt(int cnt)
{
    callbackCnt_ = cnt;
}
class XCollieCheckerAssist : public XCollieChecker {
    using XCollieChecker::XCollieChecker;
    virtual void CheckLock()
    {
    // sleep 35 seconds
        sleep(35);
    }
};
void HiCollieCppTest::DoXCollieTest()
{
    pid_t pid;
    int status;
    // Fork a child process for leak injection
    pid = fork();
    if (pid == -1) {
        cout<<"fail to fork!"<<endl;
        exit(1);
    } else if (pid == 0) {
        XcollieTestInstance("WATCHDOG");
        sleep(g_blockTime);
        exit(0);
    } else {
        wait(&status);
    }
}

void HiCollieCppTest::XcollieTestInstance(string testItem)
{
    cout<<"XcollieTestInstance start"<<endl;
    if (testItem == "TIMEOUT") {
        int id = XCollie::GetInstance().SetTimer("TimeoutTimer", g_setTimeout, nullptr, nullptr, g_type);
        if (g_updateTime != -1) {
            bool ret = XCollie::GetInstance().UpdateTimer(id, g_updateTime);
            if (ret) {
                cout<<"Update timer succeed,reset as "<<g_updateTime<<endl;
            } else {
                cout<<"Update timer failed!"<<endl;
            }
        }
        if (g_cancelTimeout != -1) {
            XCollie::GetInstance().CancelTimer(id);
        }
        sleep(g_blockTime);
        XCollie::GetInstance().CancelTimer(id);
    } else if (testItem == "WATCHDOG") {
        const OHOS::sptr<XCollieChecker> checker = new XCollieCheckerAssist("checker");
        XCollie::GetInstance().RegisterXCollieChecker(checker, g_type);
        sleep(g_waitTime);
    }
}
/**
 * @tc.name Verify that when the native layer is blocked by the main thread of
 *   the monitored service, the service restarts after 60 seconds
 * @tc.number DFX_DFR_Xcollie_Watchdog_0001
 * @tc.desc Verify that the native layer is blocked by the main thread of the monitored service
*/
HWTEST_F(HiCollieCppTest, Xcollie_watchdog_test, Function|MediumTest|Level1) {
    GTEST_LOG_(INFO) << "Xcollie_watchdog_test start" << endl;
    g_type = XCOLLIE_THREAD;
    g_blockTime = 61;
    g_waitTime = 20;
    DoXCollieTest();
    string cmd = "";
    bool result = false;
    sleep(90);
    std::vector<std::string> cmdret;
    cmd = "ps -A |grep HiCollieCppTest";
    unsigned long cmdretlen;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    if (cmdretlen == 1) {
        result = true;
    } else {
        GTEST_LOG_(INFO) << "failed to get xcollie log." << endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "Xcollie_watchdog_test end" << endl;
}
/**
 * @tc.name Verify that the native layer service deadlock and
 *   main thread blocking are monitored at the same time
 * @tc.number DFX_DFR_Xcollie_Watchdog_0002
 * @tc.desc Verify that the native layer service deadlock and main thread blocking
*/
HWTEST_F(HiCollieCppTest, Xcollie_watchdog_test1, Function|MediumTest|Level1) {
    GTEST_LOG_(INFO) << "Xcollie_watchdog_test1 start" << endl;
    string cmd = "";
    bool result = false;
    g_type = XCOLLIE_LOCK|XCOLLIE_THREAD;
    g_blockTime = 61;
    g_waitTime = 20;
    DoXCollieTest();
    sleep(90);
    std::vector<std::string> cmdret;
    cmd = "ps -A |grep HiCollieCppTest";
    unsigned long cmdretlen;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    if (cmdretlen == 1) {
        result = true;
    } else {
        GTEST_LOG_(INFO) << "failed to get xcollie log." << endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "Xcollie_watchdog_test1 end" << endl;
}
/**
 * @tc.name When a deadlock occurs in the monitored service of the native layer,
 *   the service restarts after 60 seconds
 * @tc.number DFX_DFR_Xcollie_Watchdog_0003
 * @tc.desc Verify that deadlock occurs in the monitored service of the native layer
*/
HWTEST_F(HiCollieCppTest, Xcollie_watchdog_test2, Function|MediumTest|Level1) {
    GTEST_LOG_(INFO) << "Xcollie_watchdog_test2 start" << endl;
    g_type = XCOLLIE_LOCK;
    g_blockTime = 30;
    g_waitTime = 75;
    DoXCollieTest();
    string cmd = "";
    bool result = false;
    sleep(110);
    std::vector<std::string> cmdret;
    cmd = "ps -A |grep HiCollieCppTest";
    unsigned long cmdretlen;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    if (cmdretlen == 1) {
        result = true;
    } else {
        GTEST_LOG_(INFO) << "failed to get xcollie log." << endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "Xcollie_watchdog_test2 end" << endl;
}
/**
 * @tc.name verification of callback funcitons
 * @tc.number DFX_DFR_Xcollie_Timeout_0001
 * @tc.desc callback funcitons
 */
HWTEST_F(HiCollieCppTest, Xcollie_Timeout_test, Function|MediumTest|Level1) {
    GTEST_LOG_(INFO) << "Xcollie_Timeout_test start" << endl;
    g_type = XCOLLIE_FLAG_NOOP;
    SetCallbackCnt(0);
    auto func = [this](void *data) {
        this->TimeCallback(data);
    };
    int last = XCollie::GetInstance().SetTimer("TimeoutTimer", 61, func, nullptr, g_type);
    ASSERT_TRUE(last != INVALID_ID);
    GTEST_LOG_(INFO) << "Xcollie_Timeout_test end" << endl;
}
/**
 * @tc.name UpdateTimer interface test
 * @tc.number DFX_DFR_Xcollie_interface_0001
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test, Function|MediumTest|Level1)
{
    int id = XCollie::GetInstance().SetTimer("TimeoutTimerTest", 1, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_NE(id, INVALID_ID);
    bool result = XCollie::GetInstance().UpdateTimer(id, 1);
    ASSERT_TRUE(result);
}
/**
 * @tc.name UpdateTimer interface test, id is invalid
 * @tc.number DFX_DFR_Xcollie_interface_0002
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test1, Function|MediumTest|Level1)
{
    int id = -1;
    int timeout = 100;
    bool result = XCollie::GetInstance().UpdateTimer(id, timeout);
    ASSERT_FALSE(result);
}
/**
 * @tc.name UpdateTimer interface test, timeout is 0
 * @tc.number DFX_DFR_Xcollie_interface_0003
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test2, Function|MediumTest|Level1)
{
    bool result = XCollie::GetInstance().UpdateTimer(10, 0);
    ASSERT_FALSE(result);
}
/**
 * @tc.name SetTimer interface test, timeout is 0
 * @tc.number DFX_DFR_Xcollie_interface_0004
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test3, Function|MediumTest|Level1)
{
    int id = XCollie::GetInstance().SetTimer("Timer", 0, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_EQ(id, INVALID_ID);
}
/**
 * @tc.name SetTimer interface test, timeout is negative.
 * @tc.number DFX_DFR_Xcollie_interface_0005
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test4, Function|MediumTest|Level1)
{
    int id = XCollie::GetInstance().SetTimer("Timer", -2, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_TRUE(id != INVALID_ID);
}
/**
 * @tc.name SetTimer interface test, service name is very long.
 * @tc.number DFX_DFR_Xcollie_interface_0006
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test5, Function|MediumTest|Level1)
{
    std::string name = "abcd";
    for (int i = 0; i < 1000; i++) {
        name += "abcd";
    }
    int id = XCollie::GetInstance().SetTimer(name, 5, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_TRUE(id != INVALID_ID);
}
/**
 * @tc.name SetTimer interface test, when type is invalid.
 * @tc.number DFX_DFR_Xcollie_interface_0007
 * @tc.desc interface test
 */
HWTEST_F(HiCollieCppTest, Xcollie_interface_test6, Function|MediumTest|Level1)
{
    int id = XCollie::GetInstance().SetTimer("Timer", 5, nullptr, nullptr, XCOLLIE_FLAG_NOOP + 100);
    ASSERT_TRUE(id != INVALID_ID);
}