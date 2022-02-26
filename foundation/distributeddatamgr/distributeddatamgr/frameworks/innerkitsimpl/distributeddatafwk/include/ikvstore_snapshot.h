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

#ifndef I_KVSTORE_SNAPSHOT_H
#define I_KVSTORE_SNAPSHOT_H

#include "message_parcel.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

class IKvStoreSnapshotImpl : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreSnapshotImpl")
    virtual void GetEntries(
        const Key &prefixKey, const Key &nextKey,
        std::function<void(OHOS::DistributedKv::Status, std::vector<OHOS::DistributedKv::Entry> &,
                           const OHOS::DistributedKv::Key &)>
            callback) = 0;

    virtual void GetKeys(const Key &prefixKey, const Key &nextKey,
                         std::function<void(Status, std::vector<Key> &, const Key &)> callback) = 0;

    virtual Status Get(const Key &key, Value &value) = 0;
};

class KvStoreSnapshotImplStub : public IRemoteStub<IKvStoreSnapshotImpl> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;

private:
    int32_t GetEntriesOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetKeysRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetTotalEntriesSize(std::vector<Entry> entryList);
    int32_t WriteEntriesParcelable(MessageParcel &reply, Status statusTmp,
            std::vector<Entry> entryList, int bufferSize, Key nxtKey);
    int32_t GetTotalkeysSize(std::vector<Key> keyList);
    int32_t WritekeysParcelable(MessageParcel &reply, Status statusTmp,
            std::vector<Key> keyList, int bufferSize, Key nxtKey);
};

class KvStoreSnapshotImplProxy : public IRemoteProxy<IKvStoreSnapshotImpl> {
public:
    explicit KvStoreSnapshotImplProxy(const sptr<IRemoteObject> &impl);
    virtual void GetEntries(
        const Key &prefixKey, const Key &nextKey,
        std::function<void(OHOS::DistributedKv::Status, std::vector<OHOS::DistributedKv::Entry> &,
                           const OHOS::DistributedKv::Key &)>
            callback);

    virtual void GetKeys(const Key &prefixKey, const Key &nextKey,
                         std::function<void(Status, std::vector<Key> &, const Key &)> callback);

    virtual Status Get(const Key &key, Value &value);

    virtual ~KvStoreSnapshotImplProxy();
private:
    static inline BrokerDelegator<KvStoreSnapshotImplProxy> delegator_;
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_KVSTORE_SNAPSHOT_H
