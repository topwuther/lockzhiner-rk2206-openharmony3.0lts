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

#include "event_server.h"

#include <memory>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <securec.h>

#ifdef USE_MUSL
extern "C" {
#include "init_socket.h"
}
#endif

#include "logger.h"
#include "socket_util.h"

#ifdef USE_MUSL
#define SOCKET_FILE_DIR "/dev/unix/socket/hisysevent"
#else
#define SOCKET_FILE_DIR "/dev/socket/hisysevent"
#endif

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventServer");
namespace {
constexpr int BUFFER_SIZE = 384 * 1024;
static void InitRecvBuffer(int socketId)
{
    int oldN = 0;
    socklen_t oldOutSize = sizeof(int);
    if (getsockopt(socketId, SOL_SOCKET, SO_RCVBUF, static_cast<void *>(&oldN), &oldOutSize) < 0) {
        HIVIEW_LOGE("get socket buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }

    int sendBuffSize = BUFFER_SIZE;
    if (setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, static_cast<void *>(&sendBuffSize), sizeof(int)) < 0) {
        HIVIEW_LOGE("set socket buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }

    int newN = 0;
    socklen_t newOutSize = sizeof(int);
    if (getsockopt(socketId, SOL_SOCKET, SO_RCVBUF, static_cast<void *>(&newN), &newOutSize) < 0) {
        HIVIEW_LOGE("get new socket buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }
    HIVIEW_LOGI("reset recv buffer size old=%{public}d, new=%{public}d", oldN, newN);
}
}
void EventServer::InitSocket(int &socketId)
{
    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    if (strcpy_s(serverAddr.sun_path, sizeof(serverAddr.sun_path), SOCKET_FILE_DIR) != EOK) {
        socketId = -1;
        HIVIEW_LOGE("copy hisysevent dev path fail %{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
    serverAddr.sun_path[sizeof(serverAddr.sun_path) - 1] = '\0';
    socketId = TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    if (socketId < 0) {
        HIVIEW_LOGE("create hisysevent socket fail %{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
    InitRecvBuffer(socketId_);
    unlink(serverAddr.sun_path);
    if (TEMP_FAILURE_RETRY(bind(socketId, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr))) < 0) {
        close(socketId);
        socketId = -1;
        HIVIEW_LOGE("bind hisysevent socket fail %{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
}

void EventServer::Start()
{
    HIVIEW_LOGE("start event server");
#ifdef USE_MUSL
    socketId_ = GetControlSocket("hisysevent");
#else
    socketId_ = SocketUtil::GetHiviewExistingSocketServer("hisysevent", SOCK_DGRAM);
#endif
    if (socketId_ < 0) {
        HIVIEW_LOGI("create hisysevent socket");
        InitSocket(socketId_);
    } else {
        InitRecvBuffer(socketId_);
        HIVIEW_LOGI("use hisysevent exist socket");
    }

    if (socketId_ < 0) {
        HIVIEW_LOGE("hisysevent create socket fail");
        return;
    }

    isStart_ = true;
    while (isStart_) {
        struct sockaddr_un clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        char* recvbuf = new char[BUFFER_SIZE];
        int n = recvfrom(socketId_, recvbuf, sizeof(char) * BUFFER_SIZE, 0,
            reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (n <= 0) {
            delete[] recvbuf;
            continue;
        }
        recvbuf[BUFFER_SIZE - 1] = 0;
        HIVIEW_LOGD("receive data from client %s", recvbuf);
        for (auto receiver = receivers_.begin(); receiver != receivers_.end(); receiver++) {
            (*receiver)->HandlerEvent(std::string(recvbuf));
        }
        delete[] recvbuf;
    }
}

void EventServer::Stop()
{
    isStart_ = false;
    if (socketId_ <= 0) {
        return;
    }
    close(socketId_);
    socketId_ = -1;
}

void EventServer::AddReceiver(std::shared_ptr<EventReceiver> receiver)
{
    receivers_.emplace_back(receiver);
}
} // namespace HiviewDFX
} // namespace OHOS
