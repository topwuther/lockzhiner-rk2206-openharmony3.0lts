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

#ifndef DEV_IDEVICE_STATUS_CHANGE_LISTENER_H
#define DEV_IDEVICE_STATUS_CHANGE_LISTENER_H
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {
class IDeviceStatusChangeListener : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IDeviceStatusChangeListener");
    virtual void OnChange(const DeviceInfo &results, const DeviceChangeType &type) = 0;
};

class DeviceStatusChangeListenerStub : public IRemoteStub<IDeviceStatusChangeListener> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
};

class DeviceStatusChangeListenerProxy : public IRemoteProxy<IDeviceStatusChangeListener> {
public:
    explicit DeviceStatusChangeListenerProxy(const sptr<IRemoteObject> &impl);
    ~DeviceStatusChangeListenerProxy() = default;
    void OnChange(const DeviceInfo &results, const DeviceChangeType &type) override;
private:
    static inline BrokerDelegator<DeviceStatusChangeListenerProxy> delegator_;
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif // DEV_IDEVICE_STATUS_CHANGE_LISTENER_H