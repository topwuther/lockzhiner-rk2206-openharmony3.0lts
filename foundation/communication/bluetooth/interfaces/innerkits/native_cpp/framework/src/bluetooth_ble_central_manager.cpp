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
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "i_bluetooth_ble_central_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
struct BleCentralManager::impl {
    impl();
    ~impl();

    void BindServer();
    class BluetoothBleCentralManagerCallbackImp : public BluetoothBleCentralManagerCallBackStub {
    public:
        BluetoothBleCentralManagerCallbackImp(BleCentralManager::impl &bleCentralManger)
            : bleCentralManger_(bleCentralManger){};
        ~BluetoothBleCentralManagerCallbackImp() override = default;
        void OnScanCallback(const BluetoothBleScanResult &result) override
        {
            HILOGD("BleCentralManager::impl::BluetoothBleCentralManagerCallbackImp::OnScanCallback");
            bleCentralManger_.callbacks_.ForEach([&result](std::shared_ptr<BleCentralManagerCallback> observer) {
                BluetoothBleScanResult tempResult = result;
                BleScanResult scanResult;
                for (auto &manufacturerData : tempResult.GetManufacturerData()) {
                    scanResult.AddManufacturerData(manufacturerData.first, manufacturerData.second);
                }

                for (auto &serviceUuidData : tempResult.GetServiceUuids()) {
                    UUID uuid = UUID::ConvertFrom128Bits(serviceUuidData.ConvertTo128Bits());
                    scanResult.AddServiceUuid(uuid);
                }

                for (auto &serviceData : tempResult.GetServiceData()) {
                    UUID uuid = UUID::ConvertFrom128Bits(serviceData.first.ConvertTo128Bits());
                    scanResult.AddServiceData(uuid, serviceData.second);
                }

                scanResult.SetAdvertiseFlag(tempResult.GetAdvertiseFlag());
                scanResult.SetRssi(tempResult.GetRssi());
                scanResult.SetConnectable(tempResult.IsConnectable());
                BluetoothRemoteDevice device(tempResult.GetPeripheralDevice().GetAddress(), BT_TRANSPORT_BLE);
                scanResult.SetPeripheralDevice(device);
                scanResult.SetPayload(tempResult.GetPayload());

                observer->OnScanCallback(scanResult);
            });
        }
        void OnBleBatchScanResultsEvent(std::vector<BluetoothBleScanResult> &results) override
        {
            HILOGD("BleCentralManager::impl::BluetoothBleCentralManagerCallbackImp::OnBleBatchScanResultsEvent");
            bleCentralManger_.callbacks_.ForEach([&results](std::shared_ptr<BleCentralManagerCallback> observer) {
                std::vector<BleScanResult> scanResults;
                for (auto &result : results) {
                    BleScanResult scanResult;
                    for (auto &manufaturerData : result.GetManufacturerData()) {
                        scanResult.AddManufacturerData(manufaturerData.first, manufaturerData.second);
                    }

                    for (auto &serviceData : result.GetServiceData()) {
                        UUID uuid = UUID::ConvertFrom128Bits(serviceData.first.ConvertTo128Bits());
                        scanResult.AddServiceData(uuid, serviceData.second);
                    }

                    for (auto &serviceUuidData : result.GetServiceUuids()) {
                        UUID uuid = UUID::ConvertFrom128Bits(serviceUuidData.ConvertTo128Bits());
                        scanResult.AddServiceUuid(uuid);
                    }

                    scanResult.SetAdvertiseFlag(result.GetAdvertiseFlag());
                    scanResult.SetConnectable(result.IsConnectable());
                    scanResult.SetRssi(result.GetRssi());
                    BluetoothRemoteDevice device(result.GetPeripheralDevice().GetAddress(), BT_TRANSPORT_BLE);
                    scanResult.SetPeripheralDevice(device);
                    scanResult.SetPayload(result.GetPayload());

                    scanResults.push_back(scanResult);
                }

                observer->OnBleBatchScanResultsEvent(scanResults);
            });
        }

        void OnStartScanFailed(int resultCode) override
        {
            bleCentralManger_.callbacks_.ForEach([resultCode](std::shared_ptr<BleCentralManagerCallback> observer) {
                observer->OnStartScanFailed(resultCode);
            });
        }

    private:
        BleCentralManager::impl &bleCentralManger_;
        BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleCentralManagerCallbackImp);
    };
    sptr<BluetoothBleCentralManagerCallbackImp> callbackImp_ = nullptr;

    sptr<IBluetoothBleCentralManager> proxy_ = nullptr;
    BluetoothObserverList<BleCentralManagerCallback> callbacks_;
};

BleCentralManager::impl::impl()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        HILOGE("BleCentralManager::impl::impl() error: no samgr");
        return;
    }

    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (!hostRemote) {
        HILOGE("BleCentralManager::impl::impl() error: host no remote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    if (!hostProxy) {
        HILOGE("BleCentralManager::impl::impl() error: host no proxy");
        return;
    }
    sptr<IRemoteObject> remote = hostProxy->GetBleRemote(BLE_CENTRAL_MANAGER_SERVER);
    if (!remote) {
        HILOGE("BleCentralManager::impl::impl() error: no remote");
        return;
    }
    proxy_ = iface_cast<IBluetoothBleCentralManager>(remote);
    if (!proxy_) {
        HILOGE("BleCentralManager::impl::impl() error: no proxy");
        return;
    }
    callbackImp_ = new BluetoothBleCentralManagerCallbackImp(*this);
    proxy_->RegisterBleCentralManagerCallback(callbackImp_);
}

BleCentralManager::impl::~impl()
{
    proxy_->DeregisterBleCentralManagerCallback(callbackImp_);
}

BleCentralManager::BleCentralManager(BleCentralManagerCallback &callback) : callback_(&callback), pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (pimpl == nullptr) {
            HILOGE("BleCentralManager::BleCentralManager fails: no pimpl");
        }
    }

    HILOGD("BleCentralManager::BleCentralManager success");
    std::shared_ptr<BleCentralManagerCallback> pointer(&callback, [](BleCentralManagerCallback *) {});
    bool ret = pimpl->callbacks_.Register(pointer);
    if (ret)
        return;
    callback_ = &callback;
}

BleCentralManager::~BleCentralManager()
{
    callback_ = nullptr;
}

void BleCentralManager::StartScan()
{
    if (pimpl->proxy_ != nullptr) {
        HILOGD("BleCentralManager::StartScan success");
        pimpl->proxy_->StartScan();
    }
}

void BleCentralManager::StartScan(const BleScanSettings &settings)
{
    if (pimpl->proxy_ != nullptr) {
        BluetoothBleScanSettings setting;
        setting.SetReportDelay(settings.GetReportDelayMillisValue());
        setting.SetScanMode(settings.GetScanMode());
        setting.SetLegacy(settings.GetLegacy());
        setting.SetPhy(settings.GetPhy());
        pimpl->proxy_->StartScan(setting);
    }
}

void BleCentralManager::StopScan()
{
    if (pimpl->proxy_ != nullptr) {
        pimpl->proxy_->StopScan();
    }
}

BleScanResult::BleScanResult()
{}

BleScanResult::~BleScanResult()
{}

std::vector<UUID> BleScanResult::GetServiceUuids() const
{
    return serviceUuids_;
}

std::map<uint16_t, std::string> BleScanResult::GetManufacturerData() const
{
    return manufacturerSpecificData_;
}

std::map<UUID, std::string> BleScanResult::GetServiceData() const
{
    return serviceData_;
}

BluetoothRemoteDevice BleScanResult::GetPeripheralDevice() const
{
    return peripheralDevice_;
}

int8_t BleScanResult::GetRssi() const
{
    return rssi_;
}

bool BleScanResult::IsConnectable() const
{
    return connectable_;
}

uint8_t BleScanResult::GetAdvertiseFlag() const
{
    return advertiseFlag_;
}

std::vector<uint8_t> BleScanResult::GetPayload() const
{
    return payload_;
}

void BleScanResult::AddManufacturerData(uint16_t manufacturerId, const std::string &data)
{
    manufacturerSpecificData_.insert(std::make_pair(manufacturerId, data));
}

void BleScanResult::AddServiceData(const UUID &uuid, const std::string &data)
{
    serviceData_.insert(std::make_pair(uuid, data));
}

void BleScanResult::AddServiceUuid(const UUID &serviceUuid)
{
    serviceUuids_.push_back(serviceUuid);
}

void BleScanResult::SetPayload(std::string payload)
{
    payload_.assign(payload.begin(), payload.end());
}

void BleScanResult::SetPeripheralDevice(const BluetoothRemoteDevice &device)
{
    peripheralDevice_ = device;
}

void BleScanResult::SetRssi(int8_t rssi)
{
    rssi_ = rssi;
}

void BleScanResult::SetConnectable(bool connectable)
{
    connectable_ = connectable;
}

void BleScanResult::SetAdvertiseFlag(uint8_t flag)
{
    advertiseFlag_ = flag;
}

BleScanSettings::BleScanSettings()
{}

BleScanSettings::~BleScanSettings()
{}

void BleScanSettings::SetReportDelay(long reportDelayMillis)
{
    reportDelayMillis_ = reportDelayMillis;
}

long BleScanSettings::GetReportDelayMillisValue() const
{
    return reportDelayMillis_;
}

int BleScanSettings::SetScanMode(int scanMode)
{
    if (scanMode < SCAN_MODE_LOW_POWER || scanMode > SCAN_MODE_LOW_LATENCY) {
        return RET_BAD_PARAM;
    }

    scanMode_ = scanMode;
    return RET_NO_ERROR;
}

int BleScanSettings::GetScanMode() const
{
    return scanMode_;
}

void BleScanSettings::SetLegacy(bool legacy)
{
    legacy_ = legacy;
}

bool BleScanSettings::GetLegacy() const
{
    return legacy_;
}

void BleScanSettings::SetPhy(int phy)
{
    phy_ = phy;
}

int BleScanSettings::GetPhy() const
{
    return phy_;
}
}  // namespace Bluetooth
}  // namespace OHOS
