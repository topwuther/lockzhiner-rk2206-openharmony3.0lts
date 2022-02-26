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

#include <string>

#include "bluetooth_raw_address.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
sptr<BluetoothHostProxy> GetHostProxy()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    sptr<BluetoothHostProxy> hostProxy = new BluetoothHostProxy(remote);
    return hostProxy;
}

BluetoothRemoteDevice::BluetoothRemoteDevice(const std::string &addr, const int transport)
{
    address_ = addr;
    transport_ = transport;
}

int BluetoothRemoteDevice::GetDeviceType() const
{
    int type = 0;
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGI("Invalid remote device.");
        return type;
    }

    sptr<BluetoothHostProxy> proxy = GetHostProxy();
    if (proxy == nullptr) {
        HILOGI("BT host server not created.");
        return type;
    }

    return proxy->GetDeviceType(transport_, address_);
}

bool BluetoothRemoteDevice::IsValidBluetoothRemoteDevice() const
{
    if (!BluetoothHost::IsValidBluetoothAddr(address_)) {
        HILOGI("invalid bluetooth addr.");
        return false;
    }

    if ((BT_TRANSPORT_BREDR != transport_) && (BT_TRANSPORT_BLE != transport_)) {
        HILOGI("invalid transport type.");
        return false;
    }
    return true;
}
int BluetoothRemoteDevice::GetTransportType() const
{
    return transport_;
}

int BluetoothRemoteDevice::GetPhonebookPermission() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetPhonebookPermission fails: no proxy");
        return INVALID_VALUE;
    }

    return hostProxy->GetPhonebookPermission(address_);
}

bool BluetoothRemoteDevice::SetPhonebookPermission(int permission)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::SetPhonebookPermission fails: no proxy");
        return false;
    }
    return hostProxy->SetPhonebookPermission(address_, permission);
}

int BluetoothRemoteDevice::GetMessagePermission() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetMessagePermission fails: no proxy");
        return INVALID_VALUE;
    }
    return hostProxy->GetMessagePermission(address_);
}

bool BluetoothRemoteDevice::SetMessagePermission(int permission)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::SetMessagePermission fails: no proxy");
        return false;
    }
    return hostProxy->SetMessagePermission(address_, permission);
}

int BluetoothRemoteDevice::GetPowerMode(void) const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetPowerMode fails: no proxy");
        return INVALID_VALUE;
    }
    return hostProxy->GetPowerMode(address_);
}

std::string BluetoothRemoteDevice::GetDeviceName() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_NAME;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetDeviceName fails: no proxy");
        return INVALID_NAME;
    }
    return hostProxy->GetDeviceName(transport_, address_);
}

std::string BluetoothRemoteDevice::GetDeviceAlias() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_NAME;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetDeviceAlias fails: no proxy");
        return INVALID_NAME;
    }
    return hostProxy->GetDeviceAlias(address_);
}

bool BluetoothRemoteDevice::SetDeviceAlias(const std::string &aliasName)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::SetDeviceAlias fails: no proxy");
        return false;
    }
    return hostProxy->SetDeviceAlias(address_, aliasName);
}

int BluetoothRemoteDevice::GetDeviceBatteryLevel() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetDeviceBatteryLevel fails: no proxy");
        return INVALID_VALUE;
    }
    return hostProxy->GetDeviceBatteryLevel(address_);
}

int BluetoothRemoteDevice::GetPairState() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetPairState fails: no proxy");
        return INVALID_VALUE;
    }
    return hostProxy->GetPairState(transport_, address_);
}

bool BluetoothRemoteDevice::StartPair()
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::StartPair fails: no proxy");
        return false;
    }
    return hostProxy->StartPair(transport_, address_);
}

bool BluetoothRemoteDevice::CancelPairing()
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::CancelPairing fails: no proxy");
        return false;
    }
    return hostProxy->CancelPairing(transport_, address_);
}

bool BluetoothRemoteDevice::IsBondedFromLocal() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::IsBondedFromLocal fails: no proxy");
        return false;
    }
    return hostProxy->IsBondedFromLocal(transport_, address_);
}

bool BluetoothRemoteDevice::IsAclConnected() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::IsAclConnected fails: no proxy");
        return false;
    }
    return hostProxy->IsAclConnected(transport_, address_);
}

bool BluetoothRemoteDevice::IsAclEncrypted() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::IsAclEncrypted fails: no proxy");
        return false;
    }
    return hostProxy->IsAclEncrypted(transport_, address_);
}

BluetoothDeviceClass BluetoothRemoteDevice::GetDeviceClass() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return BluetoothDeviceClass(0);
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetDeviceClass fails: no proxy");
        return BluetoothDeviceClass(0);
    }
    int cod = hostProxy->GetDeviceClass(address_);
    return BluetoothDeviceClass(cod);
}

std::vector<ParcelUuid> BluetoothRemoteDevice::GetDeviceUuids() const
{
    std::vector<ParcelUuid> parcelUuids;
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return parcelUuids;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::GetDeviceUuids fails: no proxy");
        return parcelUuids;
    }
    std::vector<bluetooth::Uuid> uuids = hostProxy->GetDeviceUuids(transport_, address_);
    for (auto iter : uuids) {
        ParcelUuid parcelUuid = UUID::ConvertFrom128Bits(iter.ConvertTo128Bits());
        parcelUuids.push_back(parcelUuid);
    }
    return parcelUuids;
}

bool BluetoothRemoteDevice::SetDevicePin(const std::string &pin)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::SetDevicePin fails: no proxy");
        return false;
    }
    return hostProxy->SetDevicePin(address_, pin);
}

bool BluetoothRemoteDevice::SetDevicePairingConfirmation(bool accept)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::SetDevicePairingConfirmation fails: no proxy");
        return false;
    }
    return hostProxy->SetDevicePairingConfirmation(transport_, address_, accept);
}

bool BluetoothRemoteDevice::SetDevicePasskey(int passkey, bool accept)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::SetDevicePasskey fails: no proxy");
        return false;
    }
    return hostProxy->SetDevicePasskey(transport_, address_, passkey, accept);
}

bool BluetoothRemoteDevice::PairRequestReply(bool accept)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::PairRequestReply fails: no proxy");
        return false;
    }
    return hostProxy->PairRequestReply(transport_, address_, accept);
}

bool BluetoothRemoteDevice::ReadRemoteRssiValue()
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    sptr<BluetoothHostProxy> hostProxy = GetHostProxy();
    if (hostProxy == nullptr) {
        HILOGE("BluetoothRemoteDevice::ReadRemoteRssiValue fails: no proxy");
        return false;
    }
    return hostProxy->ReadRemoteRssiValue(address_);
}
}  // namespace Bluetooth
}  // namespace OHOS
