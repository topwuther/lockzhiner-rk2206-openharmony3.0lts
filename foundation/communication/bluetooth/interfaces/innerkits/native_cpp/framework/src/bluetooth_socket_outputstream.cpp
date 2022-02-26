/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <unistd.h>
#include <cerrno>
#include "bluetooth_socket_outputstream.h"
#include "bluetooth_log.h"
#include "sys/socket.h"

namespace OHOS {
namespace Bluetooth {
OutputStream::OutputStream(int socketFd) : socketFd_(socketFd)
{}

OutputStream::~OutputStream()
{}

size_t OutputStream::Write(const char *buf, const size_t length)
{
    HILOGI("OutputStream::Write(2 parameters) starts");
    if (socketFd_ == -1) {
        HILOGE("[sock]socket closed");
        return -1;
    }
    auto ret = send(socketFd_, buf, length, MSG_NOSIGNAL);

    HILOGE("[sock]write ret:%{public}d errno: %{public}d", ret, errno);

    if (ret == -1) {
        HILOGE("[sock]write Error. errno: %{public}d", errno);
    }
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS