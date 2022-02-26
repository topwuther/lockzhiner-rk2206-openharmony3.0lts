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

#include "client_socket.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AppSpawn {
class MockClientSocket : public ClientSocket {
public:
    virtual ~MockClientSocket() = default;
    MockClientSocket() : ClientSocket("MockClientSocket"){};

    MOCK_METHOD0(CreateClient, int32_t());
    MOCK_METHOD0(CloseClient, void());
    MOCK_METHOD1(ConnectSocket, int32_t(int32_t socketFD));
    MOCK_METHOD0(ConnectSocket, int32_t());
    MOCK_METHOD2(WriteSocketMessage, int32_t(const void *buf, int32_t len));
    MOCK_METHOD2(ReadSocketMessage, int32_t(void *buf, int32_t len));
};
}  // namespace AppSpawn
}  // namespace OHOS
