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

#ifndef OHOS_DEVICE_MANAGER_IMPL_TEST_H
#define OHOS_DEVICE_MANAGER_IMPL_TEST_H

#include <gtest/gtest.h>
#include <refbase.h>

#include "mock/mock_ipc_client_proxy.h"
#include "device_manager_impl.h"
#include "device_manager_callback.h"
#include "device_manager.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
private:
    std::shared_ptr<DiscoverCallback> test_callback_ = nullptr;
};

class DeviceDiscoverCallback : public DiscoverCallback {
public:
    DeviceDiscoverCallback() : DiscoverCallback() {}
    virtual ~DeviceDiscoverCallback() override {}
    virtual void OnDiscoverySuccess(uint16_t subscribeId) override;
    virtual void OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason) override;
    virtual void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override;
};
} // namespace Vsync
} // namespace OHOS

#endif // OHOS_DEVICE_MANAGER_IMPL_TEST_H
