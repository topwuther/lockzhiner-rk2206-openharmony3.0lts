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

#define LOG_TAG "KvStoreImplProxy"

#include "ikvstore.h"
#include <cinttypes>
#include "message_parcel.h"
#include "constant.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
enum {
    GETKVSTORESNAPSHOT,
    RELEASEKVSTORESNAPSHOT,
    PUT,
    PUTBATCH,
    DELETE,
    DELETEBATCH,
    CLEAR,
    STARTTRANSACTION,
    COMMIT,
    ROLLBACK,
    SUBSCRIBEKVSTORE,
    UNSUBSCRIBEKVSTORE,
};

KvStoreImplProxy::KvStoreImplProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IKvStoreImpl>(impl)
{}

void KvStoreImplProxy::GetKvStoreSnapshot(sptr<IKvStoreObserver> observer,
                                          std::function<void(Status, sptr<IKvStoreSnapshotImpl>)> callback)
{
    if (observer == nullptr) {
        callback(Status::INVALID_ARGUMENT, nullptr);
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
        ZLOGW("get snapshot fail.");
        callback(Status::IPC_ERROR, nullptr);
        return;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETKVSTORESNAPSHOT, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        callback(Status::IPC_ERROR, nullptr);
        return;
    }
    Status status = static_cast<Status>(reply.ReadInt32());
    if (status == Status::SUCCESS) {
        sptr<IRemoteObject> remote = reply.ReadRemoteObject();
        if (remote == nullptr) {
            callback(status, nullptr);
            return;
        }
        sptr<IKvStoreSnapshotImpl> kvstoreImplProxy = iface_cast<IKvStoreSnapshotImpl>(remote);
        callback(status, std::move(kvstoreImplProxy));
    } else {
        callback(status, nullptr);
    }
}

Status KvStoreImplProxy::ReleaseKvStoreSnapshot(sptr<IKvStoreSnapshotImpl> kvStoreSnapshotPtr)
{
    if (kvStoreSnapshotPtr == nullptr) {
        ZLOGW("input snapshot ptr is null");
        return Status::INVALID_ARGUMENT;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteRemoteObject(kvStoreSnapshotPtr->AsObject().GetRefPtr())) {
        ZLOGW("write input snapshot ptr failed.");
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(RELEASEKVSTORESNAPSHOT, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreImplProxy::Put(const Key &key, const Value &value)
{
    ZLOGD("proxy put");
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("write capacity failed.");
        return Status::IPC_ERROR;
    }

    int bufferSize = key.RawSize() + value.RawSize();
    if (!data.WriteInt32(bufferSize)) {
        ZLOGW("write size failed.");
        return Status::IPC_ERROR;
    }
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        if (!data.WriteParcelable(&key) || !data.WriteParcelable(&value)) {
            ZLOGW("write key or value failed.");
            return Status::IPC_ERROR;
        }

        MessageOption mo { MessageOption::TF_SYNC };
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
    if (!key.WriteToBuffer(cursor, bufferLeftSize) ||
        !value.WriteToBuffer(cursor, bufferLeftSize) ||
        !data.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write failed");
        return Status::ERROR;
    }
    // Parcel before IPC:
    // buffer:  options | bufferSize
    // rawdata: keySize |    key     | ValueSize | value
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(PUT, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreImplProxy::PutBatch(const std::vector<Entry> &entries)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
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
        ZLOGW("batch size larger than Messageparcel limit.(%" PRIu64")", bufferSize);
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
            ZLOGW("write item failed.");
        }
    }
    if (!data.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write failed");
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

Status KvStoreImplProxy::Delete(const Key &key)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteParcelable(&key)) {
        ZLOGW("write key failed.");
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

Status KvStoreImplProxy::DeleteBatch(const std::vector<Key> &keys)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
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
        if (item.Size() > Constant::MAX_KEY_LENGTH) {
            ZLOGW("Delete key size larger than key size limit");
            return Status::INVALID_ARGUMENT;
        }
        if (!data.WriteParcelable(&item)) {
            ZLOGW("write parcel failed");
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

Status KvStoreImplProxy::Clear()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(CLEAR, data, reply, mo);
    if (error != 0) {
        ZLOGW("SendRequest returned %d", error);
        return Status::IPC_ERROR;
    }
    return static_cast<Status>(reply.ReadInt32());
}

Status KvStoreImplProxy::StartTransaction()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
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

Status KvStoreImplProxy::Commit()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
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

Status KvStoreImplProxy::Rollback()
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
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

Status KvStoreImplProxy::SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    if (observer == nullptr) {
        ZLOGW("observer is invalid.");
        return Status::INVALID_ARGUMENT;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(static_cast<int>(subscribeType)) ||
        !data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
        ZLOGW("subscribe type failed.");
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

Status KvStoreImplProxy::UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }

    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!data.WriteInt32(static_cast<int>(subscribeType)) ||
        !data.WriteRemoteObject(observer->AsObject().GetRefPtr())) {
        ZLOGW("unsubscribe type failed.");
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

int32_t KvStoreImplStub::GetKvStoreSnapshotOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write obj failed.");
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreObserver> kvStoreObserverProxy = iface_cast<IKvStoreObserver>(remote);
    sptr<IKvStoreSnapshotImpl> proxyTmp;
    Status statusTmp;
    GetKvStoreSnapshot(kvStoreObserverProxy, [&](Status status, sptr<IKvStoreSnapshotImpl> proxy) {
        statusTmp = status;
        proxyTmp = std::move(proxy);
    });
    if (!reply.WriteInt32(static_cast<int>(statusTmp))) {
        ZLOGW("write get snapshot result failed.");
        return -1;
    }
    if (statusTmp == Status::SUCCESS && proxyTmp != nullptr) {
        if (!reply.WriteRemoteObject(proxyTmp->AsObject().GetRefPtr())) {
            ZLOGW("write strong failed.");
            return -1;
        }
    }
    return 0;
}
int32_t KvStoreImplStub::ReleaseKvStoreSnapshotOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        ZLOGW("kvstoreSnapshotProxy nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreSnapshotImpl> kvStoreSnapshotProxy = iface_cast<IKvStoreSnapshotImpl>(remote);
    Status status = ReleaseKvStoreSnapshot(kvStoreSnapshotProxy);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write release snapshot failed.");
        return -1;
    }
    return 0;
}
int32_t KvStoreImplStub::PutOnRemoteRequest(MessageParcel &data, MessageParcel &reply)
{
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("write capacity failed");
        return -1;
    }
    const int bufferSize = data.ReadInt32();
    ZLOGD("bufferSize %d", bufferSize);
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        sptr<Key> key = data.ReadParcelable<Key>();
        sptr<Value> value = data.ReadParcelable<Value>();
        if (key == nullptr || value == nullptr) {
            if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
                ZLOGW("write key or val status failed.");
                return -1;
            }
            return 0;
        }
        Status status = Put(*key, *value);
        if (!reply.WriteInt32(static_cast<int>(status))) {
            ZLOGW("write ret status failed.");
            return -1;
        }
        return 0;
    }
    // this memory is managed by MassageParcel, DO NOT free here
    const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.ReadRawData(bufferSize));
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            ZLOGW("write buffer status failed.");
            return -1;
        }
        return 0;
    }
    int bufferLeftSize = bufferSize;
    const uint8_t *cursor = buffer;
    Key key;
    Value value;
    if (!key.ReadFromBuffer(cursor, bufferLeftSize) || !value.ReadFromBuffer(cursor, bufferLeftSize)) {
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            ZLOGW("read key or value error.");
            return -1;
        }
    }
    Status status = Put(key, value);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write ret status failed.");
        return -1;
    }
    return 0;
}
int32_t KvStoreImplStub::PutBatchOnRemoteRequest(MessageParcel &data, MessageParcel &reply)
{
    if (!data.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set batch size failed.");
        return -1;
    }
    int len = data.ReadInt32();
    if (len < 0) {
        ZLOGW("invalid status. len %d", len);
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write putbatch failed.");
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
                    ZLOGW("write putbatch failed.");
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
int32_t KvStoreImplStub::DeleteOnRemoteRequest(MessageParcel &data, MessageParcel &reply)
{
    sptr<Key> key = data.ReadParcelable<Key>();
    if (key == nullptr) {
        ZLOGW("key nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            ZLOGW("write delete failed.");
            return -1;
        }
        return 0;
    }
    Status status = Delete(*key);
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write delete failed.");
        return -1;
    }
    return 0;
}
int32_t KvStoreImplStub::DeleteBatchOnRemoteRequest(MessageParcel &data, MessageParcel &reply)
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
int32_t KvStoreImplStub::SubscribeKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = data.ReadInt32();
    if (type < 0) {
        return -1;
    }
    SubscribeType subscribeType = static_cast<SubscribeType>(type);
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        ZLOGW("kvStoreObserverProxy is null");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreObserver> kvStoreObserverProxy = iface_cast<IKvStoreObserver>(remote);
    Status status = SubscribeKvStore(subscribeType, std::move(kvStoreObserverProxy));
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write subscribe status failed.");
        return -1;
    }
    return 0;
}
int32_t KvStoreImplStub::UnSubscribeKvStoreOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = data.ReadInt32();
    if (type < 0) {
        return -1;
    }
    SubscribeType subscribeType = static_cast<SubscribeType>(type);
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        ZLOGW("unsubscribe Proxy is null");
        if (!reply.WriteInt32(static_cast<int>(Status::IPC_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IKvStoreObserver> kvStoreObserverProxy = iface_cast<IKvStoreObserver>(remote);
    Status status = UnSubscribeKvStore(subscribeType, std::move(kvStoreObserverProxy));
    if (!reply.WriteInt32(static_cast<int>(status))) {
        ZLOGW("write unsubscribe status failed.");
        return -1;
    }
    return 0;
}

int32_t KvStoreImplStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                         MessageParcel &reply, MessageOption &option)
{
    ZLOGD("%d", code);
    std::u16string descriptor = KvStoreImplStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    switch (code) {
        case GETKVSTORESNAPSHOT: {
            return GetKvStoreSnapshotOnRemote(data, reply);
        }
        case RELEASEKVSTORESNAPSHOT: {
            return ReleaseKvStoreSnapshotOnRemote(data, reply);
        }
        case PUT: {
            return PutOnRemoteRequest(data, reply);
        }
        case PUTBATCH: {
            return PutBatchOnRemoteRequest(data, reply);
        }
        case DELETE: {
            return DeleteOnRemoteRequest(data, reply);
        }
        case DELETEBATCH: {
            return DeleteBatchOnRemoteRequest(data, reply);
        }
        case CLEAR: {
            Status status = Clear();
            if (!reply.WriteInt32(static_cast<int>(status))) {
                ZLOGW("write clear failed.");
                return -1;
            }
            return 0;
        }
        case STARTTRANSACTION: {
            Status status = StartTransaction();
            if (!reply.WriteInt32(static_cast<int>(status))) {
                ZLOGW("write transaction failed.");
                return -1;
            }
            return 0;
        }
        case COMMIT: {
            Status status = Commit();
            if (!reply.WriteInt32(static_cast<int>(status))) {
                ZLOGW("write commit failed.");
                return -1;
            }
            return 0;
        }
        case ROLLBACK: {
            Status status = Rollback();
            if (!reply.WriteInt32(static_cast<int>(status))) {
                ZLOGW("write rollback failed.");
                return -1;
            }
            return 0;
        }
        case SUBSCRIBEKVSTORE: {
            return SubscribeKvStoreOnRemote(data, reply);
        }
        case UNSUBSCRIBEKVSTORE: {
            return UnSubscribeKvStoreOnRemote(data, reply);
        }
        default: {
            MessageOption mo { MessageOption::TF_SYNC };
            return IPCObjectStub::OnRemoteRequest(code, data, reply, mo);
        }
    }
}
}  // namespace DistributedKv
}  // namespace OHOS
