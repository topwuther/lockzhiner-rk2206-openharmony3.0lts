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

#ifndef MULTI_VER_KV_ENTRY_H
#define MULTI_VER_KV_ENTRY_H

#ifndef OMIT_MULTI_VER
#include <vector>

#include "multi_ver_value_object.h"

namespace DistributedDB {
class MultiVerKvEntry {
public:
    virtual ~MultiVerKvEntry() {};

    virtual int GetSerialData(std::vector<uint8_t> &data) const = 0;

    virtual int GetValueHash(std::vector<ValueSliceHash> &valueHashes) const = 0;

    virtual void GetTimestamp(uint64_t &timestamp) const = 0;

    virtual void SetTimestamp(uint64_t timestamp) = 0;
};
} // namespace DistributedDB

#endif // MULTI_VER_KV_ENTRY_H
#endif
