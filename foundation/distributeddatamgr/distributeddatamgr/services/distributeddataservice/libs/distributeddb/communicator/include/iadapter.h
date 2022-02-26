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

#ifndef IADAPTER_H
#define IADAPTER_H

#include <string>
#include <cstdint>
#include <functional>
#include "communicator_type_define.h"

namespace DistributedDB {
// SendableCallback only notify when status changed from unsendable to sendable
using BytesReceiveCallback = std::function<void(const std::string &srcTarget, const uint8_t *bytes, uint32_t length)>;
using TargetChangeCallback = std::function<void(const std::string &target, bool isConnect)>;
using SendableCallback = std::function<void(const std::string &target)>;

class IAdapter {
public:
    // Register all callback before call StartAdapter.
    // Return 0 as success. Return negative as error
    // The StartAdapter should only be called by its user not owner
    virtual int StartAdapter() = 0;

    // The StopAdapter may be called by its user in precondition of StartAdapter success
    // The StopAdapter should only be called by its user not owner
    virtual void StopAdapter() = 0;

    // Should returns the multiples of 8
    virtual uint32_t GetMtuSize() = 0;
    virtual uint32_t GetMtuSize(const std::string &target) = 0;

    // Get local target name for identify self
    virtual int GetLocalIdentity(std::string &outTarget) = 0;

    // Not assume bytes to be heap memory. Not assume SendBytes to be not blocking
    // Return 0 as success. Return negative as error
    virtual int SendBytes(const std::string &dstTarget, const uint8_t *bytes, uint32_t length) = 0;

    // Pass nullptr as inHandle to do unReg if need (inDecRef also nullptr)
    // Return 0 as success. Return negative as error
    virtual int RegBytesReceiveCallback(const BytesReceiveCallback &onReceive, const Finalizer &inOper) = 0;

    // Pass nullptr as inHandle to do unReg if need (inDecRef also nullptr)
    // Return 0 as success. Return negative as error
    virtual int RegTargetChangeCallback(const TargetChangeCallback &onChange, const Finalizer &inOper) = 0;

    // Pass nullptr as inHandle to do unReg if need (inDecRef also nullptr)
    // Return 0 as success. Return negative as error
    virtual int RegSendableCallback(const SendableCallback &onSendable, const Finalizer &inOper) = 0;

    virtual ~IAdapter() {};
};
} // namespace DistributedDB

#endif // IADAPTER_H
