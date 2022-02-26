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
#include <dirent.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include "cJSON.h"
#include "gtest/gtest.h"
#include "securec.h"
#include "init_jobs.h"
#include "init_service_manager.h"

using namespace testing::ext;

namespace OHOS {
std::vector<std::string> g_supportedCmds;
const std::string ROOT_DIR = "/storage/data/";
const std::string TEST_DRI = ROOT_DIR + "StartInitTestDir";
const std::string TEST_FILE = TEST_DRI + "/test.txt";
const std::string TEST_CFG_ILLEGAL = TEST_DRI + "/illegal.cfg";
const std::string TEST_PROC_MOUNTS = "/proc/mounts";
const uid_t TEST_FILE_UID = 999;
const gid_t TEST_FILE_GID = 999;
const mode_t TEST_FILE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
const mode_t DEFAULT_DIR_MODE = 0755;

// init.cfg releated
const std::string CFG_FILE = "/etc/init.cfg";
const std::string SERVICE_ARR_NAME_IN_JSON = "services";
const std::string JOBS_ARR_NAME_IN_JSON = "jobs";
const std::string CMDS_ARR_NAME_IN_JSON = "cmds";
const uid_t CFG_FILE_UID = 0;
const gid_t CFG_FILE_GID = 0;
const mode_t CFG_FILE_MODE = S_IRUSR;
const int JOBS_IN_FILE_COUNT = 3;  // pre-init, init, post-init
const int MAX_SERVICES_COUNT_IN_FILE = 100;
const int MAX_CAPS_CNT_FOR_ONE_SERVICE = 100;
const unsigned int MAX_CAPABILITY_VALUE = 4294967295;  // 0xFFFFFFFF
const unsigned int MAX_JSON_FILE_LEN = 102400;  // max init.cfg size 100KB
const int  MAX_PATH_ARGS_CNT = 20;  // max path and args count
const int  MAX_ONE_ARG_LEN   = 64;  // max length of one param/path
const int  CAT_BUF_SIZE = 512; // standard Cat buffer size from vfs_shell_cmd

// job test releated
const pid_t INVALID_PID = -1;
const std::string PRE_INIT_DIR = ROOT_DIR + "preInitDir/";
const std::string INIT_DIR = PRE_INIT_DIR + "initDir";
const std::string POST_INIT_DIR = INIT_DIR + "postInitDir";

class StartupInitUTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        g_supportedCmds.push_back(std::string("start "));
        g_supportedCmds.push_back(std::string("mkdir "));
        g_supportedCmds.push_back(std::string("chmod "));
        g_supportedCmds.push_back(std::string("chown "));
        g_supportedCmds.push_back(std::string("mount "));
        g_supportedCmds.push_back(std::string("loadcfg "));

        mode_t mode = DEFAULT_DIR_MODE;
        if (mkdir(TEST_DRI.c_str(), mode) != 0) {
            if (errno != EEXIST) {
                printf("[----------] StartupInitUTest, mkdir for %s failed, error %d.\n",\
                    TEST_DRI.c_str(), errno);
                return;
            }
        }

        FILE* testFile = fopen(TEST_FILE.c_str(), "w+");
        if (testFile == nullptr) {
            printf("[----------] StartupInitUTest, open file %s failed, error %d.\n",\
                TEST_FILE.c_str(), errno);
            return;
        }

        std::string writeContent = "This is a test file for startup subsystem init module.";
        if (fwrite(writeContent.c_str(), writeContent.length(), 1, testFile) != 1) {
            printf("[----------] StartupInitUTest, open file %s failed, error %d.\n", TEST_FILE.c_str(), errno);
            fclose(testFile);
            return;
        }
        fclose(testFile);

#ifndef USE_EMMC_STORAGE    // emmc storage does not support chmod/chown

        if (chmod(TEST_FILE.c_str(), TEST_FILE_MODE) != 0) {
            printf("[----------] StartupInitUTest, chmod for file %s failed, error %d.\n", TEST_FILE.c_str(), errno);
            return;
        }

        if (chown(TEST_FILE.c_str(), TEST_FILE_UID, TEST_FILE_GID) != 0) {
            printf("[----------] StartupInitUTest, chown for file %s failed, error %d.\n", TEST_FILE.c_str(), errno);
            return;
        }

#endif  // USE_EMMC_STORAGE

        printf("[----------] StartupInitUTest, cmd func test setup.\n");
    }

    static void TearDownTestCase()
    {
        if (remove(TEST_FILE.c_str()) != 0) {
            printf("[----------] StartupInitUTest, remove %s failed, error %d.\n", TEST_FILE.c_str(), errno);
        }

        if (remove(TEST_DRI.c_str()) != 0) {
            printf("[----------] StartupInitUTest, remove %s failed, error %d.\n", TEST_DRI.c_str(), errno);
        }
        printf("[----------] StartupInitUTest, cmd func test teardown.\n");
    }
    void SetUp() {}
    void TearDown() {}
};

/*
 ** @tc.name: cmdFuncParseCmdTest_001
 ** @tc.desc: parse function, nullptr test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733F
 **/
HWTEST_F(StartupInitUTest, cmdFuncParseCmdTest_001, TestSize.Level1)
{
    // do not crash
    ParseCmdLine(nullptr, nullptr);
};

/*
 ** @tc.name: cmdFuncParseCmdTest_002
 ** @tc.desc: parse function, invalid strings test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733F
 **/
HWTEST_F(StartupInitUTest, cmdFuncParseCmdTest_002, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    ParseCmdLine(nullptr, &curCmdLine);
    EXPECT_EQ(0, strlen(curCmdLine.name));
    EXPECT_EQ(0, strlen(curCmdLine.cmdContent));

    ParseCmdLine("", &curCmdLine);
    EXPECT_EQ(0, strlen(curCmdLine.name));
    EXPECT_EQ(0, strlen(curCmdLine.cmdContent));

    ParseCmdLine("xxxxxxxx", &curCmdLine);
    EXPECT_EQ(0, strlen(curCmdLine.name));
    EXPECT_EQ(0, strlen(curCmdLine.cmdContent));

    ParseCmdLine("asdnkawdqw4145a45sdqw_-+\\\\sdqwdasd", &curCmdLine);
    EXPECT_EQ(0, strlen(curCmdLine.name));
    EXPECT_EQ(0, strlen(curCmdLine.cmdContent));
}

/*
 ** @tc.name: cmdFuncParseCmdTest_003
 ** @tc.desc: parse function, cmd content empty test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733F
 **/
HWTEST_F(StartupInitUTest, cmdFuncParseCmdTest_003, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    for (size_t i = 0; i < g_supportedCmds.size(); ++i) {
        ParseCmdLine(g_supportedCmds[i].c_str(), &curCmdLine);
        EXPECT_EQ(0, strlen(curCmdLine.name));
        EXPECT_EQ(0, strlen(curCmdLine.cmdContent));
    }
}

/*
 ** @tc.name: cmdFuncParseCmdTest_004
 ** @tc.desc: parse function, cmd content too long test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733F
 **/
HWTEST_F(StartupInitUTest, cmdFuncParseCmdTest_004, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    char toLongContent[MAX_CMD_CONTENT_LEN + 10];
    memset_s(toLongContent, MAX_CMD_CONTENT_LEN + 10, 'x', MAX_CMD_CONTENT_LEN + 9);
    toLongContent[MAX_CMD_CONTENT_LEN + 9] = '\0';
    for (size_t i = 0; i < g_supportedCmds.size(); ++i) {
        size_t curCmdLen = g_supportedCmds[i].length();
        char* curCmd = (char*)malloc(curCmdLen + MAX_CMD_CONTENT_LEN + 10);
        if (curCmd == nullptr) {
            printf("[----------] StartupInitUTest, cmdFuncParseCmdTest004, malloc failed.\n");
            break;
        }
        errno_t ret = memcpy_s(curCmd, curCmdLen + MAX_CMD_CONTENT_LEN + 10, \
            g_supportedCmds[i].c_str(), curCmdLen);
        errno_t ret2 = memcpy_s(curCmd + curCmdLen, MAX_CMD_CONTENT_LEN + 10, \
            toLongContent, strlen(toLongContent));
        if (ret != EOK || ret2 != EOK) {
            printf("[----------] StartupInitUTest, cmdFuncParseCmdTest004, memcpy_s failed.\n");
            free(curCmd);
            curCmd = nullptr;
            break;
        }
        curCmd[curCmdLen + MAX_CMD_CONTENT_LEN + 9] = '\0';

        ParseCmdLine(curCmd, &curCmdLine);
        EXPECT_EQ(0, strlen(curCmdLine.name));
        EXPECT_EQ(0, strlen(curCmdLine.cmdContent));
        free(curCmd);
        curCmd = nullptr;
    }
}

/*
 ** @tc.name: cmdFuncParseCmdTest_005
 ** @tc.desc: parse function, parse success test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733E
 **/
HWTEST_F(StartupInitUTest, cmdFuncParseCmdTest_005, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    ParseCmdLine("start InitTestService", &curCmdLine);
    EXPECT_EQ(0, strcmp("start ", curCmdLine.name));
    EXPECT_EQ(0, strcmp("InitTestService", curCmdLine.cmdContent));

    ParseCmdLine("mkdir InitTestDir", &curCmdLine);
    EXPECT_EQ(0, strcmp("mkdir ", curCmdLine.name));
    EXPECT_EQ(0, strcmp("InitTestDir", curCmdLine.cmdContent));

    ParseCmdLine("chmod 0500 /bin/InitTestBin", &curCmdLine);
    EXPECT_EQ(0, strcmp("chmod ", curCmdLine.name));
    EXPECT_EQ(0, strcmp("0500 /bin/InitTestBin", curCmdLine.cmdContent));

    ParseCmdLine("chown 1000 1000 /bin/InitTestBin", &curCmdLine);
    EXPECT_EQ(0, strcmp("chown ", curCmdLine.name));
    EXPECT_EQ(0, strcmp("1000 1000 /bin/InitTestBin", curCmdLine.cmdContent));

    ParseCmdLine("mount vfat /dev/mmcblk1 /sdcard rw,umask=000", &curCmdLine);
    EXPECT_EQ(0, strcmp("mount ", curCmdLine.name));
    EXPECT_EQ(0, strcmp("vfat /dev/mmcblk1 /sdcard rw,umask=000", curCmdLine.cmdContent));
};

/*
 ** @tc.name: cmdFuncDoCmdTest_001
 ** @tc.desc: do cmd function, nullptr test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733E
 **/
HWTEST_F(StartupInitUTest, cmdFuncDoCmdTest_001, TestSize.Level1)
{
    // do not crash here
    DoCmd(nullptr);
}

/*
 ** @tc.name: cmdFuncDoCmdTest_002
 ** @tc.desc: do cmd function, do start fail test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733E
 **/
HWTEST_F(StartupInitUTest, cmdFuncDoCmdTest_002, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    std::string cmdStr = "start ";
    std::string cmdContentStr = "NameNotExist";
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);
}

/*
 ** @tc.name: cmdFuncDoCmdTest_003
 ** @tc.desc: do cmd function, do mkdir fail test
 ** @tc.type: FUNC
 ** @tc.require: AR000F733E
 **/
HWTEST_F(StartupInitUTest, cmdFuncDoCmdTest_003, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    std::string cmdStr = "mkdir ";
    std::string cmdContentStr = "/DirNotExist/DirNotExist/DirNotExist";
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    // make sure that the directory does not exist
    DIR* dirTmp = opendir(cmdContentStr.c_str());
    EXPECT_TRUE(dirTmp == nullptr);
    EXPECT_TRUE(errno == ENOENT);
    if (dirTmp != nullptr) {    // just in case
        closedir(dirTmp);
        dirTmp = nullptr;
    }

    // error argument count, bad format
    cmdContentStr = "  /storage/data/cmdFuncDoCmdTest003 0755 system";
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    // make sure that the directory does not exist
    dirTmp = opendir("/storage/data/cmdFuncDoCmdTest003");
    EXPECT_TRUE(dirTmp == nullptr);
    EXPECT_TRUE(errno == ENOENT);
    if (dirTmp != nullptr) {    // just in case
        closedir(dirTmp);
        dirTmp = nullptr;
    }
}

/*
 ** @tc.name: cmdFuncDoCmdTest_004
 ** @tc.desc: do cmd function, do chmod fail test
 ** @tc.type: FUNC
 ** @tc.require: AR000F732P
 **/
HWTEST_F(StartupInitUTest, cmdFuncDoCmdTest_004, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    std::string cmdStr = "chmod ";
    std::string cmdContentStr = "755 " + TEST_FILE;    // should be 0755, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    cmdContentStr = "0855 " + TEST_FILE;    // should not exceed 0777, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    cmdContentStr = "07b5 " + TEST_FILE;    // non-digital character, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    cmdContentStr = "075 " + TEST_FILE;    // should be 0xxx, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    cmdContentStr = "0755       " + TEST_FILE;    // too many spaces, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    struct stat testFileStat = {0};
    EXPECT_EQ(0, stat(TEST_FILE.c_str(), &testFileStat));

#ifndef USE_EMMC_STORAGE    // emmc storage does not support chmod/chown

    EXPECT_EQ(TEST_FILE_MODE, testFileStat.st_mode & TEST_FILE_MODE);   // file mode is not changed

#endif // USE_EMMC_STORAGE
}

/*
 ** @tc.name: cmdFuncDoCmdTest_005
 ** @tc.desc: do cmd function, do chown fail test
 ** @tc.type: FUNC
 ** @tc.require: AR000F732P
 **/
HWTEST_F(StartupInitUTest, cmdFuncDoCmdTest_005, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    std::string cmdStr = "chown ";
    std::string cmdContentStr = "888 " + TEST_FILE;    // uid or gid missing, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    cmdContentStr = "888 8b9 " + TEST_FILE;    // non-digital character, wrong format here
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    DoCmd(&curCmdLine);

    struct stat testFileStat = {0};
    EXPECT_EQ(0, stat(TEST_FILE.c_str(), &testFileStat));

#ifndef USE_EMMC_STORAGE    // emmc storage does not support chmod/chown

    EXPECT_EQ(testFileStat.st_uid, TEST_FILE_UID);    // uid not changed
    EXPECT_EQ(testFileStat.st_gid, TEST_FILE_GID);    // gid not changed

#endif // USE_EMMC_STORAGE
}

/*
 ** @tc.name: cmdFuncDoCmdTest_006
 ** @tc.desc: do cmd function, do success test
 ** @tc.type: FUNC
 ** @tc.require: AR000F732P
 **/
HWTEST_F(StartupInitUTest, cmdFuncDoCmdTest_006, TestSize.Level1)
{
    CmdLine curCmdLine;

    // mkdir success
    std::string cmdStr = "mkdir ";
    std::string cmdContentStr = TEST_DRI + "/cmdFuncDoCmdTest006";
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));

    DoCmd(&curCmdLine);
    DIR* dirTmp = opendir(cmdContentStr.c_str());
    EXPECT_TRUE(dirTmp != nullptr);
    if (dirTmp != nullptr) {
        closedir(dirTmp);
        dirTmp = nullptr;
    }

    // delete dir
    if (remove(cmdContentStr.c_str()) != 0) {
        printf("[----------] StartupInitUTest, cmdFuncDoCmdTest006 remove %s failed, error %d.\n",\
            TEST_DRI.c_str(), errno);
    }

    // chmod success
    cmdStr = "chmod ";
    cmdContentStr = "0440 " + TEST_FILE;
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));

#ifndef USE_EMMC_STORAGE    // emmc storage does not support chmod/chown

    DoCmd(&curCmdLine);
    struct stat testFileStat = {0};
    EXPECT_EQ(0, stat(TEST_FILE.c_str(), &testFileStat));
    mode_t targetMode = S_IRUSR | S_IRGRP;
    EXPECT_EQ(targetMode, testFileStat.st_mode & targetMode);    // changed

#endif  // USE_EMMC_STORAGE

    // chown success
    cmdStr = "chown ";
    cmdContentStr = "888 888 " + TEST_FILE;
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));

#ifndef USE_EMMC_STORAGE    // emmc storage does not support chmod/chown

    DoCmd(&curCmdLine);
    EXPECT_EQ(0, stat(TEST_FILE.c_str(), &testFileStat));
    EXPECT_EQ(testFileStat.st_uid, 888);    // changed
    EXPECT_EQ(testFileStat.st_gid, 888);    // changed

#endif  // USE_EMMC_STORAGE
}

/*
 ** @tc.name: cfgCheckStat_001
 ** @tc.desc: init.cfg file state check
 ** @tc.type: FUNC
 ** @tc.require: AR000F733F
 **/
HWTEST_F(StartupInitUTest, cfgCheckStat_001, TestSize.Level1)
{
    struct stat fileStat = {0};
    EXPECT_EQ(0, stat(CFG_FILE.c_str(), &fileStat));
    EXPECT_EQ(CFG_FILE_UID, fileStat.st_uid);
    EXPECT_EQ(CFG_FILE_GID, fileStat.st_gid);
    EXPECT_EQ(CFG_FILE_MODE, CFG_FILE_MODE & fileStat.st_mode);
    EXPECT_TRUE(fileStat.st_size > 0);
    EXPECT_TRUE(fileStat.st_size <= MAX_JSON_FILE_LEN);
};

static char* ReadFileToBuf()
{
    char* buffer = nullptr;
    FILE* fd = nullptr;
    struct stat fileStat = {0};
    (void)stat(CFG_FILE.c_str(), &fileStat);
    do {
        fd = fopen(CFG_FILE.c_str(), "r");
        if (fd == nullptr) {
            break;
        }

        buffer = static_cast<char*>(malloc(static_cast<size_t>(fileStat.st_size) + 1));
        if (buffer == nullptr) {
            break;
        }

        if (fread(buffer, fileStat.st_size, 1, fd) != 1) {
            free(buffer);
            buffer = nullptr;
            break;
        }
        buffer[fileStat.st_size] = '\0';
    } while (0);

    if (fd != nullptr) {
        fclose(fd);
        fd = nullptr;
    }
    return buffer;
}

static cJSON* GetArrItem(const cJSON* fileRoot, int& arrSize, const std::string& arrName)
{
    cJSON* arrItem = cJSON_GetObjectItemCaseSensitive(fileRoot, arrName.c_str());
    arrSize = cJSON_GetArraySize(arrItem);
    if (arrSize <= 0) {
        return nullptr;
    }
    return arrItem;
}

static int IsForbidden(const char* fieldStr)
{
    size_t fieldLen = strlen(fieldStr);
    size_t forbidStrLen =  strlen("/bin/sh");
    if (fieldLen == forbidStrLen) {
        if (strncmp(fieldStr, "/bin/sh", fieldLen) == 0) {
            return 1;
        }
        return 0;
    } else if (fieldLen > forbidStrLen) {
        // "/bin/shxxxx" is valid but "/bin/sh xxxx" is invalid
        if (strncmp(fieldStr, "/bin/sh", forbidStrLen) == 0) {
            if (fieldStr[forbidStrLen] == ' ') {
                return 1;
            }
        }
        return 0;
    } else {
        return 0;
    }
}

static void CheckService(const cJSON* curItem)
{
    if (curItem == nullptr) {
        return;
    }

    char* nameStr = cJSON_GetStringValue(cJSON_GetObjectItem(curItem, "name"));
    if (nameStr == nullptr) {
        EXPECT_TRUE(nameStr != nullptr);
    } else {
        EXPECT_TRUE(strlen(nameStr) > 0);
    }

    cJSON* pathArgsItem = cJSON_GetObjectItem(curItem, "path");
    EXPECT_TRUE(cJSON_IsArray(pathArgsItem));

    int pathArgsCnt = cJSON_GetArraySize(pathArgsItem);
    EXPECT_TRUE(pathArgsCnt > 0);
    EXPECT_TRUE(pathArgsCnt <= MAX_PATH_ARGS_CNT);

    for (int i = 0; i < pathArgsCnt; ++i) {
        char* curParam = cJSON_GetStringValue(cJSON_GetArrayItem(pathArgsItem, i));
        EXPECT_TRUE(curParam != NULL);
        EXPECT_TRUE(strlen(curParam) > 0);
        EXPECT_TRUE(strlen(curParam) <= MAX_ONE_ARG_LEN);
        if (i == 0) {
            EXPECT_TRUE(IsForbidden(curParam) == 0);
        }
    }

    cJSON* filedJ = cJSON_GetObjectItem(curItem, "uid");
    EXPECT_TRUE(cJSON_IsNumber(filedJ));
    EXPECT_TRUE(cJSON_GetNumberValue(filedJ) >= 0.0);

    filedJ = cJSON_GetObjectItem(curItem, "gid");
    EXPECT_TRUE(cJSON_IsNumber(filedJ));
    EXPECT_TRUE(cJSON_GetNumberValue(filedJ) >= 0.0);

    filedJ = cJSON_GetObjectItem(curItem, "once");
    EXPECT_TRUE(cJSON_IsNumber(filedJ));

    filedJ = cJSON_GetObjectItem(curItem, "importance");
    EXPECT_TRUE(cJSON_IsNumber(filedJ));

    filedJ = cJSON_GetObjectItem(curItem, "caps");
    EXPECT_TRUE(cJSON_IsArray(filedJ));
    int capsCnt = cJSON_GetArraySize(filedJ);
    EXPECT_TRUE(capsCnt <= MAX_CAPS_CNT_FOR_ONE_SERVICE);
    for (int i = 0; i < capsCnt; ++i) {
        cJSON* capJ = cJSON_GetArrayItem(filedJ, i);
        EXPECT_TRUE(cJSON_IsNumber(capJ));
        EXPECT_TRUE(cJSON_GetNumberValue(capJ) >= 0.0);

        // only shell can have all capabilities
        if ((unsigned int)cJSON_GetNumberValue(capJ) == MAX_CAPABILITY_VALUE) {
            if (nameStr != nullptr) {
                EXPECT_EQ(0, strcmp(nameStr, "shell"));
            }
            EXPECT_EQ(1, capsCnt);
        }
    }
}

static void CheckServices(const cJSON* fileRoot)
{
    int servArrSize = 0;
    cJSON* serviceArr = GetArrItem(fileRoot, servArrSize, SERVICE_ARR_NAME_IN_JSON);
    EXPECT_TRUE(serviceArr != nullptr);
    EXPECT_TRUE(servArrSize <= MAX_SERVICES_COUNT_IN_FILE);

    for (int i = 0; i < servArrSize; ++i) {
        cJSON* curItem = cJSON_GetArrayItem(serviceArr, i);
        EXPECT_TRUE(curItem != nullptr);
        CheckService(curItem);
    }
}

static void CheckCmd(const CmdLine* resCmd)
{
    EXPECT_TRUE(strlen(resCmd->name) > 0);
    EXPECT_TRUE(strlen(resCmd->cmdContent) > 0);

    if (strcmp("start ", resCmd->name) == 0) {
        for (size_t i = 0; i < strlen(resCmd->cmdContent); ++i) {
            EXPECT_NE(' ', resCmd->cmdContent[i]);    // no spaces in service name
        }
    } else if (strcmp("mkdir ", resCmd->name) == 0) {
        for (size_t i = 0; i < strlen(resCmd->cmdContent); ++i) {
            EXPECT_NE(' ', resCmd->cmdContent[i]);    // no spaces in path string
            EXPECT_NE('.', resCmd->cmdContent[i]);    // no dots in path string
        }
    } else if (strcmp("chmod ", resCmd->name) == 0) {
        EXPECT_TRUE(strlen(resCmd->cmdContent) >= 6);    // 0xxx x    at least 6 characters
        EXPECT_EQ('0', resCmd->cmdContent[0]);
        EXPECT_EQ(' ', resCmd->cmdContent[4]);    // 4 bytes, after 0xxx must be space
        for (int i = 1; i < 4; ++i) {    // 4 bytes, 0xxx, xxx must be digits
            EXPECT_TRUE(resCmd->cmdContent[i] >= '0' && resCmd->cmdContent[i] <= '7');
        }
        for (size_t i = 5; i < strlen(resCmd->cmdContent); ++i) {    // target starts from index 5
            EXPECT_NE(' ', resCmd->cmdContent[i]);    // no spaces allowed
            EXPECT_NE('.', resCmd->cmdContent[i]);    // no dots allowed
        }
    } else if (strcmp("chown ", resCmd->name) == 0) {
        EXPECT_TRUE(strlen(resCmd->cmdContent) >= 5);    // x y z   at least 5 characters
        EXPECT_NE(' ', resCmd->cmdContent[0]);           // should not start with space
        EXPECT_NE(' ', resCmd->cmdContent[strlen(resCmd->cmdContent) - 1]);  // should not end with space
        size_t spacePos = 0;
        size_t spaceCnt = 0;
        for (size_t i = 1; i < strlen(resCmd->cmdContent); ++i) {
            if (resCmd->cmdContent[i] == ' ') {
                ++spaceCnt;
                if (spacePos != 0) {
                    EXPECT_NE(spacePos + 1, i);    // consecutive spaces should not appear
                }
                spacePos = i;
            }
        }
        EXPECT_EQ(spaceCnt, 2);    // 2 spaces allowed in cmd content
    } else if (strcmp("mount ", resCmd->name) == 0) {
        EXPECT_NE(' ', resCmd->cmdContent[0]);    // should not start with space
    } else if (strcmp("loadcfg ", resCmd->name) == 0) {
        EXPECT_NE(' ', resCmd->cmdContent[0]);   // should not start with space
    } else {    // unknown cmd
        EXPECT_TRUE(false);
    }
}

static void CheckJob(const cJSON* jobItem)
{
    if (jobItem == nullptr) {
        return;
    }

    cJSON* cmdsItem = cJSON_GetObjectItem(jobItem, CMDS_ARR_NAME_IN_JSON.c_str());
    EXPECT_TRUE(cmdsItem != nullptr);
    EXPECT_TRUE(cJSON_IsArray(cmdsItem));

    int cmdLinesCnt = cJSON_GetArraySize(cmdsItem);
    EXPECT_TRUE(cmdLinesCnt <= MAX_CMD_CNT_IN_ONE_JOB);

    for (int i = 0; i < cmdLinesCnt; ++i) {
        char* cmdLineStr = cJSON_GetStringValue(cJSON_GetArrayItem(cmdsItem, i));
        EXPECT_TRUE(cmdLineStr != nullptr);
        EXPECT_TRUE(strlen(cmdLineStr) > 0);

        CmdLine resCmd;
        (void)memset_s(&resCmd, sizeof(resCmd), 0, sizeof(resCmd));
        ParseCmdLine(cmdLineStr, &resCmd);
        CheckCmd(&resCmd);
    }
}

static void CheckJobs(const cJSON* fileRoot)
{
    int jobArrSize = 0;
    cJSON* jobArr = GetArrItem(fileRoot, jobArrSize, JOBS_ARR_NAME_IN_JSON);
    EXPECT_TRUE(jobArr != nullptr);
    EXPECT_TRUE(jobArrSize == JOBS_IN_FILE_COUNT);

    bool findPreInit = false;
    bool findInit = false;
    bool findPostInit = false;
    for (int i = 0; i < jobArrSize; ++i) {
        cJSON* jobItem = cJSON_GetArrayItem(jobArr, i);
        EXPECT_TRUE(jobItem != nullptr);
        char* jobNameStr = cJSON_GetStringValue(cJSON_GetObjectItem(jobItem, "name"));
        EXPECT_TRUE(jobNameStr != nullptr);
        if (strcmp(jobNameStr, "pre-init") == 0) {
            findPreInit = true;
        } else if (strcmp(jobNameStr, "init") == 0) {
            findInit = true;
        }  else if (strcmp(jobNameStr, "post-init") == 0) {
            findPostInit = true;
        } else {
            EXPECT_TRUE(false);    // unknown job name
            continue;
        }

        CheckJob(jobItem);
    }

    EXPECT_TRUE(findPreInit && findInit && findPostInit);
}

/*
 ** @tc.name: cfgCheckContent_001
 ** @tc.desc: init.cfg file content check
 ** @tc.type: FUNC
 ** @tc.require: AR000F733F
 **/
HWTEST_F(StartupInitUTest, cfgCheckContent_001, TestSize.Level1)
{
    char* fileBuf = ReadFileToBuf();
    if (fileBuf == nullptr) {
        EXPECT_TRUE(fileBuf != nullptr);
        return;
    }

    cJSON* fileRoot = cJSON_Parse(fileBuf);
    free(fileBuf);
    fileBuf = nullptr;

    EXPECT_TRUE(fileRoot != nullptr);

    CheckServices(fileRoot);
    CheckJobs(fileRoot);
    cJSON_Delete(fileRoot);
    fileRoot = nullptr;
}

/*
 * @tc.name: CreateIllegalCfg
 * @tc.desc: Create illegal Config file for testing
 * @tc.type: FUNC
 * @tc.require: AR000F861Q
 */
static void CreateIllegalCfg()
{
    FILE* testCfgFile = fopen(TEST_CFG_ILLEGAL.c_str(), "w+");
    if (testCfgFile == nullptr) {
        printf("[----------] StartupInitUTest, open file %s failed, error %d.\n", TEST_CFG_ILLEGAL.c_str(), errno);
        return;
    }

    std::string writeContent = "mount zpfs /patch/etc:/etc /etc";
    if (fwrite(writeContent.c_str(), writeContent.length(), 1, testCfgFile) != 1) {
        printf("[----------] StartupInitUTest, open file %s failed, error %d.\n", TEST_CFG_ILLEGAL.c_str(), errno);
        fclose(testCfgFile);
        return;
    }

    fclose(testCfgFile);
}

/*
 * @tc.name: cmdFuncDoLoadCfgTest_001
 * @tc.desc: parse function, parse success test
 * @tc.type: FUNC
 * @tc.require: AR000F861Q
 */
HWTEST_F(StartupInitUTest, cmdFuncDoLoadCfgTest_001, TestSize.Level1)
{
    CmdLine curCmdLine;
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));

    ParseCmdLine("loadcfg /patch/fstab.cfg", &curCmdLine);
    EXPECT_EQ(0, strcmp("loadcfg ", curCmdLine.name));
    EXPECT_EQ(0, strcmp("/patch/fstab.cfg", curCmdLine.cmdContent));
};

/*
 * @tc.name: cmdFuncDoLoadCfgTest_002
 * @tc.desc: fstab.cfg file fail test
 * @tc.type: FUNC
 * @tc.require: AR000F861Q
 */
HWTEST_F(StartupInitUTest, cmdFuncDoLoadCfgTest_002, TestSize.Level1)
{
    CmdLine curCmdLine;
    std::string cmdStr = "loadcfg ";
    std::string cmdContentStr = "/patch/file_not_exist.cfg";
    struct stat testCfgStat = {0};

    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    stat(cmdContentStr.c_str(), &testCfgStat);
    EXPECT_TRUE(testCfgStat.st_size == 0);
    DoCmd(&curCmdLine);

    cmdContentStr = TEST_CFG_ILLEGAL;
    CreateIllegalCfg();
    memset_s(&curCmdLine, sizeof(curCmdLine), 0, sizeof(curCmdLine));
    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));
    EXPECT_EQ(0, stat(cmdContentStr.c_str(), &testCfgStat));
    EXPECT_TRUE(testCfgStat.st_size > 0);
    DoCmd(&curCmdLine);

    // remove tmp file
    if (remove(TEST_CFG_ILLEGAL.c_str()) != 0) {
        printf("[----------] StartupInitUTest, remove %s failed, error %d.\n",\
            TEST_CFG_ILLEGAL.c_str(), errno);
    }
}

/*
 * @tc.name: cmdFuncDoLoadCfgTest_003
 * @tc.desc: fstab.cfg file success test
 * @tc.type: FUNC
 * @tc.require: AR000F861Q
 */
HWTEST_F(StartupInitUTest, cmdFuncDoLoadCfgTest_003, TestSize.Level1)
{
    CmdLine curCmdLine;
    std::string cmdStr = "loadcfg ";
    std::string cmdContentStr = "/patch/fstab.cfg";
    char buf[CAT_BUF_SIZE] = {0};
    struct stat testCfgStat = {0};
    FILE* fd = nullptr;
    size_t size;
    bool hasZpfs = false;

    ParseCmdLine((cmdStr + cmdContentStr).c_str(), &curCmdLine);
    EXPECT_EQ(0, strcmp(cmdStr.c_str(), curCmdLine.name));
    EXPECT_EQ(0, strcmp(cmdContentStr.c_str(), curCmdLine.cmdContent));

    DoCmd(&curCmdLine);

    stat(cmdContentStr.c_str(), &testCfgStat);
    if (testCfgStat.st_size > 0) {
        fd = fopen(TEST_PROC_MOUNTS.c_str(), "r");

        if (fd == nullptr) {
            EXPECT_TRUE(fd != nullptr);
            return;
        }

        do {
            size = fread(buf, 1, CAT_BUF_SIZE - 1, fd);
            if (size < 0) {
                EXPECT_TRUE(size >= 0);
                break;
            }
            buf[CAT_BUF_SIZE - 1] = 0;
            if (strstr(buf, "zpfs") != nullptr) {
                hasZpfs = true;
                break;
            }
        } while (size > 0);
        EXPECT_TRUE(hasZpfs == true);
        fclose(fd);
    }
}

/*
 * @tc.name: cmdJobTest_001
 * @tc.desc: job functions test
 * @tc.type: FUNC
 * @tc.require: AR000F733F
 */
HWTEST_F(StartupInitUTest, cmdJobTest_001, TestSize.Level1)
{
    // functions do not crash
    ParseAllJobs(nullptr);
    DoJob(nullptr);
    DoJob("job name does not exist");
    ReleaseAllJobs();
    RegisterServices(nullptr, 0);
    StartServiceByName("service name does not exist");
    StopAllServices();
    ReapServiceByPID(INVALID_PID);
    ServiceReap(nullptr);
    EXPECT_NE(0, ServiceStart(nullptr));
    EXPECT_NE(0, ServiceStop(nullptr));
}

/*
 * @tc.name: cmdJobTest_002
 * @tc.desc: job functions test
 * @tc.type: FUNC
 * @tc.require: AR000F733F
 */
HWTEST_F(StartupInitUTest, cmdJobTest_002, TestSize.Level1)
{
    std::string cfgJson = "{\"jobs\":[{\"name\":\"pre-init\",\"cmds\":[\"mkdir " +
        PRE_INIT_DIR + "\"]},{\"name\":\"init\",\"cmds\":[\"mkdir " + INIT_DIR +
        "\"]},{\"name\":\"post-init\",\"cmds\":[\"mkdir " + POST_INIT_DIR + "\"]}]}";
    cJSON* jobItem = cJSON_Parse(cfgJson.c_str());
    EXPECT_NE(nullptr, jobItem);
    if (jobItem == nullptr) {
        printf("[----------] StartupInitUTest, job test, parse %s failed.\n", cfgJson.c_str());
        return;
    }
    ParseAllJobs(jobItem);
    DoJob("pre-init");
    DoJob("init");
    DoJob("post-init");

    // check if dir exists
    struct stat postDirStat = {0};
    EXPECT_EQ(0, stat(POST_INIT_DIR.c_str(), &postDirStat));

    // release resource
    cJSON_Delete(jobItem);
    if (remove(POST_INIT_DIR.c_str()) != 0 ||
        remove(INIT_DIR.c_str()) != 0 ||
        remove(PRE_INIT_DIR.c_str()) != 0) {
        printf("[----------] StartupInitUTest, job test, remove failed, error %d.\n", errno);
    }
    ReleaseAllJobs();
}
}  // namespace OHOS
