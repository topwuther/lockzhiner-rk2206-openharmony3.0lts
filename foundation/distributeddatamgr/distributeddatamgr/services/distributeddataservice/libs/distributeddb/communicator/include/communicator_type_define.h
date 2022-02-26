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

#ifndef COMMUNICATOR_TYPE_DEFINE_H
#define COMMUNICATOR_TYPE_DEFINE_H

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include "db_errno.h"

namespace DistributedDB {
using LabelType = std::vector<uint8_t>;
using Finalizer = std::function<void(void)>;
using OnSendEnd = std::function<void(int result)>;
using OnConnectCallback = std::function<void(const std::string &target, bool isConnect)>;
constexpr unsigned int COMM_LABEL_LENGTH = 32; // Using SHA256 which length is 32
constexpr uint32_t MAX_TOTAL_LEN = 104857600; // 100M Limitation For Max Total Length

template<typename T>
int RegCallBack(const T &newCallback, T &oldCallback, const Finalizer &newFinalizer, Finalizer &oldFinalizer)
{
    if (newCallback && oldCallback) {
        // Already registered, not allowed
        return -E_ALREADY_REGISTER;
    }
    if (newCallback && !oldCallback) {
        // Do register
        oldCallback = newCallback;
        oldFinalizer = newFinalizer;
        return E_OK;
    }
    if (!newCallback && oldCallback) {
        // Do unregister
        if (oldFinalizer) {
            oldFinalizer();
        }
        oldCallback = nullptr;
        oldFinalizer = nullptr;
        return E_OK;
    }
    return -E_NOT_PERMIT;
}

enum class Priority {
    LOW = 0,        // Usually for datasync and its response
    NORMAL = 1,     // Usually for timesync and its response
    HIGH = 2,       // Only for communicator inside
};

enum class FrameType {
    EMPTY = 0,      // Used for gossip or help version negotiation
    APPLICATION_MESSAGE = 1,
    COMMUNICATION_LABEL_EXCHANGE = 2,
    COMMUNICATION_LABEL_EXCHANGE_ACK = 3,
    INVALID_MAX_FRAME_TYPE = 4,
};
} // namespace DistributedDB

#endif // COMMUNICATOR_TYPE_DEFINE_H
