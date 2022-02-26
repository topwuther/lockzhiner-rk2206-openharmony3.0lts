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
#include <ostream>
#include <streambuf>
#include <queue>
#include <securec.h>
#include <pthread.h>
#include <sys/time.h>
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"
#include <gtest/gtest.h>
#include "file_utils.h"
using namespace OHOS;
using namespace HiviewDFX;
using namespace testing::ext;
#define MAX_LINE  ((1024) * (10))
#define MAXBUFFSIZE 1024
class LibhilogCPPtest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
};
void LibhilogCPPtest::SetUp()
{
    CleanCmd();
}
void LibhilogCPPtest::TearDown()
{
}
void LibhilogCPPtest::SetUpTestCase()
{
    std::vector<std::string> cmdret;
    string cmd = "setprop persist.sys.hilog.debug.on false";
    ExecCmdWithRet(cmd, cmdret);
    cmd = "setprop hilog.debug.on false";
    ExecCmdWithRet(cmd, cmdret);
}
void LibhilogCPPtest::TearDownTestCase()
{
    std::cout << "TearDownTestCase" << std::endl;
}

/**
 * @tc.name Provides user-mode interfaces（CPP）INFO
 * @tc.number DFX_DFT_HilogCPP_0100
 * @tc.desc Provides user-mode interfaces（CPP）INFO
 */
HWTEST_F(LibhilogCPPtest, HILOG_INFO_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    int i = 1;
    HiLog::Info(a, "123456789_1234567890_publicandprivatelogHWTEST_Fis:%{public}d,"
    "%{private}lf,%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"I 03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST: \
123456789_1234567890_publicandprivatelogHWTEST_Fis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    std::cout<<"cmdRunResult = " + cmdRunResult<<std::endl;
    std::cout<<"expected = " + expected<<std::endl;
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name Provides user-mode interfaces（CPP）debug
 * @tc.number DFX_DFT_HilogCPP_0090
 * @tc.desc Provides user-mode interfaces（CPP）debug
 */
HWTEST_F(LibhilogCPPtest, HILOG_DEBUG_CPP, Function|MediumTest|Level0)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    int i = 1;
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Debug(a, "123456789_1234567890_publicandprivatelogHWTEST_Fis:%{public}d,"
    "%{private}lf,%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"D 03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST: \
123456789_1234567890_publicandprivatelogHWTEST_Fis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -L D -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    std::cout<<"cmdRunResult = " + cmdRunResult<<std::endl;
    std::cout<<"expected = " + expected<<std::endl;
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name Provides user-mode interfaces（CPP）warning
 * @tc.number DFX_DFT_HilogCPP_0130
 * @tc.desc Provides user-mode interfaces（CPP）warning
 */
HWTEST_F(LibhilogCPPtest, HILOG_WARNING_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    int i = 1;
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Warn(a, "123456789_1234567890_publicandprivatelogHWTEST_Fis:%{public}d,%{private}lf,"
    "%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"W 03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST: \
123456789_1234567890_publicandprivatelogHWTEST_Fis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    std::cout<<"cmdRunResult = " + cmdRunResult<<std::endl;
    std::cout<<"expected = " + expected<<std::endl;
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name Provides user-mode interfaces（CPP）error
 * @tc.number DFX_DFT_HilogCPP_0110
 * @tc.desc Provides user-mode interfaces（CPP）error
  */
HWTEST_F(LibhilogCPPtest, HILOG_ERROR_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    int i = 1;
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Error(a, "123456789_1234567890_publicandprivatelogHWTEST_Fis:%{public}d,"
    "%{private}lf,%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"E 03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST: \
123456789_1234567890_publicandprivatelogHWTEST_Fis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    std::cout<<"cmdRunResult = " + cmdRunResult<<std::endl;
    std::cout<<"expected = " + expected<<std::endl;
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name Provides user-mode interfaces（CPP）fatal
 * @tc.number DFX_DFT_HilogCPP_0120
 * @tc.desc Provides user-mode interfaces（CPP）fatal
 */
HWTEST_F(LibhilogCPPtest, HILOG_FATAL_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    int i = 1;
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "123456789_1234567890_publicandprivatelogHWTEST_Fis:%{public}d,"
    "%{private}lf,%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"F 03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST: \
123456789_1234567890_publicandprivatelogHWTEST_Fis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    std::cout<<"cmdRunResult = " + cmdRunResult<<std::endl;
    std::cout<<"expected = " + expected<<std::endl;
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.desc libhilog-log integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2090
 * @tc.desc libhilog-log integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER:%{private}d,%{public}d,%d;", 1, 1, 1);
    std::string expected{"INTEGER:<private>,1,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log long integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2100
 * @tc.desc libhilog-log long integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_LONG_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_long:%{private}ld,%{public}ld,%ld;", 2147483647L, 2147483647L, 2147483647L);
    std::string expected{"INTEGER_long:<private>,2147483647,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log fixed-length integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2110
 * @tc.desc libhilog-log fixed-length integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_4_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_4:%{private}4d,%{public}4d,%4d;", 2000, 2000, 2000);
    std::string expected{"INTEGER_4:<private>,2000,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log short integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2120
 * @tc.desc libhilog-log short integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_SHORT_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_short:%{private}hd,%{public}hd,%hd;", (short)1024, (short)1024, (short)1024);
    std::string expected{"INTEGER_short:<private>,1024,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2130
 * @tc.desc libhilog-log unsigned integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_UN_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_un:%{private}u,%{public}u,%u;", 2147483647u, 2147483647u, 2147483647u);
    std::string expected{"INTEGER_un:<private>,2147483647,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned long integer type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2140
 * @tc.desc libhilog-log unsigned long integer type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_LONG_UN_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_long_un:%{private}lu,%{public}lu,%lu;", 7483647ul, 2147483647ul, 2147483647ul);
    std::string expected{"INTEGER_long_un:<private>,2147483647,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned fixed-length integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2150
 * @tc.desc libhilog-log unsigned fixed-length integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_4_UN_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_4_un:%{private}4u,%{public}4u,%4u;", 4000, 4000, 4000);
    std::string expected{"INTEGER_4_un:<private>,4000,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned short integer type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2160
 * @tc.desc libhilog-log unsigned short integer type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_INTEGER_SHORT_UN_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "INTEGER_short_un:%{private}hu,%{public}hu,%hu;", (unsigned short)65535,
    (unsigned short)65535, (unsigned short)65535);
    std::string expected{"INTEGER_short_un:<private>,65535,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log float type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2170
 * @tc.desc libhilog-log float type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_FLOAT_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "FLOAT:%{private}f,%{public}f,%f;", 1.01, 1.01, 1.01);
    std::string expected{"FLOAT:<private>,1.010000,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log long float type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2180
 * @tc.desc libhilog-log long float type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_FLOAT_LONG_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "FLOAT_long:%{private}lf,%{public}lf,%lf;", 2.147483647, 2.147483647, 2.147483647);
    std::string expected{"FLOAT_long:<private>,2.147484,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting of fixed-width floating-point identifiers in
 *          the decimal part of the log ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2190
 * @tc.desc libhilog-Formatting of fixed-width floating-point identifiers in
 *          the decimal part of the log ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_FLOAT_POINT2_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "FLOAT_.2:%{private}.2f,%{public}.2f,%.2f;", 2.147483647, 2.147483647, 2.147483647);
    std::string expected{"FLOAT_.2:<private>,2.15,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting of floating-point identifiers for
 *          the decimal part and integer part of the log ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2200
 * @tc.desc libhilog-Formatting of floating-point identifiers for
 *          the decimal part and integer part of the log ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_FLOAT_3POINT2_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "FLOAT_3.2:%{private}3.2f,%{public}4.1f,%2.6f;", 32.147483647, 321.147483647, 23.147483647);
    std::string expected{"FLOAT_3.2:<private>,321.1,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log characters ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2210
 * @tc.desc libhilog-Formatting log characters ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_CHAR_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "CHAR:%{private}c,%{public}c,%c;", 'a', 'b', 'c');
    std::string expected{"CHAR:<private>,b,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log octal identifier ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2250
 * @tc.desc libhilog-Formatting log octal identifier ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_OCTAL_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "OCTAL:%{private}o,%{public}o,%o;", 15, 16, 17);
    std::string expected{"OCTAL:<private>,20,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log long octal identifier ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2260
 * @tc.desc libhilog-Formatting log long octal identifier ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_OCTAL_LONG_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "OCTAL_long:%{private}lo,%{public}lo,%lo;", 18ul, 19ul, 20ul);
    std::string expected{"OCTAL_long:<private>,23,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log hexadecimal identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2270
 * @tc.desc libhilog-Formatting log hexadecimal identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_HEX_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "HEX:%{private}x,%{public}x,%x;", 15, 16, 17);
    std::string expected{"HEX:<private>,10,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs with prefixes in hexadecimal format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2280
 * @tc.desc libhilog-Logs with prefixes in hexadecimal format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_HEX_UPPER_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "HEX_#:%{private}#x,%{public}#x,%#x;", 18, 19, 20);
    std::string expected{"HEX_#:<private>,0x13,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2290
 * @tc.desc libhilog-Logs are formatted with long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_HEX_LONG_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "HEX_long:%{private}lx,%{public}lx,%lx;", 21l, 22l, 23l);
    std::string expected{"HEX_long:<private>,16,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with uppercase hexadecimal flags
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2300
 * @tc.desc libhilog-Logs are formatted with uppercase hexadecimal flags
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_HEX_X_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "HEX_X:%{private}X,%{public}X,%X;", 24u, 25u, 26u);
    std::string expected{"HEX_X:<private>,19,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with uppercase and prefixes in hexadecimal format
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2310
 * @tc.desc libhilog-Logs are formatted with uppercase and prefixes in hexadecimal format
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_HEX_UPPER_X_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "HEX_#X:%{private}#X,%{public}#X,%#X;", 27, 28, 28);
    std::string expected{"HEX_#X:<private>,0X1C,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with uppercase long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2320
 * @tc.desc libhilog-Logs are formatted with uppercase long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_HEX_LONG_UPPER_X_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "HEX_long_X:%{private}lX,%{public}lX,%lX;", 30ul, 31ul, 32ul);
    std::string expected{"HEX_long_X:<private>,1F,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith string identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2220
 * @tc.desc libhilog-Formatting Log whith string identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_STR_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "STRING:%{private}s,%{public}s,%s;", "STRING1", "STRING2", "STRING3");
    std::string expected{"STRING:<private>,STRING2,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith empty string identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2240
 * @tc.desc libhilog-Formatting Log whith empty string identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_STR_EMPTY_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "STRING_empty:%{private}s,%{public}s,%s;", "", "", "");
    std::string expected{"STRING_empty:<private>,,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith Chinese identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2230
 * @tc.desc libhilog-Formatting Log whith Chinese identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_STR_CHINESE_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "STRING_Chinese:%{private}s,%{public}s,%s;", "中文", "中文", "中文");
    std::string expected{"STRING_Chinese:<private>,中文,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith scientific notation identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2330
 * @tc.desc libhilog-Formatting Log whith scientific notation identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_E_DOUBLE_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "DOUBLE_e:%{private}e,%{public}e,%e;", 1e-30, 2.231e10, 3.999e-13);
    std::string expected{"DOUBLE_e:<private>,2.231000e+10,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith scientific notation capitalized identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2340
 * @tc.desc libhilog-Formatting Log whith scientific notation capitalized identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_E_UPPER_DOUBLE_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "DOUBLE_E:%{private}E,%{public}E,%E;", 4.88E2, 5.676767e-2, 6.17E13);
    std::string expected{"DOUBLE_E:<private>,5.676767E-02,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith scientific notation(%g AUTO) identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2350
 * @tc.desc libhilog-Formatting Log whith scientific notation(%g AUTO) identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_E_AUTO_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "AUTO:%{private}g,%{public}g,%g;", 1e-30, 2.231e10, 3.999e-13);
    std::string expected{"AUTO:<private>,2.231e+10,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-libhilog-Formatting Log whith scientific notation(%g AUTO) capitalized
 *          identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogCPP_2360
 * @tc.desc libhilog-libhilog-Formatting Log whith scientific notation(%g AUTO) capitalized
 *          identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCPPtest, HILOG_E_UPPER_AUTO_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "AUTO_E:%{private}G,%{public}G,%G;", 4.88E2, 5.676767e-2, 6.17E13);
    std::string expected{"AUTO_E:<private>,0.0567677,<private>;"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-The {private} flag parameter of mixed log formatting is hidden
 * @tc.number DFX_DFT_HilogCPP_31
 * @tc.desc libhilog-The {private} flag parameter of mixed log formatting is hidden
 */
HWTEST_F(LibhilogCPPtest, HILOG_PRIVATE_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "private:%{private}d,%{private}lf,%{private}.2f,%{private}s,%{private}c",
    1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"private:<private>,<private>,<private>,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-The parameters identified by {public} in mixed log formatting are displayed
 * @tc.number DFX_DFT_HilogCPP_31
 * @tc.desc libhilog-The parameters identified by {public} in mixed log formatting are displayed
 */
HWTEST_F(LibhilogCPPtest, HILOG_PUBLIC_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "public:%{public}d,%{public}lf,%{public}.2f,%{public}s,%{public}c",
    1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"public:1,1.000010,2.33,sse,a"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Parameters without any privacy flag are not displayed in mixed log formatting
 * @tc.number DFX_DFT_HilogCPP_31
 * @tc.desc libhilog-Parameters without any privacy flag are not displayed in mixed log formatting
 */
HWTEST_F(LibhilogCPPtest, HILOG_NO_SIGN_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "no_sign:%d,%lf,%.2f,%s,%c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"no_sign:<private>,<private>,<private>,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Log privacy flags are used together, and parameters are
 *          correctly displayed or hidden
 * @tc.number DFX_DFT_HilogCPP_31
 * @tc.desc libhilog-Log privacy flags are used together, and parameters are
 *          correctly displayed or hidden
 */
HWTEST_F(LibhilogCPPtest, HILOG_MIX_CPP, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tag = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tag;
    HiLog::Fatal(a, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"MIX:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log are not truncated with normal length tag
 * @tc.number DFX_DFT_HilogCPP_0280
 * @tc.desc libhilog-log are not truncated with normal length tag
 */
HWTEST_F(LibhilogCPPtest, TAG_CHECK_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tagNormal = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tagNormal;
    HiLog::Fatal(a, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST:"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log are truncated with too long tag
 * @tc.number DFX_DFT_HilogCPP_0290
 * @tc.desc libhilog-log are truncated with too long tag
 */
HWTEST_F(LibhilogCPPtest, TAG_OVER_CHECK_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd003200;
    const char *tagToolong = "HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0"
    "HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0HWTEST_Ftag0";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tagToolong;
    HiLog::Fatal(a, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST_:"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST_", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log domain
 * @tc.number DFX_DFT_HilogCPP_0270
 * @tc.desc libhilog-log domain
 */
HWTEST_F(LibhilogCPPtest, DOMAIN_CHECK_CPP, Function|MediumTest|Level1)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    const unsigned int domain = 0xd001111;
    const char *tagNormal = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tagNormal;
    HiLog::Fatal(a, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"01111/HWTEST_Ftag0HWTEST_Ftag0HWTEST:"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log LOG_APP type test
 * @tc.number DFX_DFT_HilogCPP_0280
 * @tc.desc libhilog-log LOG_APP type test
 */
HWTEST_F(LibhilogCPPtest, TYPE_APP_CHECK_CPP, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_APP;
    const unsigned int domain = 0xd003200;
    const char *tagNormal = "HWTEST_Ftag0HWTEST_Ftag0HWTEST";
    HiLogLabel a;
    a.type = type;
    a.domain = domain;
    a.tag = tagNormal;
    HiLog::Fatal(a, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"03200/HWTEST_Ftag0HWTEST_Ftag0HWTEST:"};
    CmdRun("hilog -x | grep HWTEST_Ftag0HWTEST_Ftag0HWTEST", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}
