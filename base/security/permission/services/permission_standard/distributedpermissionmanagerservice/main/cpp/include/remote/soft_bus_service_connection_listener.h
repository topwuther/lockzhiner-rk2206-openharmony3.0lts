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

#ifndef SOFT_BUS_SERVICE_CONNECTION_LISTENER_H
#define SOFT_BUS_SERVICE_CONNECTION_LISTENER_H

#include <string>
#include <atomic>
#include <memory>

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class ServiceConnectionListener {
public:
    ServiceConnectionListener() = default;
    virtual ~ServiceConnectionListener() = default;
    virtual void OnServiceConnected() = 0;
    virtual void OnServiceDisconnected() = 0;
};

class SoftBusServiceConnectionListener final : public ServiceConnectionListener {
public:
    SoftBusServiceConnectionListener(const std::shared_ptr<ServiceConnectionListener> &deviceListener,
        const std::function<void()> &fulfillLocalDeviceInfo);
    ~SoftBusServiceConnectionListener();

    void OnServiceConnected() override;

    void OnServiceDisconnected() override;

private:
    std::atomic<bool> hasConnected_;
    std::shared_ptr<ServiceConnectionListener> deviceListener_;
    std::function<void()> initCallback_;

    static const int MAX_RETRY_TIMES = 5;
    static const int DELAY_AFTER_DISCONNECT = 500;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif
