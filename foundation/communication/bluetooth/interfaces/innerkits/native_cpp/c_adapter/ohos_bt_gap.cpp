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

#include "ohos_bt_gap.h"

#include <iostream>
#include <string.h>
#include <vector>

#include "bluetooth_host.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
static BluetoothHost *g_BluetoothHost;
static BtGapCallBacks *g_GapCallback;

extern void GetAddrFromString(std::string in, unsigned char out[6]);
extern void GetAddrFromByte(unsigned char in[6], std::string &out);

class BluetoothHostObserverWapper : public BluetoothHostObserver {
public:
    /**
     * @brief Adapter state change function.
     *
     * @param transport Transport type when state change.
     *        BTTransport::ADAPTER_BREDR : classic;
     *        BTTransport::ADAPTER_BLE : ble.
     * @param state Change to the new state.
     *        BTStateID::STATE_TURNING_ON;
     *        BTStateID::STATE_TURN_ON;
     *        BTStateID::STATE_TURNING_OFF;
     *        BTStateID::STATE_TURN_OFF.
     * @since 6
     */
    void OnStateChanged(const int transport, const int status) {
        int cvtTransport = OHOS_BT_TRANSPORT_LE;
        if (transport == BTTransport::ADAPTER_BREDR) {
            cvtTransport = OHOS_BT_TRANSPORT_BR_EDR;
        }
        if (g_GapCallback != NULL && g_GapCallback->stateChangeCallback != NULL) {
            HILOGI("transport: %{public}d, status: %{public}d", cvtTransport, status);
            g_GapCallback->stateChangeCallback(cvtTransport, status);
        } else {
            HILOGW("transport: %{public}d, status: %{public}d, but callback is null!", cvtTransport, status);
        }
    }

    /**
     * @brief Discovery state changed observer.
     *
     * @param status Device discovery status.
     * @since 6
     */
    void OnDiscoveryStateChanged(int status) {}

    /**
     * @brief Discovery result observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnDiscoveryResult(const BluetoothRemoteDevice &device) {}

    /**
     * @brief Pair request observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnPairRequested(const BluetoothRemoteDevice &device) {}

    /**
     * @brief Pair confirmed observer.
     *
     * @param device Remote device.
     * @param reqType Pair type.
     * @param number Paired passkey.
     * @since 6
     */
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) {};

    /**
     * @brief Scan mode changed observer.
     *
     * @param mode Device scan mode.
     * @since 6
     */
    void OnScanModeChanged(int mode) {};

    /**
     * @brief Device name changed observer.
     *
     * @param deviceName Device name.
     * @since 6
     */
    void OnDeviceNameChanged(const std::string &deviceName) {};

    /**
     * @brief Device address changed observer.
     *
     * @param address Device address.
     * @since 6
     */
    void OnDeviceAddrChanged(const std::string &address) {};
};

static BluetoothHostObserverWapper g_HostObserver;

bool EnableBle(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    if (g_BluetoothHost->IsBleEnabled()) {
        HILOGI("ble enabled already");
        return true;
    }
    
    bool ret = g_BluetoothHost->EnableBle();
    HILOGI("result: %{public}d", ret);
    return ret;
}

bool DisableBle(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    if (!g_BluetoothHost->IsBleEnabled()) {
        HILOGI("ble disabled already");
        return true;
    }

    bool ret = g_BluetoothHost->DisableBle();
    HILOGI("result: %{public}d", ret);
    return ret;
}

bool EnableBt(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state = g_BluetoothHost->GetBtState();
    if (state == BTStateID::STATE_TURNING_ON ||
        state == BTStateID::STATE_TURN_ON) {
        HILOGI("br state is %{public}d", state);
        return true;
    }
    bool ret = g_BluetoothHost->EnableBt();
    HILOGI("result: %{public}d", ret);
    return ret;
}

bool DisableBt(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state = g_BluetoothHost->GetBtState();
    if (state == BTStateID::STATE_TURNING_OFF ||
        state == BTStateID::STATE_TURN_OFF) {
        HILOGI("br state is %{public}d", state);
        return true;
    }
    bool ret = g_BluetoothHost->DisableBt();
    HILOGI("result: %{public}d", ret);
    return ret;
}

int GetBtState()
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state = g_BluetoothHost->GetBtState();
    HILOGI("br state: %{public}d", state);
    return state;
}

bool IsBleEnabled()
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    bool ret = g_BluetoothHost->IsBleEnabled();
    HILOGI("ble enable: %{public}d", ret);
    return ret;
}

bool GetLocalAddr(unsigned char *mac, unsigned int len)
{
    if (mac == NULL || len < OHOS_BD_ADDR_LEN) {
        HILOGE("invalid param : mac:%{public}p, len:%{public}d", mac, len);
        return false;
    }
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    string localAddress = g_BluetoothHost->GetLocalAddress();
    GetAddrFromString(localAddress, mac);
    HILOGI("address: %02X:%02X:*:*:*:%02X", mac[0], mac[1], mac[5]);
    return true;
}

bool SetLocalName(unsigned char *localName, unsigned char length)
{
    if (localName == NULL) {
        HILOGE("invalid param: localName(%{public}p)", localName);
        return false;
    }

    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    string newName((const char *)localName);
    bool ret = g_BluetoothHost->SetLocalName(newName);
    HILOGI("result %{public}d: LocalName : %{public}s", ret, g_BluetoothHost->GetLocalName().c_str());
    return ret;
}

int GapRegisterCallbacks(BtGapCallBacks *func)
{
    HILOGI();
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }
    g_GapCallback = func;
    g_BluetoothHost->RegisterObserver(g_HostObserver);
    return OHOS_BT_STATUS_SUCCESS;
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
