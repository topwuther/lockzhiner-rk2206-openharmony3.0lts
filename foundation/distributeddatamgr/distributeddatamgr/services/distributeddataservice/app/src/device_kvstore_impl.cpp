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

#define LOG_TAG "DeviceKvStoreImpl"

#include "device_kvstore_impl.h"
#include <regex>
#include "kvstore_utils.h"
#include "constant.h"
#include "log_print.h"

namespace OHOS::DistributedKv {
using namespace AppDistributedKv;

DeviceKvStoreImpl::DeviceKvStoreImpl(const KvStoreParams &params, DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate)
    : SingleKvStoreImpl(params.options, params.deviceAccountId, params.appId, params.storeId,
    params.appDirectory, kvStoreNbDelegate), params_(params)
{}

DeviceKvStoreImpl::~DeviceKvStoreImpl()
{}

Status DeviceKvStoreImpl::Get(const Key &key, Value &value)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::Get(key, value);
    }

    std::vector<uint8_t> tmpkey;
    Status status = DeleteKeyPrefix(key, tmpkey);
    if (status != Status::SUCCESS) {
        ZLOGE("get deviceid failed.");
        return status;
    }
    KeyEncap ke {static_cast<int>(localDeviceId_.length())};
    tmpkey.insert(tmpkey.end(), std::begin(ke.byteLen), std::end(ke.byteLen));
    Key decorateKey(tmpkey);
    return SingleKvStoreImpl::Get(decorateKey, value);
}

Status DeviceKvStoreImpl::Put(const Key &key, const Value &value)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::Put(key, value);
    }

    std::vector<uint8_t> tmpkey;
    AddKeyPrefixAndSuffix(key, tmpkey);
    Key decorateKey(tmpkey);
    return SingleKvStoreImpl::Put(decorateKey, value);
}

Status DeviceKvStoreImpl::Delete(const Key &key)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::Delete(key);
    }

    std::vector<uint8_t> tmpkey;
    AddKeyPrefixAndSuffix(key, tmpkey);
    Key decorateKey(tmpkey);
    return SingleKvStoreImpl::Delete(decorateKey);
}

Status DeviceKvStoreImpl::GetEntries(const Key &prefixKey, std::vector<Entry> &entries)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::GetEntries(prefixKey, entries);
    }

    std::vector<uint8_t> tmpkey;
    Status status = DeleteKeyPrefix(prefixKey, tmpkey);
    if (status == Status::KEY_NOT_FOUND) {
        ZLOGE("DeleteKeyPrefix deviceId is invalid.");
        return Status::SUCCESS;
    }

    if (status != Status::SUCCESS) {
        ZLOGE("DeleteKeyPrefix failed.");
        return Status::ERROR;
    }
    Key decorateKey(tmpkey);
    std::vector<Entry> tmpEntries;
    Status ret = SingleKvStoreImpl::GetEntries(decorateKey, tmpEntries);
    if (ret != Status::SUCCESS) {
        ZLOGE("GetEntries status=%d", static_cast<int>(ret));
        return ret;
    }

    for (const auto &entry : tmpEntries) {
        std::vector<uint8_t> tmpkey;
        DeletePrefixAndSuffix(entry.key, tmpkey);
        Key retKey(tmpkey);
        Entry en;
        en.key = retKey;
        en.value = entry.value;
        entries.push_back(en);
    }
    return ret;
}

Status DeviceKvStoreImpl::RemoveDeviceData(const std::string &device)
{
    return SingleKvStoreImpl::RemoveDeviceData(device);
}

void DeviceKvStoreImpl::GetResultSet(const Key &prefixKey,
                                     std::function<void(Status, sptr<IKvStoreResultSet>)> callback)
{
    if (!params_.deviceCoordinate) {
        SingleKvStoreImpl::GetResultSet(prefixKey, callback);
        return;
    }

    std::vector<uint8_t> tmpkey;
    Status status = DeleteKeyPrefix(prefixKey, tmpkey);
    if (status != Status::SUCCESS) {
        ZLOGE("DeleteKeyPrefix failed.");
        callback(status, nullptr);
        return;
    }
    Key decorateKey(tmpkey);
    SingleKvStoreImpl::GetResultSet(decorateKey, callback, true);
}

Status DeviceKvStoreImpl::SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::SubscribeKvStore(subscribeType, observer);
    }

    return SingleKvStoreImpl::SubscribeKvStore(subscribeType, observer, true);
}

Status DeviceKvStoreImpl::PutBatch(const std::vector<Entry> &entries)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::PutBatch(entries);
    }

    std::vector<Entry> tmpEntries = entries;
    for (auto &entry : tmpEntries) {
        std::vector<uint8_t> tmpkey;
        AddKeyPrefixAndSuffix(entry.key, tmpkey);
        Key decorateKey(tmpkey);
        entry.key = decorateKey;
    }

    return SingleKvStoreImpl::PutBatch(tmpEntries);
}

Status DeviceKvStoreImpl::DeleteBatch(const std::vector<Key> &keys)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::DeleteBatch(keys);
    }

    std::vector<Key> tmpKeys = keys;
    for (auto &key : tmpKeys) {
        std::vector<uint8_t> tmpkey;
        AddKeyPrefixAndSuffix(key, tmpkey);
        Key decorateKey(tmpkey);
        key = decorateKey;
    }
    return SingleKvStoreImpl::DeleteBatch(tmpKeys);
}

Status DeviceKvStoreImpl::GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries)
{
    if (!params_.deviceCoordinate) {
        return SingleKvStoreImpl::GetEntriesWithQuery(query, entries);
    }

    Status ret = SingleKvStoreImpl::GetEntriesWithQuery(query, entries);
    if (ret != Status::SUCCESS) {
        return ret;
    }

    for (auto &entry : entries) {
        std::vector<uint8_t> tmpkey;
        DeletePrefixAndSuffix(entry.key, tmpkey);
        Key retKey(tmpkey);
        entry.key = retKey;
    }
    return ret;
}

void DeviceKvStoreImpl::GetResultSetWithQuery(const std::string &query,
                                              std::function<void(Status, sptr<IKvStoreResultSet>)> callback)
{
    if (!params_.deviceCoordinate) {
        SingleKvStoreImpl::GetResultSetWithQuery(query, callback);
        return;
    }

    SingleKvStoreImpl::GetResultSetWithQuery(query, callback, true);
}

std::string DeviceKvStoreImpl::localDeviceId_;
std::string DeviceKvStoreImpl::GetLocalDeviceId()
{
    if (!localDeviceId_.empty()) {
        return localDeviceId_;
    }

    localDeviceId_ = KvStoreUtils::GetProviderInstance().GetLocalDevice().deviceId;
    return localDeviceId_;
}

Status DeviceKvStoreImpl::DeleteKeyPrefix(const Key &in, std::vector<uint8_t> &out)
{
    GetLocalDeviceId();
    // |head length|nodeid ID| original ID|
    // |----4------|---------|------------|
    // indicate head len is 4; this key is from java;
    size_t deviceIdPrefixLen = 4;
    auto inData = in.Data();
    if (inData.size() < deviceIdPrefixLen) {
        ZLOGE("inData length is error.");
        return Status::ERROR;
    }
    std::string deviceIdLenStr(inData.begin(), inData.begin() + deviceIdPrefixLen);
    std::regex pattern("^[0-9]*$");
    if (!std::regex_match(deviceIdLenStr, pattern)) {
        ZLOGE("device id length is error.");
        return Status::ERROR;
    }
    std::string nodeid(inData.begin() + deviceIdPrefixLen, inData.begin() + deviceIdPrefixLen + localDeviceId_.size());
    std::string deviceUuID = KvStoreUtils::GetProviderInstance().GetUuidByNodeId(nodeid);
    if (deviceUuID.empty()) {
        ZLOGE("device uuid is empty.");
        return Status::KEY_NOT_FOUND;
    }
    out.insert(out.end(), deviceUuID.begin(), deviceUuID.end());

    std::string original (inData.begin() + deviceIdPrefixLen + localDeviceId_.size(), inData.end());
    out.insert(out.end(), original.begin(), original.end());
    return Status::SUCCESS;
}

// need to delete prefix UDID and suffix UDID length;
void DeviceKvStoreImpl::DeletePrefixAndSuffix(const Key &in, std::vector<uint8_t> &out)
{
    GetLocalDeviceId();
    if (localDeviceId_.empty()) {
        ZLOGE("Get deviceid failed.");
        return;
    }
    out.insert(out.end(), in.Data().begin() + localDeviceId_.size(), in.Data().end() - sizeof(int));
}

// need to add UDID to key prefix, add UDID length to key suffix to adapter 1.0;
// | UDID | KEY | UDID len |
bool DeviceKvStoreImpl::AddKeyPrefixAndSuffix(const Key &in, std::vector<uint8_t> &out)
{
    GetLocalDeviceId();
    if (localDeviceId_.empty()) {
        ZLOGE("Get deviceid failed.");
        return false;
    }
    // device ID
    out.insert(out.end(), localDeviceId_.begin(), localDeviceId_.end());
    // the original key
    out.insert(out.end(), in.Data().begin(), in.Data().end());
    // add device ID length to the tail, which include 4 uint8_t;
    KeyEncap ke {static_cast<int>(localDeviceId_.length())};
    out.insert(out.end(), std::begin(ke.byteLen), std::end(ke.byteLen));
    return true;
}
}
