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
#include "hilog/log.h"
#include <gtest/gtest.h>

#include "file_utils.h"
#include "hisysevent.h"

using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;
namespace {
string g_key = "key";
string g_reDiRectTimeout = "5";
string g_hiLogRedirect = "/data/local/tmp/hilogredirect.log";
}
class HiSysEventCPPTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void RedHiSysEventLog(std::string &hilogredirect, std::string &timeout);
private:
};
void HiSysEventCPPTest::SetUp()
{
    std::cout << "SetUp" << std::endl;
    std::vector<std::string> cmdret;
    string cmd = "hilog -r";
    ExecCmdWithRet(cmd, cmdret);
    cmd = "setprop persist.sys.hilog.debug.on false";
    ExecCmdWithRet(cmd, cmdret);
}
void HiSysEventCPPTest::TearDown()
{
    std::cout << "TearDown" << std::endl;
    std::vector<std::string> cmdret;
    string cmd = "rm " + g_hiLogRedirect;
    ExecCmdWithRet(cmd, cmdret);
}
void HiSysEventCPPTest::SetUpTestCase()
{
    std::cout << "SetUpTestCase" << std::endl;
    std::vector<std::string> cmdret;
    string cmd = "mkdir /data/local/tmp/";
    ExecCmdWithRet(cmd, cmdret);
}
void HiSysEventCPPTest::TearDownTestCase()
{
    std::cout << "TearDownTestCase" << std::endl;
}
void HiSysEventCPPTest::RedHiSysEventLog(std::string &hilogredirect, std::string &timeout)
{
    unsigned long i;
    std::vector<std::string> cmdret;
    unsigned long cmdretlen;
    std::string cmd = "rm " + hilogredirect;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    for (i = 0; i < cmdretlen; i++) {
        std::cout<<cmdret[i].c_str()<<std::endl;
    }
    cmd = "timeout " + timeout + " hilog | grep HISYSEVENT >" + hilogredirect;
    std::cout<<cmd<<std::endl;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    for (i = 0; i < cmdretlen; i++) {
        std::cout<<cmdret[i].c_str()<<std::endl;
    }
}
/**
* @tc.name HiSysEvent Native Write Interface Test, Reported When the KeyValue Is of the
*       boolean Type and the bool Value Is False
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0100
* @tc.desc The keyvalue is of the boolean type and the bool value is false.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0100, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0100 start" << endl;
    bool result = false;
    bool param = false;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo;
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"AAFWK", "eventNameDemo", "\"type_\":1", "\"key\":0"};
    if (!fileinfo.empty()) {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0100 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0100 end" << endl;
}
/**
* @tc.name HiSysEvent Native Write Interface Test, Reported When the KeyValue Is of
*       the boolean Type and the bool Value Is False
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0200
* @tc.desc The keyvalue is of the boolean type and the bool value is true.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0200, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0200 start" << endl;
    bool result = false;
    bool param = true;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::APPEXECFWK;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"APPEXECFWK", "eventNameDemo", "\"type_\":1", "\"key\":1"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0200 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0200 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Keyvalue of the Boolean List Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0300
* @tc.desc The value of keyvalue is a boolean list.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0300, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0300 start" << endl;
    bool result = false;
    bool param = false;
    std::vector<bool> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::ACCOUNT;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"ACCOUNT", "eventNameDemo", "\"type_\":1", "\"key\":[0]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0300 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0300 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Key Value of the Char Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0400
* @tc.desc The keyvalue is of the char type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0400, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0400 start" << endl;
    bool result = false;
    char param = 'a';
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::OTHERS;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"OTHERS", "eventNameDemo", "\"type_\":1", "\"key\":97"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0400 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0400 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Keyvalue of the Char List Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0500
* @tc.desc keyvalue is of the char list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0500, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0500 start" << endl;
    bool result = false;
    char param = 'a';
    std::vector<char> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::WEARABLE;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"WEARABLE", "eventNameDemo", "\"type_\":1", "\"key\":[97]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0500 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0500 end" << endl;
}
/**
* @tc.name HiSysEvent Natvie Write Interface Test, Reported When the KeyValue Is of the Double Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0600
* @tc.desc The keyvalue is of the double type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0600, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0600 start" << endl;
    bool result = false;
    double param = 30949.374;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::WEARABLE_HARDWARE;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"WEARABLEHW", "eventNameDemo", "\"type_\":1", "\"key\":30949.4"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0600 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0600 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Keyvalue of the Char List Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0700
* @tc.desc keyvalue is of the char list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0700, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0700 start" << endl;
    bool result = false;
    double param = 30949.374;
    std::vector<double> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::USB;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"USB", "eventNameDemo", "\"type_\":1", "\"key\":[30949.4]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0700 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0700 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Key Value of the Floating Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0800
* @tc.desc The keyvalue is of the float type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0800, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0800 start" << endl;
    bool result = false;
    float param = 230.47;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::UPDATE;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"UPDATE", "eventNameDemo", "\"type_\":1", "\"key\":230.47"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0800 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0800 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Keyvalue of the Char List Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_0900
* @tc.desc keyvalue is of the char list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_0900, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0900 start" << endl;
    bool result = false;
    float param = 230.47;
    std::vector<float> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::TELEPHONY;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"TELEPHONY", "eventNameDemo", "\"type_\":1", "\"key\":[230.47]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_0900 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0900 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Keyvalue of the Int Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1000
* @tc.desc The keyvalue is of the int type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1000, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1000 start" << endl;
    bool result = false;
    int param = 100;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::STARTUP;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"STARTUP", "eventNameDemo", "\"type_\":1", "\"key\":100"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1000 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1000 end" << endl;
}
/**
* @tc.name HiSysEvent Natvie Write Interface Test, Reporting of the int List of KeyValues
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1100
* @tc.desc keyvalue is of the int list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1100, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1100 start" << endl;
    bool result = false;
    int param = 100;
    std::vector<int> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::SOURCE_CODE_TRANSFORMER;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"SRCTRANSFORMER", "eventNameDemo", "\"type_\":1", "\"key\":[100]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1100 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1100 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Key Value of the Long Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1200
* @tc.desc The keyvalue is of the long type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1200, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1200 start" << endl;
    bool result = false;
    long param = 1000000;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::SENSORS;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"SENSORS", "eventNameDemo", "\"type_\":1", "\"key\":1000000"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1200 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1200 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the List of the Long Key Value
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1300
* @tc.desc The keyvalue is of the long list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1300, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1300 start" << endl;
    bool result = false;
    long param = 1000000;
    std::vector<long> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::SECURITY;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"SECURITY", "eventNameDemo", "\"type_\":1", "\"key\":[1000000]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1300 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1300 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Key Value of the Short Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1400
* @tc.desc The keyvalue is of the short type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1400, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0250 start" << endl;
    bool result = false;
    short param = 10;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::ROUTER;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::FAULT;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"ROUTER", "eventNameDemo", "\"type_\":1", "\"key\":10"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1400 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1400 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the List of Short Key Values
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1500
* @tc.desc The keyvalue is of the short list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1500, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1500 start" << endl;
    bool result = false;
    short param = 10;
    std::vector<short> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::POWERMGR;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"POWERMGR", "eventNameDemo", "\"type_\":2", "\"key\":[10]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1500 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1500 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the Key Value of the String Type
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1600
* @tc.desc The keyvalue is of the string type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1600, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1600 start" << endl;
    bool result = false;
    string param = "abc";
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::NOTIFICATION;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"NOTIFICATION", "eventNameDemo", "\"type_\":2", "\"key\":\"abc\""};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1600 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1600 end" << endl;
}
/**
* @tc.name Testing the HiSysEvent Natvie Write Interface, Reporting the List of Short Key Values
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1700
* @tc.desc The keyvalue is of the short list type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1700, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1700 start" << endl;
    bool result = false;
    string param = "abc";
    std::vector<string> test;
    test.push_back(param);
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::MULTI_MODAL_INPUT;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::SECURITY;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"MULTIMODALINPUT", "eventNameDemo", "\"type_\":3", "\"key\":[\"abc\"]"};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1700 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1700 end" << endl;
}
/**
* @tc.name HiSysEvent Natvie Write Interface Test, Reporting the List of 20 List Parameters When the Key Value Is String
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_1800
* @tc.desc The keyvalue is reported as a string list. There are 20 list parameters.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_1800, Function|MediumTest|Level4)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_0290 start" << endl;
    bool result = false;
    string param = "abc";
    std::vector<string> test;
    for (int i = 0; i < 20; i++) {
        test.push_back(param);
    }
    string str = "\"key\":[";
    for (int i = 0; i < 19; i++) {
        str = str + "\"abc\",";
    }
    str = str + "\"abc\"]";
    GTEST_LOG_(INFO) << str << endl;
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::MULTI_MEDIA;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::SECURITY;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, test);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"MULTIMEDIA", "eventNameDemo", "\"type_\":3", str};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_1800 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_1800 end" << endl;
}
/**
* @tc.name In the HiSysEvent Natvie Write interface test, 32 parameters are reported for each type of keyvalue.
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_2000
* @tc.desc The keyvalue parameter has 32 parameters for each type.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_2000, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_2000 start" << endl;
    bool result = false;
    string param = "abc";
    string str = "";
    for (int i=0; i < 31; i++){
        str = str + "\"key\":\"abc\",";
    }
    str += "\"key\":\"abc\"";
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::MSDP;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype,
        g_key, param, g_key, param, g_key, param, g_key, param, g_key, param, g_key, param,
        g_key, param, g_key, param, g_key, param, g_key, param, g_key, param, g_key, param,
        g_key, param, g_key, param, g_key, param, g_key, param, g_key, param, g_key, param,
        g_key, param, g_key, param, g_key, param, g_key, param, g_key, param, g_key, param,
        g_key, param, g_key, param, g_key, param, g_key, param, g_key, param, g_key, param,
        g_key, param, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"MSDP", "eventNameDemo", "\"type_\":4", str};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_2000 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_2000 end" << endl;
}
/**
* @tc.name In the test of the HiSysEvent Natvie Write interface, the key value is a string of 256 characters.
* @tc.number DFX_DFT_HiviewKit_HiSysEvent_Native_2200
* @tc.desc The keyvalue is a string of 256 characters.
*/
HWTEST_F(HiSysEventCPPTest, DFX_DFT_HiviewKit_HiSysEvent_Native_2200, Function|MediumTest|Level3)
{
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_2200 start" << endl;
    bool result = false;
    string param = "";
    for (int i = 0; i < 256; i++) {
        param += "a";
    }
    string domain = OHOS::HiviewDFX::HiSysEvent::Domain::LOCATION;
    OHOS::HiviewDFX::HiSysEvent::EventType eventtype = OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR;
    OHOS::HiviewDFX::HiSysEvent::Write(domain, "eventNameDemo", eventtype, g_key, param);
    RedHiSysEventLog(g_hiLogRedirect, g_reDiRectTimeout);
    string fileinfo = "";
    fileinfo = ReadFile(g_hiLogRedirect);
    std::vector<std::string> para = {"LOCATION", "eventNameDemo", "\"type_\":4", g_key, param};
    if (fileinfo != "") {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "DFX_DFT_HiviewKit_HiSysEvent_Native_2200 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "DFX_DFT_HiviewKit_HiSysEvent_Native_2200 end" << endl;
}
