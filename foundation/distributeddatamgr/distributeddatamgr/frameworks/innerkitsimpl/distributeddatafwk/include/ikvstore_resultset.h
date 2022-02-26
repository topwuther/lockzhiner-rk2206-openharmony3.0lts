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

#ifndef I_KVSTORE_RESULTSET_H
#define I_KVSTORE_RESULTSET_H

#include "message_parcel.h"
#include "iremote_broker.h"
#include "ikvstore_observer.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "types.h"

namespace OHOS::DistributedKv {
class IKvStoreResultSet : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreResultSet")
    virtual int GetCount() = 0;

    virtual int GetPosition() = 0;

    virtual bool MoveToFirst() = 0;

    virtual bool MoveToLast() = 0;

    virtual bool MoveToNext() = 0;

    virtual bool MoveToPrevious() = 0;

    virtual bool Move(int offset) = 0;

    virtual bool MoveToPosition(int position) = 0;

    virtual bool IsFirst() = 0;

    virtual bool IsLast() = 0;

    virtual bool IsBeforeFirst() = 0;

    virtual bool IsAfterLast() = 0;

    virtual Status GetEntry(Entry &entry) = 0;
};

class KvStoreResultSetStub : public IRemoteStub<IKvStoreResultSet> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;

private:
    int GetEntryOnRemote(MessageParcel &reply);
};

class KvStoreResultSetProxy : public IRemoteProxy<IKvStoreResultSet> {
public:
    explicit KvStoreResultSetProxy(const sptr<IRemoteObject> &impl);
    ~KvStoreResultSetProxy() = default;
    virtual int GetCount();

    virtual int GetPosition();

    virtual bool MoveToFirst();

    virtual bool MoveToLast();

    virtual bool MoveToNext();

    virtual bool MoveToPrevious();

    virtual bool Move(int offset);

    virtual bool MoveToPosition(int position);

    virtual bool IsFirst();

    virtual bool IsLast();

    virtual bool IsBeforeFirst();

    virtual bool IsAfterLast();

    virtual Status GetEntry(Entry &entry);
private:
    virtual int SendRequest(uint32_t code);
    virtual bool SendRequestRetBool(uint32_t code);
    static inline BrokerDelegator<KvStoreResultSetProxy> delegator_;
};
} // namespace OHOS::DistributedKv
#endif // I_KVSTORE_RESULTSET_H
