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

#include "soft_bus_service_connection_listener.h"
#include "remote_command_manager.h"
#include "object_device_permission_manager.h"
#include "subject_device_permission_manager.h"
#include "distributed_permission_manager_service.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusServiceConnectionListener"};
}
SoftBusServiceConnectionListener::SoftBusServiceConnectionListener(
    const std::shared_ptr<ServiceConnectionListener> &deviceListener,
    const std::function<void()> &fulfillLocalDeviceInfo)
    : hasConnected_(false), deviceListener_(deviceListener), initCallback_(fulfillLocalDeviceInfo)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusServiceConnectionListener()");
}
SoftBusServiceConnectionListener::~SoftBusServiceConnectionListener()
{
    PERMISSION_LOG_DEBUG(LABEL, "~SoftBusServiceConnectionListener()");
}

void SoftBusServiceConnectionListener::OnServiceConnected()
{
    bool init = false;
    if (hasConnected_.compare_exchange_strong(init, true)) {
        bool hasListener = (deviceListener_ != nullptr);
        PERMISSION_LOG_INFO(LABEL, "onServiceConnected: device listener success: %{public}d", hasListener);

        initCallback_();
        return;
    }
}

void SoftBusServiceConnectionListener::OnServiceDisconnected()
{
    PERMISSION_LOG_INFO(LABEL, "onServiceDisconnected: deviceMonitorService disConnected.");
    bool connected = true;
    if (!hasConnected_.compare_exchange_strong(connected, false)) {
        return;
    }

    RemoteCommandManager::GetInstance().Clear();
    SubjectDevicePermissionManager::GetInstance().Clear();
    ObjectDevicePermissionManager::GetInstance().Clear();
    DeviceInfoManager::GetInstance().RemoveAllRemoteDeviceInfo();

    std::function<void()> retrys = [&]() {
        int times = 0;
        while (times < MAX_RETRY_TIMES) {
            if (hasConnected_.load()) {
                PERMISSION_LOG_INFO(LABEL, "retry bind success, terminate, times: %{public}d", times);
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_AFTER_DISCONNECT));
            times += 1;
        }
        PERMISSION_LOG_INFO(LABEL, "retry bind failed, retry times: %{public}d", times);
    };

    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
        return;
    }
    std::string taskName("dpms_softbus_service_connect");
    handler->PostTask(retrys, taskName);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
