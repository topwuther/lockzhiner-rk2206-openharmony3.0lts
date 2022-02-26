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

#ifndef GENERIC_SINGLE_VER_KV_ENTRY_H
#define GENERIC_SINGLE_VER_KV_ENTRY_H

#include <cstdint>
#include <vector>
#include <string>

#include "db_types.h"
#include "single_ver_kv_entry.h"

namespace DistributedDB {
class GenericSingleVerKvEntry : public SingleVerKvEntry {
public:
    GenericSingleVerKvEntry();
    ~GenericSingleVerKvEntry() override;
    DISABLE_COPY_ASSIGN_MOVE(GenericSingleVerKvEntry);

    std::string GetOrigDevice() const override;

    void SetOrigDevice(const std::string &dev) override;

    TimeStamp GetTimestamp() const override;

    void SetTimestamp(TimeStamp time) override;

    TimeStamp GetWriteTimestamp() const override;

    void SetWriteTimestamp(TimeStamp time) override;

    void GetKey(Key &key) const;

    void GetValue(Value &value) const;

    uint64_t GetFlag() const override;

    void SetEntryData(DataItem &&dateItem);

    int SerializeData(Parcel &parcel, uint32_t targetVersion) override;

    int DeSerializeData(Parcel &parcel) override;

    uint32_t CalculateLen(uint32_t targetVersion) override;

    static int SerializeDatas(const std::vector<SingleVerKvEntry *> &kvEntries, Parcel &parcel, uint32_t targetVersion);

    static int DeSerializeDatas(std::vector<SingleVerKvEntry *> &kvEntries, Parcel &parcel);

    static uint32_t CalculateLens(const std::vector<SingleVerKvEntry *> &kvEntries, uint32_t targetVersion);

private:
    int AdaptToVersion(int operType, uint32_t targetVersion, Parcel &parcel, uint64_t &datalen);
    int AdaptToVersion(int operType, uint32_t targetVersion, uint64_t &datalen);

    int SerializeDataByVersion(uint32_t targetVersion, Parcel &parcel) const;
    int SerializeDataByFirstVersion(Parcel &parcel) const;
    int SerializeDataByLaterVersion(Parcel &parcel) const;

    int CalLenByVersion(uint32_t targetVersion, uint64_t &len) const;
    void CalLenByFirstVersion(uint64_t &len) const;
    void CalLenByLaterVersion(uint64_t &len) const;

    int DeSerializeByVersion(uint32_t targetVersion, Parcel &parcel, uint64_t &len);
    void DeSerializeByFirstVersion(uint64_t &len, Parcel &parcel);
    void DeSerializeByLaterVersion(uint64_t &len, Parcel &parcel);

    DataItem dataItem_;
};
} // namespace DistributedDB

#endif // GENERIC_SINGLE_VER_KV_ENTRY_H
