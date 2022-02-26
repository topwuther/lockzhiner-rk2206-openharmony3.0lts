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

#define LOG_TAG "KvStoreSnapshotClient"

#include "constant.h"
#include "dds_trace.h"
#include "kvstore_snapshot_client.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreSnapshotClient::KvStoreSnapshotClient() : kvStoreSnapshotProxy_(nullptr)
{}

KvStoreSnapshotClient::KvStoreSnapshotClient(sptr<IKvStoreSnapshotImpl> kvStoreSnapshotProxy)
    : kvStoreSnapshotProxy_(std::move(kvStoreSnapshotProxy))
{
    ZLOGI("construct");
}

KvStoreSnapshotClient::~KvStoreSnapshotClient()
{
    ZLOGI("destruct");
}

void KvStoreSnapshotClient::GetEntries(const Key &prefixKey, const Key &nextKey,
                                       std::function<void(Status, std::vector<Entry> &, const Key &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::vector<Entry> entries;
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid prefixKey.");
        callback(Status::INVALID_ARGUMENT, entries, nextKey);
        return;
    }
    keyData = Constant::TrimCopy<std::vector<uint8_t>>(nextKey.Data());
    if (keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid nextKey.");
        callback(Status::INVALID_ARGUMENT, entries, nextKey);
        return;
    }
    if (kvStoreSnapshotProxy_ != nullptr) {
        kvStoreSnapshotProxy_->GetEntries(prefixKey, nextKey, callback);
        return;
    }

    ZLOGE("snapshot proxy is nullptr.");
    callback(Status::SERVER_UNAVAILABLE, entries, nextKey);
}

void KvStoreSnapshotClient::GetEntries(const Key &prefixKey, std::function<void(Status, std::vector<Entry> &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::vector<Entry> allEntries;
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid prefixKey.");
        callback(Status::INVALID_ARGUMENT, allEntries);
        return;
    }
    if (kvStoreSnapshotProxy_ == nullptr) {
        ZLOGE("snapshot proxy is nullptr.");
        callback(Status::SERVER_UNAVAILABLE, allEntries);
        return;
    }
    Key startKey = "";
    Status allStatus = Status::ERROR;
    do {
        kvStoreSnapshotProxy_->GetEntries(prefixKey, startKey,
            [&allStatus, &allEntries, &startKey](Status stat, std::vector<Entry> &entries, Key next) {
                allStatus = stat;
                if (stat != Status::SUCCESS) {
                    return;
                }
                for (const auto &entry : entries) {
                    allEntries.push_back(entry);
                }
                startKey = next;
                if (entries.empty()) {
                    startKey = "";
                }
            });
    } while (allStatus == Status::SUCCESS && startKey.ToString() != "");
    if (allStatus != Status::SUCCESS) {
        ZLOGW("Error occurs during GetEntries.");
        allEntries.clear();
    }
    callback(allStatus, allEntries);
}

void KvStoreSnapshotClient::GetKeys(const Key &prefixKey, const Key &nextKey,
                                    std::function<void(Status, std::vector<Key> &, const Key &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::vector<Key> keys;
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid prefixKey.");
        callback(Status::INVALID_ARGUMENT, keys, nextKey);
        return;
    }
    keyData = Constant::TrimCopy<std::vector<uint8_t>>(nextKey.Data());
    if (keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid nextKey.");
        callback(Status::INVALID_ARGUMENT, keys, nextKey);
        return;
    }
    if (kvStoreSnapshotProxy_ != nullptr) {
        kvStoreSnapshotProxy_->GetKeys(prefixKey, nextKey, callback);
        return;
    }
    ZLOGE("snapshot proxy is nullptr.");
    callback(Status::SERVER_UNAVAILABLE, keys, nextKey);
}

void KvStoreSnapshotClient::GetKeys(const Key &prefixKey, std::function<void(Status, std::vector<Key> &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::vector<Key> allKeys;
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid prefixKey.");
        callback(Status::INVALID_ARGUMENT, allKeys);
        return;
    }
    if (kvStoreSnapshotProxy_ == nullptr) {
        ZLOGE("snapshot proxy is nullptr.");
        callback(Status::SERVER_UNAVAILABLE, allKeys);
        return;
    }
    Key startKey = "";
    Status allStatus = Status::ERROR;
    do {
        kvStoreSnapshotProxy_->GetKeys(prefixKey, startKey, [&](Status stat, std::vector<Key> &keys, Key next) {
            allStatus = stat;
            if (stat != Status::SUCCESS) {
                return;
            }
            for (const auto &key : keys) {
                allKeys.push_back(key);
            }
            startKey = next;
            if (keys.empty()) {
                startKey = "";
            }
        });
    } while (allStatus == Status::SUCCESS && startKey.ToString() != "");
    if (allStatus != Status::SUCCESS) {
        ZLOGW("Error occurs during GetKeys.");
        allKeys.clear();
    }
    callback(allStatus, allKeys);
}

Status KvStoreSnapshotClient::Get(const Key &key, Value &value)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreSnapshotProxy_ != nullptr) {
        return kvStoreSnapshotProxy_->Get(key, value);
    }
    ZLOGE("snapshot proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

sptr<IKvStoreSnapshotImpl> KvStoreSnapshotClient::GetkvStoreSnapshotProxy()
{
    return kvStoreSnapshotProxy_;
}
}  // namespace DistributedKv
}  // namespace OHOS
