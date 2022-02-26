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

#include "appspawn_socket.h"

#include <cerrno>
#include <iostream>
#include <sys/socket.h>

#include "hilog/log.h"
#include "pubdef.h"
#include "securec.h"

namespace OHOS {
namespace AppSpawn {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "AppSpawnSocket"};
constexpr static size_t ERR_STRING_SZ = 64;

AppSpawnSocket::AppSpawnSocket(const std::string &name)
{
    socketName_ = name;
}

AppSpawnSocket::~AppSpawnSocket()
{
    if (socketFd_ > 0) {
        CloseSocket(socketFd_);
        socketFd_ = -1;
    }
}

int AppSpawnSocket::GetSocketFd() const
{
    return socketFd_;
}

int AppSpawnSocket::PackSocketAddr()
{
    if (socketName_.empty()) {
        HiLog::Error(LABEL, "Invalid socket name: empty");
        return -1;
    }

    if (memset_s(&socketAddr_, sizeof(socketAddr_), 0, sizeof(socketAddr_)) != EOK) {
        HiLog::Error(LABEL, "Failed to memset socket addr");
        return -1;
    }

    socklen_t pathLen = socketDir_.length() + socketName_.length();
    socklen_t pathSize = sizeof(socketAddr_.sun_path);
    if (pathLen >= pathSize) {
        HiLog::Error(LABEL, "Invalid socket name: '%s' too long", socketName_.c_str());
        return -1;
    }

    int len =
        snprintf_s(socketAddr_.sun_path, pathSize, (pathSize - 1), "%s%s", socketDir_.c_str(), socketName_.c_str());
    if (static_cast<int>(pathLen) != len) {
        HiLog::Error(LABEL, "Failed to copy socket path");
        return -1;
    }

    socketAddr_.sun_family = AF_LOCAL;
    socketAddrLen_ = offsetof(struct sockaddr_un, sun_path) + pathLen + 1;

    return 0;
}

int AppSpawnSocket::CreateSocket()
{
    char err_string[ERR_STRING_SZ];
    int socketFd = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
    if (socketFd < 0) {
        HiLog::Error(LABEL, "Failed to create socket: %d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return -1;
    }

    HiLog::Debug(LABEL, "Created socket with fd %d", socketFd);
    return socketFd;
}

void AppSpawnSocket::CloseSocket(int &socketFd)
{
    if (socketFd >= 0) {
        HiLog::Debug(LABEL, "Closed socket with fd %d", socketFd);
        close(socketFd);
        socketFd = -1;
    }
}

int AppSpawnSocket::ReadSocketMessage(int socketFd, void *buf, int len)
{
    char err_string[ERR_STRING_SZ];
    if (socketFd < 0 || len <= 0 || buf == nullptr) {
        HiLog::Error(LABEL, "Invalid args: socket %d, len %d, buf might be nullptr", socketFd, len);
        return -1;
    }

    if (memset_s(buf, len, 0, len) != EOK) {
        HiLog::Warn(LABEL, "Failed to memset read buf");
        return -1;
    }

    ssize_t rLen = TEMP_FAILURE_RETRY(read(socketFd, buf, len));
    if (rLen < 0) {
        HiLog::Error(LABEL, "Read message from fd %d error %zd: %d",
            socketFd, rLen, strerror_r(errno, err_string, ERR_STRING_SZ));
        return -1;
    }

    return rLen;
}

int AppSpawnSocket::WriteSocketMessage(int socketFd, const void *buf, int len)
{
    char err_string[ERR_STRING_SZ];
    if (socketFd < 0 || len <= 0 || buf == nullptr) {
        HiLog::Error(LABEL, "Invalid args: socket %d, len %d, buf might be nullptr", socketFd, len);
        return -1;
    }

    ssize_t written = 0;
    ssize_t remain = static_cast<ssize_t>(len);
    const uint8_t *offset = reinterpret_cast<const uint8_t *>(buf);
    for (ssize_t wLen = 0; remain > 0; offset += wLen, remain -= wLen, written += wLen) {
        wLen = write(socketFd, offset, remain);
        HiLog::Debug(LABEL, "socket fd %d, wLen %zd", socketFd, wLen);
        if ((wLen <= 0) && (errno != EINTR)) {
            HiLog::Error(LABEL, "Failed to write message to fd %d, error %zd: %d",
                socketFd, wLen, strerror_r(errno, err_string, ERR_STRING_SZ));
            return -1;
        }
    }

    return written;
}
}  // namespace AppSpawn
}  // namespace OHOS
