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

#ifndef MULTI_VER_VALUE_OBJECT_H
#define MULTI_VER_VALUE_OBJECT_H

#ifndef OMIT_MULTI_VER
#include <vector>

#include "db_types.h"

namespace DistributedDB {
using ValueSliceHash = std::vector<uint8_t>;
using ValueSlice = std::vector<uint8_t>;

struct ValueObjectHead {
    uint8_t flag = 0;
    uint8_t reserved = 0;
    uint16_t hashNum = 1;
    uint32_t length = 0;
};

class MultiVerValueObject {
public:
    static const uint8_t HASH_FLAG = 0x01;

    MultiVerValueObject() {}
    ~MultiVerValueObject() {}

    int GetValueHash(std::vector<ValueSliceHash> &valueHashes) const;
    int SetValueHash(const std::vector<ValueSliceHash> &valueHashes);

    int GetSerialData(std::vector<uint8_t> &data) const;
    int DeSerialData(const std::vector<uint8_t> &data);

    uint32_t GetDataLength() const;
    void SetDataLength(uint32_t);

    int GetValue(Value &value) const;
    int SetValue(const Value &value);

    bool IsHash() const;

    // 1:value has been Hash. 0:Origin value
    void SetFlag(uint8_t flag);

private:
    ValueObjectHead head_;
    std::vector<uint8_t> valueHashVector_;
};
}

#endif  // MULTI_VER_VALUE_OBJECT_H
#endif