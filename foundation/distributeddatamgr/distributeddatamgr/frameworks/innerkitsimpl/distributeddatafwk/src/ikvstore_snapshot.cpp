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

#define LOG_TAG "KvStoreSnapshotImplProxy"

#include "ikvstore_snapshot.h"
#include "constant.h"
#include "log_print.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedKv {
enum {
    GETENTRIES,
    GETKEYS,
    GET,
};

KvStoreSnapshotImplProxy::KvStoreSnapshotImplProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IKvStoreSnapshotImpl>(impl)
{}

KvStoreSnapshotImplProxy::~KvStoreSnapshotImplProxy()
{}

void KvStoreSnapshotImplProxy::GetEntries(const Key &prefixKey, const Key &nextKey,
    std::function<void(Status status, std::vector<Entry> &entries, const Key &key)> callback)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreSnapshotImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    std::vector<Entry> entries;
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        ZLOGW("set max capacity failed");
        callback(Status::IPC_ERROR, entries, Key());
        return;
    }
    if (!data.WriteParcelable(&prefixKey)) {
        ZLOGW("write prefix failed");
        callback(Status::IPC_ERROR, entries, Key());
        return;
    }
    if (!data.WriteParcelable(&nextKey)) {
        ZLOGW("write nextkey failed");
        callback(Status::IPC_ERROR, entries, Key());
        return;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    // struct of returned reply:
    // buffer:  | status | entryLength | rawdatasize | (sptr<Key>)nextkey |
    // rawData: ( | keyLen | key | valueLen | value | ){entryLength}
    int32_t error = Remote()->SendRequest(GETENTRIES, data, reply, mo);

    if (error != 0) {
        ZLOGW("Transact failed");
        callback(Status::IPC_ERROR, entries, Key());
        return;
    }

    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        ZLOGW("status not success, which is %d", static_cast<int>(status));
        callback(status, entries, Key());
        return;
    }
    int replyEntryCount = reply.ReadInt32();
    int bufferSize = reply.ReadInt32();
    sptr<Key> keyTmp = reply.ReadParcelable<Key>();
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        for (int i = 0; i < replyEntryCount; i++) {
            sptr<Entry> entry = reply.ReadParcelable<Entry>();
            if (entry == nullptr) {
                ZLOGW("entry is nullptr");
                callback(Status::IPC_ERROR, entries, Key());
                return;
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
            callback(Status::IPC_ERROR, entries, Key());
            return;
        }
        const uint8_t *rawDataCursor = buffer;
        int bufferLeftSize = bufferSize;
        entries = std::vector<Entry>(replyEntryCount);
        for (auto &entry : entries) {
            if (!entry.key.ReadFromBuffer(rawDataCursor, bufferLeftSize) ||
                !entry.value.ReadFromBuffer(rawDataCursor, bufferLeftSize)) {
                ZLOGW("read key or value from buffer failed");
                callback(Status::IPC_ERROR, entries, Key());
                return;
            }
        }
    }
    if (keyTmp != nullptr) {
        callback(status, entries, *keyTmp);
    } else {
        callback(status, entries, Key());
    }
}

void KvStoreSnapshotImplProxy::GetKeys(const Key &prefixKey, const Key &nextKey,
    std::function<void(Status, std::vector<Key> &keys, const Key &key)> callback)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreSnapshotImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    std::vector<Key> keyList;
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        callback(Status::IPC_ERROR, keyList, Key());
        return;
    }
    if (!data.WriteParcelable(&prefixKey)) {
        callback(Status::IPC_ERROR, keyList, Key());
        return;
    }
    if (!data.WriteParcelable(&nextKey)) {
        callback(Status::IPC_ERROR, keyList, Key());
        return;
    }
    MessageOption mo { MessageOption::TF_SYNC };
    int32_t error = Remote()->SendRequest(GETKEYS, data, reply, mo);
    if (error != 0) {
        ZLOGW("Transact failed");
        callback(Status::IPC_ERROR, keyList, Key());
        return;
    }

    Status status = static_cast<Status>(reply.ReadInt32());
    if (status != Status::SUCCESS) {
        ZLOGW("status not success, which is %d", static_cast<int>(status));
        callback(status, keyList, Key());
        return;
    }
    int replyKeyCount = reply.ReadInt32();
    int bufferSize = reply.ReadInt32();
    sptr<Key> keyTmp = reply.ReadParcelable<Key>();
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        for (int i = 0; i < replyKeyCount; i++) {
            sptr<Key> keyInner = reply.ReadParcelable<Key>();
            if (keyInner == nullptr) {
                ZLOGW("keyInner is nullptr");
                callback(Status::IPC_ERROR, keyList, Key());
                return;
            }
            keyList.push_back(*keyInner);
        }
    } else {
        ZLOGI("getting large key set");
        // this memory is managed by MassageParcel, DO NOT free here
        const uint8_t *buffer = reinterpret_cast<const uint8_t *>(reply.ReadRawData(bufferSize));
        if (replyKeyCount < 0 || bufferSize < 0 || buffer == nullptr) {
            ZLOGW("replyKeyCount(%d) or bufferSize(%d) less than 0, or buffer is nullptr", replyKeyCount, bufferSize);
            callback(Status::IPC_ERROR, keyList, Key());
            return;
        }
        const uint8_t *rawDataCursor = buffer;
        int bufferLeftSize = bufferSize;
        keyList = std::vector<Key>(replyKeyCount);
        for (auto &key : keyList) {
            if (!key.ReadFromBuffer(rawDataCursor, bufferLeftSize)) {
                ZLOGW("read key from buffer failed");
                callback(Status::IPC_ERROR, keyList, Key());
                return;
            }
        }
    }
    if (keyTmp != nullptr) {
        callback(status, keyList, *keyTmp);
    } else {
        callback(status, keyList, Key());
    }
}

Status KvStoreSnapshotImplProxy::Get(const Key &key, Value &value)
{
    MessageParcel data, reply;
    if (!data.WriteInterfaceToken(KvStoreSnapshotImplProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return Status::IPC_ERROR;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        return Status::IPC_ERROR;
    }
    if (!data.WriteParcelable(&key)) {
        return Status::IPC_ERROR;
    }

    MessageOption mo { MessageOption::TF_SYNC };
    ZLOGI("start");
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

int32_t KvStoreSnapshotImplStub::GetTotalEntriesSize(std::vector<Entry> entryList)
{
    int bufferSize = 0;
    for (const auto &item : entryList) {
        bufferSize += item.key.RawSize() + item.value.RawSize();
    }
    return bufferSize;
}
int32_t KvStoreSnapshotImplStub::WriteEntriesParcelable(MessageParcel &reply, Status statusTmp,
    std::vector<Entry> entryList, int bufferSize, Key nxtKey)
{
    if (!reply.WriteInt32(static_cast<int>(statusTmp)) ||
        !reply.WriteInt32(entryList.size()) ||
        !reply.WriteInt32(bufferSize) ||
        !reply.WriteParcelable(&nxtKey)) {
        ZLOGW("write entry to parcel failed.");
        return -1;
    }
    for (const auto &item : entryList) {
        if (!reply.WriteParcelable(&item)) {
            ZLOGW("write item to parcel failed.");
            return -1;
        }
    }
    return 0;
}
int32_t KvStoreSnapshotImplStub::GetTotalkeysSize(std::vector<Key> keyList)
{
    int bufferSize = 0;
    for (const auto &key : keyList) {
        bufferSize += key.RawSize();
    }
    return bufferSize;
}
int32_t KvStoreSnapshotImplStub::WritekeysParcelable(MessageParcel &reply, Status statusTmp,
    std::vector<Key> keyList, int bufferSize, Key nxtKey)
{
    if (!reply.WriteInt32(static_cast<int>(statusTmp)) ||
        !reply.WriteInt32(keyList.size()) ||
        !reply.WriteInt32(bufferSize) ||
        !reply.WriteParcelable(&nxtKey)) {
        ZLOGW("write buffer size failed.");
        return -1;
    }
    for (const auto &item : keyList) {
        if (!reply.WriteParcelable(&item)) {
            ZLOGW("write item failed.");
            return -1;
        }
    }
    return 0;
}

int32_t KvStoreSnapshotImplStub::GetEntriesOnRemote(MessageParcel &data, MessageParcel &reply)
{
    // struct of returned reply:
    // buffer:  | status | entryLength | rawdatasize | (sptr<Key>)nextkey |
    // rawData: ( | keyLen | key | valueLen | value | ){entryLength}
    sptr<Key> keyPrefix = data.ReadParcelable<Key>();
    sptr<Key> nextKey = data.ReadParcelable<Key>();
    if (keyPrefix == nullptr) {
        ZLOGW("keyPrefix is null. return.");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    if (nextKey == nullptr) {
        ZLOGW("nextKey is null. return.");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        return 0;
    }
    std::vector<Entry> entryList;
    Key nxtKey;
    Status statusTmp;
    GetEntries(*keyPrefix, *nextKey, [&](Status status, const std::vector<Entry> &entries, const Key &key) {
        statusTmp = status;
        entryList = std::move(entries);
        nxtKey = key;
    });
    int bufferSize = GetTotalEntriesSize(entryList);
    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        return WriteEntriesParcelable(reply, statusTmp, entryList, bufferSize, nxtKey);
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

    if (!reply.WriteInt32(static_cast<int>(statusTmp)) ||
        !reply.WriteInt32(entryList.size()) ||
        !reply.WriteInt32(bufferSize) ||
        !reply.WriteParcelable(&nxtKey)) {
        ZLOGW("write entries failed.");
        return -1;
    }
    int bufferLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    for (const auto &item : entryList) {
        if (!item.key.WriteToBuffer(cursor, bufferLeftSize) ||
            !item.value.WriteToBuffer(cursor, bufferLeftSize)) {
            ZLOGW("write to buffer failed");
            return -1;
        }
    }
    if (!reply.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGW("write rawData failed");
        return -1;
    }
    return 0;
}
int32_t KvStoreSnapshotImplStub::GetKeysRemote(MessageParcel &data, MessageParcel &reply)
{
    // struct of returned reply:
    // buffer:  | status | keyListLength | rawdatasize | (sptr<Key>)nextkey |
    // rawData: ( | keyLen | key | ){keyListLength}
    sptr<Key> keyPrefix = data.ReadParcelable<Key>();
    sptr<Key> nextKey = data.ReadParcelable<Key>();
    if (keyPrefix == nullptr) {
        ZLOGW("keyPrefix is null");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        if (!reply.WriteParcelable(nullptr)) {
            return -1;
        }
        return 0;
    }
    if (nextKey == nullptr) {
        ZLOGW("nextKey is null. return.");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        if (!reply.WriteParcelable(nullptr)) {
            return -1;
        }
        return 0;
    }
    std::vector<Key> keyList;
    Key nxtKey;
    Status statusTmp;
    GetKeys(*keyPrefix, *nextKey, [&](Status status, const std::vector<Key> &keys, const Key &key) {
        statusTmp = status;
        keyList = std::move(keys);
        nxtKey = key;
    });
    int bufferSize = GetTotalkeysSize(keyList);

    if (bufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        return WritekeysParcelable(reply, statusTmp, keyList, bufferSize, nxtKey);
    }
    ZLOGI("getting large key set");
    if (bufferSize > static_cast<int>(reply.GetRawDataCapacity())) {
        ZLOGW("bufferSize %d larger than message parcel limit", bufferSize);
        if (!reply.WriteInt32(static_cast<int>(Status::ERROR)) ||
            !reply.WriteParcelable(&nxtKey)) {
            ZLOGW("write status failed.");
            return -1;
        }
        return 0;
    }
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(
            new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("alloc memory failed(buffer is null). perhaps low on memory.");
        if (!reply.WriteInt32(static_cast<int>(Status::ERROR)) ||
            !reply.WriteParcelable(&nxtKey)) {
            ZLOGW("write nxtkey failed.");
            return -1;
        }
        return 0;
    }
    if (!reply.WriteInt32(static_cast<int>(statusTmp)) ||
        !reply.WriteInt32(keyList.size()) ||
        !reply.WriteInt32(bufferSize) ||
        !reply.WriteParcelable(&nxtKey)) {
        ZLOGW("write meta failed.");
        return -1;
    }

    int bufferLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    for (const auto &key : keyList) {
        if (!key.WriteToBuffer(cursor, bufferLeftSize)) {
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
int32_t KvStoreSnapshotImplStub::GetRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<Key> key = data.ReadParcelable<Key>();
    if (key == nullptr) {
        ZLOGW("key is null");
        if (!reply.WriteInt32(static_cast<int>(Status::INVALID_ARGUMENT))) {
            return -1;
        }
        if (!reply.WriteInt32(0)) {
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
            ZLOGW("write meta failed.");
            return -1;
        }
        return 0;
    }
    ZLOGI("getting large entry");
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(
            new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGW("buffer is null");
        if (!reply.WriteInt32(static_cast<int>(Status::ILLEGAL_STATE)) ||
            !reply.WriteInt32(0)) {
            ZLOGW("write state failed.");
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
        ZLOGW("write rawData failed.");
        return -1;
    }
    return 0;
}

int32_t KvStoreSnapshotImplStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                                 MessageParcel &reply, MessageOption &option)
{
    ZLOGD("%d", code);
    std::u16string descriptor = KvStoreSnapshotImplStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    if (!reply.SetMaxCapacity(Constant::MAX_IPC_CAPACITY)) {
        return -1;
    }
    switch (code) {
        case GETENTRIES: {
            return GetEntriesOnRemote(data, reply);
        }
        case GETKEYS: {
            return GetKeysRemote(data, reply);
        }
        case GET: {
            return GetRemote(data, reply);
        }
        default:
            if (!reply.WriteInt32((int32_t)Status::ERROR)) {
                return -1;
            }
            return 0;
    }
}
}  // namespace DistributedKv
}  // namespace OHOS
