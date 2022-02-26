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

#ifndef GENERIC_MULTI_VER_KVENTRY_H
#define GENERIC_MULTI_VER_KVENTRY_H

#ifndef OMIT_MULTI_VER
#include <vector>

#include "macro_utils.h"
#include "multi_ver_kv_entry.h"
#include "multi_ver_value_object.h"

namespace DistributedDB {
class GenericMultiVerKvEntry : public MultiVerKvEntry {
public:
    GenericMultiVerKvEntry();
    ~GenericMultiVerKvEntry() override;
    DISABLE_COPY_ASSIGN_MOVE(GenericMultiVerKvEntry);

    int GetSerialData(std::vector<uint8_t> &data) const override;

    int GetValueHash(std::vector<ValueSliceHash> &valueHashes) const override;

    void GetTimestamp(uint64_t &timestamp) const override;
    void SetTimestamp(uint64_t timestamp) override;

    void GetOriTimestamp(uint64_t &oriTimestamp) const;
    void SetOriTimestamp(uint64_t oriTimestamp);

    int DeSerialData(const std::vector<uint8_t> &data);

    int SetKey(const Key &key);
    int GetKey(Key &key) const;

    int SetValue(const Value &value);
    int GetValue(Value &value) const;

    void SetOperFlag(uint64_t flag);
    void GetOperFlag(uint64_t &flag) const;

    void SetDataLength(uint32_t length);
    uint32_t GetDataLength() const;

private:
    std::vector<uint8_t> key_;
    MultiVerValueObject valueObject_;
    uint64_t operFlag_;
    uint64_t timestamp_;
    uint64_t oriTimestamp_;
};
} // namespace DistributedDB

#endif // GENERIC_MULTI_VER_KVENTRY_H
#endif