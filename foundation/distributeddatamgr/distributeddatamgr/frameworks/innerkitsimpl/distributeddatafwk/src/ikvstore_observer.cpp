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

#define LOG_TAG "KvStoreObserverProxy"

#include "ikvstore_observer.h"
#include <chrono>
#include "constant.h"
#include "log_print.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedKv {
using namespace std::chrono;

enum {
    ONCHANGE,
};

KvStoreObserverProxy::KvStoreObserverProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IKvStoreObserver>(impl)
{}

int64_t GetBufferSize(const std::list<Entry> &entries)
{
    int64_t bufferSize = 0;
    for (const auto &item : entries) {
        bufferSize += item.key.RawSize() + item.value.RawSize();
    }
    return bufferSize;
}

bool WriteEntryToParcelByBuf(MessageParcel &data, const int64_t &bufferSize, const std::list<Entry> &list)
{
    std::unique_ptr<uint8_t, void(*)(uint8_t *)> buffer(new uint8_t[bufferSize], [](uint8_t *ptr) { delete[] ptr; });
    if (buffer == nullptr) {
        ZLOGE("buffer is null");
        return false;
    }
    int bufLeftSize = bufferSize;
    uint8_t *cursor = buffer.get();
    for (const auto &item : list) {
        if (!item.key.WriteToBuffer(cursor, bufLeftSize) ||
            !item.value.WriteToBuffer(cursor, bufLeftSize)) {
            ZLOGE("write item to buff failed");
            return false;
        }
    }
    if (!data.WriteRawData(buffer.get(), bufferSize)) {
        ZLOGE("bigDataOnchange write RawData from buff failed");
        return false;
    }
    return true;
}

bool WriteListToParcelByBuf(MessageParcel &data, const int64_t &bufferSize, const std::list<Entry> &list)
{
    if (!data.WriteInt32(list.size()) ||
        !data.WriteInt32(bufferSize)) {
        ZLOGE("write entriesLen or bufferSize fails");
        return false;
    }
    if (bufferSize == 0) {
        return true;
    }

    if (!WriteEntryToParcelByBuf(data, bufferSize, list)) {
        ZLOGE("bigDataOnchange write RawData to parcel failed");
        return false;
    }
    return true;
}

void KvStoreObserverProxy::OnChange(const ChangeNotification &changeNotification, sptr<IKvStoreSnapshotImpl> snapshot)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(KvStoreObserverProxy::GetDescriptor())) {
        ZLOGE("write descriptor failed");
        return;
    }
    int64_t insertBufferSize = GetBufferSize(changeNotification.GetInsertEntries());
    int64_t updateBufferSize = GetBufferSize(changeNotification.GetUpdateEntries());
    int64_t deleteBufferSize = GetBufferSize(changeNotification.GetDeleteEntries());
    int64_t totalBufferSize = insertBufferSize + updateBufferSize + deleteBufferSize + sizeof(bool);
    if (!data.WriteInt32(totalBufferSize)) {
        ZLOGE("Write ChangeNotification buffer size to parcel failed.");
        return;
    }
    ZLOGD("I(%lld) U(%lld) D(%lld) T(%lld)", static_cast<long long>(insertBufferSize),
          static_cast<long long>(updateBufferSize), static_cast<long long>(deleteBufferSize),
          static_cast<long long>(totalBufferSize));
    if (totalBufferSize < Constant::SWITCH_RAW_DATA_SIZE) {
        if (!data.WriteParcelable(&changeNotification)) {
            ZLOGW("Write ChangeNotification to parcel failed.");
            return;
        }
    } else {
        if (!WriteListToParcelByBuf(data, insertBufferSize, changeNotification.GetInsertEntries()) ||
            !WriteListToParcelByBuf(data, updateBufferSize, changeNotification.GetUpdateEntries()) ||
            !WriteListToParcelByBuf(data, deleteBufferSize, changeNotification.GetDeleteEntries()) ||
            !data.WriteString(changeNotification.GetDeviceId()) ||
            !data.WriteBool(changeNotification.IsClear())) {
            ZLOGE("WriteChangeList to Parcel by buffer failed");
            return;
        }
    }

    if (snapshot != nullptr && !data.WriteRemoteObject(snapshot->AsObject().GetRefPtr())) {
        ZLOGE("write strong parcel failed.");
        return;
    }

    MessageOption mo { MessageOption::TF_WAIT_TIME };
    int error = Remote()->SendRequest(ONCHANGE, data, reply, mo);
    if (error != 0) {
        ZLOGE("SendRequest failed, error %d", error);
    }
}

bool ReadFromBuff(MessageParcel &data, const int &len, const int &bufferSize, std::list<Entry> &entries)
{
    const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.ReadRawData(bufferSize));
    if (buffer == nullptr) {
        ZLOGE("new buffer filed");
        return false;
    }
    int bufferLeftSize = bufferSize;
    const uint8_t *cursor = buffer;
    Entry entry;
    for (int i = 0; i < len; i++) {
        if (!entry.key.ReadFromBuffer(cursor, bufferLeftSize) ||
            !entry.value.ReadFromBuffer(cursor, bufferLeftSize)) {
            ZLOGE("read key and value from buff failed");
            return false;
        }
        entries.push_back(std::move(entry));
    }
    return true;
}

bool ReadListFromBuf(MessageParcel &data, std::list<Entry> &entries)
{
    int len = data.ReadInt32();
    if (len < 0) {
        ZLOGE("read onChangeLen failed len %d", len);
        return false;
    }
    int bufferSize = data.ReadInt32();
    if (bufferSize == 0) {
        return true;
    }
    if (!ReadFromBuff(data, len, bufferSize, entries)) {
        ZLOGE("bigDataOnchange read buff from parcel filed");
        return false;
    }
    return true;
}

int32_t KvStoreObserverStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                             MessageParcel &reply, MessageOption &option)
{
    ZLOGD("%d", code);
    std::u16string descriptor = KvStoreObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ZLOGE("local descriptor is not equal to remote");
        return -1;
    }
    switch (code) {
        case ONCHANGE: {
            const int errorResult = -1;
            int totalBuffSize = data.ReadInt32();
            if (totalBuffSize < Constant::SWITCH_RAW_DATA_SIZE) {
                sptr<ChangeNotification> changeNotification = data.ReadParcelable<ChangeNotification>();
                if (changeNotification == nullptr) {
                    ZLOGE("changeNotification is nullptr");
                    return errorResult;
                }
                sptr<IRemoteObject> remote = data.ReadRemoteObject();
                if (remote != nullptr) {
                    sptr<IKvStoreSnapshotImpl> kvStoreSnapshotProxy = iface_cast<IKvStoreSnapshotImpl>(remote);
                    OnChange(*changeNotification, std::move(kvStoreSnapshotProxy));
                } else {
                    OnChange(*changeNotification, nullptr);
                }
            } else {
                std::list<Entry> insertEntries;
                bool result = ReadListFromBuf(data, insertEntries);
                if (!result) {
                    ZLOGE("read insertList from buff filed");
                    return errorResult;
                }

                std::list<Entry> updateEntries;
                result = ReadListFromBuf(data, updateEntries);
                if (!result) {
                    ZLOGE("read updateList from buff filed");
                    return errorResult;
                }

                std::list<Entry> deleteEntries;
                result = ReadListFromBuf(data, deleteEntries);
                if (!result) {
                    ZLOGE("read deleteList from buff filed");
                    return errorResult;
                }

                std::string deviceId = data.ReadString();
                bool isClear = data.ReadBool();
                ChangeNotification changeNotification(insertEntries, updateEntries, deleteEntries, deviceId, isClear);
                sptr<IRemoteObject> remote = data.ReadRemoteObject();
                if (remote != nullptr) {
                    sptr<IKvStoreSnapshotImpl> kvStoreSnapshotProxy = iface_cast<IKvStoreSnapshotImpl>(remote);
                    OnChange(changeNotification, std::move(kvStoreSnapshotProxy));
                } else {
                    ZLOGD("read kvstoreSnapshot is nullptr.");
                    OnChange(changeNotification, nullptr);
                }
            }
            return 0;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
}  // namespace DistributedKv
}  // namespace OHOS
