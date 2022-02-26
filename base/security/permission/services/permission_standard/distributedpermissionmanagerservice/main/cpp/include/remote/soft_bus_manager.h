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

#ifndef SOFT_BUS_MANAGER_H
#define SOFT_BUS_MANAGER_H

#include <inttypes.h>
#include <string>
#include <functional>
#include <memory>
#include <thread>
#include "remote_command_executor.h"
#include "session.h"
#include "soft_bus_session_listener.h"
#include "soft_bus_service_connection_listener.h"
#include "soft_bus_device_connection_listener.h"

#include "device_manager.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class SoftBusManager final {
public:
    virtual ~SoftBusManager();

    /**
     * @brief Get instance of SoftBusManager
     *
     * @return SoftBusManager's instance.
     * @since 1.0
     * @version 1.0
     */
    static SoftBusManager &GetInstance();

    /**
     * @brief Bind soft bus service.
     *
     * @since 1.0
     * @version 1.0
     */
    void Initialize();

    /**
     * @brief Unbind soft bus service when DPMS has been destroyed.
     *
     * @since 1.0
     * @version 1.0
     */
    void Destroy();

    /**
     * @brief Open session with the peer device sychronized.
     *
     * @param deviceUdid The udid of peer device.
     * @return Session id if open successfully, otherwise return -1(Constant::FAILURE).
     * @since 1.0
     * @version 1.0
     */
    int OpenSession(const std::string &deviceUdid);

    /**
     * @brief Close session with the peer device.
     *
     * @param session The session id need to close.
     * @return 0 if close successfully, otherwise return -1(Constant::FAILURE).
     * @since 1.0
     * @version 1.0
     */
    int CloseSession(int sessionId);

    /**
     * @brief Get UUID(networkId) by deviceNodeId.
     *
     * @param deviceNodeId The valid networkId or deviceId(UDID) or deviceUuid.
     * @return uuid if deviceManager is ready, empty string otherwise.
     * @since 1.0
     * @version 1.0
     */
    std::string GetUniversallyUniqueIdByNodeId(const std::string &deviceNodeId);

    /**
     *  @brief Get deviceId(UDID) by deviceNodeId.
     *
     * @param deviceNodeId The valid networkId or deviceId(UDID) or deviceUuid.
     * @return udid if deviceManager work correctly, empty string otherwise.
     * @since 1.0
     * @version 1.0
     */
    std::string GetUniqueDisabilityIdByNodeId(const std::string &deviceNodeId);

public:
    static const std::string SESSION_NAME;

private:
    SoftBusManager();

    /**
     * @brief Fulfill local device info
     *
     * @return 0 if operate successfully, otherwise return -1(Constant::FAILURE).
     * @since 1.0
     * @version 1.0
     */
    int FulfillLocalDeviceInfo();
    std::string GetUuidByNodeId(const std::string &nodeId) const;
    std::string GetUdidByNodeId(const std::string &nodeId) const;

    const static std::string DPMS_PACKAGE_NAME;

    // soft bus session server opened flag
    bool isSoftBusServiceBindSuccess_;
    std::atomic_bool inited_;

    // init mutex
    std::mutex mutex_;

    // fulfill thread mutex
    std::mutex fulfillMutex_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // SOFT_BUS_MANAGER_H