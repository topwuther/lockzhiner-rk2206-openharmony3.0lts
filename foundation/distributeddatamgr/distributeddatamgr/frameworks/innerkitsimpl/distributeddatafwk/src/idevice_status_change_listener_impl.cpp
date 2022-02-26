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

#define LOG_TAG "DevChangeStatusListener"

#include "idevice_status_change_listener.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
enum {
    ONCHANGE,
};
DeviceStatusChangeListenerProxy::DeviceStatusChangeListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IDeviceStatusChangeListener>(impl)
{}

void DeviceStatusChangeListenerProxy::OnChange(const DeviceInfo &results, const DeviceChangeType &type)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DeviceStatusChangeListenerProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    if (!data.WriteInt32(static_cast<int>(type)) || !results.Marshalling(data)) {
        ZLOGW("SendRequest write parcel type failed.");
        return;
    }
    MessageOption mo { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(ONCHANGE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error %d", error);
    }
}

int DeviceStatusChangeListenerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                                    MessageParcel &reply, MessageOption &option)
{
    ZLOGD("%d", code);
    std::u16string descriptor = DeviceStatusChangeListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    switch (code) {
        case ONCHANGE: {
            DeviceChangeType type = static_cast<DeviceChangeType>(data.ReadInt32());
            DeviceInfo *deviceInfoPtr = DeviceInfo::UnMarshalling(data);
            if (deviceInfoPtr != nullptr) {
                OnChange(*deviceInfoPtr, type);
                delete deviceInfoPtr;
            } else {
                ZLOGW("device info is null");
            }
            return 0;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
}  // namespace DistributedKv
}  // namespace OHOS