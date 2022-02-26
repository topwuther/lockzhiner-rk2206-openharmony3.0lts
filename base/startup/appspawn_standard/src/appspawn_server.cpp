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

#include "appspawn_server.h"

#include <fcntl.h>
#include <memory>
#include <csignal>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <thread>

#include "errors.h"
#include "hilog/log.h"
#include "main_thread.h"
#include "securec.h"

#include <dirent.h>
#include <dlfcn.h>

#define GRAPHIC_PERMISSION_CHECK
constexpr static size_t ERR_STRING_SZ = 64;

namespace OHOS {
namespace AppSpawn {
namespace {
constexpr int32_t ERR_PIPE_FAIL = -100;
constexpr int32_t MAX_LEN_SHORT_NAME = 16;
constexpr int32_t WAIT_DELAY_US = 100 * 1000;  // 100ms
constexpr int32_t GID_MEDIA = 1023;
constexpr int32_t MAX_GIDS = 64;

constexpr std::string_view BUNDLE_NAME_CAMERA("com.ohos.camera");
constexpr std::string_view BUNDLE_NAME_PHOTOS("com.ohos.photos");
}  // namespace

using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "AppSpawnServer"};

#ifdef __cplusplus
extern "C" {
#endif

static void SignalHandler([[maybe_unused]] int sig)
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            HiLog::Info(LABEL, "Process %{public}d exited cleanly %{public}d", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            if (WTERMSIG(status) != SIGKILL) {
                HiLog::Info(LABEL, "Process %{public}d exited due to signal %{public}d", pid, WTERMSIG(status));
            }
            if (WCOREDUMP(status)) {
                HiLog::Info(LABEL, "Process %{public}d dumped core.", pid);
            }
        }
    }
}

static void InstallSigHandler()
{
    char err_string[ERR_STRING_SZ];
    struct sigaction sa = {};
    sa.sa_handler = SignalHandler;
    int err = sigaction(SIGCHLD, &sa, nullptr);
    if (err < 0) {
        HiLog::Error(LABEL, "Error installing SIGCHLD handler: %{public}d",
            strerror_r(errno, err_string, ERR_STRING_SZ));
        return;
    }

    struct sigaction sah = {};
    sah.sa_handler = SIG_IGN;
    err = sigaction(SIGHUP, &sah, nullptr);
    if (err < 0) {
        HiLog::Error(LABEL, "Error installing SIGHUP handler: %{public}d",
            strerror_r(errno, err_string, ERR_STRING_SZ));
    }
}

static void UninstallSigHandler()
{
    char err_string[ERR_STRING_SZ];
    struct sigaction sa = {};
    sa.sa_handler = SIG_DFL;
    int err = sigaction(SIGCHLD, &sa, nullptr);
    if (err < 0) {
        HiLog::Error(LABEL, "Error uninstalling SIGCHLD handler: %d", strerror_r(errno, err_string, ERR_STRING_SZ));
    }

    struct sigaction sah = {};
    sah.sa_handler = SIG_DFL;
    err = sigaction(SIGHUP, &sah, nullptr);
    if (err < 0) {
        HiLog::Error(LABEL, "Error uninstalling SIGHUP handler: %d", strerror_r(errno, err_string, ERR_STRING_SZ));
    }
}
#ifdef __cplusplus
}
#endif

AppSpawnServer::AppSpawnServer(const std::string &socketName)
{
    socketName_ = socketName;
    socket_ = std::make_shared<ServerSocket>(socketName_);
    isRunning_ = true;
}

void AppSpawnServer::MsgPeer(int connectFd)
{
    char err_string[ERR_STRING_SZ];
    std::unique_ptr<AppSpawnMsgPeer> msgPeer = std::make_unique<AppSpawnMsgPeer>(socket_, connectFd);
    if (msgPeer == nullptr || msgPeer->MsgPeer() != 0) {
        HiLog::Error(LABEL, "Failed to listen connection %d, %d",
            connectFd, strerror_r(errno, err_string, ERR_STRING_SZ));
        return;
    }

    std::lock_guard<std::mutex> lock(mut_);
    appQueue_.push(std::move(msgPeer));
    dataCond_.notify_one();
}

void AppSpawnServer::ConnectionPeer()
{
    int connectFd;

    /* AppSpawn keeps receiving msg from AppMgr and never exits */
    while (isRunning_) {
        connectFd = socket_->WaitForConnection();
        if (connectFd < 0) {
            usleep(WAIT_DELAY_US);
            HiLog::Info(LABEL, "AppSpawnServer::ConnectionPeer connectFd is %{public}d", connectFd);
            continue;
        }

        mut_.lock();  // Ensure that mutex in SaveConnection is unlocked before being forked
        socket_->SaveConnection(connectFd);
        mut_.unlock();
        std::thread(&AppSpawnServer::MsgPeer, this, connectFd).detach();
    }
}

void AppSpawnServer::LoadAceLib()
{
    std::string acelibdir("/system/lib/libace.z.so");
    void *AceAbilityLib = nullptr;
    HiLog::Info(LABEL, "MainThread::LoadAbilityLibrary. Start calling dlopen acelibdir.");
    AceAbilityLib = dlopen(acelibdir.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (AceAbilityLib == nullptr) {
        HiLog::Error(LABEL, "Fail to dlopen %{public}s, [%{public}s]", acelibdir.c_str(), dlerror());
    } else {
        HiLog::Info(LABEL, "Success to dlopen %{public}s", acelibdir.c_str());
    }
    HiLog::Info(LABEL, "MainThread::LoadAbilityLibrary. End calling dlopen.");
}

bool AppSpawnServer::ServerMain(char *longProcName, int64_t longProcNameLen)
{
    if (socket_->RegisterServerSocket() != 0) {
        HiLog::Error(LABEL, "AppSpawnServer::Failed to register server socket");
        return false;
    }
    std::thread(&AppSpawnServer::ConnectionPeer, this).detach();

    LoadAceLib();

    while (isRunning_) {
        std::unique_lock<std::mutex> lock(mut_);
        dataCond_.wait(lock, [this] { return !this->appQueue_.empty(); });
        std::unique_ptr<AppSpawnMsgPeer> msg = std::move(appQueue_.front());
        appQueue_.pop();
        int connectFd = msg->GetConnectFd();
        ClientSocket::AppProperty *appProperty = msg->GetMsg();
        if (!CheckAppProperty(appProperty)) {
            msg->Response(-EINVAL);
            continue;
        }

        int32_t fd[FDLEN2] = {FD_INIT_VALUE, FD_INIT_VALUE};
        int32_t buff = 0;
        if (pipe(fd) == -1) {
            HiLog::Error(LABEL, "create pipe fail, errno = %{public}d", errno);
            msg->Response(ERR_PIPE_FAIL);
            continue;
        }

        InstallSigHandler();
        pid_t pid = fork();
        if (pid < 0) {
            HiLog::Error(LABEL, "AppSpawnServer::Failed to fork new process, errno = %{public}d", errno);
            close(fd[0]);
            close(fd[1]);
            msg->Response(-errno);
            continue;
        } else if (pid == 0) {
            SpecialHandle(appProperty);
            return SetAppProcProperty(connectFd, appProperty, longProcName, longProcNameLen, fd);
        }

        read(fd[0], &buff, sizeof(buff));  // wait child process resutl
        close(fd[0]);
        close(fd[1]);

        HiLog::Info(LABEL, "child process init %{public}s", (buff == ERR_OK) ? "success" : "fail");
        (buff == ERR_OK) ? msg->Response(pid) : msg->Response(buff);  // response to AppManagerService
        socket_->CloseConnection(connectFd);                          // close socket connection
        HiLog::Debug(LABEL, "AppSpawnServer::parent process create app finish, pid = %{public}d", pid);
    }
    return false;
}

int32_t AppSpawnServer::SetProcessName(
    char *longProcName, int64_t longProcNameLen, const char *processName, int32_t len)
{
    char err_string[ERR_STRING_SZ];
    if (longProcName == nullptr || processName == nullptr || len <= 0) {
        HiLog::Error(LABEL, "process name is nullptr or length error");
        return -EINVAL;
    }

    char shortName[MAX_LEN_SHORT_NAME];
    if (memset_s(shortName, sizeof(shortName), 0, sizeof(shortName)) != EOK) {
        HiLog::Error(LABEL, "Failed to memset short name");
        return -EINVAL;
    }

    // process short name max length 16 bytes.
    if (len > MAX_LEN_SHORT_NAME) {
        if (strncpy_s(shortName, MAX_LEN_SHORT_NAME, processName, MAX_LEN_SHORT_NAME - 1) != EOK) {
            HiLog::Error(LABEL, "strncpy_s short name error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
            return -EINVAL;
        }
    } else {
        if (strncpy_s(shortName, MAX_LEN_SHORT_NAME, processName, len) != EOK) {
            HiLog::Error(LABEL, "strncpy_s short name error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
            return -EINVAL;
        }
    }

    // set short name
    if (prctl(PR_SET_NAME, shortName) == -1) {
        HiLog::Error(LABEL, "prctl(PR_SET_NAME) error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    }

    // reset longProcName
    if (memset_s(longProcName, static_cast<size_t>(longProcNameLen), 0, static_cast<size_t>(longProcNameLen)) != EOK) {
        HiLog::Error(LABEL, "Failed to memset long process name");
        return -EINVAL;
    }

    // set long process name
    if (strncpy_s(longProcName, len, processName, len) != EOK) {
        HiLog::Error(LABEL, "strncpy_s long name error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return -EINVAL;
    }

    return ERR_OK;
}

int32_t AppSpawnServer::SetUidGid(
    const uint32_t uid, const uint32_t gid, const uint32_t *gitTable, const uint32_t gidCount)
{
    char err_string[ERR_STRING_SZ];
    if (gitTable == nullptr) {
        HiLog::Error(LABEL, "gitTable is nullptr");
        return (-errno);
    }
    // set gids
    if (setgroups(gidCount, reinterpret_cast<const gid_t *>(&gitTable[0])) == -1) {
        HiLog::Error(LABEL, "setgroups failed: %{public}d, gids.size=%{public}u",
            strerror_r(errno, err_string, ERR_STRING_SZ), gidCount);
        return (-errno);
    }

    // set gid
    if (setresgid(gid, gid, gid) == -1) {
        HiLog::Error(LABEL, "setgid(%{public}u) failed: %{public}d", gid, strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    }

    // If the effective user ID is changed from 0 to nonzero, then all capabilities are cleared from the effective set
    if (setresuid(uid, uid, uid) == -1) {
        HiLog::Error(LABEL, "setuid(%{public}u) failed: %{public}d", uid, strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    }
    return ERR_OK;
}

int32_t AppSpawnServer::SetFileDescriptors()
{
    char err_string[ERR_STRING_SZ];
    // close stdin stdout stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // redirect to /dev/null
    int dev_null_fd = open(deviceNull_.c_str(), O_RDWR);
    if (dev_null_fd == -1) {
        HiLog::Error(LABEL, "open dev_null error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    }

    // stdin
    if (dup2(dev_null_fd, STDIN_FILENO) == -1) {
        HiLog::Error(LABEL, "dup2 STDIN error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    };

    // stdout
    if (dup2(dev_null_fd, STDOUT_FILENO) == -1) {
        HiLog::Error(LABEL, "dup2 STDOUT error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    };

    // stderr
    if (dup2(dev_null_fd, STDERR_FILENO) == -1) {
        HiLog::Error(LABEL, "dup2 STDERR error: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return (-errno);
    };

    return ERR_OK;
}

int32_t AppSpawnServer::SetCapabilities()
{
    // init cap
    __user_cap_header_struct cap_header;
    if (memset_s(&cap_header, sizeof(cap_header), 0, sizeof(cap_header)) != EOK) {
        HiLog::Error(LABEL, "Failed to memset cap header");
        return -EINVAL;
    }
    cap_header.version = _LINUX_CAPABILITY_VERSION_3;
    cap_header.pid = 0;

    __user_cap_data_struct cap_data[2];
    if (memset_s(&cap_data, sizeof(cap_data), 0, sizeof(cap_data)) != EOK) {
        HiLog::Error(LABEL, "Failed to memset cap data");
        return -EINVAL;
    }

    // init inheritable permitted effective zero
#ifdef GRAPHIC_PERMISSION_CHECK
    const uint64_t inheriTable = 0;
    const uint64_t permitted = 0;
    const uint64_t effective = 0;
#else
    const uint64_t inheriTable = 0x3fffffffff;
    const uint64_t permitted = 0x3fffffffff;
    const uint64_t effective = 0x3fffffffff;
#endif

    cap_data[0].inheritable = static_cast<__u32>(inheriTable);
    cap_data[1].inheritable = static_cast<__u32>(inheriTable >> BITLEN32);
    cap_data[0].permitted = static_cast<__u32>(permitted);
    cap_data[1].permitted = static_cast<__u32>(permitted >> BITLEN32);
    cap_data[0].effective = static_cast<__u32>(effective);
    cap_data[1].effective = static_cast<__u32>(effective >> BITLEN32);

    char err_string[ERR_STRING_SZ];
    // set capabilities
    if (capset(&cap_header, &cap_data[0]) == -1) {
        HiLog::Error(LABEL, "capset failed: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return errno;
    }

    return ERR_OK;
}

void AppSpawnServer::SetRunning(bool isRunning)
{
    isRunning_ = isRunning;
}

void AppSpawnServer::SetServerSocket(const std::shared_ptr<ServerSocket> &serverSocket)
{
    socket_ = serverSocket;
}

bool AppSpawnServer::SetAppProcProperty(int connectFd, const ClientSocket::AppProperty *appProperty, char *longProcName,
    int64_t longProcNameLen, const int32_t fd[FDLEN2])
{
    if (appProperty == nullptr) {
        HiLog::Error(LABEL, "appProperty is nullptr");
        return false;
    }
    pid_t newPid = getpid();
    HiLog::Debug(LABEL, "AppSpawnServer::Success to fork new process, pid = %{public}d", newPid);
    // close socket connection and peer socket in child process
    socket_->CloseConnection(connectFd);
    socket_->CloseServerMonitor();
    close(fd[0]);  // close read fd
    UninstallSigHandler();

    int32_t ret = ERR_OK;
    ret = SetKeepCapabilities(appProperty->uid);
    if (FAILED(ret)) {
        NotifyResToParentProc(fd[1], ret);
        return false;
    }

    ret = SetProcessName(longProcName, longProcNameLen, appProperty->processName, strlen(appProperty->processName) + 1);
    if (FAILED(ret)) {
        NotifyResToParentProc(fd[1], ret);
        return false;
    }

#ifdef GRAPHIC_PERMISSION_CHECK
    ret = SetUidGid(appProperty->uid, appProperty->gid, appProperty->gidTable, appProperty->gidCount);
    if (FAILED(ret)) {
        NotifyResToParentProc(fd[1], ret);
        return false;
    }
#endif

    ret = SetFileDescriptors();
    if (FAILED(ret)) {
        NotifyResToParentProc(fd[1], ret);
        return false;
    }

    ret = SetCapabilities();
    if (FAILED(ret)) {
        NotifyResToParentProc(fd[1], ret);
        return false;
    }
    // notify success to father process and start app process
    NotifyResToParentProc(fd[1], ret);
    AppExecFwk::MainThread::Start();

    HiLog::Error(LABEL, "Failed to start process, pid = %{public}d", newPid);
    return false;
}

void AppSpawnServer::NotifyResToParentProc(const int32_t fd, const int32_t value)
{
    write(fd, &value, sizeof(value));
    close(fd);
}

void AppSpawnServer::SpecialHandle(ClientSocket::AppProperty *appProperty)
{
    if (appProperty == nullptr) {
        HiLog::Error(LABEL, "appProperty is nullptr");
        return;
    }
    // special handle bundle name "com.ohos.photos" and "com.ohos.camera"
    if ((strcmp(appProperty->processName, BUNDLE_NAME_CAMERA.data()) == 0) ||
        (strcmp(appProperty->processName, BUNDLE_NAME_PHOTOS.data()) == 0)) {
        if (appProperty->gidCount < MAX_GIDS) {
            appProperty->gidTable[appProperty->gidCount] = GID_MEDIA;
            appProperty->gidCount++;
        } else {
            HiLog::Info(LABEL, "gidCount out of bounds !");
        }
    }
}

int32_t AppSpawnServer::SetKeepCapabilities(uint32_t uid)
{
    char err_string[ERR_STRING_SZ];
    // set keep capabilities when user not root.
    if (uid != 0) {
        if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) == -1) {
            HiLog::Error(LABEL, "set keepcaps failed: %{public}d", strerror_r(errno, err_string, ERR_STRING_SZ));
            return (-errno);
        }
    }

    return ERR_OK;
}

bool AppSpawnServer::CheckAppProperty(const ClientSocket::AppProperty *appProperty)
{
    if (appProperty == nullptr) {
        HiLog::Error(LABEL, "appProperty is nullptr");
        return false;
    }

    if (appProperty->gidCount > ClientSocket::MAX_GIDS) {
        HiLog::Error(LABEL, "gidCount error: %{public}u", appProperty->gidCount);
        return false;
    }

    if (strlen(appProperty->processName) == 0) {
        HiLog::Error(LABEL, "process name length is 0");
        return false;
    }

    return true;
}
}  // namespace AppSpawn
}  // namespace OHOS
