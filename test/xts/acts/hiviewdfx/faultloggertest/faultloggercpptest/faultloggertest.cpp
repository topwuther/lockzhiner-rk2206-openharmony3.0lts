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
#include <pthread.h>

#include "genfault.h"
#include "file_utils.h"
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD003200
#define LOG_TAG "FAULTLOGGERTEST"
using namespace OHOS;
using namespace HiviewDFX;
using namespace testing::ext;
using namespace std;

class faultloggertest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    pid_t DoTestProcess(int faulttype);
    int status; 

private:
};
void faultloggertest::SetUp()
{
}
void faultloggertest::TearDown()
{
}
void faultloggertest::SetUpTestCase()
{
}
void faultloggertest::TearDownTestCase()
{
}

pid_t faultloggertest::DoTestProcess(int faulttype)
{
    printf("DoTestProcess, param is %d\r\n", faulttype);
    pid_t pid;
    pid = fork();
    printf("pid is %d\r\n", pid);
    switch (pid) {
        case -1:
            std::cout<<"for pid failed"<<std::endl;
            break;
        case 0:
            GenFault(faulttype);
            break;
        default:
            break;
    }
    wait(&status);
    printf("sub process end with status %d\r\n", status);
    return pid;
}


/*
 * @tc.name faultlogger Detect a cpp crash happen
 * @tc.number DFX_DFR_FaultLogger_0100
 * @tc.desc inject a cppcrash fault and check faultlogger can detect the fault
*/
HWTEST_F(faultloggertest, Faultlogger_Faultdetect, Function|MediumTest|Level1)
{
    pid_t pid = DoTestProcess(3);
    printf("pid is %d\r\n", pid);
    printf("sub process end with status %d\r\n", faultloggertest::status);
    ASSERT_FALSE(status == 0);
    sleep(1);
    std::vector<std::string> faultfilelist;
    faultfilelist = getfileinpath("/data/log/faultlog/temp/");
    printf("sizeof faultfilelist is %d\r\n", faultfilelist.size());
    bool result = false;
    for (std::string filename : faultfilelist) {
        printf("file list is %s\r\n", filename.c_str());
        if (filename.find("cppcrash-" + to_string(pid)) != string::npos) {
            result = true;
            break;
        }
    }
    ASSERT_TRUE(true == result);
}

/*
 * @tc.name faultlogger log file check
 * @tc.number DFX_DFR_FaultLogger_0100
 * @tc.desc inject a cppcrash fault and check faultlogger file
*/
HWTEST_F(faultloggertest, Faultlogger_Faultdetect1, Function|MediumTest|Level1)
{
    pid_t pid = DoTestProcess(3);
    printf("pid is %d\r\n", pid);
    printf("sub process end with status %d\r\n", faultloggertest::status);
    sleep(1);
    std::vector<std::string> faultfilelist;
    faultfilelist = getfileinpath("/data/log/faultlog/temp/");
    printf("sizeof faultfilelist is %d\r\n", faultfilelist.size());
    bool result = false;
    std::string faultloggerfile = "";
    for (std::string filename : faultfilelist) {
        if (filename.find("cppcrash-" + to_string(pid)) != std::string::npos) {
            printf("file list is %s\r\n", filename.c_str());
            faultloggerfile = filename;
            break;
        }
    }
    string fileinfo;
    fileinfo = ReadFile("/data/log/faultlog/temp/" + faultloggerfile);
    std::vector<std::string> para = {"Pid:" + to_string(pid), "Uid:0",
                            "Process name:./data/local/tmp/faultloggertest",
                            "Reason:Signal:SIGILL", "Fault thread Info:",
                            "Tid:" + to_string(pid), "Name:faultloggertest"};
    if (!fileinfo.empty()) {
        result = CheckInfo(para, fileinfo);
    } else {
        std::cout << "Faultlogger_Faultdetect1 file error" << std::endl;
    }
    ASSERT_TRUE(result);
    GTEST_LOG_(INFO) << "Faultlogger_Faultdetect1 end" << endl;
}