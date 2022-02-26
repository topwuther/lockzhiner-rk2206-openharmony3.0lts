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

#define LOG_TAG "KvStoreDataServiceProxy"

#include "ikvstore_data_service.h"
#include "constant.h"
#include "message_parcel.h"
#include "types.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
constexpr KvStoreDataServiceStub::RequestHandler KvStoreDataServiceStub::HANDLERS[SERVICE_CMD_LAST];
KvStoreDataServiceProxy::KvStoreDataServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IKvStoreDataService>(impl)
{
    ZLOGI("init data service proxy.");
}

Status KvStoreDataServiceProxy::GetKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                                           std::function<void(sptr<IKvStoreImpl>)> callback)
{
    ZLOGI("%s %s", appId.appId.c_str(), storeId.storeId.c_str());
    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }

    // Passing a struct with an std::string field is a potential security exploit.
    OptionsIpc optionsIpc;
    optionsIpc.createIfMissing = options.createIfMissing;
    optionsIpc.encrypt = options.encrypt;
    optionsIpc.persistant = options.persistant;
    optionsIpc.backup = options.backup;
    optionsIpc.autoSync = options.autoSync;
    optionsIpc.securityLevel = options.securityLevel;
    optionsIpc.syncPolicy = options.syncPolicy;
    optionsIpc.kvStoreType = options.kvStoreType;
    optionsIpc.syncable = options.syncable;
    optionsIpc.dataOwnership = true; // set default value

    if (!data.WriteBuffer(&optionsIpc, sizeof(optionsIpc)) ||
        !data.WriteString(appId.appId) ||
        !data.WriteString(storeId.storeId)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    Status ret = static_cast<Status>(reply.ReadInt32());
    if (ret == Status::SUCCESS) {
        sptr<IRemoteObject> remote = reply.ReadRemoteObject();
        if (remote != nullptr) {
            sptr<IKvStoreImpl> kvstoreImplProxy = iface_cast<IKvStoreImpl>(remote);
            callback(std::move(kvstoreImplProxy));
        }
    } else {
        callback(nullptr);
    }
    return ret;
}

Status KvStoreDataServiceProxy::GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                                                 std::function<void(sptr<ISingleKvStore>)> callback)
{
    ZLOGI("%s %s", appId.appId.c_str(), storeId.storeId.c_str());
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("SetMaxCapacity failed.");
        return Status::IPC_ERROR;
    }
    // Passing a struct with an std::string field is a potential security exploit.
    OptionsIpc optionsIpc;
    optionsIpc.createIfMissing = options.createIfMissing;
    optionsIpc.encrypt = options.encrypt;
    optionsIpc.persistant = options.persistant;
    optionsIpc.backup = options.backup;
    optionsIpc.autoSync = options.autoSync;
    optionsIpc.securityLevel = options.securityLevel;
    optionsIpc.syncPolicy = options.syncPolicy;
    optionsIpc.kvStoreType = options.kvStoreType;
    optionsIpc.syncable = options.syncable;
    optionsIpc.dataOwnership = true; // set default value
    std::string schemaString = options.schema;

    if (!data.WriteBuffer(&optionsIpc, sizeof(OptionsIpc)) ||
        !data.WriteString(appId.appId) ||
        !data.WriteString(storeId.storeId) ||
        !data.WriteString(schemaString)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETSINGLEKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        return Status::IPC_ERROR;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status == Status::SUCCESS) {
        sptr<IRemoteObject> remote = reply.ReadRemoteObject();
        if (remote != nullptr) {
            sptr<ISingleKvStore> kvstoreImplProxy = iface_cast<ISingleKvStore>(remote);
            callback(std::move(kvstoreImplProxy));
        }
    } else {
        callback(nullptr);
    }
    return status;
}

void KvStoreDataServiceProxy::GetAllKvStoreId(const AppId &appId,
                                              std::function<void(Status, std::vector<StoreId> &)> callback)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    if (!data.WriteString(appId.appId)) {
        ZLOGW("failed to write parcel.");
        return;
    }
    std::vector<StoreId> storeIds;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETALLKVSTOREID, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        callback(Status::IPC_ERROR, storeIds);
        return;
    }
    std::vector<std::string> stores;
    reply.ReadStringVector(&stores);
    for (const auto &id: stores) {
        storeIds.push_back({id});
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    callback(status, storeIds);
}

Status KvStoreDataServiceProxy::CloseKvStore(const AppId &appId, const StoreId &storeId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(appId.appId) ||
        !data.WriteString(storeId.storeId)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(CLOSEKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

/* close all opened kvstore */
Status KvStoreDataServiceProxy::CloseAllKvStore(const AppId &appId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(appId.appId)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(CLOSEALLKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreDataServiceProxy::DeleteKvStore(const AppId &appId, const StoreId &storeId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(appId.appId) ||
        !data.WriteString(storeId.storeId)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(DELETEKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

/* delete all kv store */
Status KvStoreDataServiceProxy::DeleteAllKvStore(const AppId &appId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(appId.appId)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(DELETEALLKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreDataServiceProxy::RegisterClientDeathObserver(const AppId &appId, sptr<IRemoteObject> observer)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(appId.appId)) {
        ZLOGW("failed to write string.");
        return Status::IPC_ERROR;
    }
    if (observer != nullptr) {
        if (!data.WriteRemoteObject(observer)) {
            ZLOGW("failed to write parcel.");
            return Status::IPC_ERROR;
        }
    } else {
        return Status::INVALID_ARGUMENT;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(REGISTERCLIENTDEATHOBSERVER, data, reply, mo);
    if (error != 0) {
        ZLOGW("failed during IPC. errCode %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreDataServiceProxy::GetLocalDevice(OHOS::DistributedKv::DeviceInfo &device)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETLOCALDEVICE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status == Status::SUCCESS) {
        device = {reply.ReadString(), reply.ReadString(), reply.ReadString()};
    }
    return status;
}

Status KvStoreDataServiceProxy::GetDeviceList(std::vector<DeviceInfo> &deviceInfoList, DeviceFilterStrategy strategy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(static_cast<int>(strategy))) {
        ZLOGW("write int failed.");
        return Status::IPC_ERROR;
    }
    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETDEVICELIST, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status == Status::SUCCESS) {
        int len = reply.ReadInt32();
        for (int i = 0; i < len; i++) {
            DeviceInfo deviceInfo = {
                .deviceId = reply.ReadString(),
                .deviceName = reply.ReadString(),
                .deviceType = reply.ReadString()
            };
            deviceInfoList.push_back(std::move(deviceInfo));
        }
    }
    return status;
}

Status KvStoreDataServiceProxy::StartWatchDeviceChange(sptr<IDeviceStatusChangeListener> observer,
                                                       DeviceFilterStrategy strategy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(static_cast<int>(strategy))) {
        ZLOGW("write int failed.");
        return Status::IPC_ERROR;
    }
    if (observer != nullptr) {
        if (!data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
            return Status::IPC_ERROR;
        }
    } else {
        return Status::INVALID_ARGUMENT;
    }
    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(STARTWATCHDEVICECHANGE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreDataServiceProxy::StopWatchDeviceChange(sptr<IDeviceStatusChangeListener> observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(KvStoreDataServiceProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (observer != nullptr) {
        if (!data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
            return Status::IPC_ERROR;
        }
    } else {
        return Status::INVALID_ARGUMENT;
    }
    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(STOPWATCHDEVICECHANGE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

int32_t KvStoreDataServiceStub::GetKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    OptionsIpc optionsIpc;
    AppId appId;
    StoreId storeId;

    const OptionsIpc *optionIpcPtr = reinterpret_cast<const OptionsIpc *>(data.ReadBuffer(sizeof(OptionsIpc)));
    if (optionIpcPtr == nullptr) {
        ZLOGW("optionPtr is nullptr");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    optionsIpc = *optionIpcPtr;
    Options options;
    options.createIfMissing = optionsIpc.createIfMissing;
    options.encrypt = optionsIpc.encrypt;
    options.persistant = optionsIpc.persistant;
    options.backup = optionsIpc.backup;
    options.autoSync = optionsIpc.autoSync;
    options.securityLevel = optionsIpc.securityLevel;
    options.syncPolicy = optionsIpc.syncPolicy;
    options.kvStoreType = optionsIpc.kvStoreType;
    options.syncable = optionsIpc.syncable;
    options.dataOwnership = optionsIpc.dataOwnership;
    appId.appId = data.ReadString();
    storeId.storeId = data.ReadString();
    sptr<IKvStoreImpl> proxyTmp;
    Status status = GetKvStore(options, appId, storeId,
        [&proxyTmp](sptr<IKvStoreImpl> proxy) { proxyTmp = std::move(proxy); });
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    if (proxyTmp == nullptr) {
        ZLOGW("proxy is null.");
        return 0;
    }
    if (status == Status::SUCCESS && !reply.WriteRemoteObject(proxyTmp->AsObject().GetRefPtr())) {
        ZLOGW("write ipc failed.");
        return -1;
    }
    return 0;
}
int32_t KvStoreDataServiceStub::GetAllKvStoreIdOnRemote(MessageParcel &data, MessageParcel &reply)
{
    AppId appId;
    appId.appId = data.ReadString();
    std::vector<std::string> storeIdList;
    Status statusTmp;
    GetAllKvStoreId(appId, [&](Status status, std::vector<StoreId> &storeIds) {
        for (const auto &id : storeIds) {
            storeIdList.push_back(id.storeId);
        }
        statusTmp = status;
    });

    if (!reply.WriteStringVector(storeIdList)) {
        return -1;
    }

    if (!reply.WriteInt32(static_cast<int>(statusTmp))) {
        return -1;
    }
    return 0;
}
int32_t KvStoreDataServiceStub::GetDeviceListOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::vector<DeviceInfo> infos;
    DeviceFilterStrategy strategy = static_cast<DeviceFilterStrategy>(data.ReadInt32());
    Status status = GetDeviceList(infos, strategy);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    if (status == Status::SUCCESS) {
        if (!reply.WriteInt32(infos.size())) {
            return -1;
        }
        for (DeviceInfo const &info : infos) {
            if (!reply.WriteString(info.deviceId) || !reply.WriteString(info.deviceName) ||
                !reply.WriteString(info.deviceType)) {
                return -1;
            }
        }
    }
    return 0;
}
int32_t KvStoreDataServiceStub::StartWatchDeviceChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    DeviceFilterStrategy strategy = static_cast<DeviceFilterStrategy>(data.ReadInt32());
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        ZLOGW("observerProxy nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IDeviceStatusChangeListener> observerProxy = iface_cast<IDeviceStatusChangeListener>(remote);
    Status status = StartWatchDeviceChange(std::move(observerProxy), strategy);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}
int32_t KvStoreDataServiceStub::StopWatchDeviceChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        ZLOGW("observerProxy nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IDeviceStatusChangeListener> observerProxy = iface_cast<IDeviceStatusChangeListener>(remote);
    Status status = StopWatchDeviceChange(std::move(observerProxy));
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}
int32_t KvStoreDataServiceStub::GetSingleKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    OptionsIpc optionsIpc;
    AppId appId;
    StoreId storeId;
    const OptionsIpc *optionIpcPtr = reinterpret_cast<const OptionsIpc *>(data.ReadBuffer(sizeof(OptionsIpc)));
    if (optionIpcPtr == nullptr) {
        ZLOGW("optionPtr is nullptr");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    optionsIpc = *optionIpcPtr;
    appId.appId = data.ReadString();
    storeId.storeId = data.ReadString();
    Options options;
    options.createIfMissing = optionsIpc.createIfMissing;
    options.encrypt = optionsIpc.encrypt;
    options.persistant = optionsIpc.persistant;
    options.backup = optionsIpc.backup;
    options.autoSync = optionsIpc.autoSync;
    options.securityLevel = optionsIpc.securityLevel;
    options.syncPolicy = optionsIpc.syncPolicy;
    options.kvStoreType = optionsIpc.kvStoreType;
    options.syncable = optionsIpc.syncable;
    options.dataOwnership = optionsIpc.dataOwnership;
    options.schema = data.ReadString();
    sptr<ISingleKvStore> proxyTmp;
    Status status = GetSingleKvStore(options, appId, storeId,
                                     [&](sptr<ISingleKvStore> proxy) { proxyTmp = std::move(proxy); });
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    if (status == Status::SUCCESS && proxyTmp != nullptr) {
        if (!reply.WriteRemoteObject(proxyTmp->AsObject().GetRefPtr())) {
            return -1;
        }
    }
    return 0;
}

int32_t KvStoreDataServiceStub::CloseKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    AppId appId;
    StoreId storeId;
    appId.appId = data.ReadString();
    storeId.storeId = data.ReadString();
    Status status = CloseKvStore(appId, storeId);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}

int32_t KvStoreDataServiceStub::CloseAllKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    AppId appId;
    appId.appId = data.ReadString();
    Status status = CloseAllKvStore(appId);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}

int32_t KvStoreDataServiceStub::DeleteKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    AppId appId;
    StoreId storeId;
    appId.appId = data.ReadString();
    storeId.storeId = data.ReadString();
    Status status = DeleteKvStore(appId, storeId);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}

int32_t KvStoreDataServiceStub::DeleteAllKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    AppId appId;
    appId.appId = data.ReadString();
    Status status = DeleteAllKvStore(appId);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}

int32_t KvStoreDataServiceStub::RegisterClientDeathObserverOnRemote(MessageParcel &data, MessageParcel &reply)
{
    AppId appId;
    appId.appId = data.ReadString();
    sptr<IRemoteObject> kvStoreClientDeathObserverProxy = data.ReadRemoteObject();
    if (kvStoreClientDeathObserverProxy == nullptr) {
        return -1;
    }
    Status status = RegisterClientDeathObserver(appId, std::move(kvStoreClientDeathObserverProxy));
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}

int32_t KvStoreDataServiceStub::GetLocalDeviceOnRemote(MessageParcel &data, MessageParcel &reply)
{
    DeviceInfo info;
    Status status = GetLocalDevice(info);
    if (!reply.WriteInt32(static_cast<int>(status)) || !reply.WriteString(info.deviceId) ||
        !reply.WriteString(info.deviceName) || !reply.WriteString(info.deviceType)) {
        return -1;
    }
    return 0;
}

int32_t KvStoreDataServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                                MessageParcel &reply, MessageOption &option)
{
    ZLOGD("%d", code);
    std::u16string descriptor = KvStoreDataServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    if (code >= 0 && code < SERVICE_CMD_LAST) {
        return (this->*HANDLERS[code])(data, reply);
    } else {
        MessageOption mo { MessageOption::TF_SYNC };
        return IPCObjectStub::OnRemoteRequest(code, data, reply, mo);
    }
}
}  // namespace DistributedKv
}  // namespace OHOS
