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

#include "bluetooth_ble_central_manager_server.h"
#include "ble_service_data.h"
#include "bluetooth_log.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_manager.h"
#include "remote_observer_list.h"
#include <string>

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
class BleCentralManagerCallback : public IBleCentralManagerCallback {
public:
    BleCentralManagerCallback() = default;
    ~BleCentralManagerCallback() override = default;

    void OnScanCallback(const BleScanResultImpl &result) override
    {
        HILOGI("BleCentralManageCallback::OnScanCallback start.");

        observers_->ForEach([result](IBluetoothBleCentralManagerCallback *observer) {
            HILOGI("BleCentralManageCallback::OnScanCallback:Address= %{public}s",
                result.GetPeripheralDevice().GetRawAddress().GetAddress().c_str());

            BluetoothBleScanResult bleScanResult;
            if (result.GetPeripheralDevice().IsRSSI()) {
                bleScanResult.SetRssi(result.GetPeripheralDevice().GetRSSI());
            }

            bleScanResult.SetAdvertiseFlag(result.GetPeripheralDevice().GetAdFlag());

            if (result.GetPeripheralDevice().IsManufacturerData()) {
                std::map<uint16_t, std::string> manuData = result.GetPeripheralDevice().GetManufacturerData();
                for (auto it = manuData.begin(); it != manuData.end(); it++) {
                    bleScanResult.AddManufacturerData(it->first, it->second);
                }
            }

            bleScanResult.SetConnectable(result.GetPeripheralDevice().IsConnectable());

            if (result.GetPeripheralDevice().IsServiceUUID()) {
                std::vector<Uuid> uuids = result.GetPeripheralDevice().GetServiceUUID();
                for (auto iter = uuids.begin(); iter != uuids.end(); iter++) {
                    bleScanResult.AddServiceUuid(*iter);
                }
            }

            if (result.GetPeripheralDevice().IsServiceData()) {
                std::vector<Uuid> uuids = result.GetPeripheralDevice().GetServiceDataUUID();
                int index = 0;
                for (auto iter = uuids.begin(); iter != uuids.end(); iter++) {
                    bleScanResult.AddServiceData(*iter, result.GetPeripheralDevice().GetServiceData(index));
                    ++index;
                }
            }

            bleScanResult.SetPeripheralDevice(result.GetPeripheralDevice().GetRawAddress());

            bleScanResult.SetPayload(std::string(result.GetPeripheralDevice().GetPayload(),
                result.GetPeripheralDevice().GetPayload() + result.GetPeripheralDevice().GetPayloadLen()));

            observer->OnScanCallback(bleScanResult);
        });
    }

    void OnBleBatchScanResultsEvent(std::vector<BleScanResultImpl> &results) override
    {
        HILOGI("BleCentralManageCallback::OnBleBatchScanResultsEvent start.");

        observers_->ForEach([results](IBluetoothBleCentralManagerCallback *observer) {
            std::vector<BluetoothBleScanResult> bleScanResults;

            for (auto iter = results.begin(); iter != results.end(); iter++) {
                HILOGI("BleCentralManageCallback::OnScanCallback:Address= %{public}s",
                    iter->GetPeripheralDevice().GetRawAddress().GetAddress().c_str());

                BluetoothBleScanResult bleScanResult;

                if (iter->GetPeripheralDevice().IsRSSI()) {
                    bleScanResult.SetRssi(iter->GetPeripheralDevice().GetRSSI());
                }

                bleScanResult.SetAdvertiseFlag(iter->GetPeripheralDevice().GetAdFlag());

                if (iter->GetPeripheralDevice().IsManufacturerData()) {
                    std::map<uint16_t, std::string> manuData = iter->GetPeripheralDevice().GetManufacturerData();
                    for (auto manuDataIter = manuData.begin(); manuDataIter != manuData.end(); manuDataIter++) {
                        bleScanResult.AddManufacturerData(manuDataIter->first, manuDataIter->second);
                    }
                }

                bleScanResult.SetConnectable(iter->GetPeripheralDevice().IsConnectable());

                if (iter->GetPeripheralDevice().IsServiceUUID()) {
                    std::vector<Uuid> uuids = iter->GetPeripheralDevice().GetServiceUUID();
                    for (auto serviceUuidIter = uuids.begin(); serviceUuidIter != uuids.end(); serviceUuidIter++) {
                        bleScanResult.AddServiceUuid(*serviceUuidIter);
                    }
                }

                if (iter->GetPeripheralDevice().IsServiceData()) {
                    std::vector<Uuid> uuids = iter->GetPeripheralDevice().GetServiceDataUUID();
                    int index = 0;
                    for (auto serviceDataIter = uuids.begin(); serviceDataIter != uuids.end(); serviceDataIter++) {
                        bleScanResult.AddServiceData(
                            *serviceDataIter, iter->GetPeripheralDevice().GetServiceData(index));
                        ++index;
                    }
                }

                bleScanResult.SetPeripheralDevice(iter->GetPeripheralDevice().GetRawAddress());

                bleScanResult.SetPayload(std::string(iter->GetPeripheralDevice().GetPayload(),
                    iter->GetPeripheralDevice().GetPayload() + iter->GetPeripheralDevice().GetPayloadLen()));

                bleScanResults.push_back(bleScanResult);
            }
            observer->OnBleBatchScanResultsEvent(bleScanResults);
        });
    }

    void OnStartScanFailed(int resultCode) override
    {
        HILOGI("BleCentralManageCallback::OnStartScanFailed start.");
        observers_->ForEach(
            [resultCode](IBluetoothBleCentralManagerCallback *observer) { observer->OnStartScanFailed(resultCode); });
    }

    void SetObserver(RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothBleCentralManagerCallback> *observers_;
};

struct BluetoothBleCentralManagerServer::impl {
    impl();
    ~impl();

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    RemoteObserverList<IBluetoothBleCentralManagerCallback> observers_;
    std::unique_ptr<BleCentralManagerCallback> observerImp_ = std::make_unique<BleCentralManagerCallback>();
    IAdapterBle *bleService_ = nullptr;
};

class BluetoothBleCentralManagerServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothBleCentralManagerServer::impl *pimpl) : pimpl_(pimpl){};
    void OnSystemStateChange(const BTSystemState state) override
    {
        pimpl_->bleService_ =
            static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
        switch (state) {
            case BTSystemState::ON:
                if (pimpl_->bleService_ != nullptr) {
                    pimpl_->bleService_->RegisterBleCentralManagerCallback(*pimpl_->observerImp_.get());
                }
                break;
            case BTSystemState::OFF:
                pimpl_->bleService_ = nullptr;
                break;
            default:
                break;
        }
    };

private:
    BluetoothBleCentralManagerServer::impl *pimpl_ = nullptr;
};

BluetoothBleCentralManagerServer::impl::impl()
{}

BluetoothBleCentralManagerServer::impl::~impl()
{
    bleService_ = static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
    if (bleService_ != nullptr) {
        bleService_->DeregisterBleCentralManagerCallback();
    }
}

BluetoothBleCentralManagerServer::BluetoothBleCentralManagerServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    pimpl->bleService_ =
        static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
    if (pimpl->bleService_ != nullptr) {
        pimpl->bleService_->RegisterBleCentralManagerCallback(*pimpl->observerImp_.get());
    }
}

BluetoothBleCentralManagerServer::~BluetoothBleCentralManagerServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
}

void BluetoothBleCentralManagerServer::StartScan()
{
    HILOGI("BluetoothBleCentralManagerServer::StartScan start.");

    pimpl->bleService_ =
        static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    if (pimpl->bleService_ != nullptr) {
        pimpl->bleService_->StartScan();
    }
}

void BluetoothBleCentralManagerServer::StartScan(const BluetoothBleScanSettings &settings)
{
    HILOGI("BluetoothBleCentralManagerServer::StartScan with settings start.");

    pimpl->bleService_ =
        static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    if (pimpl->bleService_ != nullptr) {
        BleScanSettingsImpl settingsImpl;
        settingsImpl.SetReportDelay(settings.GetReportDelayMillisValue());
        settingsImpl.SetScanMode(settings.GetScanMode());
        settingsImpl.SetLegacy(settings.GetLegacy());
        settingsImpl.SetPhy(settings.GetPhy());
        pimpl->bleService_->StartScan(settingsImpl);
    }
}

void BluetoothBleCentralManagerServer::StopScan()
{
    HILOGI("BluetoothBleCentralManagerServer::StopScan start.");

    pimpl->bleService_ =
        static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    if (pimpl->bleService_ != nullptr) {
        pimpl->bleService_->StopScan();
    }
}

void BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback(
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    HILOGI("BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback start.");

    if (callback == nullptr) {
        HILOGI(
            "BluetoothBleCentralManagerServer::RegisterBleCentralManagerCallback called with NULL binder. Ignoring.");
        return;
    }
    pimpl->observers_.Register(callback);
}

void BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback(
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    HILOGI("BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback start.");

    if (callback == nullptr) {
        HILOGI(
            "BluetoothBleCentralManagerServer::DeregisterBleCentralManagerCallback called with NULL binder. Ignoring.");
        return;
    }
    pimpl->observers_.Deregister(callback);
}
}  // namespace Bluetooth
}  // namespace OHOS