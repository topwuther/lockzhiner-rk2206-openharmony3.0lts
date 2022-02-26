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

#ifndef I_KV_STORE_DATA_SERVICE_H
#define I_KV_STORE_DATA_SERVICE_H

#include "iremote_broker.h"
#include "ikvstore.h"
#include "ikvstore_client_death_observer.h"
#include "ikvstore_observer.h"
#include "ikvstore_single.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "types.h"
#include "idevice_status_change_listener.h"

namespace OHOS {
namespace DistributedKv {
/*
 * IPC-friendly Options struct without std::string schema field.
 * Passing a struct with an std::string field is a potential security exploit.
 *
 */
struct OptionsIpc {
    bool createIfMissing;
    bool encrypt;
    bool persistant;
    bool backup;
    bool autoSync;
    int securityLevel;
    SyncPolicy syncPolicy;
    KvStoreType kvStoreType;
    bool syncable; // let bms delete first
    bool dataOwnership; // true indicates the ownership of distributed data is DEVICE, otherwise, ACCOUNT
};

class IKvStoreDataService : public IRemoteBroker {
public:
    enum {
        GETKVSTORE,
        GETALLKVSTOREID,
        CLOSEKVSTORE,
        CLOSEALLKVSTORE,
        DELETEKVSTORE,
        DELETEALLKVSTORE,
        REGISTERCLIENTDEATHOBSERVER,
        GETSINGLEKVSTORE,
        GETLOCALDEVICE,
        GETDEVICELIST,
        STARTWATCHDEVICECHANGE,
        STOPWATCHDEVICECHANGE,
        SERVICE_CMD_LAST,
        DATAUSAGESTART = 20,
        DATAUSAGEEND = 40,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreDataService");
    /* create and open kv store instance. */
    virtual Status GetKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                              std::function<void(sptr<IKvStoreImpl>)> callback) = 0;

    virtual Status GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                              std::function<void(sptr<ISingleKvStore>)> callback) = 0;

    /* get all kv store names */
    virtual void GetAllKvStoreId(const AppId &appId, std::function<void(Status, std::vector<StoreId> &)> callback) = 0;

    /* open kv store instance will not receive subscribe any more. */
    virtual Status CloseKvStore(const AppId &appId, const StoreId &id) = 0;

    /* close all kvstore. */
    virtual Status CloseAllKvStore(const AppId &appId) = 0;

    /* delete kv store */
    virtual Status DeleteKvStore(const AppId &appId, const StoreId &id) = 0;

    /* delete kv store */
    virtual Status DeleteAllKvStore(const AppId &appId) = 0;

    virtual Status RegisterClientDeathObserver(const AppId &appId, sptr<IRemoteObject> observer) = 0;

    virtual Status GetLocalDevice(DeviceInfo &device) = 0;
    virtual Status GetDeviceList(std::vector<DeviceInfo> &deviceInfoList, DeviceFilterStrategy strategy) = 0;
    virtual Status StartWatchDeviceChange(sptr<IDeviceStatusChangeListener> observer,
            DeviceFilterStrategy strategy) = 0;
    virtual Status StopWatchDeviceChange(sptr<IDeviceStatusChangeListener> observer) = 0;
};

class KvStoreDataServiceStub : public IRemoteStub<IKvStoreDataService> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
private:
    int32_t GetKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetAllKvStoreIdOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t CloseKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t CloseAllKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t DeleteKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t DeleteAllKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterClientDeathObserverOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetLocalDeviceOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceListOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t StartWatchDeviceChangeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t StopWatchDeviceChangeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetSingleKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);

    using RequestHandler = int32_t(KvStoreDataServiceStub::*)(MessageParcel&, MessageParcel&);
    static constexpr RequestHandler HANDLERS[SERVICE_CMD_LAST] = {
        [GETKVSTORE] = &KvStoreDataServiceStub::GetKvStoreOnRemote,
        [GETALLKVSTOREID] = &KvStoreDataServiceStub::GetAllKvStoreIdOnRemote,
        [CLOSEKVSTORE] = &KvStoreDataServiceStub::CloseKvStoreOnRemote,
        [CLOSEALLKVSTORE] = &KvStoreDataServiceStub::CloseAllKvStoreOnRemote,
        [DELETEKVSTORE] = &KvStoreDataServiceStub::DeleteKvStoreOnRemote,
        [DELETEALLKVSTORE] = &KvStoreDataServiceStub::DeleteAllKvStoreOnRemote,
        [REGISTERCLIENTDEATHOBSERVER] = &KvStoreDataServiceStub::RegisterClientDeathObserverOnRemote,
        [GETSINGLEKVSTORE] = &KvStoreDataServiceStub::GetSingleKvStoreOnRemote,
        [GETLOCALDEVICE] = &KvStoreDataServiceStub::GetLocalDeviceOnRemote,
        [GETDEVICELIST] = &KvStoreDataServiceStub::GetDeviceListOnRemote,
        [STARTWATCHDEVICECHANGE] = &KvStoreDataServiceStub::StartWatchDeviceChangeOnRemote,
        [STOPWATCHDEVICECHANGE] = &KvStoreDataServiceStub::StopWatchDeviceChangeOnRemote,
    };
};

class KvStoreDataServiceProxy : public IRemoteProxy<IKvStoreDataService> {
public:
    explicit KvStoreDataServiceProxy(const sptr<IRemoteObject> &impl);
    ~KvStoreDataServiceProxy() = default;

    /* create and open kv store instance. */
    virtual Status GetKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                              std::function<void(sptr<IKvStoreImpl>)> callback);

    virtual Status GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                              std::function<void(sptr<ISingleKvStore>)> callback);

    /* get all kv store names */
    virtual void GetAllKvStoreId(const AppId &appId, std::function<void(Status, std::vector<StoreId> &)> callback);

    /* open kv store instance will not receive subscribe any more. */
    virtual Status CloseKvStore(const AppId &appId, const StoreId &storeId);

    /* close all kvstore. */
    virtual Status CloseAllKvStore(const AppId &appId);

    /* delete kv store */
    virtual Status DeleteKvStore(const AppId &appId, const StoreId &id);

    /* delete kv store */
    virtual Status DeleteAllKvStore(const AppId &appId);

    virtual Status RegisterClientDeathObserver(const AppId &appId, sptr<IRemoteObject> observer);

    virtual Status GetLocalDevice(DeviceInfo &device);
    virtual Status GetDeviceList(std::vector<DeviceInfo> &deviceInfoList, DeviceFilterStrategy strategy);
    virtual Status StartWatchDeviceChange(sptr<IDeviceStatusChangeListener> observer, DeviceFilterStrategy strategy);
    virtual Status StopWatchDeviceChange(sptr<IDeviceStatusChangeListener> observer);
private:
    static inline BrokerDelegator<KvStoreDataServiceProxy> delegator_;
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_KV_STORE_DATA_SERVICE_H
