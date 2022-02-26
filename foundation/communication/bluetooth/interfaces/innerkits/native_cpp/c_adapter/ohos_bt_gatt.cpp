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

#include "ohos_bt_gatt.h"

#include <cstring>
#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_log.h"

#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
#define MAX_BLE_ADV_NUM 7

class BleCentralManagerCallbackWapper;
class BleAdvCallback;

static BtGattCallbacks *g_AppCallback;
static BleCentralManager *g_BleCentralManager;
static BleCentralManagerCallbackWapper *g_ScanCallback;

static BleAdvCallback *g_BleAdvCallbacks[MAX_BLE_ADV_NUM];

extern void GetAddrFromString(std::string in, unsigned char out[6]);
extern void GetAddrFromByte(unsigned char in[6], std::string &out);

class BleCentralManagerCallbackWapper : public BleCentralManagerCallback {
public:
    /**
     * @brief Scan result callback.
     *
     * @param result Scan result.
     * @since 6
     */
    void OnScanCallback(const BleScanResult &result) {
        BtScanResultData scanResult;
        scanResult.eventType = OHOS_BLE_EVT_LEGACY_CONNECTABLE; // result.IsConnectable();
        scanResult.dataStatus = OHOS_BLE_DATA_COMPLETE;
        scanResult.addrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
        scanResult.primaryPhy = OHOS_BLE_SCAN_PHY_1M;
        scanResult.secondaryPhy = OHOS_BLE_SCAN_PHY_1M;
        scanResult.advSid = 1;
        scanResult.txPower = 1;
        scanResult.rssi = result.GetRssi();
        scanResult.directAddrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
        GetAddrFromString(result.GetPeripheralDevice().GetDeviceAddr(), scanResult.addr.addr);
        vector<uint8_t> vec = result.GetPayload();
        scanResult.advData = vec.data();
        scanResult.advLen = vec.size();

        std::string strs;
        std::stringstream strStream;
        for (int i = 0; i < scanResult.advLen; i++) {
            char token[3] = {0};
            (void)sprintf_s(token, 3, "%02X", scanResult.advData[i]);
            strStream << token;
        }
        strStream >> strs;
        HILOGI("address: %{public}02X:%{public}02X:***:%{public}02X,scan data: %{public}s",
            scanResult.addr.addr[0], scanResult.addr.addr[1], scanResult.addr.addr[5], strs.c_str());

        g_AppCallback->scanResultCb(&scanResult);
    }

    /**
     * @brief Batch scan results event callback.
     *
     * @param results Scan results.
     * @since 6
     */
    void OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results) {}

    /**
     * @brief Start scan failed callback.
     *
     * @param resultCode Scan result code.
     * @since 6
     */
    void OnStartScanFailed(int resultCode) {}
};

class BleAdvCallback : public BleAdvertiseCallback {
public:
    BleAdvCallback(BleAdvertiser *handle, int advId) {
        advHandle = handle;
        isStarted = false;
        advId_ = advId;
    }

    void OnStartResultEvent(int result) {
        if (result != 0) {
            HILOGE("result : %{public}d", result);
            return;
        }

        if (isStarted) {
            HILOGI("adv stoped.");
            isStarted = false;
            if (g_AppCallback != NULL && g_AppCallback->advDisableCb != NULL) {
                HILOGI("adv stoped advId_: %{public}d.", advId_);
                g_AppCallback->advDisableCb(advId_, 0);
            }
            g_BleAdvCallbacks[advId_] = NULL;
            HILOGI("g_BleAdvCallbacks[%{public}d] = %{public}p.", advId_, g_BleAdvCallbacks[advId_]);
            delete this;
        } else {
            HILOGI("adv started.");
            isStarted = true;
            if (g_AppCallback != NULL && g_AppCallback->advEnableCb != NULL) {
                g_AppCallback->advEnableCb(advId_, 0);
            }
            
        }
    }

    BleAdvertiser *GetAdvHandle() {
        return advHandle;
    }
protected:
    ~BleAdvCallback() {
        delete advHandle;
    }

    BleAdvertiserData *advData;
    BleAdvertiserData *advResponseData;
    BleAdvertiserSettings *advSetting;

private:
    BleAdvertiser *advHandle;
    bool isStarted;
    int advId_;
};

/**
 * @brief Initializes the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is initialized;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int InitBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Enables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is enabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int EnableBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Disables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is disabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int DisableBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets the Bluetooth device name.
 *
 * @param name Indicates the pointer to the name to set.
 * @param len Indicates the length of the name to set.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth device name is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int SetDeviceName(const char *name, unsigned int len) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets advertising data.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param data Indicates the pointer to the advertising data. For details, see {@link BleConfigAdvData}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising data is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetAdvData(int advId, const BleConfigAdvData *data)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Starts advertising.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param param Indicates the pointer to the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdv(int advId, const BleAdvParams *param)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Stops advertising.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStopAdv(int advId)
{
    if (advId >= 0 && advId < MAX_BLE_ADV_NUM) {
        g_BleAdvCallbacks[advId]->GetAdvHandle()->StopAdvertising(*g_BleAdvCallbacks[advId]);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Updates advertising parameters.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param param Indicates the pointer to the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising parameters are updated;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleUpdateAdv(int advId, const BleAdvParams *param) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets the secure I/O capability mode.
 *
 * @param mode Indicates the capability mode to set. For details, see {@link BleIoCapMode}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the capability mode is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetSecurityIoCap(BleIoCapMode mode) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets the authentication mode for secure connection requests.
 *
 * @param mode Indicates the authentication mode to set. For details, see {@link BleAuthReqMode}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the authentication mode is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetSecurityAuthReq(BleAuthReqMode mode) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Responds to a secure connection request.
 *
 * @param bdAddr Indicates the address of the device that sends the request.
 * @param accept Specifies whether to accept the request. The value <b>true</b> means to accept the request,
 * and <b>false</b> means to reject the request.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the request is responded to;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattSecurityRsp(BdAddr bdAddr, bool accept) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Obtains the device MAC address.
 *
 * @param mac Indicates the pointer to the device MAC address.
 * @param len Indicates the length of the device MAC address.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the device MAC address is obtained;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int ReadBtMacAddr(unsigned char *mac, unsigned int len) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets scan parameters.
 *
 * @param clientId Indicates the client ID, which is obtained during client registration.
 * @param param Indicates the pointer to the scan parameters. For details, see {@link BleScanParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan parameters are set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetScanParameters(int clientId, BleScanParams *param)
{
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a scan.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartScan(void)
{
    HILOGI("");
    if (g_BleCentralManager == NULL) {
        return 1;
    }

    g_BleCentralManager->StartScan();
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Stops a scan.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStopScan(void)
{
    if (g_BleCentralManager == NULL) {
        return 1;
    }

    g_BleCentralManager->StopScan();
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Registers GATT callbacks.
 *
 * @param func Indicates the pointer to the callbacks to register. For details, see {@link BtGattCallbacks}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT callbacks are registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattRegisterCallbacks(BtGattCallbacks *func)
{
    g_AppCallback = func;

    if (g_ScanCallback == NULL) {
        g_ScanCallback = new BleCentralManagerCallbackWapper();
    }

    if (g_BleCentralManager == NULL) {
        g_BleCentralManager = new BleCentralManager(*g_ScanCallback);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets advertising data and parameters and starts advertising.
 *
 * This function is available for system applications only. \n
 *
 * @param advId Indicates the pointer to the advertisement ID.
 * @param rawData Indicates the advertising data. For details, see {@link StartAdvRawData}.
 * @param advParam Indicates the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdvEx(int *advId, const StartAdvRawData rawData, BleAdvParams advParam) {
    HILOGI("enter");
    int i = 0;
    for (i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_BleAdvCallbacks[i] == NULL) {
            g_BleAdvCallbacks[i] = new BleAdvCallback(new BleAdvertiser(), i);
            break;
        }
        HILOGI("g_BleAdvCallbacks[%{public}d] = %{public}p.", i, g_BleAdvCallbacks[i]);
    }

    if (i == MAX_BLE_ADV_NUM) {
        return OHOS_BT_STATUS_UNHANDLED;
    }

    *advId = i;

    BleAdvertiser *advHandle = g_BleAdvCallbacks[i]->GetAdvHandle();
    
    BleAdvertiserSettings settings;
    settings.SetInterval(advParam.minInterval);
    if (advParam.advType == OHOS_BLE_ADV_SCAN_IND ||
        advParam.advType == OHOS_BLE_ADV_NONCONN_IND) {
        settings.SetConnectable(false);
    }

    vector<uint8_t> advData;
    if (rawData.advData != NULL) {
        for (unsigned int i = 0; i < rawData.advDataLen; i++) {
            advData.push_back(rawData.advData[i]);
        }
    }

    vector<uint8_t> scanResponse;
    if (rawData.rspData != NULL) {
        for (unsigned int i = 0; i < rawData.rspDataLen; i++) {
            scanResponse.push_back(rawData.rspData[i]);
        }
    }

    advHandle->StartAdvertising(settings, advData, scanResponse, *g_BleAdvCallbacks[i]);
    return OHOS_BT_STATUS_SUCCESS;
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
/** @} */
