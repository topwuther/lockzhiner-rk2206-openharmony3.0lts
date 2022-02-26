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

#ifndef DISTRIBUTEDDATAFWK_SRC_SOFTBUS_ADAPTER_H
#define DISTRIBUTEDDATAFWK_SRC_SOFTBUS_ADAPTER_H
#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <tuple>
#include <memory>
#include "app_data_change_listener.h"
#include "app_device_status_change_listener.h"
#include "app_types.h"
#include "platform_specific.h"
#include "session.h"
#include "softbus_bus_center.h"
namespace OHOS {
namespace AppDistributedKv {
enum IdType {
    NETWORKID,
    UUID,
    UDID,
};
class Semaphore {
public:
    explicit Semaphore(unsigned int resCount) : count(resCount), data(-1) {}
    ~Semaphore() {}

public:
    int Wait()
    {
        std::unique_lock<std::mutex> uniqueLock(mutex);
        --count;
        while (count < 0) {
            cv.wait(uniqueLock);
        }
        return data;
    }
    void Signal(const int &sendData)
    {
        std::lock_guard<std::mutex> lg(mutex);
        data = sendData;
        if (++count < 1) {
            cv.notify_one();
        }
    }

private:
    int count;
    int data;
    std::mutex mutex;
    std::condition_variable cv;
};

class SoftBusAdapter {
public:
    SoftBusAdapter();
    ~SoftBusAdapter();
    static std::shared_ptr<SoftBusAdapter> GetInstance();

    void Init();
    // add DeviceChangeListener to watch device change;
    Status StartWatchDeviceChange(const AppDeviceStatusChangeListener *observer, const PipeInfo &pipeInfo);
    // stop DeviceChangeListener to watch device change;
    Status StopWatchDeviceChange(const AppDeviceStatusChangeListener *observer, const PipeInfo &pipeInfo);
    void NotifyAll(const DeviceInfo &deviceInfo, const DeviceChangeType &type);
    DeviceInfo GetLocalDevice();
    std::vector<DeviceInfo> GetDeviceList() const;
    std::string GetUuidByNodeId(const std::string &nodeId) const;
    std::string GetUdidByNodeId(const std::string &nodeId) const;
    // get local device node information;
    DeviceInfo GetLocalBasicInfo() const;
    // get all remote connected device's node information;
    std::vector<DeviceInfo> GetRemoteNodesBasicInfo() const;
    // transfer nodeId or udid to uuid
    // input: id
    // output: uuid
    // return: transfer success or not
    std::string ToUUID(const std::string &id) const;
    // transfer uuid or udid to nodeId
    // input: id
    // output: nodeId
    // return: transfer success or not
    std::string ToNodeID(const std::string &id, const std::string &nodeId) const;
    static std::string ToBeAnonymous(const std::string &name);

    // add DataChangeListener to watch data change;
    Status StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo);

    // stop DataChangeListener to watch data change;
    Status StopWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo);

    // Send data to other device, function will be called back after sent to notify send result.
    Status SendData(const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size,
                    const MessageInfo &info);

    bool IsSameStartedOnPeer(const struct PipeInfo &pipeInfo, const struct DeviceId &peer);

    void SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag);

    int CreateSessionServerAdapter(const std::string &sessionName);

    int RemoveSessionServerAdapter(const std::string &sessionName) const;

    void UpdateRelationship(const std::string &networkid, const DeviceChangeType &type);

    void InsertSession(const std::string &sessionName);

    void DeleteSession(const std::string &sessionName);

    void NotifyDataListeners(const uint8_t *ptr, const int size, const std::string &deviceId,
        const PipeInfo &pipeInfo);

    int WaitSessionOpen();

    void NotifySessionOpen(const int &state);

    int GetOpenSessionId();

    void SetOpenSessionId(const int &sessionId);
private:
    mutable std::mutex networkMutex_ {};
    mutable std::map<std::string, std::tuple<std::string, std::string>> networkId2UuidUdid_ {};
    DeviceInfo localInfo_ {};
    static std::shared_ptr<SoftBusAdapter> instance_;
    std::mutex deviceChangeMutex_;
    std::set<const AppDeviceStatusChangeListener *> listeners_ {};
    std::mutex dataChangeMutex_ {};
    std::map<std::string, const AppDataChangeListener *> dataChangeListeners_ {};
    std::mutex busSessionMutex_ {};
    std::map<std::string, bool> busSessionMap_ {};
    bool flag_ = true; // only for br flag
    INodeStateCb nodeStateCb_ {};
    ISessionListener sessionListener_ {};
    std::unique_ptr<Semaphore> semaphore_ {};
    std::mutex notifyFlagMutex_ {};
    bool notifyFlag_ = false;
    std::mutex openSessionIdMutex_ {};
    int openSessionId_ = -1;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif /* DISTRIBUTEDDATAFWK_SRC_SOFTBUS_ADAPTER_H */