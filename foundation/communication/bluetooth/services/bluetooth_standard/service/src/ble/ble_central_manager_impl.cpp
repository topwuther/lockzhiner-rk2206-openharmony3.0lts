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

#include "ble_central_manager_impl.h"

#include "ble_adapter.h"
#include "ble_feature.h"
#include "ble_properties.h"
#include "ble_utils.h"
#include "common/adapter_manager.h"
#include "securec.h"

namespace bluetooth {
struct BleCentralManagerImpl::impl {
    /**
     * @brief register scan callback to gap
     *
     * @return @c status.
     */
    int RegisterCallbackToGap();
    /**
     * @brief register extend scan callback to gap
     *
     * @return @c status.
     */
    int RegisterExScanCallbackToGap();
    /**
     * @brief Set report delay.
     *
     * @return @c status
     */
    void StartReportDelay();

    std::recursive_mutex mutex_ {};
    /// callback type
    CALLBACK_TYPE callBackType_ = CALLBACK_TYPE_FIRST_MATCH;
    /// scan status
    int scanStatus_ = SCAN_NOT_STARTED;
    // stop scan type
    STOP_SCAN_TYPE stopScanType_ = STOP_SCAN_TYPE_NOR;
    /// scan result list
    std::vector<BleScanResultImpl> bleScanResult_ {};
    /// Is stop scan
    bool isStopScan_ = false;
    /// scan settings
    BleScanSettingsImpl settings_ {};
    /// scan parameters
    BleScanParams scanParams_ {};
    /// Report delay timer
    std::unique_ptr<utility::Timer> timer_ = nullptr;
    std::map<std::string, std::vector<uint8_t>> incompleteData_ {};
    BleCentralManagerImpl *bleCentralManagerImpl_ = nullptr;
};

BleCentralManagerImpl::BleCentralManagerImpl(
    IBleCentralManagerCallback &callback, IAdapterBle &bleAdapter, utility::Dispatcher &dispatch)
    : centralManagerCallbacks_(&callback),
      bleAdapter_(&bleAdapter),
      dispatcher_(&dispatch),
      pimpl(std::make_unique<BleCentralManagerImpl::impl>())
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    pimpl->bleCentralManagerImpl_ = this;
    pimpl->scanParams_.ownAddrType = BLE_ADDR_TYPE_PUBLIC;
    pimpl->scanParams_.scanFilterPolicy = BLE_SCAN_FILTER_ALLOW_ALL;
    SetActiveScan(true);
    uint16_t interval = BLE_SCAN_MODE_LOW_POWER_INTERVAL_MS;
    SetInterval(interval);
    uint16_t window = BLE_SCAN_MODE_LOW_POWER_WINDOW_MS;
    SetWindow(window);
}

BleCentralManagerImpl::~BleCentralManagerImpl()
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    if (pimpl->timer_ != nullptr) {
        pimpl->timer_->Stop();
        pimpl->timer_ = nullptr;
    }
}

int BleCentralManagerImpl::impl::RegisterCallbackToGap()
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    GapScanCallback scanCallbacks{};
    scanCallbacks.advertisingReport = &BleCentralManagerImpl::AdvertisingReport;
    scanCallbacks.scanSetParamResult = &BleCentralManagerImpl::ScanSetParamResult;
    scanCallbacks.scanSetEnableResult = &BleCentralManagerImpl::ScanSetEnableResult;
    return GAPIF_RegisterScanCallback(&scanCallbacks, bleCentralManagerImpl_);
}

int BleCentralManagerImpl::impl::RegisterExScanCallbackToGap()
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    GapExScanCallback exScanCallbacks{};
    exScanCallbacks.exAdvertisingReport = &BleCentralManagerImpl::ExAdvertisingReport;
    exScanCallbacks.scanExSetParamResult = &BleCentralManagerImpl::ScanExSetParamResult;
    exScanCallbacks.scanExSetEnableResult = &BleCentralManagerImpl::ScanExSetEnableResult;
    exScanCallbacks.directedAdvertisingReport = &BleCentralManagerImpl::DirectedAdvertisingReport;
    exScanCallbacks.scanTimeoutEvent = &BleCentralManagerImpl::ScanTimeoutEvent;
    return GAPIF_RegisterExScanCallback(&exScanCallbacks, bleCentralManagerImpl_);
}

int BleCentralManagerImpl::DeregisterCallbackToGap() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);
    int ret;
    if (BleFeature::GetInstance().IsLeExtendedAdvertisingSupported()) {
        ret = GAPIF_DeregisterExScanCallback();
    } else {
        ret = GAPIF_DeregisterScanCallback();
    }
    return ret;
}

void BleCentralManagerImpl::AdvertisingReport(
    uint8_t advType, const BtAddr *peerAddr, GapAdvReportParam reportParam, const BtAddr *currentAddr, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *centralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((centralManager != nullptr) && (centralManager->dispatcher_ != nullptr)) {
        BtAddr addr;
        (void)memset_s(&addr, sizeof(addr), 0x00, sizeof(addr));
        addr.type = peerAddr->type;
        (void)memcpy_s(addr.addr, BT_ADDRESS_SIZE, peerAddr->addr, BT_ADDRESS_SIZE);
        std::vector<uint8_t> datas(reportParam.data, reportParam.data + reportParam.dataLen);
        LOG_INFO("AdvertisingReport Data=%{public}s", BleUtils::ConvertIntToHexString(datas).c_str());
        centralManager->dispatcher_->PostTask(std::bind(
            &BleCentralManagerImpl::AdvertisingReportTask, centralManager, advType, addr, datas, reportParam.rssi));
    }
}

void BleCentralManagerImpl::ExAdvertisingReport(
    uint8_t advType, const BtAddr *addr, GapExAdvReportParam reportParam, const BtAddr *currentAddr, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *pCentralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((pCentralManager != nullptr) && (pCentralManager->dispatcher_)) {
        BtAddr peerAddr;
        peerAddr.type = addr->type;
        (void)memcpy_s(peerAddr.addr, BT_ADDRESS_SIZE, addr->addr, BT_ADDRESS_SIZE);

        BtAddr peerCurrentAddr;
        if (currentAddr != nullptr) {
            peerCurrentAddr.type = currentAddr->type;
            (void)memcpy_s(peerCurrentAddr.addr, BT_ADDRESS_SIZE, currentAddr->addr, BT_ADDRESS_SIZE);
        } else {
            peerCurrentAddr = peerAddr;
        }

        std::vector<uint8_t> datas(reportParam.data, reportParam.data + reportParam.dataLen);
        LOG_INFO("ExAdvertisingReport Data=%{public}s", BleUtils::ConvertIntToHexString(datas).c_str());
        pCentralManager->dispatcher_->PostTask(std::bind(&BleCentralManagerImpl::ExAdvertisingReportTask,
            pCentralManager,
            advType,
            peerAddr,
            datas,
            reportParam.rssi,
            peerCurrentAddr));
    }
}

void BleCentralManagerImpl::AdvertisingReportTask(
    uint8_t advType, const BtAddr &peerAddr, const std::vector<uint8_t> &data, int8_t rssi) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Data = %{public}s", __func__, BleUtils::ConvertIntToHexString(data).c_str());

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    RawAddress advertisedAddress(RawAddress::ConvertToString(peerAddr.addr));
    bool ret = false;
    BlePeripheralDevice device;
    if (FindDevice(advertisedAddress.GetAddress(), device)) {
        ret = AddPeripheralDevice(advType, peerAddr, data, rssi, device);
        if (ret) {  /// LE General Discoverable Mode LE Limited Discoverable Mode
            return;
        }
        HandleGapEvent(BLE_GAP_SCAN_RESULT_EVT, 0);
        return;
    }

    ret = AddPeripheralDevice(advType, peerAddr, data, rssi, device);
    if (ret) {  /// LE General Discoverable Mode LE Limited Discoverable Mode
        return;
    }
    HandleGapEvent(BLE_GAP_SCAN_RESULT_EVT, 0);
}

bool BleCentralManagerImpl::ExtractIncompleteData(uint8_t advType, const std::string &advertisedAddress,
    const std::vector<uint8_t> &data, std::vector<uint8_t> &completeData) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    if ((advType & BLE_EX_SCAN_DATE_STATUS_INCOMPLETE_MORE) == BLE_EX_SCAN_DATE_STATUS_INCOMPLETE_MORE) {
        auto iter = pimpl->incompleteData_.find(advertisedAddress);
        if (iter == pimpl->incompleteData_.end()) {
            pimpl->incompleteData_.insert(std::make_pair(advertisedAddress, data));
        } else {
            iter->second.insert(iter->second.end(), data.begin(), data.end());
        }
        return true;
    } else if ((advType & BLE_EX_SCAN_DATE_STATUS_INCOMPLETE_NO_MORE) == 0 &&
               (advType & BLE_EX_SCAN_DATE_STATUS_INCOMPLETE_MORE) == 0) {
        auto iter = pimpl->incompleteData_.find(advertisedAddress);
        if (iter != pimpl->incompleteData_.end()) {
            iter->second.insert(iter->second.end(), data.begin(), data.end());
            completeData = iter->second;
        }
    } else if ((advType & BLE_EX_SCAN_DATE_STATUS_INCOMPLETE_NO_MORE) == BLE_EX_SCAN_DATE_STATUS_INCOMPLETE_NO_MORE) {
        auto iter = pimpl->incompleteData_.find(advertisedAddress);
        if (iter != pimpl->incompleteData_.end()) {
            iter->second.insert(iter->second.end(), data.begin(), data.end());
            completeData = iter->second;
        }
    }
    return false;
}

void BleCentralManagerImpl::ExAdvertisingReportTask(uint8_t advType, const BtAddr &peerAddr,
    const std::vector<uint8_t> &data, int8_t rssi, const BtAddr &peerCurrentAddr) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Data = %{public}s", __func__, BleUtils::ConvertIntToHexString(data).c_str());

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    RawAddress advAddress(RawAddress::ConvertToString(peerAddr.addr));
    /// Set whether only legacy advertisments should be returned in scan results.
    if (pimpl->settings_.GetLegacy()) {
        if ((advType & BLE_LEGACY_ADV_NONCONN_IND_WITH_EX_ADV) == 0) {
            LOG_DEBUG("[BleCentralManagerImpl] %{public}s: Excepted addr = %{public}s; advType = %{public}d",
                __func__,
                advAddress.GetAddress().c_str(),
                advType);
            return;
        }
    }

    /// incomplete data
    RawAddress advCurrentAddress(RawAddress::ConvertToString(peerCurrentAddr.addr));
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:peerAddr = %{public}s, peerCurrentAddr = %{public}s",
        __func__,
        advAddress.GetAddress().c_str(),
        advCurrentAddress.GetAddress().c_str());
    std::vector<uint8_t> incompleteData(data.begin(), data.end());
    if (ExtractIncompleteData(advType, advCurrentAddress.GetAddress(), data, incompleteData)) {
        return;
    }

    bool ret = false;
    BlePeripheralDevice device;
    if (FindDevice(advAddress.GetAddress(), device)) {
        ret = AddPeripheralDevice(advType, peerAddr, incompleteData, rssi, device);
        if (ret) {  /// not discovery
            pimpl->incompleteData_.clear();
            return;
        }
        HandleGapExScanEvent(BLE_GAP_EX_SCAN_RESULT_EVT, 0);
        pimpl->incompleteData_.clear();
        return;
    }

    ret = AddPeripheralDevice(advType, peerAddr, incompleteData, rssi, device);
    if (ret) {  /// not discovery
        pimpl->incompleteData_.clear();
        return;
    }
    pimpl->incompleteData_.clear();
    HandleGapExScanEvent(BLE_GAP_EX_SCAN_RESULT_EVT, 0);
}

bool BleCentralManagerImpl::AddPeripheralDevice(uint8_t advType, const BtAddr &peerAddr,
    const std::vector<uint8_t> &data, int8_t rssi, const BlePeripheralDevice &dev) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    BlePeripheralDevice device = dev;
    RawAddress advertisedAddress(RawAddress::ConvertToString(peerAddr.addr));
    device.SetAddress(advertisedAddress);
    device.SetManufacturerData("");
    device.SetRSSI(rssi);
    if (data.size() > 0) {
        BlePeripheralDeviceParseAdvData parseAdvData = {
            .payload = const_cast<uint8_t *>(data.data()),
            .length = data.size(),
        };
        device.ParseAdvertiserment(parseAdvData);
        uint8_t falg = device.GetAdFlag();
        if (CheckBleScanMode(falg)) {
            return true;
        }
    } else {
        uint8_t falg = device.GetAdFlag();
        if (CheckBleScanMode(falg)) {
            return true;
        }
    }
    device.SetAddressType(peerAddr.type);
    if ((advType == SCAN_ADV_SCAN_IND) || (advType == SCAN_ADV_NONCONN_IND)) {
        device.SetConnectable(false);
    } else if ((advType == SCAN_ADV_IND) || (advType == SCAN_ADV_DIRECT_IND)) {
        device.SetConnectable(true);
    }

    BleScanResultImpl result;
    result.SetPeripheralDevice(device);
    pimpl->bleScanResult_.push_back(result);

    return false;
}

bool BleCentralManagerImpl::CheckBleScanMode(uint8_t falg)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:adv flag = %u", __func__, falg);

    switch (BleConfig::GetInstance().GetBleScanMode()) {
        case BLE_SCAN_MODE_NON_DISC:
            if (falg == BLE_ADV_FLAG_NON_LIMIT_DISC) {
                return false;
            }
            break;
        case BLE_SCAN_MODE_GENERAL:
            if ((falg & BLE_ADV_FLAG_LIMIT_DISC) == BLE_ADV_FLAG_LIMIT_DISC ||
                (falg & BLE_ADV_FLAG_GEN_DISC) == BLE_ADV_FLAG_GEN_DISC) {
                return false;
            }
            break;
        case BLE_SCAN_MODE_LIMITED:
            if ((falg & BLE_ADV_FLAG_LIMIT_DISC) == BLE_ADV_FLAG_LIMIT_DISC) {
                return false;
            }
            break;
        case BLE_SCAN_MODE_ALL:
        default:
            return false;
    }
    return true;
}

void BleCentralManagerImpl::ScanSetParamResult(uint8_t status, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *centralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((centralManager != nullptr) && (centralManager->dispatcher_ != nullptr)) {
        centralManager->dispatcher_->PostTask(std::bind(
            &BleCentralManagerImpl::HandleGapEvent, centralManager, BLE_GAP_SCAN_PARAM_SET_COMPLETE_EVT, status));
    }
}

void BleCentralManagerImpl::ScanExSetParamResult(uint8_t status, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *centralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((centralManager != nullptr) && (centralManager->dispatcher_ != nullptr)) {
        centralManager->dispatcher_->PostTask(std::bind(&BleCentralManagerImpl::HandleGapExScanEvent,
            centralManager,
            BLE_GAP_EX_SCAN_PARAM_SET_COMPLETE_EVT,
            status));
    }
}

void BleCentralManagerImpl::ScanSetEnableResult(uint8_t status, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *centralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((centralManager != nullptr) && (centralManager->dispatcher_ != nullptr)) {
        centralManager->dispatcher_->PostTask(
            std::bind(&BleCentralManagerImpl::ScanSetEnableResultTask, centralManager, status));
        (static_cast<BleAdapter *>(centralManager->bleAdapter_))->NotifyAllWaitContinue();
    }
}

void BleCentralManagerImpl::ScanSetEnableResultTask(uint8_t status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (pimpl->isStopScan_) {
        if (pimpl->stopScanType_ == STOP_SCAN_TYPE_RESOLVING_LIST) {
            HandleGapEvent(BLE_GAP_RESOLVING_LIST_ADV_SCAN_STOP_COMPLETE_EVT, status);
        } else {
            HandleGapEvent(BLE_GAP_SCAN_STOP_COMPLETE_EVT, status);
        }
    } else {
        if (pimpl->stopScanType_ == STOP_SCAN_TYPE_RESOLVING_LIST) {
            HandleGapEvent(BLE_GAP_RESOLVING_LIST_ADV_SCAN_START_COMPLETE_EVT, status);
        } else {
            HandleGapEvent(BLE_GAP_SCAN_START_COMPLETE_EVT, status);
        }
    }
}

void BleCentralManagerImpl::ScanExSetEnableResult(uint8_t status, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *centralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((centralManager != nullptr) && (centralManager->dispatcher_ != nullptr)) {
        centralManager->dispatcher_->PostTask(
            std::bind(&BleCentralManagerImpl::ScanExSetEnableResultTask, centralManager, status));
        (static_cast<BleAdapter *>(centralManager->bleAdapter_))->NotifyAllWaitContinue();
    }
}

void BleCentralManagerImpl::ScanExSetEnableResultTask(uint8_t status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (pimpl->isStopScan_) {
        if (pimpl->stopScanType_ == STOP_SCAN_TYPE_RESOLVING_LIST) {
            HandleGapExScanEvent(BLE_GAP_EX_RESOLVING_LIST_ADV_SCAN_STOP_COMPLETE_EVT, status);
        } else {
            HandleGapExScanEvent(BLE_GAP_EX_SCAN_STOP_COMPLETE_EVT, status);
        }
    } else {
        if (pimpl->stopScanType_ == STOP_SCAN_TYPE_RESOLVING_LIST) {
            HandleGapExScanEvent(BLE_GAP_EX_RESOLVING_LIST_ADV_SCAN_START_COMPLETE_EVT, status);
        } else {
            HandleGapExScanEvent(BLE_GAP_EX_SCAN_START_COMPLETE_EVT, status);
        }
    }
}

void BleCentralManagerImpl::StartScan() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:<-- Start scan start", __func__);

    int status = AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE);
    if (status != BTStateID::STATE_TURN_ON) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:%{public}s", __func__, "Bluetooth adapter is invalid.");
        pimpl->scanStatus_ = SCAN_NOT_STARTED;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return;
    }
    pimpl->callBackType_ = CALLBACK_TYPE_FIRST_MATCH;
    Start(false);

    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:<-- Start scan end", __func__);
}

void BleCentralManagerImpl::StartScan(const BleScanSettingsImpl &setting) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:<-- Start scan start", __func__);

    int status = AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE);
    if (status != BTStateID::STATE_TURN_ON) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:%{public}s", __func__, "Bluetooth adapter is invalid.");
        pimpl->scanStatus_ = SCAN_NOT_STARTED;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return;
    }

    pimpl->settings_ = setting;
    int matchingMode;
    if (setting.GetReportDelayMillisValue() > 0) {
        matchingMode = CALLBACK_TYPE_ALL_MATCHES;
    } else {
        matchingMode = CALLBACK_TYPE_FIRST_MATCH;
    }
    SetScanModeDuration(setting.GetScanMode(), matchingMode);
    if (matchingMode == CALLBACK_TYPE_FIRST_MATCH) {
        pimpl->callBackType_ = CALLBACK_TYPE_FIRST_MATCH;
    } else {
        pimpl->callBackType_ = CALLBACK_TYPE_ALL_MATCHES;
    }
    Start(false);

    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:<-- Start scan end", __func__);
}

void BleCentralManagerImpl::StopScan() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:-> Stop scan start", __func__);

    if (pimpl->scanStatus_ == SCAN_NOT_STARTED) {
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return;
    }

    Stop();

    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:-> Stop scan end", __func__);
}

void BleCentralManagerImpl::StartOrStopScan(const STOP_SCAN_TYPE &scanType, bool isStartScan) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:-> StartOrStopScan scan start", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    int ret;
    if (BleFeature::GetInstance().IsExtendedScanSupported()) {
        ret = SetExScanEnable(isStartScan);
    } else {
        ret = SetScanEnable(isStartScan);
    }

    if (ret != BT_NO_ERROR) {
        LOG_ERROR("stop or start extend scan: err: %{public}d isScan : %{public}d", ret, isStartScan);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        return;
    }
    pimpl->stopScanType_ = scanType;
    pimpl->isStopScan_ = !isStartScan;
    if (isStartScan) {
        pimpl->scanStatus_ = SCAN_FAILED_ALREADY_STARTED;
        LOG_DEBUG("start extend scan successful");
    } else {
        if (scanType != STOP_SCAN_TYPE_RESOLVING_LIST) {
            pimpl->scanStatus_ = SCAN_NOT_STARTED;
        } else {
            pimpl->scanStatus_ = SCAN_FAILED_ALREADY_STARTED;
        }
        LOG_DEBUG("stop extend scan successful");
    }
}

void BleCentralManagerImpl::SetActiveScan(bool active) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    if (active) {
        pimpl->scanParams_.scanType = BLE_SCAN_TYPE_ACTIVE;
    } else {
        pimpl->scanParams_.scanType = BLE_SCAN_TYPE_PASSIVE;
    }
}

void BleCentralManagerImpl::SetInterval(uint16_t intervalMSecs) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    pimpl->scanParams_.scanInterval = intervalMSecs / BLE_SCAN_UNIT_TIME;
}

void BleCentralManagerImpl::SetWindow(uint16_t windowMSecs) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    pimpl->scanParams_.scanWindow = windowMSecs / BLE_SCAN_UNIT_TIME;
}

void BleCentralManagerImpl::ClearResults() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    pimpl->bleScanResult_.clear();
}

void BleCentralManagerImpl::SetScanModeDuration(int scanMode, int type) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    switch (scanMode) {
        case SCAN_MODE_LOW_POWER:
            if (type == CALLBACK_TYPE_ALL_MATCHES) {
                uint16_t interval = BLE_SCAN_MODE_BATCH_LOW_POWER_INTERVAL_MS;
                SetInterval(interval);
                uint16_t window = BLE_SCAN_MODE_BATCH_LOW_POWER_WINDOW_MS;
                SetWindow(window);
            } else {
                uint16_t interval = BLE_SCAN_MODE_LOW_POWER_INTERVAL_MS;
                SetInterval(interval);
                uint16_t window = BLE_SCAN_MODE_LOW_POWER_WINDOW_MS;
                SetWindow(window);
            }
            break;
        case SCAN_MODE_BALANCED:
            if (type == CALLBACK_TYPE_ALL_MATCHES) {
                uint16_t interval = BLE_SCAN_MODE_BATCH_BALANCED_INTERVAL_MS;
                SetInterval(interval);
                uint16_t window = BLE_SCAN_MODE_BATCH_BALANCED_WINDOW_MS;
                SetWindow(window);
            } else {
                uint16_t interval = BLE_SCAN_MODE_BALANCED_INTERVAL_MS;
                SetInterval(interval);
                uint16_t window = BLE_SCAN_MODE_BALANCED_WINDOW_MS;
                SetWindow(window);
            }
            break;
        case SCAN_MODE_LOW_LATENCY:
            if (type == CALLBACK_TYPE_ALL_MATCHES) {
                uint16_t interval = BLE_SCAN_MODE_BATCH_LOW_LATENCY_INTERVAL_MS;
                SetInterval(interval);
                uint16_t window = BLE_SCAN_MODE_BATCH_LOW_LATENCY_WINDOW_MS;
                SetWindow(window);
            } else {
                uint16_t interval = BLE_SCAN_MODE_LOW_LATENCY_INTERVAL_MS;
                SetInterval(interval);
                uint16_t window = BLE_SCAN_MODE_LOW_LATENCY_WINDOW_MS;
                SetWindow(window);
            }
            break;
        default:
            break;
    }
}

void BleCentralManagerImpl::TimerCallback(void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    auto *centralManager = static_cast<BleCentralManagerImpl *>(context);
    if ((centralManager != nullptr) && (centralManager->dispatcher_ != nullptr)) {
        centralManager->dispatcher_->PostTask(
            std::bind(&BleCentralManagerImpl::HandleGapEvent, centralManager, BLE_GAP_SCAN_DELAY_REPORT_RESULT_EVT, 0));
        centralManager->dispatcher_->PostTask(std::bind(&BleCentralManagerImpl::StopScan, centralManager));
    }
}

int BleCentralManagerImpl::SetScanParamToGap() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    GapLeScanParam param;
    param.scanType = pimpl->scanParams_.scanType;
    param.param.scanInterval = pimpl->scanParams_.scanInterval;
    param.param.scanWindow = pimpl->scanParams_.scanWindow;
    return GAPIF_LeScanSetParam(param, pimpl->scanParams_.scanFilterPolicy);
}

int BleCentralManagerImpl::SetExScanParamToGap() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::vector<GapLeScanParam> params;
    uint8_t scanPhys = GAP_EX_SCAN_PHY_1M;
    if (pimpl->settings_.GetLegacy()) {
        GapLeScanParam param;
        param.scanType = pimpl->scanParams_.scanType;
        param.param.scanInterval = pimpl->scanParams_.scanInterval;
        param.param.scanWindow = pimpl->scanParams_.scanWindow;
        params.push_back(param);
    } else {
        switch (pimpl->settings_.GetPhy()) {
            case PHY_LE_CODED:
                scanPhys = GAP_EX_SCAN_PHY_CODED;
                GapLeScanParam paramCoded;
                paramCoded.scanType = pimpl->scanParams_.scanType;
                paramCoded.param.scanInterval = pimpl->scanParams_.scanInterval;
                paramCoded.param.scanWindow = pimpl->scanParams_.scanWindow;
                params.push_back(paramCoded);
                break;
            case PHY_LE_ALL_SUPPORTED:
                scanPhys = GAP_EX_SCAN_PHY_1M | GAP_EX_SCAN_PHY_CODED;
                GapLeScanParam param1M;
                param1M.scanType = pimpl->scanParams_.scanType;
                param1M.param.scanInterval = pimpl->scanParams_.scanInterval;
                param1M.param.scanWindow = pimpl->scanParams_.scanWindow;
                params.push_back(param1M);

                GapLeScanParam paramCodedAll;
                paramCodedAll.scanType = pimpl->scanParams_.scanType;
                paramCodedAll.param.scanInterval = pimpl->scanParams_.scanInterval;
                paramCodedAll.param.scanWindow = pimpl->scanParams_.scanWindow;
                params.push_back(paramCodedAll);
                break;
            case PHY_LE_1M:
            default:
                scanPhys = GAP_EX_SCAN_PHY_1M;
                GapLeScanParam param;
                param.scanType = pimpl->scanParams_.scanType;
                param.param.scanInterval = pimpl->scanParams_.scanInterval;
                param.param.scanWindow = pimpl->scanParams_.scanWindow;
                params.push_back(param);
                break;
        }
    }
    return GAPIF_LeExScanSetParam(pimpl->scanParams_.scanFilterPolicy, scanPhys, &params[0]);
}

int BleCentralManagerImpl::SetScanEnable(bool enable) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:%{public}d", __func__, enable);

    return GAPIF_LeScanSetEnable(enable, isDuplicates_);
}

int BleCentralManagerImpl::SetExScanEnable(bool enable) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    return GAPIF_LeExScanSetEnable(enable, isDuplicates_, 0, 0);
}

void BleCentralManagerImpl::impl::StartReportDelay()
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    if ((settings_.GetReportDelayMillisValue() > 0) && (callBackType_ == CALLBACK_TYPE_ALL_MATCHES)) {
        if (timer_ == nullptr) {
            timer_ = std::make_unique<utility::Timer>(std::bind(&TimerCallback, bleCentralManagerImpl_));
        }
        timer_->Start(settings_.GetReportDelayMillisValue());
    }
}

bool BleCentralManagerImpl::SetLegacyScanParamToGap() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    bool ret = true;
    ret = pimpl->RegisterCallbackToGap();
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:RegisterCallbackToGap failed.", __func__);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return false;
    }

    pimpl->isStopScan_ = false;
    ret = SetScanParamToGap();
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:SetScanParamToGap failed.", __func__);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return false;
    }
    return ret;
}

bool BleCentralManagerImpl::SetExtendScanParamToGap() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    bool ret = true;
    ret = pimpl->RegisterExScanCallbackToGap();
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:RegisterExScanCallbackToGap failed.", __func__);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return false;
    }

    pimpl->isStopScan_ = false;
    ret = SetExScanParamToGap();
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:SetExScanParamToGap failed.", __func__);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return false;
    }
    return ret;
}

bool BleCentralManagerImpl::SetScanParamOrExScanParamToGap() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    bool ret = true;
    if (BleFeature::GetInstance().IsExtendedScanSupported()) {
        ret = SetExtendScanParamToGap();
    } else {
        ret = SetLegacyScanParamToGap();
    }
    return ret;
}

bool BleCentralManagerImpl::Start(bool isContinue) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    if (pimpl->scanStatus_ == SCAN_FAILED_ALREADY_STARTED) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:%{public}s", __func__, "Scan already started.");
        centralManagerCallbacks_->OnStartScanFailed(SCAN_FAILED_ALREADY_STARTED);
        return true;
    }

    pimpl->scanStatus_ = SCAN_FAILED_ALREADY_STARTED;
    pimpl->stopScanType_ = STOP_SCAN_TYPE_NOR;

    if (!isContinue) {
        ClearResults();
    }
    return SetScanParamOrExScanParamToGap();
}

void BleCentralManagerImpl::Stop() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    int ret;
    if (BleFeature::GetInstance().IsExtendedScanSupported()) {
        ret = SetExScanEnable(false);
    } else {
        ret = SetScanEnable(false);
    }
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("stop scanning: err: %{public}d", ret);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return;
    } else {
        std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
        pimpl->stopScanType_ = STOP_SCAN_TYPE_NOR;
        pimpl->isStopScan_ = true;
        pimpl->settings_.SetReportDelay(0);
    }
}

int BleCentralManagerImpl::GetDeviceType(const std::string &address) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    for (auto value : pimpl->bleScanResult_) {
        if (!address.compare(value.GetPeripheralDevice().GetRawAddress().GetAddress())) {
            return value.GetPeripheralDevice().GetDeviceType();
        }
    }
    return BLE_BT_DEVICE_TYPE_UNKNOWN;
}

int BleCentralManagerImpl::GetDeviceAddrType(const std::string &address) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    for (auto value : pimpl->bleScanResult_) {
        if (!address.compare(value.GetPeripheralDevice().GetRawAddress().GetAddress())) {
            return value.GetPeripheralDevice().GetAddressType();
        }
    }
    return BLE_ADDR_TYPE_UNKNOWN;
}

std::string BleCentralManagerImpl::GetDeviceName(const std::string &address) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    for (auto value : pimpl->bleScanResult_) {
        if (!address.compare(value.GetPeripheralDevice().GetRawAddress().GetAddress())) {
            return value.GetPeripheralDevice().GetName();
        }
    }
    return std::string("");
}

bool BleCentralManagerImpl::FindDevice(const std::string &address, BlePeripheralDevice &dev) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    for (auto it = pimpl->bleScanResult_.begin(); it != pimpl->bleScanResult_.end(); it++) {
        if (!address.compare(it->GetPeripheralDevice().GetRawAddress().GetAddress())) {
            dev = it->GetPeripheralDevice();
            pimpl->bleScanResult_.erase(it);
            return true;
        }
    }
    return false;
}

int BleCentralManagerImpl::GetScanStatus() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    return pimpl->scanStatus_;
}

void BleCentralManagerImpl::DirectedAdvertisingReport(uint8_t advType, const BtAddr *addr,
    GapDirectedAdvReportParam reportParam, const BtAddr *currentAddr, void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);
}

void BleCentralManagerImpl::ScanTimeoutEvent(void *context)
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);
}

void BleCentralManagerImpl::GapScanParamSetCompleteEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(status);
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Set scan param failed! %{public}d.", __func__, status);
        return;
    }

    pimpl->StartReportDelay();
    if (pimpl->isStopScan_) {
        return;
    }
    int ret = SetScanEnable(true);
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:SetScanEnable param failed! %{public}d.", __func__, ret);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return;
    }
}

void BleCentralManagerImpl::GapScanResultEvt() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Scan result", __func__);

    if ((centralManagerCallbacks_ != nullptr) && (pimpl->callBackType_ == CALLBACK_TYPE_FIRST_MATCH)) {
        std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
        centralManagerCallbacks_->OnScanCallback(pimpl->bleScanResult_.back());
    }
}

void BleCentralManagerImpl::GapScanDelayReportResultEvt() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Scan batch results", __func__);

    if ((centralManagerCallbacks_ != nullptr) && (pimpl->callBackType_ == CALLBACK_TYPE_ALL_MATCHES)) {
        if (pimpl->timer_ != nullptr) {
            pimpl->timer_->Stop();
        }
        std::lock_guard<std::recursive_mutex> legacyLock(pimpl->mutex_);
        centralManagerCallbacks_->OnBleBatchScanResultsEvent(pimpl->bleScanResult_);
    }
}

void BleCentralManagerImpl::GapScanStartCompleteEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Start scan", __func__);

    centralManagerCallbacks_->OnStartScanFailed(status);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Start scan failed! %{public}d.", __func__, status);
    }
}

void BleCentralManagerImpl::GapScanStopCompleteEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Stop scan", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Stop scan failed! %{public}d.", __func__, status);
    }
    pimpl->scanStatus_ = SCAN_NOT_STARTED;
    centralManagerCallbacks_->OnStartScanFailed(status);
}

void BleCentralManagerImpl::GapScanResolvingCompletEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s: ", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Resovling stop scan failed! %{public}d.", __func__, status);
    }
}

void BleCentralManagerImpl::HandleGapEvent(const BLE_GAP_CB_EVENT &event, int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:[event no: %{public}d]", __func__, (int)event);

    switch (event) {
        case BLE_GAP_SCAN_PARAM_SET_COMPLETE_EVT:
            GapScanParamSetCompleteEvt(status);
            break;
        case BLE_GAP_SCAN_RESULT_EVT:
            GapScanResultEvt();
            break;
        case BLE_GAP_SCAN_DELAY_REPORT_RESULT_EVT:
            GapScanDelayReportResultEvt();
            break;
        case BLE_GAP_SCAN_START_COMPLETE_EVT:
            GapScanStartCompleteEvt(status);
            break;
        case BLE_GAP_RESOLVING_LIST_ADV_SCAN_START_COMPLETE_EVT:
            GapScanResolvingCompletEvt(status);
            break;
        case BLE_GAP_SCAN_STOP_COMPLETE_EVT:
            GapScanStopCompleteEvt(status);
            break;
        case BLE_GAP_RESOLVING_LIST_ADV_SCAN_STOP_COMPLETE_EVT:
            GapScanResolvingCompletEvt(status);
            break;
        default: {
            LOG_ERROR("[BleCentralManagerImpl] %{public}s:Invalid event! %{public}d.", __func__, event);
            break;
        }
    }
}

void BleCentralManagerImpl::GapExScanParamSetCompleteEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Set scan param failed! %{public}d.", __func__, status);
        centralManagerCallbacks_->OnStartScanFailed(status);
        return;
    }
    pimpl->StartReportDelay();
    if (pimpl->isStopScan_) {
        return;
    }
    int ret = SetExScanEnable(true);
    if (ret != BT_NO_ERROR) {
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:SetExScanEnable param failed! %{public}d.", __func__, ret);
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        centralManagerCallbacks_->OnStartScanFailed(pimpl->scanStatus_);
        return;
    }
}

void BleCentralManagerImpl::GapExScanResultEvt() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Scan result", __func__);

    if ((centralManagerCallbacks_ != nullptr) && (pimpl->callBackType_ == CALLBACK_TYPE_FIRST_MATCH)) {
        std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
        centralManagerCallbacks_->OnScanCallback(pimpl->bleScanResult_.back());
    }
}

void BleCentralManagerImpl::GapExScanDelayReportResultEvt() const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Scan batch results", __func__);

    if ((centralManagerCallbacks_ != nullptr) && (pimpl->callBackType_ == CALLBACK_TYPE_ALL_MATCHES)) {
        if (pimpl->timer_ != nullptr) {
            pimpl->timer_->Stop();
        }
        std::lock_guard<std::recursive_mutex> exAdvLock(pimpl->mutex_);
        centralManagerCallbacks_->OnBleBatchScanResultsEvent(pimpl->bleScanResult_);
    }
}

void BleCentralManagerImpl::GapExScanStartCompleteEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Start scan", __func__);

    centralManagerCallbacks_->OnStartScanFailed(status);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Start scan failed! %{public}d.", __func__, status);
    }
}

void BleCentralManagerImpl::GapExScanStopCompleteEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:Stop scan", __func__);

    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Stop scan failed! %{public}d.", __func__, status);
    }
    pimpl->scanStatus_ = SCAN_NOT_STARTED;
    centralManagerCallbacks_->OnStartScanFailed(status);
}

void BleCentralManagerImpl::GapExScanResolvingCompletEvt(int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:", __func__);
    std::lock_guard<std::recursive_mutex> lk(pimpl->mutex_);
    if (status != BT_NO_ERROR) {
        pimpl->scanStatus_ = SCAN_FAILED_INTERNAL_ERROR;
        LOG_ERROR("[BleCentralManagerImpl] %{public}s:Resolving stop scan failed! %{public}d.", __func__, status);
    }
}

void BleCentralManagerImpl::HandleGapExScanEvent(const BLE_GAP_CB_EVENT &event, int status) const
{
    LOG_DEBUG("[BleCentralManagerImpl] %{public}s:[event no: %{public}d]", __func__, (int)event);

    switch (event) {
        case BLE_GAP_EX_SCAN_PARAM_SET_COMPLETE_EVT:
            GapExScanParamSetCompleteEvt(status);
            break;
        case BLE_GAP_EX_SCAN_RESULT_EVT:
            GapExScanResultEvt();
            break;
        case BLE_GAP_EX_SCAN_DELAY_REPORT_RESULT_EVT:
            GapExScanDelayReportResultEvt();
            break;
        case BLE_GAP_EX_SCAN_START_COMPLETE_EVT:
            GapExScanStartCompleteEvt(status);
            break;
        case BLE_GAP_EX_RESOLVING_LIST_ADV_SCAN_START_COMPLETE_EVT:
            GapExScanResolvingCompletEvt(status);
            break;
        case BLE_GAP_EX_SCAN_STOP_COMPLETE_EVT:
            GapExScanStopCompleteEvt(status);
            break;
        case BLE_GAP_EX_RESOLVING_LIST_ADV_SCAN_STOP_COMPLETE_EVT:
            GapExScanResolvingCompletEvt(status);
            break;
        default: {
            LOG_ERROR("[BleCentralManagerImpl] %{public}s:Invalid event! %{public}d.", __func__, event);
            break;
        }
    }
}
}  // namespace bluetooth
