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

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD003E00
#define LOG_TAG "testtag0testtag0testtag0testta"
using namespace std;
using namespace testing::ext;

class LibhilogCtest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
};
void LibhilogCtest::SetUp()
{
    CleanCmd();
}
void LibhilogCtest::TearDown()
{
}
void LibhilogCtest::SetUpTestCase()
{
    std::vector<std::string> cmdret;
    string cmd = "setprop persist.sys.hilog.debug.on false";
    ExecCmdWithRet(cmd, cmdret);
    cmd = "setprop hilog.debug.on false";
    ExecCmdWithRet(cmd, cmdret);
}
void LibhilogCtest::TearDownTestCase()
{
    std::cout << "TearDownTestCase" << std::endl;
}

/**
 * @tc.name Provides user-mode interfaces（C）INFO
 * @tc.number DFX_DFT_HilogC_0180
 * @tc.desc Provides user-mode interfaces（C）INFO
 */
HWTEST_F(LibhilogCtest, HILOG_INFO, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    int i = 1;
    HILOG_INFO(type, "123456789_1234567890_publicandprivatelogtestis:%{public}d,"
    "%{private}lf,%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"I 03e00/testtag0testtag0testtag0testta: \
123456789_1234567890_publicandprivatelogtestis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    std::cout<<"cmdRunResult = " + cmdRunResult<<std::endl;
    std::cout<<"expected = " + expected<<std::endl;
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.desc Provides user-mode interfaces（C）debug
 * @tc.number DFX_DFT_HilogC_0170
 * @tc.desc Provides user-mode interfaces（C）debug
 */
HWTEST_F(LibhilogCtest, HILOG_DEBUG, Function|MediumTest|Level1)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    int i = 1;
    HILOG_DEBUG(type, "123456789_1234567890_publicandprivatelogtestis:%{public}d,%{private}lf,"
    "%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"D 03e00/testtag0testtag0testtag0testta: \
123456789_1234567890_publicandprivatelogtestis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name Provides user-mode interfaces（C）warning
 * @tc.number DFX_DFT_HilogC_0210
 * @tc.desc Provides user-mode interfaces（C）warning
 */
HWTEST_F(LibhilogCtest, HILOG_WARNING, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    int i = 1;
    HILOG_WARN(type, "123456789_1234567890_publicandprivatelogtestis:%{public}d,"
    "%{private}lf,%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"W 03e00/testtag0testtag0testtag0testta: \
123456789_1234567890_publicandprivatelogtestis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.desc Provides user-mode interfaces（C）error
 * @tc.number DFX_DFT_HilogC_0190
 * @tc.desc Provides user-mode interfaces（C）error
 */
HWTEST_F(LibhilogCtest, HILOG_ERROR, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    int i = 1;
    HILOG_ERROR(type, "123456789_1234567890_publicandprivatelogtestis:%{public}d,%{private}lf,"
    "%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"E 03e00/testtag0testtag0testtag0testta: \
123456789_1234567890_publicandprivatelogtestis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name Provides user-mode interfaces（C）fatal
 * @tc.number DFX_DFT_HilogC_0200
 * @tc.desc Provides user-mode interfaces（C）fatal
 */
HWTEST_F(LibhilogCtest, HILOG_FATAL, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    int i = 1;
    HILOG_FATAL(type, "123456789_1234567890_publicandprivatelogtestis:%{public}d,%{private}lf,"
    "%{public}.2f,%s,%{private}c", i, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"F 03e00/testtag0testtag0testtag0testta: \
123456789_1234567890_publicandprivatelogtestis:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0360
 * @tc.desc libhilog-log integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER:%{private}d,%{public}d,%d;", 1, 1, 1);
    std::string expected{"INTEGER:<private>,1,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log long integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0370
 * @tc.desc libhilog-log long integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_LONG, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_long:%{private}ld,%{public}ld,%ld;", 2147483647L, 2147483647L, 2147483647L);
    std::string expected{"INTEGER_long:<private>,2147483647,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log fixed-length integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0380
 * @tc.desc libhilog-log fixed-length integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_4, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_4:%{private}4d,%{public}4d,%4d;", 2000, 2000, 2000);
    std::string expected{"INTEGER_4:<private>,2000,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log short integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0390
 * @tc.desc libhilog-log short integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_SHORT, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_short:%{private}hd,%{public}hd,%hd;", (short)1024, (short)1024, (short)1024);
    std::string expected{"INTEGER_short:<private>,1024,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0400
 * @tc.desc libhilog-log unsigned integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_UN, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_un:%{private}u,%{public}u,%u;", 2147483647u, 2147483647u, 2147483647u);
    std::string expected{"INTEGER_un:<private>,2147483647,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned long integer type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0410
 * @tc.desc libhilog-log unsigned long integer type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_LONG_UN, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_long_un:%{private}lu,%{public}lu,%lu;", 7483647lu, 2147483647lu, 2147483647lu);
    std::string expected{"INTEGER_long_un:<private>,2147483647,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned fixed-length integer format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0420
 * @tc.desc libhilog-log unsigned fixed-length integer format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_4_UN, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_4_un:%{private}4u,%{public}4u,%4u;", 4000u, 4000u, 4000u);
    std::string expected{"INTEGER_4_un:<private>,4000,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log unsigned short integer type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0430
 * @tc.desc libhilog-log unsigned short integer type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_INTEGER_SHORT_UN, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "INTEGER_short_un:%{private}hu,%{public}hu,%hu;",
    (unsigned short)65535, (unsigned short)65535, (unsigned short)65535);
    std::string expected{"INTEGER_short_un:<private>,65535,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log float type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0440
 * @tc.desc libhilog-log float type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_FLOAT, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "FLOAT:%{private}f,%{public}f,%f;", 1.01, 1.01, 1.01);
    std::string expected{"FLOAT:<private>,1.010000,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log long float type identifier format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0450
 * @tc.desc libhilog-log long float type identifier format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_FLOAT_LONG, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "FLOAT_long:%{private}lf,%{public}lf,%lf;", 2.147483647, 2.147483647, 2.147483647);
    std::string expected{"FLOAT_long:<private>,2.147484,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting of fixed-width floating-point identifiers in
 *          the decimal part of the log ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0460
 * @tc.desc libhilog-Formatting of fixed-width floating-point identifiers in
 *          the decimal part of the log ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_FLOAT_POINT2, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "FLOAT_.2:%{private}.2f,%{public}.2f,%.2f;", 2.147483647, 2.147483647, 2.147483647);
    std::string expected{"FLOAT_.2:<private>,2.15,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting of floating-point identifiers for
 *          the decimal part and integer part of the log ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0470
 * @tc.desc libhilog-Formatting of floating-point identifiers for
 *          the decimal part and integer part of the log ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_FLOAT_3POINT2, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "FLOAT_3.2:%{private}3.2f,%{public}4.1f,%2.6f;", 32.147483647, 321.147483647, 23.147483647);
    std::string expected{"FLOAT_3.2:<private>,321.1,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log characters ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0480
 * @tc.desc libhilog-Formatting log characters ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_CHAR, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "CHAR:%{private}c,%{public}c,%c;", 'a', 'b', 'c');
    std::string expected{"CHAR:<private>,b,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log octal identifier ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0520
 * @tc.desc libhilog-Formatting log octal identifier ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_OCTAL, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "OCTAL:%{private}o,%{public}o,%o;", 15, 16, 17);
    std::string expected{"OCTAL:<private>,20,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log long octal identifier ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0530
 * @tc.desc libhilog-Formatting log long octal identifier ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_OCTAL_LONG, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "OCTAL_long:%{private}lo,%{public}lo,%lo;", 022l, 023l, 024l);
    std::string expected{"OCTAL_long:<private>,23,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting log hexadecimal identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0540
 * @tc.desc libhilog-Formatting log hexadecimal identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_HEX, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "HEX:%{private}x,%{public}x,%x;", 0x0F, 0x10, 0x11);
    std::string expected{"HEX:<private>,10,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs with prefixes in hexadecimal format ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0550
 * @tc.desc libhilog-Logs with prefixes in hexadecimal format ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_HEX_UPPER, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "HEX_#:%{private}#x,%{public}#x,%#x;", 0x12, 0x13, 0x14);
    std::string expected{"HEX_#:<private>,0x13,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0560
 * @tc.desc libhilog-Logs are formatted with long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_HEX_LONG, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "HEX_long:%{private}lx,%{public}lx,%lx;", 0x15l, 0x16l, 0x17l);
    std::string expected{"HEX_long:<private>,16,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with uppercase hexadecimal flags
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0570
 * @tc.desc libhilog-Logs are formatted with uppercase hexadecimal flags
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_HEX_X, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "HEX_X:%{private}X,%{public}X,%X;", 0x18, 0x19, 0x1A);
    std::string expected{"HEX_X:<private>,19,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with uppercase and prefixes in hexadecimal format
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0580
 * @tc.desc libhilog-Logs are formatted with uppercase and prefixes in hexadecimal format
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_HEX_UPPER_X, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "HEX_#X:%{private}#X,%{public}#X,%#X;", 0x1B, 0x1C, 0x1C);
    std::string expected{"HEX_#X:<private>,0X1C,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Logs are formatted with uppercase long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0590
 * @tc.desc libhilog-Logs are formatted with uppercase long hexadecimal identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_HEX_LONG_UPPER_X, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "HEX_long_X:%{private}lX,%{public}lX,%lX;", 0x1El, 0x1Fl, 0x20l);
    std::string expected{"HEX_long_X:<private>,1F,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith string identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0490
 * @tc.desc libhilog-Formatting Log whith string identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_STR, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "STRING:%{private}s,%{public}s,%s;", "STRING1", "STRING2", "STRING3");
    std::string expected{"STRING:<private>,STRING2,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith empty string identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0510
 * @tc.desc libhilog-Formatting Log whith empty string identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_STR_EMPTY, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "STRING_empty:%{private}s,%{public}s,%s;", "", "", "");
    std::string expected{"STRING_empty:<private>,,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith Chinese identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0500
 * @tc.desc libhilog-Formatting Log whith Chinese identifiers ({public}, {private}, no identifier)
 */

HWTEST_F(LibhilogCtest, HILOG_STR_CHINESE, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "STRING_Chinese:%{private}s,%{public}s,%s;", "中文", "中文", "中文");
    std::string expected{"STRING_Chinese:<private>,中文,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith scientific notation identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0600
 * @tc.desc libhilog-Formatting Log whith scientific notation identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_E_DOUBLE, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "DOUBLE_e:%{private}e,%{public}e,%e;", 1e-30, 2.231e10, 3.999e-13);
    std::string expected{"DOUBLE_e:<private>,2.231000e+10,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith scientific notation capitalized identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0610
 * @tc.desc libhilog-Formatting Log whith scientific notation capitalized identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_E_UPPER_DOUBLE, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "DOUBLE_E:%{private}E,%{public}E,%E;", 4.88E2, 5.676767e-2, 6.17E13);
    std::string expected{"DOUBLE_E:<private>,5.676767E-02,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Formatting Log whith scientific notation(%g AUTO) identifiers
 *          ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0620
 * @tc.desc libhilog-Formatting Log whith scientific notation(%g AUTO) identifiers
 *          ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_E_AUTO, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "AUTO:%{private}g,%{public}g,%g;", 1e-30, 2.231e10, 3.999e-13);
    std::string expected{"AUTO:<private>,2.231e+10,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-libhilog-Formatting Log whith scientific notation(%g AUTO) capitalized
 *          identifiers ({public}, {private}, no identifier)
 * @tc.number DFX_DFT_HilogC_0630
 * @tc.desc libhilog-libhilog-Formatting Log whith scientific notation(%g AUTO) capitalized
 *          identifiers ({public}, {private}, no identifier)
 */
HWTEST_F(LibhilogCtest, HILOG_E_UPPER_AUTO, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "AUTO_E:%{private}G,%{public}G,%G;", 4.88E2, 5.676767e-2, 6.17E13);
    std::string expected{"AUTO_E:<private>,0.0567677,<private>;"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-The {private} flag parameter of mixed log formatting is hidden
 * @tc.number DFX_DFT_HilogC_3100
 * @tc.desc libhilog-The {private} flag parameter of mixed log formatting is hidden
 */
HWTEST_F(LibhilogCtest, HILOG_PRIVATE, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "private:%{private}d,%{private}lf,%{private}.2f,"
    "%{private}s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"private:<private>,<private>,<private>,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-The parameters identified by {public} in mixed log formatting are displayed
 * @tc.number DFX_DFT_HilogC_3200
 * @tc.desc libhilog-The parameters identified by {public} in mixed log formatting are displayed
 */
HWTEST_F(LibhilogCtest, HILOG_PUBLIC, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "public:%{public}d,%{public}lf,%{public}.2f,%{public}s,%{public}c",
    1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"public:1,1.000010,2.33,sse,a"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Parameters without any privacy flag are not displayed in mixed log formatting
 * @tc.number DFX_DFT_HilogC_3300
 * @tc.desc libhilog-Parameters without any privacy flag are not displayed in mixed log formatting
 */
HWTEST_F(LibhilogCtest, HILOG_NO_SIGN, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "no_sign:%d,%lf,%.2f,%s,%c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"no_sign:<private>,<private>,<private>,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-Log privacy flags are used together, and parameters are
 *          correctly displayed or hidden
 * @tc.number DFX_DFT_HilogC_3400
 * @tc.desc libhilog-Log privacy flags are used together, and parameters are
 *          correctly displayed or hidden
 */
HWTEST_F(LibhilogCtest, HILOG_MIX, Function|MediumTest|Level3)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"MIX:1,<private>,2.33,<private>,<private>"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log are not truncated with normal length tag
 * @tc.number DFX_DFT_HilogC_0280
 * @tc.desc libhilog-log are not truncated with normal length tag
 */
HWTEST_F(LibhilogCtest, TAG_CHECK, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
    HILOG_FATAL(type, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"03e00/testtag0testtag0testtag0testta:"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log are not truncated with normal length tag
 * @tc.number DFX_DFT_HilogC_0280
 * @tc.desc libhilog-log are not truncated with normal length tag
 */
HWTEST_F(LibhilogCtest, TYPE_APP_CHECK, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_APP;
    HILOG_FATAL(type, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"03e00/testtag0testtag0testtag0testta:"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testta", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log are truncated with too long tag
 * @tc.number DFX_DFT_HilogC_0290
 * @tc.desc libhilog-log are truncated with too long tag
 */
HWTEST_F(LibhilogCtest, TAG_OVER_CHECK, Function|MediumTest|Level2)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
#undef LOG_TAG
#define LOG_TAG "testtag0testtag0testtag0testtag0testtag0testtag0testtag0testtag0testtag0testtag0testtag0testtag0"
    HILOG_FATAL(type, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"03e00/testtag0testtag0testtag0testtag:"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testtag", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}

/**
 * @tc.name libhilog-log domain
 * @tc.number DFX_DFT_HilogC_0270
 * @tc.desc libhilog-log domain
 */
HWTEST_F(LibhilogCtest, DOMAIN_CHECK, Function|MediumTest|Level1)
{
    std::string cmdRunResult;
    LogType type = LOG_CORE;
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD001111
    HILOG_FATAL(type, "MIX:%{public}d,%{private}lf,%{public}.2f,%s,%{private}c", 1, 1.00001, 2.333333, "sse", 'a');
    std::string expected{"01111/testtag0testtag0testtag0testtag:"};
    CmdRun("hilog -x| grep testtag0testtag0testtag0testtag", cmdRunResult);
    ASSERT_TRUE(cmdRunResult.find(expected) != std::string::npos);
}