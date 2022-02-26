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

#include "virtual_single_ver_sync_db_Interface.h"

#include <sys/time.h>
#include <algorithm>
#include <thread>

#include "db_errno.h"
#include "log_print.h"
#include "meta_data.h"
#include "securec.h"
#include "generic_single_ver_kv_entry.h"

namespace DistributedDB {
int VirtualSingleVerSyncDBInterface::GetInterfaceType() const
{
    return SYNC_SVD;
}

void VirtualSingleVerSyncDBInterface::IncRefCount()
{
}

void VirtualSingleVerSyncDBInterface::DecRefCount()
{
}

std::vector<uint8_t> VirtualSingleVerSyncDBInterface::GetIdentifier() const
{
    std::vector<uint8_t> identifier;
    return identifier;
}

int VirtualSingleVerSyncDBInterface::GetMetaData(const Key &key, Value &value) const
{
    auto iter = metadata_.find(key);
    if (iter != metadata_.end()) {
        value = iter->second;
        return E_OK;
    }
    return -E_NOT_FOUND;
}

int VirtualSingleVerSyncDBInterface::PutMetaData(const Key &key, const Value &value)
{
    metadata_[key] = value;
    return E_OK;
}

int VirtualSingleVerSyncDBInterface::GetAllMetaKeys(std::vector<Key> &keys) const
{
    for (auto iter = metadata_.begin(); iter != metadata_.end(); ++iter) {
        keys.push_back(iter->first);
    }
    LOGD("GetAllMetaKeys size %d", keys.size());
    return E_OK;
}

int VirtualSingleVerSyncDBInterface::GetSyncData(TimeStamp begin, TimeStamp end, std::vector<DataItem> &dataItems,
    ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const
{
    return -E_NOT_SUPPORT;
}

int VirtualSingleVerSyncDBInterface::GetSyncDataNext(std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken,
    const DataSizeSpecInfo &dataSizeInfo) const
{
    return -E_NOT_SUPPORT;
}

void VirtualSingleVerSyncDBInterface::ReleaseContinueToken(ContinueToken& continueStmtToken) const
{
    return;
}

int VirtualSingleVerSyncDBInterface::PutSyncData(std::vector<DataItem>& dataItems,
    const std::string &deviceName)
{
    return -E_NOT_SUPPORT;
}

SchemaObject VirtualSingleVerSyncDBInterface::GetSchemaInfo() const
{
    return schemaObj_;
}

bool VirtualSingleVerSyncDBInterface::CheckCompatible(const std::string& schema) const
{
    if (schema_.empty() && schema.empty()) {
        return true;
    }
    return (schemaObj_.CompareAgainstSchemaString(schema) == -E_SCHEMA_EQUAL_EXACTLY);
}

void VirtualSingleVerSyncDBInterface::ReleaseKvEntry(const SingleVerKvEntry *entry)
{
    delete entry;
    entry = nullptr;
}

int VirtualSingleVerSyncDBInterface::PutData(const Key &key, const Value &value, const TimeStamp &time, int flag)
{
    VirtualDataItem item;
    item.key = key;
    item.value = value;
    item.timeStamp = time;
    item.writeTimeStamp = time;
    item.flag = flag;
    item.isLocal = true;
    dbData_.push_back(item);
    return E_OK;
}

void VirtualSingleVerSyncDBInterface::GetMaxTimeStamp(TimeStamp& stamp) const
{
    for (auto iter = dbData_.begin(); iter != dbData_.end(); ++iter) {
        if (stamp < iter->writeTimeStamp) {
            stamp = iter->writeTimeStamp;
        }
    }
    LOGD("VirtualSingleVerSyncDBInterface::GetMaxTimeStamp time = %llu", stamp);
}

int VirtualSingleVerSyncDBInterface::RemoveDeviceData(const std::string &deviceName, bool isNeedNotify)
{
    return E_OK;
}

int VirtualSingleVerSyncDBInterface::GetSyncData(const Key &key, VirtualDataItem &item)
{
    auto iter = std::find_if(dbData_.begin(), dbData_.end(),
        [key](VirtualDataItem item) { return item.key == key; });
    if (iter != dbData_.end()) {
        item.key = iter->key;
        item.value = iter->value;
        item.timeStamp = iter->timeStamp;
        item.writeTimeStamp = iter->writeTimeStamp;
        item.flag = iter->flag;
        item.isLocal = iter->isLocal;
        return E_OK;
    }
    return -E_NOT_FOUND;
}

int VirtualSingleVerSyncDBInterface::GetSyncData(TimeStamp begin, TimeStamp end,
    std::vector<SingleVerKvEntry *> &entries, ContinueToken &continueStmtToken,
    const DataSizeSpecInfo &dataSizeInfo) const
{
    std::vector<VirtualDataItem> dataItems;
    int errCode = GetSyncData(begin, end, dataSizeInfo.blockSize, dataItems, continueStmtToken);
    if (errCode != E_OK) {
        LOGE("[VirtualSingleVerSyncDBInterface][GetSyncData] GetSyncData failed err %d", errCode);
        return errCode;
    }
    for (auto item : dataItems) {
        GenericSingleVerKvEntry *entry = new (std::nothrow) GenericSingleVerKvEntry();
        if (entry == nullptr) {
            LOGE("Create entry failed.");
            errCode = -E_OUT_OF_MEMORY;
            break;
        }
        DataItem storageItem;
        storageItem.key = item.key;
        storageItem.value = item.value;
        storageItem.flag = item.flag;
        storageItem.timeStamp = item.timeStamp;
        storageItem.writeTimeStamp = item.writeTimeStamp;
        entry->SetEntryData(std::move(storageItem));
        entries.push_back(entry);
    }
    if (errCode != E_OK) {
        for (auto kvEntry : entries) {
            delete kvEntry;
            kvEntry = nullptr;
        }
        entries.clear();
    }
    return errCode;
}

int VirtualSingleVerSyncDBInterface::GetSyncDataNext(std::vector<SingleVerKvEntry *> &entries,
    ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const
{
    if (continueStmtToken == nullptr) {
        return -E_NOT_SUPPORT;
    }
    return 0;
}

int VirtualSingleVerSyncDBInterface::PutSyncData(const std::vector<SingleVerKvEntry *> &entries,
    const std::string &deviceName)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(saveDataDelayTime_));
    std::vector<VirtualDataItem> dataItems;
    for (auto kvEntry : entries) {
        auto genricKvEntry = static_cast<GenericSingleVerKvEntry *>(kvEntry);
        VirtualDataItem item;
        genricKvEntry->GetKey(item.key);
        genricKvEntry->GetValue(item.value);
        item.timeStamp = genricKvEntry->GetTimestamp();
        item.writeTimeStamp = genricKvEntry->GetWriteTimestamp();
        item.flag = genricKvEntry->GetFlag();
        item.isLocal = false;
        dataItems.push_back(item);
    }
    return PutSyncData(dataItems, deviceName);
}

int VirtualSingleVerSyncDBInterface::GetSyncData(TimeStamp begin, TimeStamp end, uint32_t blockSize,
    std::vector<VirtualDataItem> &dataItems, ContinueToken &continueStmtToken) const
{
    for (const auto &data : dbData_) {
        if (data.isLocal) {
            if (data.writeTimeStamp >= begin && data.writeTimeStamp < end) {
                dataItems.push_back(data);
            }
        }
    }
    continueStmtToken = nullptr;
    LOGD("dataItems size %d", dataItems.size());
    return E_OK;
}

void VirtualSingleVerSyncDBInterface::SetSaveDataDelayTime(uint64_t milliDelayTime)
{
    saveDataDelayTime_ = milliDelayTime;
}

int VirtualSingleVerSyncDBInterface::GetSyncDataNext(std::vector<VirtualDataItem>& dataItems,
    uint32_t blockSize, ContinueToken& continueStmtToken) const
{
    if (continueStmtToken == nullptr) {
        return -E_NOT_SUPPORT;
    }
    return 0;
}

int VirtualSingleVerSyncDBInterface::PutSyncData(std::vector<VirtualDataItem>& dataItems,
    const std::string &deviceName)
{
    for (auto iter = dataItems.begin(); iter != dataItems.end(); ++iter) {
        LOGD("PutSyncData");
        auto dbDataIter = std::find_if(dbData_.begin(), dbData_.end(),
            [iter](VirtualDataItem item) { return item.key == iter->key; });
        if ((dbDataIter != dbData_.end()) && (dbDataIter->writeTimeStamp < iter->writeTimeStamp)) {
            // if has conflict, compare writeTimeStamp
            LOGI("conflict data time local %llu, remote %llu", dbDataIter->writeTimeStamp, iter->writeTimeStamp);
            dbDataIter->key = iter->key;
            dbDataIter->value = iter->value;
            dbDataIter->timeStamp = iter->timeStamp;
            dbDataIter->writeTimeStamp = iter->writeTimeStamp;
            dbDataIter->flag = iter->flag;
            dbDataIter->isLocal = false;
        } else {
            LOGI("PutSyncData, use remote data %llu", iter->timeStamp);
            VirtualDataItem dataItem;
            dataItem.key = iter->key;
            dataItem.value = iter->value;
            dataItem.timeStamp = iter->timeStamp;
            dataItem.writeTimeStamp = iter->writeTimeStamp;
            dataItem.flag = iter->flag;
            dataItem.isLocal = false;
            dbData_.push_back(dataItem);
        }
    }
    return E_OK;
}

void VirtualSingleVerSyncDBInterface::SetSchemaInfo(const std::string& schema)
{
    schema_ = schema;
    SchemaObject emptyObj;
    schemaObj_ = emptyObj;
    schemaObj_.ParseFromSchemaString(schema);
}

const KvDBProperties &VirtualSingleVerSyncDBInterface::GetDbProperties() const
{
    return properties_;
}

int VirtualSingleVerSyncDBInterface::GetSecurityOption(SecurityOption &option) const
{
    option = secOption_;
    return E_OK;
}

bool VirtualSingleVerSyncDBInterface::IsReadable() const
{
    return true;
}

void VirtualSingleVerSyncDBInterface::SetSecurityOption(SecurityOption &option)
{
    secOption_ = option;
}

void VirtualSingleVerSyncDBInterface::NotifyRemotePushFinished(const std::string &targetId) const
{
}
}  // namespace DistributedDB
