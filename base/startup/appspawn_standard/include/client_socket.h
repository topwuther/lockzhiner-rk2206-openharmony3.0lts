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

#ifndef APPSPAWN_SOCKET_CLIENT_H
#define APPSPAWN_SOCKET_CLIENT_H

#include "appspawn_socket.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppSpawn {
class ClientSocket : public AppSpawnSocket {
public:
    /**
     * Constructor used to create a ClientSocket
     */
    explicit ClientSocket(const std::string &client);

    /**
     * Destructor used to destory a ClientSocket
     */
    virtual ~ClientSocket() = default;

    /**
     * Disables copying and moving for the ClientSocket.
     */
    DISALLOW_COPY_AND_MOVE(ClientSocket);

    /**
     * Creates a local client socket.
     */
    virtual int CreateClient();

    /**
     * Closes a client socket.
     */
    virtual void CloseClient();

    /**
     * Connects a client socket.
     */
    virtual int ConnectSocket();

    /**
     * Writes messages to a client socket.
     *
     * @param buf Indicates the pointer to the message buffer.
     * @param len Indicates the message length.
     */
    virtual int WriteSocketMessage(const void *buf, int len);

    /**
     * Reads messages from a client socket.
     *
     * @param buf Indicates the pointer to the message buffer.
     * @param len Indicates the message length.
     */
    virtual int ReadSocketMessage(void *buf, int len);

    /**
     * Uses functions of the parent class.
     */
    using AppSpawnSocket::CloseSocket;
    using AppSpawnSocket::CreateSocket;
    using AppSpawnSocket::GetSocketFd;
    using AppSpawnSocket::ReadSocketMessage;
    using AppSpawnSocket::WriteSocketMessage;

    enum AppType {
        APP_TYPE_DEFAULT = 0,  // JavaScript app
        APP_TYPE_NATIVE        // Native C++ app
    };

    static constexpr int APPSPAWN_MSG_MAX_SIZE = 4096;  // appspawn message max size
    static constexpr int LEN_PROC_NAME = 256;           // process name length
    static constexpr int LEN_SO_PATH = 256;             // load so lib
    static constexpr int MAX_GIDS = 64;

    struct AppProperty {
        uint32_t uid;                     // the UNIX uid that the child process setuid() to after fork()
        uint32_t gid;                     // the UNIX gid that the child process setgid() to after fork()
        uint32_t gidTable[MAX_GIDS];      // a list of UNIX gids that the child process setgroups() to after fork()
        uint32_t gidCount;                // the size of gidTable
        char processName[LEN_PROC_NAME];  // process name
        char soPath[LEN_SO_PATH];         // so lib path
    };

private:
    /**
     * Connects a client socket.
     *
     * @param connectFd Indicates the connection's FD.
     */
    int ConnectSocket(int connectFd);
};
}  // namespace AppSpawn
}  // namespace OHOS
#endif
