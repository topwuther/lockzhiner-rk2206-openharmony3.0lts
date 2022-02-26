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

#ifndef KVSTORE_SNAPSHOT_H
#define KVSTORE_SNAPSHOT_H

#include "types.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreSnapshot {
public:
    KVSTORE_API KvStoreSnapshot() = default;

    KVSTORE_API virtual ~KvStoreSnapshot()
    {}

    // Deprecated. use the GetEntries interface without nextKey as parameter instead.
    // Get a list of entries from kvstore by keyPrefix,
    // key length must be less than 1024,
    // GetEntries will return all entries whose Key.StartsWith(keyPrefix) is true,
    // if keyPrefix is empty, all entries in the kvstore will be returned.
    // if data size is larger than 800k, data may be transported by several times. each time callback will give you the
    // first key of the not-transported part. You can use this key as nextKey to get next part of data. When you get an
    // empty nextKey, It means all data has been transported.
    // parameters:
    // prefixKey: perfix key to search
    // nextKey: The first key to start in this search.
    // callback: all entries satisfied perfixKey, status of this call and the first key of the next part of data.
    [[deprecated]]
    KVSTORE_API virtual void GetEntries(const Key &prefixKey, const Key &nextKey,
                                        std::function<void(Status, std::vector<Entry> &, const Key &)> callback) = 0;

    // Get a list of entries from kvstore by keyPrefix,
    // key length must be less than 1024,
    // GetEntries will return all entries whose Key.StartsWith(keyPrefix) is true,
    // if keyPrefix is empty, all entries in the kvstore will be returned.
    // if some entry in the return set large then 750k, GetEntries may only return entries before this entry. you need
    // to use GetKeys interface to get all keys, then use Get interface to get each entry.
    // parameters:
    // prefixKey: perfix key to search
    // callback: all entries satisfies perfixKey, and Stauts for this call.
    KVSTORE_API
    virtual void GetEntries(const Key &prefixKey, std::function<void(Status, std::vector<Entry> &)> callback) = 0;

    // Deprecated. use the GetKeys interface without nextKey as parameter instead.
    // Get a list of keys from kvstore by keyPrefix,
    // key length must be less than 1024,
    // GetKeys will return all keys whose Key.StartsWith(keyPrefix) is true,
    // if keyPrefix is empty, all keys in the kvstore will be returned.
    // if data size is larger than 800k, data may be transported by several times. each time callback will give you the
    // first key of the not-transported part. You can use this key as nextKey to get next part of data. When you get an
    // empty nextKey, It means all data has been transported.
    // parameters:
    // prefixKey: perfix key to search
    // nextKey: The first key to start in this search.
    // callback: all keys satisfies perfixKey, status of this call and the first key of the next part of data.
    [[deprecated]]
    KVSTORE_API virtual void GetKeys(const Key &prefixKey, const Key &nextKey,
                                     std::function<void(Status, std::vector<Key> &, const Key &)> callback) = 0;

    // Get a list of keys from kvstore by keyPrefix,
    // key length must be less than 1024,
    // GetKeys will return all keys whose Key.StartsWith(keyPrefix) is true,
    // if keyPrefix is empty, all keys in the kvstore will be returned.
    // parameters:
    // prefixKey: perfix key to search
    // callback: all keys satisfies perfixKey, and Stauts for this call.
    KVSTORE_API
    virtual void GetKeys(const Key &prefixKey, std::function<void(Status, std::vector<Key> &)> callback) = 0;

    // Get value by key from kvstore, key length must be less than 256 and can not be empty.
    // if key not found in kvstore, KEY_NOT_FOUND will be returned.
    // otherwise, SUCCESS will be returned and value can be retrieved from the second parameter.
    // parameters:
    // key: key specified by client,
    // value: value stored in kvstore, or empty and KEY_NOT_FOUND returned.
    KVSTORE_API virtual Status Get(const Key &key, Value &value) = 0;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_SNAPSHOT_H
