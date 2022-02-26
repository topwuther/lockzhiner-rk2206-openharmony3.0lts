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

#ifndef I_SINGLE_KVSTORE_H
#define I_SINGLE_KVSTORE_H

#include <map>

#include "message_parcel.h"
#include "iremote_broker.h"
#include "ikvstore_observer.h"
#include "ikvstore_sync_callback.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "types.h"
#include "ikvstore_resultset.h"

namespace OHOS {
namespace DistributedKv {
class ISingleKvStore : public IRemoteBroker {
public:
    enum {
        PUT,
        DELETE,
        GET,
        SUBSCRIBEKVSTORE,
        UNSUBSCRIBEKVSTORE,
        GETENTRIES,
        GETRESULTSET,
        CLOSERESULTSET,
        REMOVEDEVICEDATA,
        SYNC,
        REGISTERSYNCCALLBACK,
        UNREGISTERSYNCCALLBACK,
        PUTBATCH,
        DELETEBATCH,
        STARTTRANSACTION,
        COMMIT,
        ROLLBACK,
        GETENTRIESWITHQUERY,
        GETRESULTSETWITHQUERY,
        GETCOUNTWITHQUERY,
        CONTROL,
        SETCAPABILITYENABLED,
        SETCAPABILITYRANGE,
        SETSECURITLEVEL,
        SINGLE_CMD_LAST,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.ISingleKvStore")
    virtual Status Put(const Key &key, const Value &value) = 0;
    virtual Status Delete(const Key &key) = 0;
    virtual Status Get(const Key &key, Value &value) = 0;
    virtual Status SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer) = 0;
    virtual Status UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer) = 0;
    virtual Status GetEntries(const Key &prefixKey, std::vector<Entry> &entries) = 0;
    virtual Status GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries) = 0;
    virtual void GetResultSet(const Key &prefixKey,
                              std::function<void(Status, sptr<IKvStoreResultSet>)> callback) = 0;
    virtual void GetResultSetWithQuery(const std::string &query,
                                       std::function<void(Status, sptr<IKvStoreResultSet>)> callback) = 0;
    virtual Status CloseResultSet(sptr<IKvStoreResultSet> resultSet) = 0;
    virtual Status GetCountWithQuery(const std::string &query, int &result) = 0;
    virtual Status Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                        uint32_t allowedDelayMs) = 0;
    virtual Status RemoveDeviceData(const std::string &device) = 0;
    virtual Status RegisterSyncCallback(sptr<IKvStoreSyncCallback> callback) = 0;
    virtual Status UnRegisterSyncCallback() = 0;
    virtual Status PutBatch(const std::vector<Entry> &entries) = 0;
    virtual Status DeleteBatch(const std::vector<Key> &keys) = 0;
    virtual Status StartTransaction() = 0;
    virtual Status Commit() = 0;
    virtual Status Rollback() = 0;
    virtual Status Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output) = 0;
    virtual Status SetCapabilityEnabled(bool enabled) = 0;
    virtual Status SetCapabilityRange(const std::vector<std::string> &localLabels,
                                      const std::vector<std::string> &remoteSupportLabels) = 0;
    virtual Status GetSecurityLevel(SecurityLevel &securityLevel) = 0;
};

class SingleKvStoreStub : public IRemoteStub<ISingleKvStore> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
private:
    int PutOnRemote(MessageParcel& data, MessageParcel& reply);
    int DeleteOnRemote(MessageParcel& data, MessageParcel& reply);
    int GetOnRemote(MessageParcel& data, MessageParcel& reply);
    int SubscribeKvStoreOnRemote(MessageParcel& data, MessageParcel& reply);
    int UnSubscribeKvStoreOnRemote(MessageParcel& data, MessageParcel& reply);
    int GetEntriesOnRemote(MessageParcel& data, MessageParcel& reply);
    int GetEntriesWithQueryOnRemote(MessageParcel& data, MessageParcel& reply);
    int SyncOnRemote(MessageParcel& data, MessageParcel& reply);
    int GetResultSetOnRemote(MessageParcel& data, MessageParcel& reply);
    int GetResultSetWithQueryOnRemote(MessageParcel& data, MessageParcel& reply);
    int GetCountWithQueryOnRemote(MessageParcel& data, MessageParcel& reply);
    int CloseResultSetOnRemote(MessageParcel& data, MessageParcel& reply);
    int RemoveDeviceDataOnRemote(MessageParcel& data, MessageParcel& reply);
    int RegisterSyncCallbackOnRemote(MessageParcel& data, MessageParcel& reply);
    int UnRegisterSyncCallbackOnRemote(MessageParcel& data, MessageParcel& reply);
    int PutBatchOnRemote(MessageParcel& data, MessageParcel& reply);
    int DeleteBatchOnRemote(MessageParcel& data, MessageParcel& reply);
    int StartTransactionOnRemote(MessageParcel& data, MessageParcel& reply);
    int CommitOnRemote(MessageParcel& data, MessageParcel& reply);
    int RollbackOnRemote(MessageParcel& data, MessageParcel& reply);
    int ControlOnRemote(MessageParcel& data, MessageParcel& reply);
    int OnCapabilityEnableRequest(MessageParcel& data, MessageParcel& reply);
    int OnCapabilityRangeRequest(MessageParcel& data, MessageParcel& reply);
    int OnSecurityLevelRequest(MessageParcel& data, MessageParcel& reply);

    int WriteEntriesParcelable(MessageParcel& reply, Status status, std::vector<Entry> entries, int bufferSize);
    int GetTotalEntriesSize(std::vector<Entry> entries);

    using RequestHandler = int(SingleKvStoreStub::*)(MessageParcel&, MessageParcel&);
    static constexpr RequestHandler HANDLERS[SINGLE_CMD_LAST] = {
        [PUT] = &SingleKvStoreStub::PutOnRemote,
        [DELETE] = &SingleKvStoreStub::DeleteOnRemote,
        [GET] = &SingleKvStoreStub::GetOnRemote,
        [SUBSCRIBEKVSTORE] = &SingleKvStoreStub::SubscribeKvStoreOnRemote,
        [UNSUBSCRIBEKVSTORE] = &SingleKvStoreStub::UnSubscribeKvStoreOnRemote,
        [GETENTRIES] = &SingleKvStoreStub::GetEntriesOnRemote,
        [GETRESULTSET] = &SingleKvStoreStub::GetResultSetOnRemote,
        [CLOSERESULTSET] = &SingleKvStoreStub::CloseResultSetOnRemote,
        [REMOVEDEVICEDATA] = &SingleKvStoreStub::RemoveDeviceDataOnRemote,
        [SYNC] = &SingleKvStoreStub::SyncOnRemote,
        [REGISTERSYNCCALLBACK] = &SingleKvStoreStub::RegisterSyncCallbackOnRemote,
        [UNREGISTERSYNCCALLBACK] = &SingleKvStoreStub::UnRegisterSyncCallbackOnRemote,
        [PUTBATCH] = &SingleKvStoreStub::PutBatchOnRemote,
        [DELETEBATCH] = &SingleKvStoreStub::DeleteBatchOnRemote,
        [STARTTRANSACTION] = &SingleKvStoreStub::StartTransactionOnRemote,
        [COMMIT] = &SingleKvStoreStub::CommitOnRemote,
        [ROLLBACK] = &SingleKvStoreStub::RollbackOnRemote,
        [GETENTRIESWITHQUERY] = &SingleKvStoreStub::GetEntriesWithQueryOnRemote,
        [GETRESULTSETWITHQUERY] = &SingleKvStoreStub::GetResultSetWithQueryOnRemote,
        [GETCOUNTWITHQUERY] = &SingleKvStoreStub::GetCountWithQueryOnRemote,
        [CONTROL] = &SingleKvStoreStub::ControlOnRemote,
        [SETCAPABILITYENABLED] = &SingleKvStoreStub::OnCapabilityEnableRequest,
        [SETCAPABILITYRANGE] = &SingleKvStoreStub::OnCapabilityRangeRequest,
        [SETSECURITLEVEL] = &SingleKvStoreStub::OnSecurityLevelRequest,
    };
};

class SingleKvStoreProxy : public IRemoteProxy<ISingleKvStore> {
public:
    explicit SingleKvStoreProxy(const sptr<IRemoteObject> &impl);
    ~SingleKvStoreProxy() = default;
    virtual Status Put(const Key &key, const Value &value);
    virtual Status Delete(const Key &key);
    virtual Status Get(const Key &key, Value &value);
    virtual Status SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer);
    virtual Status UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer);
    virtual Status GetEntries(const Key &prefixKey, std::vector<Entry> &entries);
    virtual Status GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries);
    virtual void GetResultSet(const Key &prefixKey, std::function<void(Status, sptr<IKvStoreResultSet>)> callback);
    virtual void GetResultSetWithQuery(const std::string &query,
                                       std::function<void(Status, sptr<IKvStoreResultSet>)> callback);
    virtual Status CloseResultSet(sptr<IKvStoreResultSet> resultSet);
    virtual Status GetCountWithQuery(const std::string &query, int &result);
    virtual Status Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode, uint32_t allowedDelayMs);
    virtual Status RemoveDeviceData(const std::string &device);
    virtual Status RegisterSyncCallback(sptr<IKvStoreSyncCallback> callback);
    virtual Status UnRegisterSyncCallback();
    virtual Status PutBatch(const std::vector<Entry> &entries);
    virtual Status DeleteBatch(const std::vector<Key> &keys);
    virtual Status StartTransaction();
    virtual Status Commit();
    virtual Status Rollback();
    virtual Status Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output);
    virtual Status SetCapabilityEnabled(bool enabled);
    virtual Status SetCapabilityRange(const std::vector<std::string> &localLabels,
                                      const std::vector<std::string> &remoteSupportLabels);
    virtual Status GetSecurityLevel(SecurityLevel &securityLevel);
private:
    static inline BrokerDelegator<SingleKvStoreProxy> delegator_;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_SINGLE_KVSTORE_H
