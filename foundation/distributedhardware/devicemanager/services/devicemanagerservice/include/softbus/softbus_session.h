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

#ifndef OHOS_DEVICE_MANAGER_SOFTBUS_SESSION_H
#define OHOS_DEVICE_MANAGER_SOFTBUS_SESSION_H

#include <set>
#include <vector>
#include <string>

#include "session.h"
#include "single_instance.h"
#include "softbus_common.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusSession {
DECLARE_SINGLE_INSTANCE_BASE(SoftbusSession);
public:
    int32_t Start();
    int32_t OnSessionOpened(int32_t sessionId, int32_t result);
    void OnSessionClosed(int32_t sessionId);
    void OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen);
    void CloseSession(int32_t sessionId);
    int32_t SendMessages(const char *deviceId, std::vector<std::string> &messages);
    int32_t SendMsg(int32_t sessionId, std::string &message);
    void GetPeerDeviceId(int32_t sessionId, std::string &peerDevId);
private:
    SoftbusSession() = default;
    ~SoftbusSession();
private:
    int32_t SendData(int32_t sessionId, const void *data, int32_t len);
private:
    const char *PKG_NAME = "ohos.distributedhardware.devicemanager";
    const char *SESSION_NAME = "ohos.distributedhardware.devicemanager.resident";
    std::set<int32_t> sessionIdSet_;
    std::vector<std::string> messages_ {};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_SOFTBUS_SESSION_H
