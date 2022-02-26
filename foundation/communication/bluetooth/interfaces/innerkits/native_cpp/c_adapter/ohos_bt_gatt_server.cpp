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

#include "ohos_bt_gatt_server.h"

#include <iostream>
#include <string.h>
#include <vector>
#include <map>

#include "bluetooth_gatt_server.h"
#include "bluetooth_log.h"

#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
static BtGattServerCallbacks *g_GattsCallback;

struct ConnectedDevice {
    int serverId;
    BdAddr remoteAddr;
};
static int g_connId = 0;

static std::map<int, struct ConnectedDevice> g_MapConnectedDevice;

#define MAXIMUM_NUMBER_APPLICATION 64
#define MAXIMUM_NUMBER_GATTSERVICE 64
struct GattServiceWapper {
    GattService *gattService;
    int index;
    int maxNum;
    int handleOffset;
    bool isAdding;
};

struct GattServerWapper {
    GattServer *gattServer;
    struct GattServiceWapper gattServices[MAXIMUM_NUMBER_GATTSERVICE];
};

GattServerWapper g_GattServers[MAXIMUM_NUMBER_APPLICATION];

#define GATTSERVER(x) g_GattServers[x].gattServer
#define GATTSERVICES(x, y) g_GattServers[x].gattServices[y]
#define GATTSERVICE(x, y) GATTSERVICES(x, y).gattService

void GetAddrFromString(std::string in, unsigned char out[6]);
void GetAddrFromByte(unsigned char in[6], std::string &out);
static GattCharacteristic *FindCharacteristic(int serverId, int attrHandle, bool isOffset, int *srvcHandle);

class GattServerCallbackWapper : public GattServerCallback {
public:
    GattServerCallbackWapper(BtGattServerCallbacks *callback, int serverId)
    {
        appCallback_ = callback;
        serverId_ = serverId;
    }

    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
    {
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);
        HILOGI("device:%{public}02X:%{public}02X:***:%{public}02X, connect state: %{public}d",
            dev.remoteAddr.addr[0], dev.remoteAddr.addr[1], dev.remoteAddr.addr[5], state);

        if (state == static_cast<int>(BTConnectState::CONNECTED)) {
            std::map<int, struct ConnectedDevice>::iterator iter;
            iter = FindDeviceRecord(dev);
            if (iter != g_MapConnectedDevice.end()) {
                HILOGW("device already in maps! connId: %{public}d", iter->first);
                g_GattsCallback->connectServerCb(iter->first, serverId_, &dev.remoteAddr);
            } else {
                g_MapConnectedDevice.insert(std::pair<int, struct ConnectedDevice>(g_connId, dev));
                HILOGI("device connected. connId: %{public}d", g_connId);
                g_GattsCallback->connectServerCb(g_connId, serverId_, &dev.remoteAddr);
                g_connId++;
            }
        }

        if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            std::map<int, struct ConnectedDevice>::iterator iter;
            iter = FindDeviceRecord(dev);
            if (iter != g_MapConnectedDevice.end()) {
                HILOGI("device disconnected. connId: %{public}d", iter->first);
                g_GattsCallback->disconnectServerCb(iter->first, serverId_, &dev.remoteAddr);
                g_MapConnectedDevice.erase(iter);
            }
        }
    }

    void OnServiceAdded(GattService *Service, int ret)
    {
        int i;
        int err = OHOS_BT_STATUS_SUCCESS;
        for (i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
            if (GATTSERVICE(serverId_, i) != NULL) {
                HILOGI("isAdding: %{public}d, srvcUuid: %{public}s, ind: %{public}s",
                    GATTSERVICES(serverId_, i).isAdding,
                    GATTSERVICE(serverId_, i)->GetUuid().ToString().c_str(),
                    Service->GetUuid().ToString().c_str());
            } else {
                HILOGE("services is empty!");
            }
            if (GATTSERVICE(serverId_, i) != NULL &&
                GATTSERVICES(serverId_, i).isAdding &&
                GATTSERVICE(serverId_, i)->GetUuid().CompareTo(Service->GetUuid()) == 0) {
                GATTSERVICES(serverId_, i).isAdding = false;
                GATTSERVICES(serverId_, i).handleOffset = Service->GetHandle() - i;
                HILOGI("serverId: %{public}d, service handle is: %{public}d, offset: %{public}d",
                    serverId_, GATTSERVICE(serverId_, i)->GetHandle(), GATTSERVICES(serverId_, i).handleOffset);
                break;
            }
        }

        if (i == MAXIMUM_NUMBER_GATTSERVICE) {
            HILOGE("add service failed, invalid srvcHandle: %d", Service->GetHandle());
            err = OHOS_BT_STATUS_FAIL;
        }

        vector<GattCharacteristic> &characteristics = Service->GetCharacteristics();
        for (auto item = characteristics.begin(); item != characteristics.end(); item++) {
            HILOGI("charHandle: %{public}d, uuid: %{public}s",
                item->GetHandle(), item->GetUuid().ToString().c_str());
            vector<GattDescriptor> &descriptors = item->GetDescriptors();
            for (auto des = descriptors.begin(); des != descriptors.end(); des++) {
                HILOGI("    desHandle: %{public}d, uuid: %{public}s",
                    des->GetHandle(), des->GetUuid().ToString().c_str());
            }
        }

        if (g_GattsCallback != NULL && g_GattsCallback->serviceAddCb != NULL) {
            g_GattsCallback->serviceStartCb(err, serverId_, i);
        }
    }

    void OnCharacteristicReadRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
    {
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::map<int, struct ConnectedDevice>::iterator iter;
        iter = FindDeviceRecord(dev);

        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic.GetHandle(), true, &srvcHandle);

        BtReqReadCbPara readInfo;
        readInfo.connId = iter->first;
        readInfo.transId = requestId;
        readInfo.bdAddr = &dev.remoteAddr;
        readInfo.attrHandle = characteristic.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        readInfo.offset = 0;
        readInfo.isLong = false;
        HILOGI("connId: %{public}d, requestId: %{public}d, attrHandle: %{public}d",
            iter->first, requestId, readInfo.attrHandle);
        if (g_GattsCallback != NULL && g_GattsCallback->requestReadCb != NULL) {
            g_GattsCallback->requestReadCb(readInfo);
        }
    }

    void OnCharacteristicWriteRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
    {
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::map<int, struct ConnectedDevice>::iterator iter;
        iter = FindDeviceRecord(dev);

        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic.GetHandle(), true, &srvcHandle);
        
        BtReqWriteCbPara writeInfo;
        size_t length = 0;
        writeInfo.connId = iter->first;
        writeInfo.transId = requestId;
        writeInfo.bdAddr = &dev.remoteAddr;
        writeInfo.attrHandle = characteristic.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        writeInfo.offset = 0;
        writeInfo.value = characteristic.GetValue(&length).get();
        writeInfo.length = length;
        writeInfo.needRsp = true;
        HILOGI("connId: %{public}d, requestId: %{public}d, attrHandle: %{public}d, valueLen: %{public}d",
            iter->first, requestId, writeInfo.attrHandle, writeInfo.length);
        if (g_GattsCallback != NULL && g_GattsCallback->requestWriteCb != NULL) {
            g_GattsCallback->requestWriteCb(writeInfo);
        }
    }

    void OnDescriptorReadRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId)
    {
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::map<int, struct ConnectedDevice>::iterator iter;
        iter = FindDeviceRecord(dev);

        const GattCharacteristic *characteristic = descriptor.GetCharacteristic();
        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic->GetHandle(), true, &srvcHandle);

        BtReqReadCbPara readInfo;
        readInfo.connId = iter->first;
        readInfo.transId = requestId;
        readInfo.bdAddr = &dev.remoteAddr;
        readInfo.attrHandle = descriptor.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        readInfo.offset = 0;
        readInfo.isLong = false;
        if (g_GattsCallback != NULL && g_GattsCallback->requestReadCb != NULL) {
            g_GattsCallback->requestReadCb(readInfo);
        }
    }

    void OnDescriptorWriteRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId)
    {
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::map<int, struct ConnectedDevice>::iterator iter;
        iter = FindDeviceRecord(dev);

        const GattCharacteristic *characteristic = descriptor.GetCharacteristic();
        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic->GetHandle(), true, &srvcHandle);
        
        BtReqWriteCbPara writeInfo;
        size_t length = 0;
        writeInfo.connId = iter->first;
        writeInfo.transId = requestId;
        writeInfo.bdAddr = &dev.remoteAddr;
        writeInfo.attrHandle = descriptor.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        writeInfo.offset = 0;
        writeInfo.value = descriptor.GetValue(&length).get();
        writeInfo.length = length;
        writeInfo.needRsp = true;
        if (g_GattsCallback != NULL && g_GattsCallback->requestWriteCb != NULL) {
            g_GattsCallback->requestWriteCb(writeInfo);
        }
    }

    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
    {
        HILOGI("mtu: %{public}d", mtu);
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::map<int, struct ConnectedDevice>::iterator iter;
        iter = FindDeviceRecord(dev);
        if (g_GattsCallback != NULL && g_GattsCallback->mtuChangeCb != NULL) {
            g_GattsCallback->mtuChangeCb(iter->first, mtu);
        }
    }

    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result)
    {
        HILOGI("result:%{public}d", result);
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::map<int, struct ConnectedDevice>::iterator iter;
        iter = FindDeviceRecord(dev);
        if (g_GattsCallback != NULL && g_GattsCallback->indicationSentCb != NULL) {
            g_GattsCallback->indicationSentCb(iter->first, result);
        }
    }

    void OnConnectionParameterChanged(
        const BluetoothRemoteDevice &device, int interval, int latency, int timeout, int status)
    {
        HILOGI("");
    }

private:
    std::map<int, struct ConnectedDevice>::iterator FindDeviceRecord(struct ConnectedDevice &device)
    {
        std::map<int, struct ConnectedDevice>::iterator iter;
        for (iter = g_MapConnectedDevice.begin(); iter != g_MapConnectedDevice.end(); iter++) {
            struct ConnectedDevice value = iter->second;
            if (value.serverId == device.serverId &&
                memcmp(value.remoteAddr.addr, device.remoteAddr.addr, sizeof(value.remoteAddr.addr)) == 0) {
                return iter;
            }
        }
        return iter;
    }

    BtGattServerCallbacks *appCallback_;
    int serverId_;
};

static GattCharacteristic *FindCharacteristic(int serverId, int attrHandle, bool isOffset, int *srvcHandle)
{
    if (srvcHandle == NULL) {
        return NULL;
    }
    for (int i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
        GattService *gattService = GATTSERVICE(serverId, i);
        if (gattService == NULL) {
            continue;
        }

        std::vector<GattCharacteristic> gattCharacteristics = gattService->GetCharacteristics();
        int tempHandle = attrHandle;
        if (isOffset) {
            tempHandle -= GATTSERVICES(serverId, i).handleOffset;
        }

        for (auto &character : gattCharacteristics) {
            if (character.GetHandle() == attrHandle) {
                *srvcHandle = i;
                return &character;
            }
        }
    }
    return NULL;
}

void GetAddrFromString(string in, unsigned char out[6]) {
    int j = 0;
    for (unsigned int i = 0; i < in.length(); i++) {
        if (in.at(i) != ':') {
            out[j] = strtoul(in.substr(i, 2).c_str(), 0, 16);
            i += 2;
            j++;
        }
    }
}

void GetAddrFromByte(unsigned char in[6], std::string &out)
{
    char temp[18] = {0};
    sprintf_s(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
        in[0], in[1], in[2], in[3], in[4], in[5]);
    out = string(temp);
}

/**
 * @brief Registers a GATT server with a specified application UUID.
 *
 * The <b>RegisterServerCallback</b> is invoked to return the GATT server ID.
 *
 * @param appUuid Indicates the UUID of the application for which the GATT server is to be registered.
 * The UUID is defined by the application.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsRegister(BtUuid appUuid) 
{
    HILOGI();
    if (g_GattsCallback == NULL) {
        HILOGE("callback is null, call BleGattsRegisterCallbacks first");
        return OHOS_BT_STATUS_FAIL;
    }
    for (int i = 0; i < MAXIMUM_NUMBER_APPLICATION; i++) {
        if (GATTSERVER(i) == NULL) {
            GattServerCallbackWapper *callbackWapper = new GattServerCallbackWapper(g_GattsCallback, i);
            GATTSERVER(i) = new GattServer(*callbackWapper);
            HILOGI("register gattServer: %{public}d", i);
            if (g_GattsCallback->registerServerCb != NULL) {
                g_GattsCallback->registerServerCb(0, i, &appUuid);
            }
            return OHOS_BT_STATUS_SUCCESS;
        }
    }

    if (g_GattsCallback->registerServerCb != NULL) {
        g_GattsCallback->registerServerCb(1, 0, &appUuid);
    }
    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief Unregisters a GATT server with a specified ID.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is unregistered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsUnRegister(int serverId)
{
    if (serverId >= 0 && serverId < MAXIMUM_NUMBER_APPLICATION) {
        if (GATTSERVER(serverId) != NULL) {
            delete GATTSERVER(serverId);
            GATTSERVER(serverId) = NULL;
            //TODO delete services;
            return OHOS_BT_STATUS_SUCCESS;
        }
    }

    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief Disconnects the GATT server from the client.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param bdAddr Indicates the address of the client.
 * @param connId Indicates the connection ID, which is returned during the server registration.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is disconnected from the client;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsDisconnect(int serverId, BdAddr bdAddr, int connId)
{
    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (GATTSERVER(serverId) == NULL) {
        return OHOS_BT_STATUS_UNHANDLED;
    }

    string strAddress;
    GetAddrFromByte(bdAddr.addr, strAddress);

    //TODO transport_的值如何取
    BluetoothRemoteDevice device(strAddress, 0);

    GATTSERVER(serverId)->CancelConnection(device);
    HILOGI("serverId: %{public}d, connId: %{public}d", serverId, connId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Adds a service.
 *
 * This function adds the service, its characteristics, and descriptors separately in sequence.\n
 * A service is a collection of data and related behavior that enable a specific capability or feature.\n
 * It consists of a service declaration and one or more included services and characteristics.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcUuid Indicates the UUID of the service.
 * @param isPrimary Specifies whether the service is primary or secondary.
 * Value <b>true</b> indicates that the service is primary, and <b>false</b> indicates that the service is secondary.
 * @param number Indicates the number of attribute handles.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is added;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddService(int serverId, BtUuid srvcUuid, bool isPrimary, int number)
{
    string strUuid(srvcUuid.uuid);
    UUID uuid(UUID::FromString(strUuid));

    for (int i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
        if (GATTSERVICE(serverId, i) == NULL) {
            HILOGI("add srvcHandle: %{public}d", i);
            GATTSERVICE(serverId, i) = new GattService(
                uuid, i, number, isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY);
            GATTSERVICES(serverId, i).maxNum = number;
            GATTSERVICES(serverId, i).index = i + 1;
            GATTSERVICES(serverId, i).isAdding = false;
            if (g_GattsCallback != NULL && g_GattsCallback->serviceAddCb != NULL) {
                g_GattsCallback->serviceAddCb(0, serverId, &srvcUuid, i);
            }
            return OHOS_BT_STATUS_SUCCESS;
        }
    }
    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief Adds an included service to a specified service.
 *
 * An included service is referenced to define another service on the GATT server.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @param includedHandle Indicates the attribute handle ID of the included service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the included service is added to the service;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddIncludedService(int serverId, int srvcHandle, int includedHandle)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Adds a characteristic to a specified service.
 *
 * A characteristic consists of data, the data access method, data format, and how the data manifests itself.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @param characUuid Indicates the UUID of the characteristic to add.
 * @param properties Indicates the access methods supported by the characteristic,
 * as enumerated in {@link GattCharacteristicProperty}.
 * @param permissions Indicates the access permissions supported by the characteristic,
 * as enumerated in {@link GattAttributePermission}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the characteristic is added to the service;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
static int ConvertPermissions(int permission)
{
    int newPermission = 0;

    if ((permission & OHOS_GATT_PERMISSION_READ) == OHOS_GATT_PERMISSION_READ) {
        newPermission = static_cast<int>(GattPermission::READABLE);
    }

    if ((permission & OHOS_GATT_PERMISSION_WRITE) == OHOS_GATT_PERMISSION_WRITE) {
        newPermission += static_cast<int>(GattPermission::WRITABLE);
    }

    return newPermission;
}

int BleGattsAddCharacteristic(int serverId, int srvcHandle, BtUuid characUuid,
                              int properties, int permissions)
{
    HILOGI("properties: %{public}d, permissions:%{public}d", properties, permissions);
    string strUuid(characUuid.uuid);
    UUID uuid(UUID::FromString(strUuid));

    int chHandle = GATTSERVICES(serverId, srvcHandle).index;
    GATTSERVICES(serverId, srvcHandle).index += 2;
    GattCharacteristic characteristic(uuid, chHandle, ConvertPermissions(permissions), properties);

    unsigned char stubValue[1] = {0x31};
    characteristic.SetValue(stubValue, sizeof(stubValue));
    GATTSERVICE(serverId, srvcHandle)->AddCharacteristic(characteristic);
    
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d, charHandle: %{public}d", serverId, srvcHandle, chHandle);
    if (g_GattsCallback != NULL && g_GattsCallback->characteristicAddCb != NULL) {
        g_GattsCallback->characteristicAddCb(0, serverId, &characUuid, srvcHandle, chHandle);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Adds a descriptor to a specified characteristic.
 *
 * A descriptor contains the description, configuration, and format of a characteristic.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service to which the characteristic belongs.
 * @param descUuid Indicates the UUID of the descriptor to add.
 * @param permissions Indicates the access permissions supported by the descriptor,
 * as enumerated in {@link GattAttributePermission}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the descriptor is added to the characteristic;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddDescriptor(int serverId, int srvcHandle, BtUuid descUuid, int permissions)
{
    string strUuid(descUuid.uuid);
    HILOGI("descUuid: %{public}s", strUuid.c_str());
    UUID uuid(UUID::FromString(strUuid));
    GattCharacteristic &characteristic = GATTSERVICE(serverId, srvcHandle)->GetCharacteristics().back();
    int desHandle = GATTSERVICES(serverId, srvcHandle).index++;
    GattDescriptor descriptor(uuid, desHandle, ConvertPermissions(permissions));

    unsigned char stubValue[2] = {0x01, 0x00};
    descriptor.SetValue(stubValue, sizeof(stubValue));

    characteristic.AddDescriptor(descriptor);
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d, desHandle: %{public}d", serverId, srvcHandle, desHandle);
    if (g_GattsCallback != NULL && g_GattsCallback->descriptorAddCb != NULL) {
        g_GattsCallback->descriptorAddCb(0, serverId, &descUuid, srvcHandle, desHandle);
    }

    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a service.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStartService(int serverId, int srvcHandle)
{
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d, uuid: %{public}s",
        serverId, srvcHandle, GATTSERVICE(serverId, srvcHandle)->GetUuid().ToString().c_str());
    GATTSERVICES(serverId, srvcHandle).isAdding = true;
    GATTSERVER(serverId)->AddService(*GATTSERVICE(serverId, srvcHandle));
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Stops a service.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStopService(int serverId, int srvcHandle)
{
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d", serverId, srvcHandle);
    GATTSERVICES(serverId, srvcHandle).isAdding = false;
    GATTSERVER(serverId)->RemoveGattService(*GATTSERVICE(serverId, srvcHandle));
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Deletes a service.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is deleted;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsDeleteService(int serverId, int srvcHandle)
{
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d", serverId, srvcHandle);
    GATTSERVER(serverId)->RemoveGattService(*GATTSERVICE(serverId, srvcHandle));
    delete GATTSERVICE(serverId, srvcHandle);
    GATTSERVICE(serverId, srvcHandle) = NULL;
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Clears all services.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the services are cleared;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsClearServices(int serverId) {
    HILOGI("serverId: %{public}d", serverId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sends a response to the client from which a read or write request has been received.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param param Indicates the pointer to the response parameters. For details, see {@link GattsSendRspParam}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the response is sent;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsSendResponse(int serverId, GattsSendRspParam *param)
{
    std::map<int, struct ConnectedDevice>::iterator iter;
    iter = g_MapConnectedDevice.find(param->connectId);

    struct ConnectedDevice value = iter->second;

    string strAddress;
    GetAddrFromByte(value.remoteAddr.addr, strAddress);
    
    BluetoothRemoteDevice device(strAddress, 1);
    // request id
    HILOGI("attrHandle: %{public}d", param->attrHandle);

    int ret = GATTSERVER(serverId)->SendResponse(device, param->attrHandle,
        param->status, 0, (unsigned char *)param->value, param->valueLen);
    
    if (g_GattsCallback != NULL && g_GattsCallback->responseConfirmationCb != NULL) {
        g_GattsCallback->responseConfirmationCb(ret, param->attrHandle);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sends an indication or notification to the client.
 *
 * The <b>confirm</b> field in <b>param</b> determines whether to send an indication or a notification.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param param Indicates the pointer to the sending parameters. For details, see {@link GattsSendIndParam}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the indication or notification is sent;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsSendIndication(int serverId, GattsSendIndParam *param)
{
    std::map<int, struct ConnectedDevice>::iterator iter;
    iter = g_MapConnectedDevice.find(param->connectId);

    struct ConnectedDevice value = iter->second;

    HILOGI("device:%{public}02X:%{public}02X:%{public}02X:%{public}02X:%{public}02X:%{public}02X",
            value.remoteAddr.addr[0], value.remoteAddr.addr[1], value.remoteAddr.addr[2],
            value.remoteAddr.addr[3], value.remoteAddr.addr[4], value.remoteAddr.addr[5]);

    string strAddress;
    GetAddrFromByte(value.remoteAddr.addr, strAddress);
    
    BluetoothRemoteDevice device(strAddress, 1);

    int srvcHandle = 0;
    GattCharacteristic *appCharacteristic = FindCharacteristic(serverId, param->attrHandle, false, &srvcHandle);
    if (appCharacteristic == NULL) {
        HILOGE("not find characteristic, serverId:%{public}d, attrHandle:%{public}d",
            serverId, param->attrHandle);
        return OHOS_BT_STATUS_FAIL;
    }

    HILOGE("serverId:%{public}d, srvcHandle: %{public}d, attrHandle:%{public}d",
        serverId, param->attrHandle, srvcHandle);
    GattCharacteristic characteristic(appCharacteristic->GetUuid(),
        appCharacteristic->GetHandle() + GATTSERVICES(serverId, srvcHandle).handleOffset,
        appCharacteristic->GetPermissions(),
        appCharacteristic->GetProperties());

    characteristic.SetValue((unsigned char*)param->value, param->valueLen);

    GATTSERVER(serverId)->NotifyCharacteristicChanged(device, characteristic,
        (param->confirm == 1) ? true : false);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets the encryption type for the GATT connection.
 *
 * @param bdAddr Indicates the address of the client.
 * @param secAct Indicates the encryption type, as enumerated in {@link BleSecAct}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the encryption type is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsSetEncryption(BdAddr bdAddr, BleSecAct secAct) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Registers GATT server callbacks.
 *
 * @param func Indicates the pointer to the callbacks to register, as enumerated in {@link BtGattServerCallbacks}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the callbacks are registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsRegisterCallbacks(BtGattServerCallbacks *func) 
{
    if (func == NULL) {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    g_GattsCallback = func;
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Adds a service, its characteristics, and descriptors and starts the service.
 *
 * This function is available for system applications only.
 *
 * @param srvcHandle Indicates the pointer to the handle ID of the service,
 * which is returned by whoever implements this function.
 * @param srvcInfo Indicates the pointer to the service information.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStartServiceEx(int *srvcHandle, BleGattService *srvcInfo) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Stops a service.
 *
 * This function is available for system applications only.
 *
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStopServiceEx(int srvcHandle) {
    return OHOS_BT_STATUS_UNSUPPORTED;
}
}  // namespace Bluetooth
}  // namespace OHOS

#ifdef __cplusplus
}
#endif
/** @} */
