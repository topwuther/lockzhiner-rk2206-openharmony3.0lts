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

#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

#include "gtest/gtest.h"
#include "securec.h"

#include "app_spawn_client.h"
#include "hilog/log.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace AppSpawn {
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "AppSpawnMST"};

namespace {
const bool CHECK_OK = true;
const bool CHECK_ERROR = false;
const int32_t DEFAULT_PID = 0;
const int32_t FILE_PATH_SIZE = 50;
const int32_t CMD_SIZE = 50;
const int32_t BUFFER_SIZE = 512;
const int32_t BASE_TYPE = 10;
const int32_t CONNECT_RETRY_DELAY = 50 * 1000;
const int32_t CONNECT_RETRY_MAX_TIMES = 5;
const int32_t UID_POSITION_MOVE = 5;
const int32_t GID_POSITION_MOVE = 5;
const int32_t GROUPS_POSITION_MOVE = 8;
const char *DELIMITER_SPACE = " ";
const char *DELIMITER_NEWLINE = "\n";

char buffer[BUFFER_SIZE] = {"\0"};
int32_t newPid = 0;
int32_t retryCount = 0;
}  // namespace

bool checkFileIsExists(const char *filepath)
{
    retryCount = 0;
    while ((access(filepath, F_OK) != 0) && (retryCount < CONNECT_RETRY_MAX_TIMES)) {
        usleep(CONNECT_RETRY_DELAY);
        retryCount++;
    }
    GTEST_LOG_(INFO) << "retryCount :" << retryCount << ".";
    if (retryCount < CONNECT_RETRY_MAX_TIMES) {
        return CHECK_OK;
    }
    return CHECK_ERROR;
}

bool readFileInfo(char *buffer, const int32_t &pid, const char *fileName)
{
    // Set file path
    char filePath[FILE_PATH_SIZE];
    if (sprintf_s(filePath, sizeof(filePath), "/proc/%d/%s", pid, fileName) <= 0) {
        HiLog::Error(LABEL, "filePath sprintf_s fail .");
        return CHECK_ERROR;
    }
    if (!checkFileIsExists(filePath)) {
        HiLog::Error(LABEL, "file %{public}s is not exists .", fileName);
        return CHECK_ERROR;
    }
    // Open file
    int fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        HiLog::Error(LABEL, "file %{public}s open failed . error:%{publid}s", fileName, strerror(errno));
        return CHECK_ERROR;
    }
    // Read file
    int t = read(fd, buffer, BUFFER_SIZE);
    if (t <= 0 || buffer == nullptr) {
        HiLog::Info(LABEL, "read proc status file failed.");
        close(fd);
        fd = -1;
        return CHECK_ERROR;
    }
    HiLog::Info(LABEL, "buffer:\n %{public}s", buffer);
    close(fd);

    return CHECK_OK;
}

bool checkUid(const int32_t &pid, const AppSpawnStartMsg &params)
{
    if (readFileInfo(buffer, pid, "status")) {
        // Move to Uid position
        char *uidPtr = strstr(buffer, "Uid") + UID_POSITION_MOVE;
        if (uidPtr == nullptr) {
            HiLog::Error(LABEL, "get Uid info failed.");
            return CHECK_ERROR;
        }
        if (strlen(uidPtr) > UID_POSITION_MOVE) {
            uidPtr = uidPtr + UID_POSITION_MOVE;
        }
        int32_t uid = (int32_t)strtol(uidPtr, NULL, BASE_TYPE);
        HiLog::Info(LABEL, "new proc(%{public}d) uid = %{public}d, setUid=%{public}d.", pid, uid, params.uid);
        if (uid == params.uid) {
            return CHECK_OK;
        }
    }
    return CHECK_ERROR;
}

bool checkGid(const int32_t &pid, const AppSpawnStartMsg &params)
{
    if (readFileInfo(buffer, pid, "status")) {
        // Move to Gid position
        char *gidPtr = strstr(buffer, "Gid");
        if (gidPtr == nullptr) {
            HiLog::Error(LABEL, "get Gid info failed.");
            return CHECK_ERROR;
        }
        if (strlen(gidPtr) > GID_POSITION_MOVE) {
            gidPtr = gidPtr + GID_POSITION_MOVE;
        }
        if (gidPtr == nullptr) {
            HiLog::Error(LABEL, "get Gid info failed.");
            return CHECK_ERROR;
        }
        int32_t gid = (int32_t)strtol(gidPtr, NULL, BASE_TYPE);
        HiLog::Info(LABEL, "new proc(%{public}d) gid = %{public}d, setGid=%{public}d.", pid, gid, params.gid);
        if (gid == params.gid) {
            return CHECK_OK;
        }
    }
    return CHECK_ERROR;
}
std::size_t getGids(const int32_t &pid, std::vector<int32_t> &gids)
{
    if (readFileInfo(buffer, pid, "status")) {
        // Move to Groups position
        char *groupsPtr = strstr(buffer, "Groups");
        if (groupsPtr == nullptr || strlen(groupsPtr) > BUFFER_SIZE) {
            HiLog::Error(LABEL, "get Groups info failed.");
            return CHECK_ERROR;
        }
        if (strlen(groupsPtr) > GROUPS_POSITION_MOVE) {
            groupsPtr = groupsPtr + GROUPS_POSITION_MOVE;
        }
        // Get the row content of Groups
        char *saveptr = nullptr;
        if (groupsPtr == nullptr || strlen(groupsPtr) > BUFFER_SIZE) {
            HiLog::Error(LABEL, "get Groups info failed.");
            return CHECK_ERROR;
        }
        char *line = strtok_r(groupsPtr, DELIMITER_NEWLINE, &saveptr);
        if (line == nullptr || strlen(line) > BUFFER_SIZE) {
            HiLog::Error(LABEL, "get Groups line info failed.");
            return CHECK_ERROR;
        }
        // Get each gid and insert into vector
        char *gid = strtok_r(line, DELIMITER_SPACE, &saveptr);
        while (gid != nullptr) {
            gids.push_back(atoi(gid));
            gid = strtok_r(nullptr, DELIMITER_SPACE, &saveptr);
        }
    }

    return gids.size();
}

bool checkGids(const int32_t &pid, const AppSpawnStartMsg &params)
{
    // Get Gids
    std::vector<int32_t> gids;
    std::size_t gCount = getGids(pid, gids);
    if ((gCount == params.gids.size()) && (gids == params.gids)) {
        return CHECK_OK;
    }

    return CHECK_ERROR;
}

bool checkGidsCount(const int32_t &pid, const AppSpawnStartMsg &params)
{
    // Get GidsCount
    std::vector<int32_t> gids;
    std::size_t gCount = getGids(pid, gids);
    if (gCount == params.gids.size()) {
        return CHECK_OK;
    }

    return CHECK_ERROR;
}

bool checkProcName(const int32_t &pid, const AppSpawnStartMsg &params)
{
    FILE *fp = nullptr;
    char cmd[CMD_SIZE];
    if (sprintf_s(cmd, sizeof(cmd), "ps -o ARGS=CMD -p %d |grep -v CMD", pid) <= 0) {
        HiLog::Error(LABEL, "cmd sprintf_s fail .");
        return CHECK_ERROR;
    }
    if(strlen(cmd) > CMD_SIZE) {
        HiLog::Error(LABEL, " cmd length is too long  .");
        return CHECK_ERROR;
    }
    fp = popen(cmd, "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed .");
        return CHECK_ERROR;
    }
    char procName[BUFFER_SIZE];
    if (fgets(procName, sizeof(procName), fp) != nullptr) {
        for (unsigned int i = 0; i < sizeof(procName); i++) {
            if (procName[i] == '\n') {
                procName[i] = '\0';
                break;
            }
        }
        GTEST_LOG_(INFO) << "strcmp"
                         << " :" << strcmp(params.procName.c_str(), procName) << ".";

        if (params.procName.compare(0, params.procName.size(), procName, params.procName.size()) == 0) {
            pclose(fp);
            return CHECK_OK;
        }
        HiLog::Error(LABEL, " procName=%{public}s, params.procName=%{public}s.", procName, params.procName.c_str());
    } else {
        HiLog::Error(LABEL, "Getting procName failed.");
    }
    pclose(fp);

    return CHECK_ERROR;
}
bool checkProcessIsDestroyed(const int32_t &pid)
{
    char filePath[FILE_PATH_SIZE];
    if (sprintf_s(filePath, sizeof(filePath), "/proc/%d", pid) <= 0) {
        HiLog::Error(LABEL, "filePath sprintf_s fail .");
        return CHECK_ERROR;
    }

    if (checkFileIsExists(filePath)) {
        HiLog::Error(LABEL, "File %{public}d is not exists .", pid);
        return CHECK_ERROR;
    }

    return CHECK_OK;
}

bool checkAppspawnPID()
{
    FILE *fp = nullptr;
    fp = popen("pidof appspawn", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed.");
        return CHECK_ERROR;
    }
    char pid[BUFFER_SIZE];
    if (fgets(pid, sizeof(pid), fp) != nullptr) {
        pclose(fp);
        return CHECK_OK;
    }

    HiLog::Error(LABEL, "Getting Pid failed.");

    pclose(fp);

    return CHECK_ERROR;
}

bool startAppspawn()
{
    FILE *fp = nullptr;
    fp = popen("/system/bin/appspawn&", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed.");
        return CHECK_ERROR;
    }

    pclose(fp);

    return CHECK_OK;
}

bool stopAppspawn()
{
    FILE *fp = nullptr;
    fp = popen("kill -9 $(pidof appspawn)", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed.");
        return CHECK_ERROR;
    }

    pclose(fp);

    return CHECK_OK;
}

class AppSpawnModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AppSpawnModuleTest::SetUpTestCase()
{
    if (!checkAppspawnPID()) {
        EXPECT_EQ(startAppspawn(), CHECK_OK);
    }
}

void AppSpawnModuleTest::TearDownTestCase()
{
    if (checkAppspawnPID()) {
        EXPECT_EQ(stopAppspawn(), CHECK_OK);
    }
}

void AppSpawnModuleTest::SetUp()
{
    newPid = 0;
    auto ret = memset_s(buffer, sizeof(buffer), 0x00, BUFFER_SIZE);
    if (ret != EOK) {
        HiLog::Error(LABEL, "memset_s is failed.");
    }
}

void AppSpawnModuleTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: Listen
 * SubFunction: Message listener
 * FunctionPoints: Process start message monitoring
 * EnvConditions: AppSpawn main process has started.
 *                 The socket server has been established.
 * CaseDescription: 1. Query the process of appspawn through the ps command
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_listen_001, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_listen_001 start");

    EXPECT_EQ(CHECK_OK, checkAppspawnPID());

    HiLog::Info(LABEL, "AppSpawn_HF_listen_001 end");
}

/*
 * Feature: AppSpawn
 * Function: Listen
 * SubFunction: Message listener
 * FunctionPoints: Process start message monitoring.
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_listen_002, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_listen_002 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();

    appSpawnClient->SetSocket(appSpawnSocket);

    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());

    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_listen_002 end");
}

/*
 * Feature: AppSpawn
 * Function: Fork
 * SubFunction: fork process
 * FunctionPoints: Fork the process and run the App object.
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_DEFAULT
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_fork_001, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_fork_001 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-fork_001", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_fork_001 end");
}

/*
 * Feature: AppSpawn
 * Function: Fork
 * SubFunction: fork process
 * FunctionPoints: Fork the process and run the App object.
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_NATIVE
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_fork_002, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_fork_002 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-fork_002", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_fork_002 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_DEFAULT
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_001, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_001 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_001", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkUid(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_001 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_DEFAULT
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_002, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_002 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_002", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkGid(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_002 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_DEFAULT
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_003, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_003 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_003", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkGids(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_003 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_DEFAULT
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_004, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_004 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_004", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkGidsCount(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_004 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_NATIVE
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_005, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_005 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_005", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkUid(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_005 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_NATIVE
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_006, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_006 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_006", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkGid(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_006 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_NATIVE
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_007, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_007 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_007", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkGids(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_007 end");
}

/*
 * Feature: AppSpawn
 * Function: SetUid
 * SubFunction: Set child process permissions
 * FunctionPoints: Set the permissions of the child process to increase the priority of the new process
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_NATIVE
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setUid_008, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_008 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setUid_008", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(CHECK_OK, checkGidsCount(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setUid_008 end");
}

/*
 * Feature: AppSpawn
 * Function: setProcName
 * SubFunction: Set process name
 * FunctionPoints: Set process information .
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_DEFAULT
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setProcName_001, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setProcName_001 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setProcName_001", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    // Check new app proc name
    EXPECT_EQ(CHECK_OK, checkProcName(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setProcName_001 end");
}

/*
 * Feature: AppSpawn
 * Function: setProcName
 * SubFunction: Set process name
 * FunctionPoints: Set process information .
 * EnvConditions: AppSpawn main process has started.
 *                The socket server has been established.
 * CaseDescription: 1. Establish a socket client and connect with the Appspawn server
 *                  2. Send the message and the message format is correct, the message type is APP_TYPE_NATIVE
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_setProcName_002, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_setProcName_002 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-setProcName_002", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    // Check new app proc name
    EXPECT_EQ(CHECK_OK, checkProcName(newPid, params));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_setProcName_002 end");
}

/*
 * Feature: AppSpawn
 * Function: recycleProc
 * SubFunction: Recycling process
 * FunctionPoints: Recycling zombie processes.
 * EnvConditions: Start a js ability
 * CaseDescription: 1. Use the command kill to kill the process pid of the ability
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_recycleProc_001, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_recycleProc_001 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-recycleProc_001", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(ERR_OK, kill(newPid, SIGKILL));
    newPid = DEFAULT_PID;

    // Check Process Is Destroyed
    EXPECT_EQ(CHECK_OK, checkProcessIsDestroyed(newPid));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_recycleProc_001 end");
}

/*
 * Feature: AppSpawn
 * Function: recycleProc
 * SubFunction: Recycling process
 * FunctionPoints: Recycling zombie processes
 * EnvConditions: Start a native ability .
 * CaseDescription: 1. Use the command kill to kill the process pid of the ability
 */
HWTEST_F(AppSpawnModuleTest, AppSpawn_HF_recycleProc_002, TestSize.Level0)
{
    HiLog::Info(LABEL, "AppSpawn_HF_recycleProc_002 start");
    std::unique_ptr<AppSpawnClient> appSpawnClient = std::make_unique<AppSpawnClient>();
    std::shared_ptr<AppExecFwk::AppSpawnSocket> appSpawnSocket = std::make_shared<AppExecFwk::AppSpawnSocket>();
    appSpawnClient->SetSocket(appSpawnSocket);
    EXPECT_EQ(ERR_OK, appSpawnClient->OpenConnection());
    AppSpawnStartMsg params = {10003, 10004, {10003, 10004}, "processName-recycleProc_002", "soPath"};

    appSpawnClient->StartProcess(params, newPid);
    // 0 < newPid, new process fork success
    GTEST_LOG_(INFO) << "newPid :" << newPid << ".";
    EXPECT_LT(DEFAULT_PID, newPid);

    EXPECT_EQ(ERR_OK, kill(newPid, SIGKILL));
    newPid = DEFAULT_PID;

    // Check Process Is Destroyed
    EXPECT_EQ(CHECK_OK, checkProcessIsDestroyed(newPid));

    EXPECT_EQ(ERR_OK, appSpawnSocket->OpenAppSpawnConnection());
    appSpawnClient->CloseConnection();
    EXPECT_EQ(SpawnConnectionState::STATE_NOT_CONNECT, appSpawnClient->QueryConnectionState());
    HiLog::Info(LABEL, "AppSpawn_HF_recycleProc_002 end");
}
}  // namespace AppSpawn
}  // namespace OHOS
