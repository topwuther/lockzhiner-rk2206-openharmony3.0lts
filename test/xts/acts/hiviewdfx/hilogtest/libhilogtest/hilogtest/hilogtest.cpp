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
#include <cstdarg>
#include <ctime>
#include <gtest/gtest.h>
#include <list>
#include <ostream>
#include <pthread.h>
#include <queue>
#include <regex>
#include <securec.h>
#include <streambuf>
#include <sys/time.h>
#include <thread>

#include "file_utils.h"
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD003200
#define LOG_TAG "HILOGTOOLTEST"
#define MAX_LINE  1024*10
using namespace OHOS;
using namespace HiviewDFX;
using namespace testing::ext;
using namespace std;

class hilogtest : public testing::Test {
public:
    string input = "";
    string result = "";
    string expect = "";
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    string gHilogtoolExecutable = "hilog ";
    string g_logContent = "123456789_1234567890_public and private log test is:\
    %{public}d, %{private}lf, %{public}.2f, %s, %{private}c\n";
    string g_commonContent = "03200/HILOGTOOLTEST: 123456789_1234567890_public and private log test is:";
private:
};
void hilogtest::SetUp()
{
}
void hilogtest::TearDown()
{
}
void hilogtest::SetUpTestCase()
{
    ExeCmd("hilog -p on");
    ExeCmd("hilog -Q domainoff");
    ExeCmd("hilog -Q pidoff");
    ExeCmd("hilog -b debug");
    ExeCmd("hilog -b debug -D 218116608");
    ExeCmd("hilog -b debug -T HILOGTOOLTEST");
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -G 1m -t all");
    ExeCmd("hilog -S -t all");
    ExeCmd("hilog -S -D 218116608");
}
void hilogtest::TearDownTestCase()
{
    ExeCmd("hilog -p on");
    ExeCmd("hilog -Q domainoff");
    ExeCmd("hilog -Q pidoff");
    ExeCmd("hilog -b debug");
    ExeCmd("hilog -b debug -D 218116608");
    ExeCmd("hilog -b debug -T HILOGTOOLTEST");
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -G 1m -t all");
    ExeCmd("hilog -S -t all");
    ExeCmd("hilog -S -D 218116608");
    std::cout << "TearDownTestCase" << std::endl;
}


/*
 * @tc.name one-time read
 * @tc.number DFX_DFT_HilogCPP_0840
 * @tc.desc one-time read
*/
HWTEST_F(hilogtest, Hilogtool_exit, Function|MediumTest|Level3)
{
    CleanCmd();
    std::string saveFile= "/data/local/tmp/test_data_31.txt";
    std::string cmd1 = gHilogtoolExecutable + " -r";
    std::string cmdResult;
    CmdRun(cmd1, cmdResult);
    LogType type = LOG_APP;
    int i = 0;
    while (i++ <= 5) {
        usleep(1000);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd2 = gHilogtoolExecutable + "-T HILOGTOOLTEST -x";
    SaveCmdOutput(cmd2, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_exit error" << std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name The log tool can read app log types at a time.
 * @tc.number DFX_DFT_HilogCPP_0860
 * @tc.desc The log tool can read app log types at a time.
*/
HWTEST_F(hilogtest, Hilogtool_type_app, Function|MediumTest|Level3)
{
    CleanCmd();
    std::string saveFile= "/data/local/tmp/test_data_33_1.txt";
    LogType type = LOG_APP;
    int i = 0;
    while (i++ <= 5) {
        usleep(1000);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd = gHilogtoolExecutable + " -t app -x ";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_type_app error";
    }
    ASSERT_TRUE(true == result);
}


/*
 * @tc.name The log tool can read core log types at a time.
 * @tc.number DFX_DFT_HilogCPP_0870
 * @tc.desc The log tool can read core log types at a time
*/
HWTEST_F(hilogtest, Hilogtool_type_core, Function|MediumTest|Level3)
{
    CleanCmd();
    std::string saveFile= "/data/local/tmp/test_data_33_2.txt";
    LogType type = LOG_CORE;
    int i = 0;
    while (i++ <= 5) {
        usleep(1000);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd = gHilogtoolExecutable + " -t core -x ";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_type_app error"<<std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name The log tool can read init log types at a time.
 * @tc.number DFX_DFT_HilogCPP_0880
 * @tc.desc The log tool can read init log types at a time.
*/
HWTEST_F(hilogtest, Hilogtool_type_init, Function|MediumTest|Level3)
{
    CleanCmd();
    std::string saveFile= "/data/local/tmp/test_data_33_3.txt";
    LogType type = LOG_INIT;
    int i = 0;
    while (i++ <= 5) {
        usleep(1000);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd = gHilogtoolExecutable + " -t init -x ";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_type_init error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name The log tool can read Hilogtool_type_multiple log types at a time.
 * @tc.number DFX_DFT_HilogCPP_0890
 * @tc.desc The log tool can read Hilogtool_type_multiple log types at a time.
*/
HWTEST_F(hilogtest, Hilogtool_type_multiple, Function|MediumTest|Level2)
{
    CleanCmd();
    std::string saveFile= "/data/local/tmp/test_data_33_4.txt";
    LogType type = LOG_INIT;
    int i = 0;
    while (i++ <= 3) {
        usleep(1000);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
        type = LOG_INIT;
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
        type = LOG_APP;
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd = gHilogtoolExecutable + "-D 0xd003200 -t app core init -x ";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_type_multiple error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name show local time
 * @tc.number DFX_DFT_HilogCPP_0950
 * @tc.desc show local time
*/
HWTEST_F(hilogtest, Hilogtool_time, Function|MediumTest|Level3)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v time ";
    std::string saveFile= "/data/local/tmp/test_data_35_1.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::regex pattern(
    "(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])\\s(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})(\\.(\\d){0,3})?$");
    std::cout << hilogInfo.substr(0, 18);
    string timeBuffer = hilogInfo.substr(0, 18);
    std::smatch match;
    bool ismatch = regex_match(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout<<"match successed"<<std::endl;
        result = true;
    } else {
        std::cout<<"match failed"<<std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name show the time from 1970
 * @tc.number DFX_DFT_HilogCPP_0960
 * @tc.desc show the time from 1970
*/
HWTEST_F(hilogtest, Hilogtool_epoch, Function|MediumTest|Level4)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v epoch ";
    std::string saveFile= "/data/local/tmp/test_data_35_2.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::regex pattern("[^0-9]");
    std::cout << hilogInfo.substr(0, 19);
    string timeBuffer = hilogInfo.substr(0, 19);
    std::smatch match;
    bool ismatch = regex_search(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout << "match successed" << std::endl;
        result = true;
    } else {
        std::cout <<"match failed"<< std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name show the time from last restart
 * @tc.number DFX_DFT_HilogCPP_0970
 * @tc.desc show the time from last restart
*/
HWTEST_F(hilogtest, Hilogtool_monotonic, Function|MediumTest|Level3)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v monotonic";
    std::string saveFile= "/data/local/tmp/test_data_35_3.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::regex pattern("[^0-9]");
    std::cout << hilogInfo.substr(0, 14);
    string timeBuffer = hilogInfo.substr(0, 14);
    std::smatch match;
    bool ismatch = regex_search(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout << "match successed"  << std::endl;
        result = true;
    } else {
        std::cout << "match failed" << std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Displays time in microsecond accuracy
 * @tc.number DFX_DFT_HilogCPP_0980
 * @tc.desc Displays time in microsecond accuracy
*/
HWTEST_F(hilogtest, Hilogtool_usec, Function|MediumTest|Level4)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v usec";
    std::string saveFile= "/data/local/tmp/test_data_35_4.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::regex pattern(
    "(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])\\s(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})(\\.(\\d){0,6})?$");
    std::cout << hilogInfo.substr(0, 21);
    string timeBuffer = hilogInfo.substr(0, 21);
    std::smatch match;
    bool ismatch = regex_match(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout << "match successed"  << std::endl;
        result = true;
    } else {
        std::cout << "match failed" << std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Displays time in nanosecond precision.
 * @tc.number DFX_DFT_HilogCPP_0990
 * @tc.desc Displays time in nanosecond precision.
*/
HWTEST_F(hilogtest, Hilogtool_nsec, Function|MediumTest|Level4)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v nsec";
    std::string saveFile= "/data/local/tmp/test_data_35_5.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::regex pattern(
    "(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])\\s(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})(\\.(\\d){0,9})?$");

    std::cout << hilogInfo.substr(0, 24);
    string timeBuffer = hilogInfo.substr(0, 24);
    std::smatch match;
    bool ismatch = regex_match(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout << "match successed"<< std::endl;
        result = true;
    } else {
        std::cout << "match failed"<< std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name added year to the displayed time.
 * @tc.number DFX_DFT_HilogCPP_1000
 * @tc.desc added year to the displayed time.
*/
HWTEST_F(hilogtest, Hilogtool_year, Function|MediumTest|Level4)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v year";
    char saveFile356[] = "/data/local/tmp/test_data_35_6.txt";
    SaveCmdOutput(cmd, saveFile356);
    string hilogInfo = ReadFile(saveFile356);
    bool result = false;
    std::regex pattern(
    "(\\d{4})-(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])\\s(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})(\\.(\\d){0,3})?$");
    std::cout << hilogInfo.substr(0, 23);
    string timeBuffer = hilogInfo.substr(0, 23);
    std::smatch match;
    bool ismatch = regex_match(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout << "match successed"<< std::endl;
        result = true;
    } else {
        std::cout << "match failed"<< std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name show the local time zone
 * @tc.number DFX_DFT_HilogCPP_1010
 * @tc.desc show the local time zone
*/
HWTEST_F(hilogtest, Hilogtool_zone, Function|MediumTest|Level4)
{
    std::string cmd = gHilogtoolExecutable + " -a 1 -v zone";
    std::string saveFile= "/data/local/tmp/test_data_35_7.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::regex pattern(
    "(.*)(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])\\s(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})");
    std::cout << hilogInfo.substr(0, 20);
    string timeBuffer = hilogInfo.substr(0, 20);
    std::smatch match;
    bool ismatch = regex_match(timeBuffer, match, pattern);
    if (ismatch) {
        std::cout << "match successed"<< std::endl;
        result = true;
    } else {
        std::cout << "match failed"<< std::endl;
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Filtering by regular expression, The expression is empty.
 * @tc.number DFX_DFT_HilogCPP_1870
 * @tc.desc Filtering by regular expression, The expression is empty.
*/
HWTEST_F(hilogtest, Hilogtool_regex_null, Function|MediumTest|Level3)
{
    CleanCmd();
    std::string cmd = gHilogtoolExecutable + " -x -e \"\"";
    // " -type app core init -x "
    std::string saveFile= "/data/local/tmp/test_data_46_1.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_regex_null error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Filtering by regular expression
 * @tc.number DFX_DFT_HilogCPP_1880
 * @tc.desc Filtering by regular expression
*/
HWTEST_F(hilogtest, Hilogtool_regex, Function|MediumTest|Level3)
{
    CleanCmd();
    std::string cmd1 = gHilogtoolExecutable + " -x -e \"^(123)\"";

    char saveFile462[] = "/data/local/tmp/test_data_46_2.txt";
    SaveCmdOutput(cmd1, saveFile462);
    string hilogInfo = ReadFile(saveFile462);
    bool result = false;
    std::vector<std::string> para = {"03200/HILOGTOOLTEST"};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_regex error";
    }
    ASSERT_TRUE(true == result);
    CleanCmd();
    std::string cmd2 = gHilogtoolExecutable + " -x -e \"^(www)\"";
    char saveFile463[] = "test_data_46_3.txt";
    SaveCmdOutput(cmd2, saveFile463);
    hilogInfo = ReadFile(saveFile463);
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_regex error";
    }
    ASSERT_TRUE(false == result);
}

/*
 * @tc.name show the first 1 row
 * @tc.number DFX_DFT_HilogCPP_1890
 * @tc.desc show the first 1 row
*/

HWTEST_F(hilogtest, Hilogtool_head_1, Function|MediumTest|Level3)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd = gHilogtoolExecutable + " -a 1 -x ";
    std::string saveFile = "/data/local/tmp/test_data_47_1.txt";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_n error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name show first 20 rows
 * @tc.number DFX_DFT_HilogCPP_1900
 * @tc.desc show first 20 rows
*/
HWTEST_F(hilogtest, Hilogtool_head_20, Function|MediumTest|Level4)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_47_2.txt";
    std::string cmd = gHilogtoolExecutable + " -a 20 -x ";
    SaveCmdOutput(cmd, saveFile);
    int n = GetTxtLine(saveFile);
    ASSERT_TRUE(n == 20);
}

/*
 * @tc.name show the last 1 row
 * @tc.number DFX_DFT_HilogCPP_1910
 * @tc.desc show the last 1 row
*/
HWTEST_F(hilogtest, Hilogtool_tail_1, Function|MediumTest|Level4)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_47_3.txt";
    std::string cmd = gHilogtoolExecutable + "-t init -z 1";
    SaveCmdOutput(cmd, saveFile);
    sleep(2);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_n error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name show last 20 rows
 * @tc.number DFX_DFT_HilogCPP_1920
 * @tc.desc show last 20 rows
*/
HWTEST_F(hilogtest, Hilogtool_tail_20, Function|MediumTest|Level4)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_47_4.txt";
    std::string cmd = gHilogtoolExecutable + "-t init -z 20";
    SaveCmdOutput(cmd, saveFile);
    int n = GetTxtLine(saveFile);
    ASSERT_TRUE(n == 20);
}

/*
 * @tc.name The filtering parameter is empty.
 * @tc.number DFX_DFT_HilogCPP_1930
 * @tc.desc The filtering parameter is empty.
*/
HWTEST_F(hilogtest, Hilogtool_filter_null, Function|MediumTest|Level4)
{
    CleanCmd();
    LogType type = LOG_INIT;

    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1000);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_48_1.txt";
    std::string cmd = gHilogtoolExecutable + " -t init  -x";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_filter error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Filter level only.
 * @tc.number DFX_DFT_HilogCPP_1960
 * @tc.desc Filter level only.
*/

HWTEST_F(hilogtest, Hilogtool_filter_level, Function|MediumTest|Level4)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_48_4.txt";
    std::string cmd = gHilogtoolExecutable + " -t init -L D -x";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_filter error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Filter domain only.
 * @tc.number DFX_DFT_HilogCPP_1970
 * @tc.desc Filter domain only.
*/

HWTEST_F(hilogtest, Hilogtool_filter_domain, Function|MediumTest|Level3)
{
    CleanCmd();
    LogType type = LOG_INIT;

    int cnt =20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_48_3.txt";
    std::string cmd = gHilogtoolExecutable + " -t init -D 0xd003200 -x";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != "") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_filter error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Filter tag only.
 * @tc.number DFX_DFT_HilogCPP_1980
 * @tc.desc Filter tag only.
*/
HWTEST_F(hilogtest, Hilogtool_filter_tag, Function|MediumTest|Level4)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int cnt = 20;
    int i = 0;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_48_5.txt";
    std::string cmd = gHilogtoolExecutable + " -T HILOGTOOLTEST -x";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_filter error";
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name Filter Multi-parameter
 * @tc.number DFX_DFT_HilogCPP_1990
 * @tc.desc Filter Multi-parameter
*/

HWTEST_F(hilogtest, Hilogtool_filter_multiple, Function|MediumTest|Level2)
{
    CleanCmd();
    LogType type = LOG_INIT;
    int i = 0;
    int cnt = 20;
    while (i++ <= cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string saveFile= "/data/local/tmp/test_data_48_6.txt";
    std::string cmd = gHilogtoolExecutable + \
    " -T HILOGTOOLTEST -L D -x";
    SaveCmdOutput(cmd, saveFile);
    string hilogInfo = ReadFile(saveFile);
    bool result = false;
    std::vector<std::string> para = {g_commonContent};
    if (hilogInfo != " ") {
        result = CheckInfo(para, hilogInfo);
    } else {
        std::cout << "Hilogtool_filter error";
    }
    ASSERT_TRUE(true == result);
}
/*
 * @tc.name Logs are not duplicated and are not lost.
 * @tc.number DFX_DFT_HilogCPP_2080
 * @tc.desc Logs are not duplicated and are not lost.
*/
HWTEST_F(hilogtest, Hilogtool_count, Function|MediumTest|Level2)
{
    CleanCmd();
    std::string saveFile= "/data/local/tmp/test_data_30.txt";
    std::string cmd1 = gHilogtoolExecutable + " -r";
    std::string cmdResult;
    CmdRun(cmd1, cmdResult);
    LogType type = LOG_APP;
    int i = 0;
    int cnt = 1000;
    while (i++ < cnt) {
        usleep(1);
        HILOG_DEBUG(type, g_logContent.c_str(), i, 1.00001, 2.333333, "sse", 'a');
    }
    std::string cmd2 = gHilogtoolExecutable + "-t app -T HILOGTOOLTEST -x";
    SaveCmdOutput(cmd2, saveFile);
    ASSERT_TRUE(cnt == GetTxtLine(saveFile));
    ifstream in(saveFile);
    std::string filename;
    std::string line;
    std::vector<string> mVec;
    if (in) {
        while (getline(in, line)) {
            mVec.push_back(line);
        }
    } else {
        std::cout << "no such file" << std::endl;
    }
    sort(mVec.begin(), mVec.end());
    vector<string>::iterator endUnique = unique(mVec.begin(), mVec.end());
    vector<string> vec;
    while (endUnique != mVec.end()) {
        vec.push_back(*endUnique++);
    }
    ASSERT_TRUE(0 == vec.size());
}
/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1020
 * @tc.desc Querying the buffer size of default log types (Ccore and app)
*/
HWTEST_F(hilogtest, buff_size_default, Function|MediumTest|Level2)
{
    result = ExecuteCmd("hilog -g");
    expect = "core buffer size is 1M\napp buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1030
 * @tc.desc Queries the buffer size of a specified type (core).
*/
HWTEST_F(hilogtest, buff_size_core, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -g -t core");
    expect = "core buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1040
 * @tc.desc Queries the buffer size of a specified type (app).
*/
HWTEST_F(hilogtest, buff_size_app, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -g -t app");
    expect = "app buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1050
 * @tc.desc Queries the buffer size of a specified type (init).
*/
HWTEST_F(hilogtest, buff_size_init, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -g -t init");
    expect = "init buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1060
 * @tc.desc Queries the buffer size of multiple types
*/
HWTEST_F(hilogtest, buff_size_multiple, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -g -t 'core app'");
    expect = "core buffer size is 1M\napp buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1070
 * @tc.desc Queries the buffer size of all types
*/
HWTEST_F(hilogtest, buff_size_all, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -g -t all");
    expect = "core buffer size is 1M\napp buffer size is 1M\ninit buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1080
 * @tc.desc Queries the buffer size of a nonexistent type
*/
HWTEST_F(hilogtest, buff_size_illegal, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -g -t abc");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1090
 * @tc.desc Queries the buffer size of multiple types, including nonexistent types.
*/
HWTEST_F(hilogtest, buff_size_illegal2, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -g -t 'core abc'");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1100
 * @tc.desc Set the buffer size without unit. (The default log type is core and app.)
*/
HWTEST_F(hilogtest, buff_resize_byte, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 200");
    expect = "core buffer size is 200B\napp buffer size is 200B\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1110
 * @tc.desc Set the buffer size to **b.
*/
HWTEST_F(hilogtest, buff_resize_byte2, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 200b");
    expect = "core buffer size is 200B\napp buffer size is 200B\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1120
 * @tc.desc Set the buffer size to **B
*/
HWTEST_F(hilogtest, buff_resize_byte3, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 200B");
    expect = "core buffer size is 200B\napp buffer size is 200B\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1130
 * @tc.desc Set the buffer size to **k
*/
HWTEST_F(hilogtest, buff_resize_kbyte, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2k");
    expect = "core buffer size is 2K\napp buffer size is 2K\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1140
 * @tc.desc Set the buffer size to **K
*/
HWTEST_F(hilogtest, buff_resize_kbyte2, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2K");
    expect = "core buffer size is 2K\napp buffer size is 2K\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1150
 * @tc.desc Set the buffer size to **m
*/
HWTEST_F(hilogtest, buff_resize_mbyte, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2m");
    expect = "core buffer size is 2M\napp buffer size is 2M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1160
 * @tc.desc Set the buffer size to **M
*/
HWTEST_F(hilogtest, buff_resize_mbyte2, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2M");
    expect = "core buffer size is 2M\napp buffer size is 2M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1170
 * @tc.desc Set the buffer size to **g
*/
HWTEST_F(hilogtest, buff_resize_gbyte, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 1g");
    expect = "core buffer size is 1G\napp buffer size is 1G\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1180
 * @tc.desc Set the buffer size to **G
*/
HWTEST_F(hilogtest, buff_resize_gbyte2, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 1G");
    expect = "core buffer size is 1G\napp buffer size is 1G\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1190
 * @tc.desc Set the buffer size to 0
*/
HWTEST_F(hilogtest, buff_resize_illegal, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -G 0");
    ret = result.find("Invalid buffer size") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1200
 * @tc.desc Set the buffer size to exceed the maximum length
*/
HWTEST_F(hilogtest, buff_resize_illegal2, Function|MediumTest|Level3)
{
    bool ret = true;
    result = ExecuteCmd("hilog -G 2g");
    ret = result.find("Invalid buffer size") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1210
 * @tc.desc Set the buffer size according to a single log type (core)
*/

HWTEST_F(hilogtest, buff_resize_core, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2m -t core");
    expect = "core buffer size is 2M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1220
 * @tc.desc Set the buffer size according to a single log type (app)
*/
HWTEST_F(hilogtest, buff_resize_app, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2m -t app");
    expect = "app buffer size is 2M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1230
 * @tc.desc Set the buffer size according to a single log type (init)
*/
HWTEST_F(hilogtest, buff_resize_init, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -G 2m -t init");
    expect = "init buffer size is 2M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1240
 * @tc.desc Set the buffer size according to multiple log types
*/
HWTEST_F(hilogtest, buff_resize_multyple, Function|MediumTest|Level2)
{
    result = ExecuteCmd("hilog -G 2m -t 'core app'");
    expect = "core buffer size is 2M\napp buffer size is 2M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1250
 * @tc.desc Set the buffer size of all log types
*/
HWTEST_F(hilogtest, buff_resize_all, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -G 1m -t all");
    expect = "core buffer size is 1M\napp buffer size is 1M\ninit buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1260
 * @tc.desc Set the non-existent log type buffer size
*/
HWTEST_F(hilogtest, buff_resize_illegal_type, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -G 2m -t abc");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1270
 * @tc.desc Set multiple log types, including non-existent log types
*/
HWTEST_F(hilogtest, buff_resize_illegal_type2, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -G 2m -t 'core abc'");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1280
 * @tc.desc Re-query the buffer size after the setting operation
*/
HWTEST_F(hilogtest, buff_resize_check, Function|MediumTest|Level3)
{
    // re query after set
    result = ExecuteCmd("hilog -g -t all");
    expect = "core buffer size is 1M\napp buffer size is 1M\ninit buffer size is 1M\n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1290
 * @tc.desc The size of the core buffer is set to 200 bytes, and the size of logs exceeds 200 bytes.
*/
HWTEST_F(hilogtest, buff_resize_check_core, Function|MediumTest|Level3)
{
    int i = 1;
    ExeCmd("hilog -G 200 -t core");
    ExeCmd("hilog -r -t all");
    i = 1;
    while (i <= 10) {
        HILOG_FATAL(LOG_CORE, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    result = ExecuteCmd("hilog -x -t core |wc -l");
    expect = "10";
    EXPECT_LT(stoi(result), stoi(expect));
    // end recover default
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1300
 * @tc.desc The size of the app buffer is set to 200 bytes, and the size of logs exceeds 200 bytes.
*/
HWTEST_F(hilogtest, buff_resize_check_app, Function|MediumTest|Level4)
{
    int i = 1;
    ExeCmd("hilog -G 200 -t app");
    ExeCmd("hilog -r -t all");
    i = 1;
    while (i <= 10) {
        HILOG_FATAL(LOG_APP, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    result = ExecuteCmd("hilog -x -t app |wc -l");
    expect = "10";
    EXPECT_LT(stoi(result), stoi(expect));
    // end recover default
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name buffer size test
 * @tc.number DFX_DFT_HilogCPP_1310
 * @tc.desc The size of the init buffer is set to 200 bytes, and the size of logs exceeds 200 bytes.
*/
HWTEST_F(hilogtest, buff_resize_check_init, Function|MediumTest|Level4)
{
    int i = 1;
    ExeCmd("hilog -G 200 -t init");
    ExeCmd("hilog -r -t all");
    i = 1;
    while (i <= 10) {
        HILOG_FATAL(LOG_INIT, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    result = ExecuteCmd("hilog -x -t init |wc -l");
    expect = "10";
    EXPECT_LT(stoi(result), stoi(expect));
    // end recover default
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -G 1m -t all");
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1320
 * @tc.desc Query the statistics of the core log
*/
HWTEST_F(hilogtest, statistic_info_query_core, Function|MediumTest|Level2)
{
    bool ret = true;
    int i = 1;
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -S -t core");

    // write and read
    i = 1;
    while (i <= 1000) {
        HILOG_FATAL(LOG_CORE, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    ExeCmd("hilog -x -t core >/dev/null 2>&1");
    result = ExecuteCmd("hilog -s -t core");
    std::regex reg("core print log length is [1-9]\\w+\ncore cache log length is [1-9]\\w+\n\
core dropped log lines is \\w+\n");
    ret = std::regex_match(result, reg);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1330
 * @tc.desc Query the statistics of the app log
*/
HWTEST_F(hilogtest, statistic_info_query_app, Function|MediumTest|Level3)
{
    bool ret = true;
    int i = 1;
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -S -t app");

    // write and read
    i = 1;
    while (i <= 1000) {
        HILOG_FATAL(LOG_APP, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    ExeCmd("hilog -x -t app >/dev/null 2>&1");
    result = ExecuteCmd("hilog -s -t app");
    std::regex reg("app print log length is [1-9]\\w+\napp cache log length is [1-9]\\w+\n\
app dropped log lines is \\w+\n");
    ret = std::regex_match(result, reg);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1340
 * @tc.desc Query the statistics of the init log
*/
HWTEST_F(hilogtest, statistic_info_query_init, Function|MediumTest|Level3)
{
    bool ret = true;
    int i = 1;
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -S -t init");
    // write and read
    i = 1;
    while (i <= 1000) {
        HILOG_FATAL(LOG_INIT, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    ExeCmd("hilog -x -t init >/dev/null 2>&1");
    result = ExecuteCmd("hilog -s -t init");
    std::regex reg("init print log length is [1-9]\\w+\ninit cache log length is [1-9]\\w+\n\
init dropped log lines is \\w+\n");
    ret = std::regex_match(result, reg);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1350
 * @tc.desc Clear the statistics of the core log
*/
HWTEST_F(hilogtest, statistic_info_clear_core, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -S -t core");
    expect = "core statistic info clear success \n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1360
 * @tc.desc Clear the statistics of the app log
*/
HWTEST_F(hilogtest, statistic_info_clear_app, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -S -t app");
    expect = "app statistic info clear success \n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1370
 * @tc.desc Clear the statistics of the init log
*/
HWTEST_F(hilogtest, statistic_info_clear_init, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -S -t init");
    expect = "init statistic info clear success \n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1380
 * @tc.desc Query statistics of non-existent log
*/
HWTEST_F(hilogtest, statistic_info_query_illegal, Function|MediumTest|Level3)
{
    bool ret = true;
    result = ExecuteCmd("hilog -s -t abc");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1390
 * @tc.desc Clear statistics of non-existent log
*/
HWTEST_F(hilogtest, statistic_info_clear_illegal, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -S -t abc");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by log
 * @tc.number DFX_DFT_HilogCPP_1400
 * @tc.desc Re-query after clearing
*/
HWTEST_F(hilogtest, statistic_info_check, Function|MediumTest|Level3)
{
    bool ret = true;
    // re query after clear
    ExeCmd("hilog -S -t core");
    result = ExecuteCmd("hilog -s -t core");
    std::regex reg("core print log length is \\w+\ncore cache log length is \\w+\n\
core dropped log lines is \\w+\n");
    ret = std::regex_match(result, reg);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by domain
 * @tc.number DFX_DFT_HilogCPP_1410
 * @tc.desc Query the statistics of the specified domain
*/
HWTEST_F(hilogtest, statistic_info_query_domain, Function|MediumTest|Level2)
{
    bool ret = true;
    int i = 1;
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -S -D 218116608");
    // write and read
    i = 1;
    while (i <= 1000) {
        HILOG_FATAL(LOG_CORE, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    ExeCmd("hilog -x -t core >/dev/null 2>&1");
    result = ExecuteCmd("hilog -s -D 218116608");
    std::regex reg("218116608 print log length is [1-9]\\w+\n218116608 cache log length is [1-9]\\w+\n\
218116608 dropped log lines is \\w+\n");
    ret = std::regex_match(result, reg);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by domain
 * @tc.number DFX_DFT_HilogCPP_1420
 * @tc.desc Clear the statistics of the specified domain
*/
HWTEST_F(hilogtest, statistic_info_clear_domain, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -S -D 218116608");
    expect = "218116608 statistic info clear success \n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Querying and clearing statistics by domain
 * @tc.number DFX_DFT_HilogCPP_1430
 * @tc.desc Query the statistics of nonexistent domain
*/
HWTEST_F(hilogtest, statistic_info_query_domain2, Function|MediumTest|Level3)
{
    bool ret = true;
    int i = 1;
    ExeCmd("hilog -r -t all");
    ExeCmd("hilog -S -D 218116608");
    // write and read
    i = 1;
    while (i <= 1000) {
        HILOG_FATAL(LOG_CORE, "123456789_1234567890_public and private log test is: \
%{public}d, %{public}lf, %{public}.2f, %{public}s, %{public}c", i, 1.00001, 2.333333, "sse", 'a');
        i++;
    }
    ExeCmd("hilog -x -t core >/dev/null 2>&1");
    result = ExecuteCmd("hilog -s -D 1");
    std::regex reg("1 print log length is \\w+\n1 cache log length is \\w+\n1 dropped log lines is \\w+\n");
    ret = std::regex_match(result, reg);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Querying and clearing statistics by domain
 * @tc.number DFX_DFT_HilogCPP_1440
 * @tc.desc Clear the statistics of nonexistent domain
*/
HWTEST_F(hilogtest, statistic_info_clear_domain2, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -S -D 1");
    expect = "1 statistic info clear success \n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}


/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1630
 * @tc.desc Clear the default logs (core、app)
*/
HWTEST_F(hilogtest, log_clear_default, Function|MediumTest|Level2)
{
    result = ExecuteCmd("hilog -r");
    expect = "core log clear success \napp log clear success \n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1640
 * @tc.desc Clear the core log
*/
HWTEST_F(hilogtest, log_clear_core, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -r -t core");
    expect = "core log clear success \n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1650
 * @tc.desc Clear the app log
*/
HWTEST_F(hilogtest, log_clear_app, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -r -t app");
    expect = "app log clear success \n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1660
 * @tc.desc Clear the init log
*/
HWTEST_F(hilogtest, log_clear_inti, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -r -t init");
    expect = "init log clear success \n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1670
 * @tc.desc Clear multiple types of logs
*/
HWTEST_F(hilogtest, log_clear_multiple, Function|MediumTest|Level3)
{
    result = ExecuteCmd("hilog -r -t 'core app'");
    expect = "core log clear success \napp log clear success \n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1680
 * @tc.desc Clear all types of logs.
*/
HWTEST_F(hilogtest, log_clear_all, Function|MediumTest|Level4)
{
    result = ExecuteCmd("hilog -r -t all");
    expect = "core log clear success \napp log clear success \ninit log clear success \n\n";
    EXPECT_STREQ(result.c_str(), expect.c_str());
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1690
 * @tc.desc Clear nonexistent types of logs
*/
HWTEST_F(hilogtest, log_clear_illegal, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -r -t abc");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name Clearing the logs of the corresponding buffer by log type
 * @tc.number DFX_DFT_HilogCPP_1700
 * @tc.desc Clear multiple types of logs including nonexistent types
*/
HWTEST_F(hilogtest, log_clear_illegal2, Function|MediumTest|Level4)
{
    bool ret = true;
    result = ExecuteCmd("hilog -r -t 'abc core'");
    ret = result.find("Invalid log type") != std::string::npos;
    EXPECT_EQ(ret, true);
}
