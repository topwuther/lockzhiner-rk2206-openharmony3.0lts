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

#ifndef VIRTUAL_TIME_SYNC_COMMUNICATOR_H
#define VIRTUAL_TIME_SYNC_COMMUNICATOR_H

#include <string>
#include <cstdint>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "db_types.h"
#include "ref_object.h"
#include "serial_buffer.h"
#include "icommunicator.h"
#include "communicator_aggregator.h"
#include "vitural_device.h"
#include "time_sync.h"

namespace DistributedDB {
class VirtualTimeSyncCommunicator : public ICommunicator {
public:
    VirtualTimeSyncCommunicator();
    ~VirtualTimeSyncCommunicator();

    DISABLE_COPY_ASSIGN_MOVE(VirtualTimeSyncCommunicator);

    int RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper) override;
    int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override;
    int RegOnSendableCallback(const std::function<void(void)> &onSendable, const Finalizer &inOper) override;

    void Activate() override;

    // return maximum allowed data size
    uint32_t GetCommunicatorMtuSize() const override;
    uint32_t GetCommunicatorMtuSize(const std::string &target) const override;
    // Get local target name for identify self
    int GetLocalIdentity(std::string &outTarget) const override;

    int SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout) override;
    int SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout,
        const OnSendEnd &onEnd) override;

    int GetRemoteCommunicatorVersion(const std::string &deviceId, uint16_t &version) const override;

    void SetTimeSync(TimeSync *srcTimeSync, TimeSync *dstTimeSync,
        const std::string &deviceID, SyncTaskContext *syncTaskcontext);

    void GetTimeOffset(TimeOffset &timeOffset) const;

    void Disable();

private:
    TimeSync *srcTimeSync_;
    TimeSync *dstTimeSync_;
    TimeOffset timeOffset_;
    std::string deviceID_;
    SyncTaskContext *syncTaskcontext_;
    bool isEnable_ = true;
};
} // namespace DistributedDB

#endif // VIRTUAL_TIME_SYNC_COMMUNICATOR_H
