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

#define LOG_TAG "SingleKvStoreProxy"

#include "ikvstore_single.h"
#include <cinttypes>
#include "constant.h"
#include "log_print.h"

namespace OHOS::DistributedKv {
constexpr SingleKvStoreStub::RequestHandler SingleKvStoreStub::HANDLERS[SINGLE_CMD_LAST];

SingleKvStoreProxy::SingleKvStoreProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<ISingleKvStore>(impl)
{}

Status SingleKvStoreProxy::Put(const Key &key, const Value &value)
{
    ZLOGD("proxy put");
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set max capacity fail.");
        return Status::IPC_ERROR;
    }

    int bufferSize = key.RawSize() + value.RawSize();
    if (!data.WriteInt32(bufferSize)) {
        ZLOGW("write buffer size failed.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        if (!data.WriteParcelable(&key) || !data.WriteParcelable(&value)) {
            ZLOGW("write parcelable failed.");
            return Status::IPC_ERROR;
        }
        int error = Remote()->SendRequest(PUT, data, reply, mo);
        if (error != 0) {
            ZLOGW("SendRequest failed with error code %d", error);
            return Status::IPC_ERROR;
        }
        return static_cast<Status>(reply.ReadInt32());
    }
    ZLOGI("putting large data.");
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        return Status::ERROR;
    }
    int bufferLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    if (!key.WriteToBuffer(cursor, bufferLeftSize) || !value.WriteToBuffer(cursor, bufferLeftSize) ||
        !data.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write big data to buffer failed");
        return Status::IPC_ERROR;
    }
    // Parcel before IPC:
    // buffer:  options | bufferSize
    // rawdata: keySize |    key     | ValueSize | value
    int32_t error = Remote()->SendRequest(PUT, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::Delete(const Key &key)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteParcelable(&key)) {
        ZLOGW("write key to parcel fail");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(DELETE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::Get(const Key &key, Value &value)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set max capacity fail.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteParcelable(&key)) {
        ZLOGW("write parcel key fail");
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GET, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error is %d", error);
        return Status::IPC_ERROR;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        ZLOGW("status not success(%d)", static_cast<int>(status));
        return status;
    }

    int bufferSize = reply.ReadInt32();
    if (bufferSize < 0) {
        ZLOGW("bufferSize < 0(%d)", bufferSize);
        return Status::ERROR;
    }
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        sptr<Value> valueTmp = reply.ReadParcelable<Value>();
        if (valueTmp != nullptr) {
            value = *valueTmp;
        }
        return status;
    }
    ZLOGI("getting big data");
    // this memory is managed by MassageParcel, DO NOT free here
    const uint8_t *buffer = reinterpret_cast<const uint8_t *>(reply.ReadRawData(bufferSize));
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        return Status::IPC_ERROR;
    }
    if (!value.ReadFromBuffer(buffer, bufferSize)) {
        ZLOGW("read value from buffer failed");
        return Status::IPC_ERROR;
    }
    return Status::SUCCESS;
}

Status SingleKvStoreProxy::SubscribeKvStore(const SubscribeType subscribeType,
                                            sptr<IKvStoreObserver> observer)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    if (!data.WriteInt32(static_cast<int>(subscribeType)) ||
        !data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
        ZLOGW("write subscribe type or parcel failed.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(SUBSCRIBEKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::UnSubscribeKvStore(const SubscribeType subscribeType,
                                              sptr<IKvStoreObserver> observer)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    if (!data.WriteInt32(static_cast<int>(subscribeType)) ||
        !data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
        ZLOGW("write subscribe type or parcel failed.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(UNSUBSCRIBEKVSTORE, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::GetEntries(const Key &prefixKey, std::vector<Entry> &entries)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY) || !data.WriteParcelable(&prefixKey)) {
        ZLOGW("set max capacity or write parcel failed.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETENTRIES, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error is %d", error);
        return Status::IPC_ERROR;
    }

    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        return status;
    }

    int replyEntryCount = reply.ReadInt32();
    int bufferSize = reply.ReadInt32();
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        for (int i = 0; i < replyEntryCount; i++) {
            sptr<Entry> entry = reply.ReadParcelable<Entry>();
            if (entry == nullptr) {
                ZLOGW("entry is nullptr");
                entries.clear();
                return Status::IPC_ERROR;
            }
            entries.push_back(*entry);
        }
    } else {
        ZLOGI("getting large entry set");
        // this memory is managed by MassageParcel, DO NOT free here
        const uint8_t *buffer = reinterpret_cast<const uint8_t *>(reply.ReadRawData(bufferSize));
        if (replyEntryCount < 0 || bufferSize < 0 || buffer == nullptr) {
            ZLOGW("replyEntryCount(%d) or bufferSize(%d) less than 0, or buffer is nullptr", replyEntryCount,
                bufferSize);
            return Status::IPC_ERROR;
        }
        const uint8_t *rawDataCursor = buffer;
        int bufferLeftSize = bufferSize;
        std::vector<Entry> entriesTmp = std::vector<Entry>(replyEntryCount);
        for (auto &entry : entriesTmp) {
            if (!entry.key.ReadFromBuffer(rawDataCursor, bufferLeftSize) ||
                !entry.value.ReadFromBuffer(rawDataCursor, bufferLeftSize)) {
                ZLOGW("read entry from buffer failed");
                entries.clear();
                return Status::IPC_ERROR;
            }
        }
        entries = std::move(entriesTmp);
    }
    return Status::SUCCESS;
}

Status SingleKvStoreProxy::GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries)
{
    ZLOGD("begin");
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set max capacity failed.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(query)) {
        ZLOGW("set max capacity or write parcel failed.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETENTRIESWITHQUERY, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error is %d", error);
        return Status::IPC_ERROR;
    }

    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        return status;
    }

    int replyEntryCount = reply.ReadInt32();
    int bufferSize = reply.ReadInt32();
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        for (int i = 0; i < replyEntryCount; i++) {
            sptr<Entry> entry = reply.ReadParcelable<Entry>();
            if (entry == nullptr) {
                ZLOGW("entry is nullptr");
                entries.clear();
                return Status::IPC_ERROR;
            }
            entries.push_back(*entry);
        }
    } else {
        ZLOGI("getting large entry set");
        // this memory is managed by MassageParcel, DO NOT free here
        const uint8_t *buffer = reinterpret_cast<const uint8_t *>(reply.ReadRawData(bufferSize));
        if (replyEntryCount < 0 || bufferSize < 0 || buffer == nullptr) {
            ZLOGW("replyEntryCount(%d) or bufferSize(%d) less than 0, or buffer is nullptr", replyEntryCount,
                bufferSize);
            return Status::IPC_ERROR;
        }
        const uint8_t *rawDataCursor = buffer;
        int bufferLeftSize = bufferSize;
        std::vector<Entry> entriesTmp = std::vector<Entry>(replyEntryCount);
        for (auto &entry : entriesTmp) {
            if (!entry.key.ReadFromBuffer(rawDataCursor, bufferLeftSize) ||
                !entry.value.ReadFromBuffer(rawDataCursor, bufferLeftSize)) {
                ZLOGW("read entry from buffer failed");
                entries.clear();
                return Status::IPC_ERROR;
            }
        }
        entries = std::move(entriesTmp);
    }
    return Status::SUCCESS;
}

void SingleKvStoreProxy::GetResultSet(const Key &prefixKey,
    std::function<void(Status, sptr<IKvStoreResultSet>)> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    if (!data.WriteParcelable(&prefixKey)) {
        ZLOGW("SendRequest GetResultSet WriteParcel fail.");
        callback(Status::IPC_ERROR, nullptr);
        return;
    }
    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETRESULTSET, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        callback(Status::IPC_ERROR, nullptr);
        return;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        callback(status, nullptr);
        return;
    }
    sptr<IRemoteObject> remote = reply.ReadRemoteObject();
    if (remote == nullptr) {
        callback(status, nullptr);
        return;
    }
    sptr<IKvStoreResultSet> kvstoreResultSetProxy = iface_cast<IKvStoreResultSet>(remote);
    callback(status, std::move(kvstoreResultSetProxy));
}

void SingleKvStoreProxy::GetResultSetWithQuery(const std::string &query,
                                               std::function<void(Status, sptr<IKvStoreResultSet>)> callback)
{
    ZLOGD("begin");
    MessageParcel data;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    if (!data.WriteString(query)) {
        ZLOGW("SendRequest GetResultSet WriteParcel fail.");
        callback(Status::IPC_ERROR, nullptr);
        return;
    }
    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETRESULTSETWITHQUERY, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        callback(Status::IPC_ERROR, nullptr);
        return;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        callback(status, nullptr);
        return;
    }
    sptr<IRemoteObject> remote = reply.ReadRemoteObject();
    if (remote == nullptr) {
        callback(status, nullptr);
        return;
    }
    sptr<IKvStoreResultSet> kvstoreResultSetProxy = iface_cast<IKvStoreResultSet>(remote);
    callback(status, std::move(kvstoreResultSetProxy));
}

Status SingleKvStoreProxy::GetCountWithQuery(const std::string &query, int &result)
{
    ZLOGD("begin");
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY) ||
        !data.WriteString(query)) {
        ZLOGW("set max capacity or write parcel failed.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETCOUNTWITHQUERY, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error is %d", error);
        return Status::IPC_ERROR;
    }

    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        return status;
    }

    result = reply.ReadInt32();
    return Status::SUCCESS;
}

Status SingleKvStoreProxy::CloseResultSet(sptr<IKvStoreResultSet> resultSetPtr)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (resultSetPtr == nullptr) {
        return Status::INVALID_ARGUMENT;
    }

    if (!data.WriteRemoteObject(resultSetPtr->AsObject().GetRefPtr())) {
        ZLOGW("Write Strong Parcel fail.");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(CLOSERESULTSET, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                                uint32_t allowedDelayMs)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    if (!data.WriteStringVector(deviceIdList) ||
        !data.WriteInt32(static_cast<int>(mode))) {
        ZLOGW("SendRequest write parcel failed.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(static_cast<int>(allowedDelayMs))) {
        ZLOGW("sync allowedDelayMs");
        return Status::IPC_ERROR;
    }
    int32_t error = Remote()->SendRequest(SYNC, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::RemoveDeviceData(const std::string &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteString(device)) {
        ZLOGW("WriteParcel failed.");
        return Status::IPC_ERROR;
    }

    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(REMOVEDEVICEDATA, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::RegisterSyncCallback(sptr<IKvStoreSyncCallback> callback)
{
    if (callback == nullptr) {
        ZLOGW("RegisterSyncCallback input is null");
        return Status::INVALID_ARGUMENT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        ZLOGW("RegisterSyncCallback write input binder is null");
        return Status::IPC_ERROR;
    }

    MessageParcel reply;
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(REGISTERSYNCCALLBACK, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::UnRegisterSyncCallback()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(UNREGISTERSYNCCALLBACK, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::PutBatch(const std::vector<Entry> &entries)
{
    ZLOGI("PutBatch begin");
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set capacity failed.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(entries.size())) {
        ZLOGW("write entries size failed.");
        return Status::IPC_ERROR;
    }

    int64_t bufferSize = 0;
    for (const auto &item : entries) {
        if (item.key.Size() > Constant::MAX_KEY_LENGTH || item.value.Size() > Constant::MAX_VALUE_LENGTH) {
            return Status::INVALID_ARGUMENT;
        }
        bufferSize += item.key.RawSize() + item.value.RawSize();
    }
    if (!data.WriteInt32(bufferSize)) {
        ZLOGW("write buffer size failed.");
        return Status::IPC_ERROR;
    }
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        for (const auto &item : entries) {
            if (!data.WriteParcelable(&item)) {
                ZLOGW("write parcel failed.");
                return Status::IPC_ERROR;
            }
        }
        MessageOption mo { MessageOption::TF_SYNC };
        if (Remote()->SendRequest(PUTBATCH, data, reply, mo) != 0) {
            return Status::IPC_ERROR;
        }
        return static_cast<Status>(reply.ReadInt32());
    }
    ZLOGI("putting large data.");
    if (bufferSize > static_cast<int64_t>(reply.GetRawDataCapacity())) {
        ZLOGW("batch size larger than Messageparcel limit.(%" PRId64")", bufferSize);
        return Status::INVALID_ARGUMENT;
    }
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        return Status::ERROR;
    }
    int bufferLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    for (const auto &item : entries) {
        if (!item.key.WriteToBuffer(cursor, bufferLeftSize) ||
            !item.value.WriteToBuffer(cursor, bufferLeftSize)) {
            ZLOGW("write to buffer failed.");
            return Status::ERROR;
        }
    }
    if (!data.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write rawData failed");
        return Status::ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(PUTBATCH, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::DeleteBatch(const std::vector<Key> &keys)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set capacity failed.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(keys.size())) {
        ZLOGW("write keys size failed.");
        return Status::IPC_ERROR;
    }
    for (const auto &item : keys) {
        if (keys.size() > Constant::MAX_KEY_LENGTH) {
            ZLOGW("Delete key size larger than key size limit");
            return Status::INVALID_ARGUMENT;
        }
        if (!data.WriteParcelable(&item)) {
            ZLOGW("write parcel failed.");
            return Status::IPC_ERROR;
        }
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(DELETEBATCH, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::StartTransaction()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(STARTTRANSACTION, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::Commit()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(COMMIT, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::Rollback()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(ROLLBACK, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status SingleKvStoreProxy::Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set max capacity fail.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(static_cast<int32_t>(cmd))) {
        ZLOGW("write cmd failed.");
        return Status::IPC_ERROR;
    }
    if (!data.WriteParcelable(&inputParam)) {
        ZLOGW("write parcel fail");
        return Status::IPC_ERROR;
    }

    MessageOption mo{MessageOption::TF_SYNC};
    int32_t error = Remote()->SendRequest(CONTROL, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest failed, error is %d", error);
        return Status::IPC_ERROR;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        ZLOGW("status not success(%d)", static_cast<int>(status));
        return status;
    }

    int bufferSize = reply.ReadInt32();
    if (bufferSize > 0 && bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        output = reply.ReadParcelable<KvParam>();
    }

    return status;
}

Status SingleKvStoreProxy::SetCapabilityEnabled(bool enabled)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteBool(enabled)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(SETCAPABILITYENABLED, data, reply, mo);

    Status status = Status::IPC_ERROR;
    if (error == 0) {
        status = static_cast<Status>(reply.ReadInt32());
    }
    return status;
}

Status SingleKvStoreProxy::SetCapabilityRange(const std::vector<std::string> &localLabels,
                                              const std::vector<std::string> &remoteSupportLabels)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }

    if (!data.WriteStringVector(localLabels)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }

    if (!data.WriteStringVector(remoteSupportLabels)) {
        ZLOGW("failed to write parcel.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(SETCAPABILITYRANGE, data, reply, mo);

    Status status = Status::IPC_ERROR;
    if (error == 0) {
        status = static_cast<Status>(reply.ReadInt32());
    }
    return status;
}

Status SingleKvStoreProxy::GetSecurityLevel(SecurityLevel &securityLevel)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(SingleKvStoreProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(SETSECURITLEVEL, data, reply, mo);

    Status status = Status::IPC_ERROR;
    if (error == 0) {
        status = static_cast<Status>(reply.ReadInt32());
    }
    if (status == Status::SUCCESS) {
        securityLevel = static_cast<SecurityLevel>(reply.ReadInt32());
    }
    return status;
}

int SingleKvStoreStub::PutOnRemote(MessageParcel &data, MessageParcel &reply)
{
    const int bufferSize = data.ReadInt32();
    ZLOGD("bufferSize %d", bufferSize);
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        sptr<Key> key = data.ReadParcelable<Key>();
        sptr<Value> value = data.ReadParcelable<Value>();
        if (key == nullptr || value == nullptr) {
            ZLOGW("nullptr after ipc");
            reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT));
            return 0;
        }
        Status status = Put(*key, *value);
        if (!reply.WriteInt32(static_cast<int>(status))) {
            ZLOGW("write status fail");
            return -1;
        }
        return 0;
    }
    // this memory is managed by MassageParcel, DO NOT free here
    const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.ReadRawData(bufferSize));
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            ZLOGW("write status fail");
            return -1;
        }
        return 0;
    }
    int bufferLeftSize = bufferSize;
    const uint8_t *cursor = buffer;
    Key key;
    if (!key.ReadFromBuffer(cursor, bufferLeftSize)) {
        ZLOGW("read key error.");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    Value value;
    if (!value.ReadFromBuffer(cursor, bufferLeftSize)) {
        ZLOGW("read value error");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    Status status = Put(key, value);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::DeleteOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<Key> key = data.ReadParcelable<Key>();
    if (key == nullptr) {
        ZLOGW("key nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    Status status = Delete(*key);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::GetOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set reply MessageParcel capacity failed");
        return -1;
    }
    sptr<Key> key = data.ReadParcelable<Key>();
    if (key == nullptr) {
        ZLOGW("key is null");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    Value value;
    Status status = Get(*key, value);
    int bufferSize = value.RawSize();
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        if (!reply.WriteInt32(static_cast<int>(status)) ||
            !reply.WriteInt32(bufferSize) ||
            !reply.WriteParcelable(&value)) {
            ZLOGW("write value to parcel fail.");
            return -1;
        }
        return 0;
    }
    ZLOGI("getting large entry");
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(
            new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::ILLEGAL_STATE))) {
            return -1;
        }
        return 0;
    }
    int bufferLeftSize = bufferSize;
    if (!reply.WriteInt32(static_cast<int>(status)) ||
        !reply.WriteInt32(bufferSize)) {
        ZLOGW("write bufferSize failed.");
        return -1;
    }

    uint8_t *cursor = buffer.get();
    if (!value.WriteToBuffer(cursor, bufferLeftSize) ||
        !reply.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write bufferSize failed.");
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::SubscribeKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    SubscribeType subscribeType = static_cast<SubscribeType>(data.ReadInt32());
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreObserver> kvStoreObserverProxy = iface_cast<IKvStoreObserver>(remote);

    Status status = SubscribeKvStore(subscribeType, std::move(kvStoreObserverProxy));
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::UnSubscribeKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    SubscribeType subscribeType = static_cast<SubscribeType>(data.ReadInt32());
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        ZLOGW("kvStoreObserverProxy nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreObserver> kvStoreObserverProxy = iface_cast<IKvStoreObserver>(remote);
    Status status = UnSubscribeKvStore(subscribeType, std::move(kvStoreObserverProxy));
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::WriteEntriesParcelable(MessageParcel &reply, Status status,
    std::vector<Entry> entries, int bufferSize)
{
    if (!reply.WriteInt32(static_cast<int>(status)) ||
        !reply.WriteInt32(entries.size()) ||
        !reply.WriteInt32(bufferSize)) {
        ZLOGW("write status to parcel failed.");
        return -1;
    }
    for (auto const &entry : entries) {
        if (!reply.WriteParcelable(&entry)) {
            ZLOGW("write entry to parcel failed.");
            return -1;
        }
    }
    return 0;
}

int SingleKvStoreStub::GetTotalEntriesSize(std::vector<Entry> entries)
{
    int bufferSize = 0;
    for (const auto &entry : entries) {
        bufferSize += entry.key.RawSize();
        bufferSize += entry.value.RawSize();
    }
    return bufferSize;
}
int SingleKvStoreStub::GetEntriesOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set reply MessageParcel capacity failed");
        return -1;
    }
    sptr<Key> keyPrefix = data.ReadParcelable<Key>();
    if (keyPrefix == nullptr) {
        ZLOGW("keyPrefix is null");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    std::vector<Entry> entries;
    Status status = GetEntries(*keyPrefix, entries);
    if (status != Status::SUCCESS) {
        if (!reply.WriteInt32(static_cast<int>(status))) {
            return -1;
        }
        return 0;
    }

    int bufferSize = GetTotalEntriesSize(entries);
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        return WriteEntriesParcelable(reply, status, entries, bufferSize);
    }
    ZLOGI("getting large entry set");
    if (bufferSize > static_cast<int64_t>(reply.GetRawDataCapacity())) {
        ZLOGW("bufferSize %d larger than message parcel limit", bufferSize);
        if (!reply.WriteInt32(static_cast<int>(Status::ERROR))) {
            return -1;
        }
        return 0;
    }
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(
            new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::ERROR))) {
            return -1;
        }
        return 0;
    }

    if (!reply.WriteInt32(static_cast<int>(status)) ||
        !reply.WriteInt32(entries.size()) ||
        !reply.WriteInt32(bufferSize)) {
        ZLOGW("write entry size failed.");
    }
    int bufferLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    for (const auto &item : entries) {
        if (!item.key.WriteToBuffer(cursor, bufferLeftSize) ||
            !item.value.WriteToBuffer(cursor, bufferLeftSize)) {
            ZLOGW("write wo buffer failed.");
            return -1;
        }
    }
    if (!reply.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write rawData failed");
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::GetEntriesWithQueryOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set reply MessageParcel capacity failed");
        return -1;
    }
    std::string query = data.ReadString();
    std::vector<Entry> entries;
    Status status = GetEntriesWithQuery(query, entries);
    if (status != Status::SUCCESS) {
        if (!reply.WriteInt32(static_cast<int>(status))) {
            return -1;
        }
        return 0;
    }

    int bufferSize = GetTotalEntriesSize(entries);
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        return WriteEntriesParcelable(reply, status, entries, bufferSize);
    }
    ZLOGI("getting large entry set");
    if (bufferSize > static_cast<int64_t>(reply.GetRawDataCapacity())) {
        ZLOGW("bufferSize %d larger than message parcel limit", bufferSize);
        if (!reply.WriteInt32(static_cast<int>(Status::ERROR))) {
            return -1;
        }
        return 0;
    }
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(
            new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::ERROR))) {
            return -1;
        }
        return 0;
    }

    if (!reply.WriteInt32(static_cast<int>(status)) ||
        !reply.WriteInt32(entries.size()) ||
        !reply.WriteInt32(bufferSize)) {
        ZLOGW("write entry failed.");
        return -1;
    }
    int bufferLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    for (const auto &item : entries) {
        if (!item.key.WriteToBuffer(cursor, bufferLeftSize) ||
            !item.value.WriteToBuffer(cursor, bufferLeftSize)) {
            ZLOGW("write to buffer failed.");
            return -1;
        }
    }
    if (!reply.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write rawData failed");
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::SyncOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> devices;
    if (!data.ReadStringVector(&devices) || devices.empty()) {
        ZLOGW("SYNC list:%zu", devices.size());
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write sync status fail");
            return -1;
        }
        return 0;
    }
    auto mode = static_cast<SyncMode>(data.ReadInt32());
    auto allowedDelayMs = static_cast<uint32_t>(data.ReadInt32());
    Status status = Sync(devices, mode, allowedDelayMs);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write sync status fail");
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::GetResultSetOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<Key> key = data.ReadParcelable<Key>();
    if (key == nullptr) {
        ZLOGW("keyPrefix is null");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write getResultSet status fail");
            return -1;
        }
        return 0;
    }

    sptr<IKvStoreResultSet> proxyTmp;
    Status statusTmp;
    auto fun = [&](Status status, sptr<IKvStoreResultSet> proxy) {
        statusTmp = status;
        proxyTmp = std::move(proxy);
    };
    GetResultSet(*key, fun);
    if (!reply.WriteInt32(static_cast<int>(statusTmp))) {
        ZLOGW("write statusTmp fail");
        return -1;
    }
    if (statusTmp == Status::SUCCESS && proxyTmp != nullptr) {
        if (!reply.WriteRemoteObject(proxyTmp->AsObject().GetRefPtr())) {
            ZLOGW("write strong fail.");
            return -1;
        }
    }
    return 0;
}
int SingleKvStoreStub::GetResultSetWithQueryOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::string query = data.ReadString();
    sptr<IKvStoreResultSet> proxyTmp;
    Status statusTmp;
    auto fun = [&](Status status, sptr<IKvStoreResultSet> proxy) {
        statusTmp = status;
        proxyTmp = std::move(proxy);
    };
    GetResultSetWithQuery(query, fun);
    if (!reply.WriteInt32(static_cast<int>(statusTmp))) {
        ZLOGW("write statusTmp fail");
        return -1;
    }
    if (proxyTmp != nullptr) {
        if (!reply.WriteRemoteObject(proxyTmp->AsObject().GetRefPtr())) {
            ZLOGW("write strong fail.");
            return -1;
        }
    } else {
        ZLOGW("service side snapshot proxy is nullptr");
        if (!reply.WriteParcelable(nullptr)) {
            ZLOGW("write nullptr fail.");
            return -1;
        }
    }
    return 0;
}
int SingleKvStoreStub::PutBatchOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set batch size failed.");
        return -1;
    }
    int len = data.ReadInt32();
    if (len < 0) {
        ZLOGW("invalid status. len %d", len);
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write to parcel failed.");
            return -1;
        }
        return 0;
    }
    const int bufferSize = data.ReadInt32();
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        std::vector<Entry> entries;
        for (int i = 0; i < len; i++) {
            sptr<Entry> entry = data.ReadParcelable<Entry>();
            if (entry == nullptr) {
                ZLOGW("putbatch got null entry pointer");
                if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
                    ZLOGW("write to parcel failed.");
                    return -1;
                }
                return 0;
            }
            entries.push_back(*entry);
        }
        Status status = PutBatch(entries);
        if (!reply.WriteInt32(static_cast<int>(status))) {
            ZLOGW("write putbatch failed.");
            return -1;
        }
        return 0;
    }
    // this memory is managed by MassageParcel, DO NOT free here
    const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.ReadRawData(bufferSize));
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            ZLOGW("write putbatch big failed.");
            return -1;
        }
        return 0;
    }
    int bufferLeftSize = bufferSize;
    const uint8_t *cursor = buffer;
    std::vector<Entry> entries;
    Entry entry;
    for (int i = 0; i < len; i++) {
        bool success = entry.key.ReadFromBuffer(cursor, bufferLeftSize);
        success = success && entry.value.ReadFromBuffer(cursor, bufferLeftSize);
        entries.push_back(std::move(entry));
        if (!success) {
            ZLOGW("get key or value failed");
            if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
                ZLOGW("write putbatch big failed.");
                return -1;
            }
            return 0;
        }
    }
    Status status = PutBatch(entries);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write putbatch big failed.");
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::DeleteBatchOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int len = data.ReadInt32();
    if (len < 0) {
        ZLOGW("len %d invalid after ipc", len);
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write delete failed.");
            return -1;
        }
        return 0;
    }
    std::vector<Key> keys;
    for (int i = 0; i < len; i++) {
        sptr<Key> key = data.ReadParcelable<Key>();
        if (key == nullptr) {
            ZLOGW("key nullptr");
            if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
                ZLOGW("write delete failed.");
                return -1;
            }
            return 0;
        }
        keys.push_back(*key);
    }
    Status status = DeleteBatch(keys);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write delete failed.");
        return -1;
    }
    return 0;
}
int SingleKvStoreStub::ControlOnRemote(MessageParcel &data, MessageParcel &reply)
{
    KvControlCmd cmd = static_cast<KvControlCmd>(data.ReadInt32());
    sptr<KvParam> inputParam = data.ReadParcelable<KvParam>();
    if (inputParam == nullptr) {
        ZLOGW("inputParam is null");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    sptr<KvParam> output = nullptr;
    Status status = Control(cmd, *inputParam, output);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write control failed.");
        return -1;
    }
    if ((output != nullptr) && (output->RawSize() < Constant::SWITCH_RAW_DATA_SIZE)) {
        if (!reply.WriteInt32(output->RawSize())) {
            ZLOGW("write bufferSize failed.");
            return -1;
        }
        if (!reply.WriteParcelable(output)) {
            ZLOGW("write control output failed");
            return -1;
        }
    } else {
        if (!reply.WriteInt32(0)) {
            ZLOGW("write bufferSize 0 failed.");
            return -1;
        }
    }
    return 0;
}

int SingleKvStoreStub::OnSecurityLevelRequest(MessageParcel& data, MessageParcel &reply)
{
    SecurityLevel securityLevel = SecurityLevel::NO_LABEL;
    auto status = GetSecurityLevel(securityLevel);
    if (!reply.WriteInt32(static_cast<int32_t>(status))) {
        ZLOGW("Get SecurityLevel ipc failed.");
        return -1;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(securityLevel))) {
        ZLOGW("Get SecurityLevel ipc failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::OnCapabilityRangeRequest(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> locals;
    if (!data.ReadStringVector(&locals)) {
        ZLOGW("read local capability range ipc failed.");
        return -1;
    }

    std::vector<std::string> remotes;
    if (!data.ReadStringVector(&remotes)) {
        ZLOGW("remote local capability range ipc failed.");
        return -1;
    }

    Status ret = SetCapabilityRange(locals, remotes);
    if (!reply.WriteInt32(static_cast<int>(ret))) {
        ZLOGW("set capability range ipc failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::GetCountWithQueryOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::string query = data.ReadString();
    int result;
    Status status = GetCountWithQuery(query, result);
    if (status != Status::SUCCESS) {
        if (!reply.WriteInt32(static_cast<int>(status))) {
            return -1;
        }
        return 0;
    }
    if (!reply.WriteInt32(static_cast<int>(status)) || !reply.WriteInt32(result)) {
        ZLOGW("write result and status failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::CloseResultSetOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    if (obj == nullptr) {
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreResultSet> kvStoreResultSetProxy = iface_cast<IKvStoreResultSet>(obj);
    Status status = CloseResultSet(kvStoreResultSetProxy);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::RemoveDeviceDataOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write remove data status fail.");
            return -1;
        }
        return 0;
    }

    Status status = RemoveDeviceData(deviceId);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write status failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::RegisterSyncCallbackOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    if (obj == nullptr) {
        ZLOGW("kvStoreSyncCallbackProxy nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreSyncCallback> kvStoreSyncCallbackProxy = iface_cast<IKvStoreSyncCallback>(obj);
    Status status = RegisterSyncCallback(kvStoreSyncCallbackProxy);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("kvStoreSyncCallbackProxy write status fail");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::UnRegisterSyncCallbackOnRemote(MessageParcel &data, MessageParcel &reply)
{
    Status status = UnRegisterSyncCallback();
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write status fail.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::StartTransactionOnRemote(MessageParcel &data, MessageParcel &reply)
{
    Status status = StartTransaction();
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write starttransaction failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::CommitOnRemote(MessageParcel &data, MessageParcel &reply)
{
    Status status = Commit();
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write commit failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::RollbackOnRemote(MessageParcel &data, MessageParcel &reply)
{
    Status status = Rollback();
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write rollback failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::OnCapabilityEnableRequest(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt32(static_cast<int>(SetCapabilityEnabled(data.ReadBool())))) {
        ZLOGW("set capability enable ipc failed.");
        return -1;
    }
    return 0;
}

int SingleKvStoreStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                       MessageOption &option)
{
    ZLOGD("%d", code);
    std::u16string descriptor = SingleKvStoreStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    if (code >= 0 && code < SINGLE_CMD_LAST) {
        return (this->*HANDLERS[code])(data, reply);
    } else {
        MessageOption mo { MessageOption::TF_SYNC };
        return IPCObjectStub::OnRemoteRequest(code, data, reply, mo);
    }
}
} // namespace OHOS::DistributedKv
