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

#include "appspawn_msg_peer.h"

#include <memory>

#include "hilog/log.h"
#include "securec.h"

namespace OHOS {
namespace AppSpawn {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "AppSpawnMain"};

AppSpawnMsgPeer::AppSpawnMsgPeer(const std::shared_ptr<ServerSocket> &socket, int connectFd)
    : connectFd_(connectFd), socket_(socket)
{}

AppSpawnMsgPeer::~AppSpawnMsgPeer()
{
    if ((socket_ != nullptr) && (connectFd_ >= 0)) {
        socket_->CloseConnection(connectFd_);
    }
}

ClientSocket::AppProperty *AppSpawnMsgPeer::GetMsg() const
{
    return reinterpret_cast<ClientSocket::AppProperty *>(buf_.get());
}

int AppSpawnMsgPeer::GetConnectFd() const
{
    return connectFd_;
}

int AppSpawnMsgPeer::Response(pid_t pid)
{
    if ((socket_ == nullptr) || (connectFd_ < 0)) {
        HiLog::Error(LABEL, "Invalid socket params: connectFd %d", connectFd_);
        return -1;
    }

    if (socket_->WriteSocketMessage(connectFd_, &pid, sizeof(pid)) != sizeof(pid)) {
        HiLog::Error(LABEL, "Failed to write message: connectFd %d", connectFd_);
        return -1;
    }

    return 0;
}

int AppSpawnMsgPeer::MsgPeer()
{
    if ((socket_ == nullptr) || (connectFd_ < 0)) {
        HiLog::Error(LABEL, "Failed to init socket: connectFd %{public}d", connectFd_);
        return -1;
    }

    int32_t msgLen = sizeof(ClientSocket::AppProperty);
    buf_ = std::make_unique<int8_t[]>(msgLen);
    if (buf_ == nullptr) {
        HiLog::Error(LABEL, "buf_ is null pointer!");
        return -1;
    }

    int32_t rLen = 0;

    for (int8_t *offset = buf_.get(); msgLen > 0; offset += rLen, msgLen -= rLen) {
        rLen = socket_->ReadSocketMessage(connectFd_, offset, msgLen);
        if (rLen == 0) {
            HiLog::Info(LABEL, "AppSpawnMsgPeer::MsgPeer ReadSocketMessage function value is 0.");
            break;
        }

        if ((rLen < 0) || (rLen > msgLen)) {
            HiLog::Error(LABEL, "AppSpawnMsgPeer::Failed to read msg from socket %{public}d", connectFd_);
            return -1;
        }
    }

    return 0;
}
}  // namespace AppSpawn
}  // namespace OHOS
