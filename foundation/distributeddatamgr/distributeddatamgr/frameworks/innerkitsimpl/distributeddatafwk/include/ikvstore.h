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

#ifndef I_KVSTORE_H
#define I_KVSTORE_H

#include "message_parcel.h"
#include "iremote_broker.h"
#include "ikvstore_observer.h"
#include "ikvstore_snapshot.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

class IKvStoreImpl : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreImpl")

    virtual void GetKvStoreSnapshot(sptr<IKvStoreObserver> observer,
                                    std::function<void(Status, sptr<IKvStoreSnapshotImpl>)> callback) = 0;

    virtual Status ReleaseKvStoreSnapshot(sptr<IKvStoreSnapshotImpl> iKvStoreSnapshot) = 0;

    virtual Status Put(const Key &key, const Value &value) = 0;

    virtual Status PutBatch(const std::vector<Entry> &entries) = 0;

    virtual Status Delete(const Key &key) = 0;

    virtual Status DeleteBatch(const std::vector<Key> &keys) = 0;

    virtual Status Clear() = 0;

    virtual Status StartTransaction() = 0;

    virtual Status Commit() = 0;

    virtual Status Rollback() = 0;

    virtual Status SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer) = 0;

    virtual Status UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer) = 0;
};

class KvStoreImplStub : public IRemoteStub<IKvStoreImpl> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;

private:
    int32_t GetKvStoreSnapshotOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t ReleaseKvStoreSnapshotOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t PutOnRemoteRequest(MessageParcel &data, MessageParcel &reply);
    int32_t PutBatchOnRemoteRequest(MessageParcel &data, MessageParcel &reply);
    int32_t DeleteOnRemoteRequest(MessageParcel &data, MessageParcel &reply);
    int32_t DeleteBatchOnRemoteRequest(MessageParcel &data, MessageParcel &reply);
    int32_t SubscribeKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t UnSubscribeKvStoreOnRemote(MessageParcel &data, MessageParcel &reply);
};

class KvStoreImplProxy : public IRemoteProxy<IKvStoreImpl> {
public:
    explicit KvStoreImplProxy(const sptr<IRemoteObject> &impl);
    ~KvStoreImplProxy() = default;
    virtual void GetKvStoreSnapshot(sptr<IKvStoreObserver> observer,
                                    std::function<void(Status, sptr<IKvStoreSnapshotImpl>)> callback);
    virtual Status ReleaseKvStoreSnapshot(sptr<IKvStoreSnapshotImpl> iKvStoreSnapshot);
    virtual Status Put(const Key &key, const Value &value);
    virtual Status PutBatch(const std::vector<Entry> &entries);
    virtual Status Delete(const Key &key);
    virtual Status DeleteBatch(const std::vector<Key> &keys);
    virtual Status Clear();
    virtual Status StartTransaction();
    virtual Status Commit();
    virtual Status Rollback();
    virtual Status SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer);
    virtual Status UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer);
private:
    static inline BrokerDelegator<KvStoreImplProxy> delegator_;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_KVSTORE_H
