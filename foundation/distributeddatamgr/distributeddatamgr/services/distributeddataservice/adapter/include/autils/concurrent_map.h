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

#ifndef DISTRIBUTEDDATAMGR_CURRENT_MAP_H
#define DISTRIBUTEDDATAMGR_CURRENT_MAP_H

#include <map>
#include <mutex>
#include <functional>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
template<typename K, typename V>
class ConcurrentMap {
public:
    KVSTORE_API explicit ConcurrentMap() {};

    KVSTORE_API ~ConcurrentMap() {};

    KVSTORE_API bool Put(const K &key, const V &value)
    {
        std::unique_lock<std::mutex> lock(mapMutex_);
        auto ret = map_.insert(std::pair<K, V>(key, value));
        return ret.second;
    }

    KVSTORE_API bool PutValue(const K key, const V value)
    {
        std::unique_lock<std::mutex> lock(mapMutex_);
        auto ret = map_.insert(std::pair<K, V>(key, value));
        return ret.second;
    }

    KVSTORE_API bool Empty()
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        return map_.empty();
    }

    KVSTORE_API int Size()
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        return map_.size();
    }

    KVSTORE_API bool Delete(const K &key)
    {
        std::unique_lock<std::mutex> lock(mapMutex_);
        return map_.erase(key);
    }

    KVSTORE_API bool Get(const K &key, V &v)
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto ret = map_.find(key);
        if (ret != map_.end()) {
            v = ret->second;
            return true;
        }
        return false;
    }

    KVSTORE_API bool ContainsKey(const K &key)
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        return map_.count(key);
    }

    KVSTORE_API void Clear()
    {
        std::unique_lock<std::mutex> lock(mapMutex_);
        map_.clear();
    }

    KVSTORE_API void ForEach(std::function<void(K, V)> fun)
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (auto const &kvPair : map_) {
            fun(kvPair.first, kvPair.second);
        }
    }
private:
    std::mutex mapMutex_;
    std::map<K, V> map_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_CURRENT_MAP_H
