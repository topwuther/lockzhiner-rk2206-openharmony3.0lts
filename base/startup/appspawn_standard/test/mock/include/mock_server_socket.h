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
#include "client_socket.h"

#include "gmock/gmock.h"
#include "securec.h"
#include "hilog/log.h"

namespace OHOS {
namespace AppSpawn {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "MockServerSocket"};

class MockServerSocket : public ServerSocket {
public:
    MockServerSocket() = delete;
    virtual ~MockServerSocket() = default;
    MockServerSocket(const std::string &server) : ServerSocket(server){};

    MOCK_METHOD1(SaveConnection, void(int32_t connectFd));
    MOCK_METHOD1(CloseConnection, void(int32_t connectFd));
    MOCK_METHOD0(CloseServer, void());
    MOCK_METHOD0(CloseServerMonitor, void());
    MOCK_METHOD0(RegisterServerSocket, int32_t());
    MOCK_METHOD0(WaitForConnection, int32_t());
    MOCK_METHOD1(VerifyConnection, int32_t(int32_t connectFd));
    MOCK_METHOD3(ReadSocketMessage, int32_t(int32_t socketFd, void *buf, int32_t len));
    MOCK_METHOD3(WriteSocketMessage, int32_t(int32_t socketFd, const void *buf, int32_t len));

    int32_t ReadImplGidCountMax([[maybe_unused]] int32_t socketFd, void *buf, [[maybe_unused]] int32_t len)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // assign AppProperty
        ClientSocket::AppProperty *params = static_cast<ClientSocket::AppProperty *>(buf);
        if (params == nullptr) {
            return 0;
        }
        params->uid = 0;
        params->gid = 0;
        auto ret = memset_s(params->gidTable, sizeof(params->gidTable), 0, sizeof(params->gidTable));
        if (ret != EOK) {
            HiLog::Info(LABEL, "memset_s failed");
            return 0;
        }
        params->gidCount = ClientSocket::MAX_GIDS + 1;
        std::string processName = "processName";
        ret = memcpy_s(params->processName, sizeof(params->processName), processName.c_str(), processName.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }
        std::string soPath = "soPath";
        ret = memcpy_s(params->soPath, sizeof(params->soPath), soPath.c_str(), soPath.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }

        return sizeof(ClientSocket::AppProperty);
    }

    int32_t ReadImplProcessName([[maybe_unused]] int32_t socketFd, void *buf, [[maybe_unused]] int32_t len)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // assign AppProperty
        ClientSocket::AppProperty *params = static_cast<ClientSocket::AppProperty *>(buf);
        if (params == nullptr) {
            return 0;
        }
        params->uid = 0;
        params->gid = 0;
        auto ret = memset_s(params->gidTable, sizeof(params->gidTable), 0, sizeof(params->gidTable));
        if (ret != EOK) {
            HiLog::Info(LABEL, "memset_s failed");
            return 0;
        }
        params->gidCount = ClientSocket::MAX_GIDS;
        std::string processName = "";
        ret = memcpy_s(params->processName, sizeof(params->processName), processName.c_str(), processName.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }
        std::string soPath = "soPath";
        ret = memcpy_s(params->soPath, sizeof(params->soPath), soPath.c_str(), soPath.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }

        return sizeof(ClientSocket::AppProperty);
    }

    int32_t ReadImplValid([[maybe_unused]] int32_t socketFd, void *buf, [[maybe_unused]] int32_t len)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // assign AppProperty
        ClientSocket::AppProperty *params = static_cast<ClientSocket::AppProperty *>(buf);
        if (params == nullptr) {
            return 0;
        }
        params->uid = 0;
        params->gid = 0;
        auto ret = memset_s(params->gidTable, sizeof(params->gidTable), 0, sizeof(params->gidTable));
        if (ret != EOK) {
            HiLog::Info(LABEL, "memset_s failed");
            return 0;
        }
        params->gidCount = ClientSocket::MAX_GIDS;
        std::string processName = "processName";
        ret = memcpy_s(params->processName, sizeof(params->processName), processName.c_str(), processName.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }
        std::string soPath = "soPath";
        ret = memcpy_s(params->soPath, sizeof(params->soPath), soPath.c_str(), soPath.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }

        return sizeof(ClientSocket::AppProperty);
    }

    int32_t ReadImplValidLongProcessName([[maybe_unused]] int32_t socketFd, void *buf, [[maybe_unused]] int32_t len)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // assign AppProperty
        ClientSocket::AppProperty *params = static_cast<ClientSocket::AppProperty *>(buf);
        if (params == nullptr) {
            return 0;
        }
        params->uid = 0;
        params->gid = 0;
        auto ret = memset_s(params->gidTable, sizeof(params->gidTable), 0, sizeof(params->gidTable));
        if (ret != EOK) {
            HiLog::Info(LABEL, "memset_s failed");
            return 0;
        }
        params->gidCount = ClientSocket::MAX_GIDS;
        std::string processName = "ProcessName1234567890";
        ret = memcpy_s(params->processName, sizeof(params->processName), processName.c_str(), processName.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }
        std::string soPath = "soPath";
        ret = memcpy_s(params->soPath, sizeof(params->soPath), soPath.c_str(), soPath.length());
        if (ret != EOK) {
            HiLog::Info(LABEL, "memcpy_s failed");
            return 0;
        }

        return sizeof(ClientSocket::AppProperty);
    }

private:
    std::mutex mutex_;
};
}  // namespace AppSpawn
}  // namespace OHOS
