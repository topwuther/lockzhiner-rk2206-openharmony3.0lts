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

#ifndef APPSPAWN_SOCKET_H
#define APPSPAWN_SOCKET_H

#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include "nocopyable.h"

namespace OHOS {
namespace AppSpawn {
class AppSpawnSocket {
public:
    /**
     * Constructor used to delete the default constructor.
     */
    AppSpawnSocket() = delete;

    /**
     * Constructor used to create a AppSpawnSocket.
     */
    explicit AppSpawnSocket(const std::string &name);

    /**
     * Destructor used to destory a AppSpawnSocket
     */
    virtual ~AppSpawnSocket();

    /**
     * Disables copying and moving for the AppSpawnSocket.
     */
    DISALLOW_COPY_AND_MOVE(AppSpawnSocket);

    /**
     * Gets the socket's file descriptor (FD).
     */
    int GetSocketFd() const;

    /**
     * Reads messages from the AppSpawn socket.
     *
     * @param socketFd Indicates the socket's FD.
     * @param buf Indicates the message buffer.
     * @param len Indicates the message size.
     *
     * @return -1:failed;other means message size.
     */
    virtual int ReadSocketMessage(int socketFd, void *buf, int len);

    /**
     * Writes messages to the AppSpawn socket.
     *
     * @param socketFd Indicates the socket's FD.
     * @param buf  Indicates the message buffer.
     * @param len Indicates the message size.
     *
     * @return -1:failed;other means message size.
     */
    virtual int WriteSocketMessage(int socketFd, const void *buf, int len);

    /**
     * Closes an AppSpawn socket.
     *
     * @param socketFd Indicates the socket's FD.
     */
    static void CloseSocket(int &socketFd);

    /**
     * Creates an AppSpawn socket.
     *
     * @return -1:failed;other means connection FD.
     */
    static int CreateSocket();

protected:
    /**
     * Sets the socket name to the socket address.
     *
     * @return -1:failed; 0:success
     */
    int PackSocketAddr();

protected:
    int socketFd_ = -1;
    std::string socketName_ {};
    struct sockaddr_un socketAddr_ {};
    socklen_t socketAddrLen_ = 0;
#ifdef __MUSL__
    const std::string socketDir_ = "/dev/unix/socket/";
#else
    const std::string socketDir_ = "/dev/socket/";
#endif
    const unsigned int listenBacklog_ = 50;                   // 50: max num of clients
    static constexpr struct timeval SOCKET_TIMEOUT = {5, 0};  // 5, 0: { 5 sec, 0 msec } for timeout
};
}  // namespace AppSpawn
}  // namespace OHOS
#endif
