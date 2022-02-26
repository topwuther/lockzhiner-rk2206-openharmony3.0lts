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

#ifndef APPSPAWN_SOCKET_SERVER_H
#define APPSPAWN_SOCKET_SERVER_H

#include <mutex>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "appspawn_socket.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppSpawn {
class ServerSocket : public AppSpawnSocket {
public:
    /**
     * Constructor used to create a ServerSocket
     */
    explicit ServerSocket(const std::string &server);

    /**
     * Destructor used to destory a ServerSocket
     */
    virtual ~ServerSocket();

    /**
     * Disables copying and moving for ServerSocket.
     */
    DISALLOW_COPY_AND_MOVE(ServerSocket);

    /**
     * Closes a socket connection.
     *
     * @param connectFd Indicates the connection's file descriptor (FD).
     */
    virtual void CloseConnection(int connectFd);

    /**
     * Saves the connection's FD.
     *
     * @param connectFd Indicates the connection's file descriptor (FD).
     */
    virtual void SaveConnection(int connectFd);

    /**
     * Closes a server socket.
     */
    virtual void CloseServer();

    /**
     * Closes the server monitor.
     */
    virtual void CloseServerMonitor();

    /**
     * Creates server socket, binds socket and listens it.
     *
     * @return -1:failed; 0:success
     */
    virtual int RegisterServerSocket();

    /**
     * Sets socket option and waits for connection.
     *
     * @return -1:failed;other means connection FD.
     */
    virtual int WaitForConnection();

    /**
     * Verifies the connection's FD.
     *
     * @return -1:failed; 0:success
     */
    virtual int VerifyConnection(int connectFd);

    /**
     * Uses functions of the parent class.
     */
    using AppSpawnSocket::CloseSocket;
    using AppSpawnSocket::CreateSocket;
    using AppSpawnSocket::GetSocketFd;
    using AppSpawnSocket::ReadSocketMessage;
    using AppSpawnSocket::WriteSocketMessage;

    static constexpr uid_t APPSPAWN_ID_ROOT = 0;         // chown owner
    static constexpr gid_t APPSPAWN_ID_SYSTEM = 1000;    // chown group
    static constexpr mode_t SOCKET_PERM = 0660;  // root system can read and write appspawn socket

private:
    /**
     * Binds a socket and sets socket attributes.
     *
     * @param connectFd Indicates the connection's FD.
     */
    int BindSocket(int connectFd);

    /**
     * Creates a socket and binds it.
     *
     * @param connectFd Indicates the connection's FD.
     */
    int RegisterServerSocket(int &connectFd);

    /**
     * Accepts a socket.
     *
     * @param connectFd Indicates the connection's FD.
     */
    int WaitForConnection(int connectFd);

private:
    std::vector<int> connectFds_ = {};
    std::mutex mutexConnect_;
};
}  // namespace AppSpawn
}  // namespace OHOS
#endif
