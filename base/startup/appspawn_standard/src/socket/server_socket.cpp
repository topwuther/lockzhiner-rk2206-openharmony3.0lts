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

#include "server_socket.h"

#include <cerrno>
#include <iostream>
#include <sys/socket.h>

#include "hilog/log.h"
#include "securec.h"

namespace OHOS {
namespace AppSpawn {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "ServerSocket"};
constexpr static size_t ERR_STRING_SZ = 64;

ServerSocket::ServerSocket(const std::string &server) : AppSpawnSocket(server)
{}

ServerSocket::~ServerSocket()
{
    CloseServer();
}

int ServerSocket::VerifyConnection(int connectFd)
{
    std::lock_guard<std::mutex> lock(mutexConnect_);

    std::vector<int>::iterator it = find(connectFds_.begin(), connectFds_.end(), connectFd);
    if (it == connectFds_.end()) {
        return -1;
    }

    return 0;
}

void ServerSocket::CloseConnection(int connectFd)
{
    if (connectFd < 0) {
        HiLog::Error(LABEL, "Server: Invalid connectFd %d", connectFd);
        return;
    }

    std::lock_guard<std::mutex> lock(mutexConnect_);

    std::vector<int>::iterator it = find(connectFds_.begin(), connectFds_.end(), connectFd);
    if (it == connectFds_.end()) {
        close(connectFd);
        return;
    }

    close(connectFd);
    connectFds_.erase(it);
    HiLog::Debug(LABEL, "Server: Erase connect fd %d from list", connectFd);
}

void ServerSocket::SaveConnection(int connectFd)
{
    if (connectFd >= 0) {
        std::lock_guard<std::mutex> lock(mutexConnect_);
        connectFds_.push_back(connectFd);
    }
}

void ServerSocket::CloseServer()
{
    std::lock_guard<std::mutex> lock(mutexConnect_);

    for (const int &fd : connectFds_) {
        HiLog::Debug(LABEL, "Server: Closed connection fd %d", fd);
        close(fd);
    }
    connectFds_.clear();
    if (socketFd_ >= 0) {
        CloseSocket(socketFd_);
        socketFd_ = -1;
    }
}

void ServerSocket::CloseServerMonitor()
{
    if (socketFd_ >= 0) {
        CloseSocket(socketFd_);
        socketFd_ = -1;
    }
}

int ServerSocket::BindSocket(int connectFd)
{
    char err_string[ERR_STRING_SZ];
    if (connectFd < 0) {
        HiLog::Error(LABEL, "Server: Invalid socket fd: %d", connectFd);
        return -1;
    }

    if (PackSocketAddr() != 0) {
        return -1;
    }

    if ((unlink(socketAddr_.sun_path) != 0) && (errno != ENOENT)) {
        HiLog::Error(LABEL, "Server: Failed to unlink, err %d", strerror_r(errno, err_string, ERR_STRING_SZ));
        return -1;
    }

    int reuseAddr = 0;
    if ((setsockopt(connectFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) != 0) ||
        (setsockopt(connectFd, SOL_SOCKET, SO_RCVTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) != 0) ||
        (setsockopt(connectFd, SOL_SOCKET, SO_SNDTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) != 0)) {
        HiLog::Warn(LABEL, "Server: Failed to set opt of socket %d, err %d",
            connectFd, strerror_r(errno, err_string, ERR_STRING_SZ));
    }

    if (bind(connectFd, reinterpret_cast<struct sockaddr *>(&socketAddr_), socketAddrLen_) < 0) {
        HiLog::Error(LABEL, "Server: Bind socket fd %d, failed: %d",
            connectFd, strerror_r(errno, err_string, ERR_STRING_SZ));
        return -1;
    }

    if (chown(socketAddr_.sun_path, APPSPAWN_ID_ROOT, APPSPAWN_ID_SYSTEM)) {
        HiLog::Error(LABEL, "Server: failed to chown socket fd %d, failed: %d",
            connectFd, strerror_r(errno, err_string, ERR_STRING_SZ));
        return -1;
    }
    if (chmod(socketAddr_.sun_path, SOCKET_PERM)) {
        HiLog::Error(LABEL, "Server: failed to chmod socket fd %d, failed: %d",
            connectFd, strerror_r(errno, err_string, ERR_STRING_SZ));
        if ((unlink(socketAddr_.sun_path) != 0) && (errno != ENOENT)) {
            HiLog::Error(LABEL, "Server: Failed to unlink, err %d", strerror_r(errno, err_string, ERR_STRING_SZ));
        }
        return -1;
    }
    HiLog::Debug(LABEL, "Server: Bind socket fd %d", connectFd);
    return 0;
}

int ServerSocket::RegisterServerSocket(int &connectFd)
{
    char err_string[ERR_STRING_SZ];
    if (socketName_.empty()) {
        HiLog::Error(LABEL, "Server: Invalid socket name: empty");
        return -1;
    }

    connectFd = CreateSocket();
    if (connectFd < 0) {
        return -1;
    }

    if ((BindSocket(connectFd) != 0) || (listen(connectFd, listenBacklog_) < 0)) {
        HiLog::Error(LABEL,
            "Server: Register socket fd %d with backlog %d error: %d",
            connectFd,
            listenBacklog_,
            strerror_r(errno, err_string, ERR_STRING_SZ));
        close(connectFd);
        connectFd = -1;
        return -1;
    }

    HiLog::Debug(LABEL, "Server: Suc to register server socket fd %d", connectFd);
    return 0;
}

int ServerSocket::RegisterServerSocket()
{
    if (socketFd_ >= 0) {
        HiLog::Info(LABEL, "Server: Already register server socket %d", socketFd_);
        return 0;
    }

    return RegisterServerSocket(socketFd_);
}

int ServerSocket::WaitForConnection(int connectFd)
{
    char err_string[ERR_STRING_SZ];
    if (connectFd < 0) {
        HiLog::Error(LABEL, "Server: Invalid args: connectFd %d", connectFd);
        return -1;
    }

    struct sockaddr_un clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    if (memset_s(&clientAddr, clientLen, 0, clientLen) != EOK) {
        HiLog::Warn(LABEL, "Server: Failed to memset client addr");
    }

    int connFd = accept(connectFd, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientLen);
    if (connFd < 0) {
        return -1;
    }

    if ((setsockopt(connFd, SOL_SOCKET, SO_RCVTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) < 0) ||
        (setsockopt(connFd, SOL_SOCKET, SO_SNDTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) < 0)) {
        HiLog::Warn(LABEL, "Server: Failed to set opt of Connection %d, err %d",
            connFd, strerror_r(errno, err_string, ERR_STRING_SZ));
    }

    HiLog::Debug(LABEL, "Server: Connection accepted, connect fd %d", connFd);
    return connFd;
}

int ServerSocket::WaitForConnection()
{
    int connectFd = WaitForConnection(socketFd_);
    SaveConnection(connectFd);

    return connectFd;
}
}  // namespace AppSpawn
}  // namespace OHOS
