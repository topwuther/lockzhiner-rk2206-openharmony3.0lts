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

#ifndef VIRTUAL_DEVICE_H
#define VIRTUAL_DEVICE_H

#include "icommunicator.h"

#include "ikvdb_sync_interface.h"
#include "single_ver_sync_task_context.h"
#include "virtual_single_ver_sync_db_Interface.h"

namespace DistributedDB {
class VirtualCommunicatorAggregator;

class VituralDevice {
public:
    explicit VituralDevice(const std::string &deviceId);
    ~VituralDevice();

    int Initialize(VirtualCommunicatorAggregator *communicatorAggregator, IKvDBSyncInterface *syncInterface);
    void SetDeviceId(const std::string &deviceId);
    std::string GetDeviceId() const;
    int GetData(const Key &key, VirtualDataItem &item);
    int GetData(const Key &key, Value &value);
    int PutData(const Key &key, const Value &value, const TimeStamp &time, int flag);
    int PutData(const Key &key, const Value &value);
    int DeleteData(const Key &key);
    int StartTransaction();
    int Commit();
    int MessageCallback(const std::string &deviceId, Message *inMsg);
    void OnRemoteDataChanged(const std::function<void(const std::string &)> &callback);
    void Online();
    void Offline();
    int StartResponseTask();
    TimeOffset GetLocalTimeOffset() const;
    void SetSaveDataDelayTime(uint64_t milliDelayTime);
    int Sync(SyncMode mode, bool wait);

private:
    ICommunicator *communicateHandle_;
    VirtualCommunicatorAggregator *communicatorAggregator_ = nullptr;
    IKvDBSyncInterface *storage_;
    std::shared_ptr<Metadata> metadata_;
    std::string deviceId_;
    std::string remoteDeviceId_ = "real_device";
    SyncTaskContext *context_ = nullptr;
    std::function<void(const std::string &)> onRemoteDataChanged_;
};
} // namespace DistributedDB

#endif  // VIRTUAL_DEVICE_H
