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
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "MockAppSpawnMsgPeer"};

AppSpawnMsgPeer::AppSpawnMsgPeer(const std::shared_ptr<ServerSocket> &socket, int connectFd)
    : connectFd_(connectFd), socket_(socket)
{
    buf_ = std::make_unique<int8_t[]>(sizeof(ClientSocket::AppProperty));
    if (buf_ == nullptr) {
        HiLog::Error(LABEL, "buf_ is null pointer!");
    }
}

AppSpawnMsgPeer::~AppSpawnMsgPeer()
{
    connectFd_ = -1;
}

ClientSocket::AppProperty *AppSpawnMsgPeer::GetMsg() const
{
    HiLog::Info(LABEL, "GetMsg");
    if (buf_ != nullptr) {
        ClientSocket::AppProperty *appProperty = reinterpret_cast<ClientSocket::AppProperty *>(buf_.get());
        appProperty->uid = 0;
        appProperty->gid = 0;
        auto ret = memset_s(appProperty->gidTable, sizeof(appProperty->gidTable), 0, sizeof(appProperty->gidTable));
        if (ret != EOK) {
            HiLog::Error(LABEL, "memset_s failed!");
            return nullptr;
        }
        appProperty->gidCount = ClientSocket::MAX_GIDS;
        std::string processName = "processName";
        ret = memcpy_s(
            appProperty->processName, sizeof(appProperty->processName), processName.c_str(), processName.length());
        if (ret != EOK) {
            HiLog::Error(LABEL, "memcpy_s failed!");
            return nullptr;
        }
        std::string soPath = "soPath";
        ret = memcpy_s(appProperty->soPath, sizeof(appProperty->soPath), soPath.c_str(), soPath.length());
        if (ret != EOK) {
            HiLog::Error(LABEL, "memcpy_s failed!");
            return nullptr;
        }
    }

    return reinterpret_cast<ClientSocket::AppProperty *>(buf_.get());
}

int AppSpawnMsgPeer::GetConnectFd() const
{
    HiLog::Info(LABEL, "GetConnectFd");
    return 0;
}

int AppSpawnMsgPeer::Response(pid_t pid)
{
    HiLog::Info(LABEL, "Response");
    return 0;
}

int AppSpawnMsgPeer::MsgPeer()
{
    HiLog::Info(LABEL, "MsgPeer");
    return 0;
}
}  // namespace AppSpawn
}  // namespace OHOS
