/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_SERVER_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_SERVER_H

#include "bluetooth_ble_central_manager_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_ble_central_manager.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleCentralManagerServer : public BluetoothBleCentralManagerStub {
public:
    BluetoothBleCentralManagerServer();
    ~BluetoothBleCentralManagerServer() override;

    virtual void RegisterBleCentralManagerCallback(const sptr<IBluetoothBleCentralManagerCallback> &callback) override;
    virtual void DeregisterBleCentralManagerCallback(
        const sptr<IBluetoothBleCentralManagerCallback> &callback) override;
    virtual void StartScan() override;
    virtual void StartScan(const BluetoothBleScanSettings &settings) override;
    virtual void StopScan() override;

private:
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleCentralManagerServer);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_SERVER_H