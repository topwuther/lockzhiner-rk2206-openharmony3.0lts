/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <directory_ex.h>
#include <file_ex.h>
#include <hilog/log.h>
#include <securec.h>

#include "fault_logger_daemon.h"
#include "faultloggerd_client.h"

static const OHOS::HiviewDFX::HiLogLabel g_LOG_LABLE = {LOG_CORE, 0xD002D20, "FaultLoggerd"};
namespace OHOS {
namespace HiviewDFX {
using FaultLoggerdRequest = struct FaultLoggerdRequest;
namespace {
constexpr int32_t MAX_CONNECTION = 4;
constexpr int32_t REQUEST_BUF_SIZE = 1024;
constexpr int32_t MSG_BUF_SIZE = 256;
constexpr int32_t SYSTEM_UID = 1000;
#if defined(USE_MUSL)
static const char FAULTLOGGERD_SOCK_PATH[] = "/dev/unix/socket/faultloggerd.server";
#else
static const char FAULTLOGGERD_SOCK_PATH[] = "/dev/socket/faultloggerd.server";
#endif
static const char FAULTLOGGERD_BASE_PATH[] = "/data/log/faultlog/temp/";

static std::string GetRequestTypeName(int32_t type)
{
    switch (type) {
        case CPP_CRASH:
            return "cppcrash";
        case CPP_STACKTRACE:
            return "stacktrace";
        default:
            return "unsupported";
    }
}

static void SendFileDescriptorBySocket(int socket, int fd)
{
    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(fd))] = { 0 };
    char iovBase[] = "";
    struct iovec io = {
        .iov_base = reinterpret_cast<void *>(iovBase),
        .iov_len = 1
    };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    *(reinterpret_cast<int *>(CMSG_DATA(cmsg))) = fd;
    msg.msg_controllen = CMSG_SPACE(sizeof(fd));
    if (sendmsg(socket, &msg, 0) < 0) {
        OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Failed to send message");
    }
}

__attribute__((unused)) static int ReadFileDescriptorFromSocket(int socket)
{
    struct msghdr msg = { 0 };

    char msgBuffer[MSG_BUF_SIZE] = { 0 };
    struct iovec io = {
        .iov_base = msgBuffer,
        .iov_len = sizeof(msgBuffer)
    };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char ctlBuffer[MSG_BUF_SIZE] = { 0 };
    msg.msg_control = ctlBuffer;
    msg.msg_controllen = sizeof(ctlBuffer);

    if (recvmsg(socket, &msg, 0) < 0) {
        OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Failed to receive message");
        return -1;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg == nullptr) {
        return -1;
    }
    return *(reinterpret_cast<int *>(CMSG_DATA(cmsg)));
}
} // namespace

bool FaultLoggerDaemon::InitEnvironment()
{
    if (!OHOS::ForceCreateDirectory(FAULTLOGGERD_BASE_PATH)) {
        return false;
    }

    if (chmod(FAULTLOGGERD_SOCK_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH) < 0) {
        HiLog::Warn(g_LOG_LABLE, "Failed to chmod, %{public}s", strerror(errno));
    }

    std::vector<std::string> files;
    OHOS::GetDirFiles(FAULTLOGGERD_BASE_PATH, files);
    currentLogCounts_ = files.size();
    setuid(SYSTEM_UID);
    setgid(SYSTEM_UID);
    return true;
}

void FaultLoggerDaemon::HandleRequest(int32_t connectionFd)
{
    int nread = -1;
    char buf[REQUEST_BUF_SIZE] = {0};
    while (true) {
        nread = read(connectionFd, buf, sizeof(buf));
        if (nread < 0) {
            OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Failed to read message");
            break;
        } else if (nread == 0) {
            OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Read null from request socket");
            break;
        } else if (nread != sizeof(FaultLoggerdRequest)) {
            OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Unmatched request length");
            break;
        }

        RemoveTempFileIfNeed();
        auto request = reinterpret_cast<FaultLoggerdRequest *>(buf);
        int fd = CreateFileForRequest(request->type, request->pid);
        if (fd < 0) {
            OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Failed to create log file");
            break;
        }
        currentLogCounts_++;
        SendFileDescriptorBySocket(connectionFd, fd);
        close(fd);
        break;
    }
}

int32_t FaultLoggerDaemon::CreateFileForRequest(int32_t type, int32_t pid) const
{
    if (type != CPP_STACKTRACE && type != CPP_CRASH) {
        OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Unsupported request type");
        return -1;
    }

    std::string path = std::string(FAULTLOGGERD_BASE_PATH) + GetRequestTypeName(type) + "-" + std::to_string(pid) +
        "-" + std::to_string(time(nullptr));

    return open(path.c_str(), O_RDWR | O_CREAT, 00660); // 00660 :rw-rw----
}

void FaultLoggerDaemon::RemoveTempFileIfNeed()
{
    const int maxFileCount = 50;
    if (currentLogCounts_ < maxFileCount) {
        return;
    }

    std::vector<std::string> files;
    OHOS::GetDirFiles(FAULTLOGGERD_BASE_PATH, files);
    std::sort(files.begin(), files.end(),
        [](const std::string& lhs, const std::string& rhs) -> int
    {
        auto lhsSplitPos = lhs.find_last_of("-");
        auto rhsSplitPos = rhs.find_last_of("-");
        if (lhsSplitPos == std::string::npos || rhsSplitPos == std::string::npos) {
            return lhs.compare(rhs) > 0;
        }

        return lhs.substr(lhsSplitPos).compare(rhs.substr(rhsSplitPos)) > 0; 
    });

    int startIndex = maxFileCount / 2;
    for (int index = startIndex; index < files.size(); index++) {
        OHOS::RemoveFile(files[index]);
        OHOS::HiviewDFX::HiLog::Warn(g_LOG_LABLE, "Remove temp fault file:%{public}s", files[index].c_str());
    }
    currentLogCounts_ = startIndex;
}
} // namespace HiviewDFX
} // namespace OHOS

int32_t StartServer(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int socketFd;
    if ((socketFd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        OHOS::HiviewDFX::HiLog::Error(g_LOG_LABLE, "Failed to create socket");
        return -1;
    }

    struct sockaddr_un server;
    memset_s(&server, sizeof(server), 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    if (strncpy_s(server.sun_path, sizeof(server.sun_path), OHOS::HiviewDFX::FAULTLOGGERD_SOCK_PATH,
        strlen(OHOS::HiviewDFX::FAULTLOGGERD_SOCK_PATH)) != 0) {
        OHOS::HiviewDFX::HiLog::Error(g_LOG_LABLE, "Failed to set sock path");
        close(socketFd);
        return -1;
    }

    unlink(OHOS::HiviewDFX::FAULTLOGGERD_SOCK_PATH);
    if (bind(socketFd, (struct sockaddr *)&server,
        offsetof(struct sockaddr_un, sun_path) + strlen(server.sun_path)) < 0) {
        OHOS::HiviewDFX::HiLog::Error(g_LOG_LABLE, "Failed to bind socket");
        close(socketFd);
        return -1;
    }

    if (listen(socketFd, OHOS::HiviewDFX::MAX_CONNECTION) < 0) {
        OHOS::HiviewDFX::HiLog::Error(g_LOG_LABLE, "Failed to listen socket");
        close(socketFd);
        return -1;
    }

    OHOS::HiviewDFX::FaultLoggerDaemon deamon;
    if (!deamon.InitEnvironment()) {
        OHOS::HiviewDFX::HiLog::Error(g_LOG_LABLE, "Failed to init environment");
        close(socketFd);
        return -1;
    }

    struct sockaddr_un clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int connectionFd = -1;
    while (true) {
        if ((connectionFd = accept(socketFd, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientAddrSize)) < 0) {
            OHOS::HiviewDFX::HiLog::Error(g_LOG_LABLE, "Failed to accpet connection");
            continue;
        }

        deamon.HandleRequest(connectionFd);
        close(connectionFd);
        connectionFd = -1;
    }
    close(socketFd);
    return 0;
}
