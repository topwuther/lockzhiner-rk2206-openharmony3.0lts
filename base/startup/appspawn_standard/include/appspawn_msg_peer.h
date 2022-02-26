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

#ifndef APPSPAWN_MSG_PEER_H
#define APPSPAWN_MSG_PEER_H

#include <string>

#include "client_socket.h"
#include "server_socket.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppSpawn {
class AppSpawnMsgPeer {
public:
    /**
     * Constructor used to delete the default constructor.
     */
    AppSpawnMsgPeer() = delete;

    /**
     * Constructor used to create a AppSpawnMsgPeer.
     */
    AppSpawnMsgPeer(const std::shared_ptr<ServerSocket> &socket, int connectFd);

    /**
     * Destructor used to destory a AppSpawnMsgPeer
     */
    ~AppSpawnMsgPeer();

    /**
     * Disables copy and moving of AppSpawnMsgPeer.
     */
    DISALLOW_COPY_AND_MOVE(AppSpawnMsgPeer);

    /**
     * Gets the message about the app property.
     */
    ClientSocket::AppProperty *GetMsg() const;

    /**
     * Returns the PID of the application process in the response sent to the AMS service after AppSpawn forks the
     * application process.
     *
     * @param pid Indicates the PID of the application process.
     */
    int Response(pid_t pid);

    /**
     * Reads the message from MsgPeer over the socket.
     */
    int MsgPeer();

    /**
     * Gets the connection file description used by the AMS service to connect to AppSpawn.
     */
    int GetConnectFd() const;

private:
    int connectFd_ = -1;
    std::shared_ptr<ServerSocket> socket_ = nullptr;
    std::unique_ptr<int8_t[]> buf_ = nullptr;
};
}  // namespace AppSpawn
}  // namespace OHOS
#endif
