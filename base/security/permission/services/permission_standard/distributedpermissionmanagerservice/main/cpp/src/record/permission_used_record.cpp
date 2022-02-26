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

#include "permission_used_record.h"

using namespace std;

namespace OHOS {
namespace Security {
namespace Permission {
bool PermissionUsedRecord::Marshalling(Parcel &out) const
{
    if (!out.WriteString(this->permissionName)) {
        return false;
    }
    if (!out.WriteInt32(this->accessCountFg)) {
        return false;
    }
    if (!out.WriteInt32(this->rejectCountFg)) {
        return false;
    }
    if (!out.WriteInt32(this->accessCountBg)) {
        return false;
    }
    if (!out.WriteInt32(this->rejectCountBg)) {
        return false;
    }
    if (!out.WriteInt64(this->lastAccessTime)) {
        return false;
    }
    if (!out.WriteInt64(this->lastRejectTime)) {
        return false;
    }
    if (!out.WriteInt64Vector(this->accessRecordFg)) {
        return false;
    }
    if (!out.WriteInt64Vector(this->rejectRecordFg)) {
        return false;
    }
    if (!out.WriteInt64Vector(this->accessRecordBg)) {
        return false;
    }
    if (!out.WriteInt64Vector(this->rejectRecordBg)) {
        return false;
    }
    return true;
}

PermissionUsedRecord *PermissionUsedRecord::Unmarshalling(Parcel &in)
{
    auto *record = new (nothrow) PermissionUsedRecord();
    if (record == nullptr) {
        delete record;
        record = nullptr;
        return record;
    }
    record->permissionName = in.ReadString();
    if (!in.ReadInt32(record->accessCountFg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt32(record->rejectCountFg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt32(record->accessCountBg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt32(record->rejectCountBg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt64(record->lastAccessTime)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt64(record->lastRejectTime)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt64Vector(&record->accessRecordFg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt64Vector(&record->rejectRecordFg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt64Vector(&record->accessRecordBg)) {
        delete record;
        record = nullptr;
        return record;
    }
    if (!in.ReadInt64Vector(&record->rejectRecordBg)) {
        delete record;
        record = nullptr;
        return record;
    }
    return record;
}

size_t PermissionUsedRecord::GetDataSize() const
{
    size_t size = sizeof(int32_t) + permissionName.size() * sizeof(char16_t);
    size += sizeof(int32_t);
    size += sizeof(int32_t);
    size += sizeof(int32_t);
    size += sizeof(int32_t);
    size += sizeof(int64_t);
    size += sizeof(int64_t);
    size += sizeof(int32_t) + accessRecordFg.size() * sizeof(int64_t);
    size += sizeof(int32_t) + rejectRecordFg.size() * sizeof(int64_t);
    size += sizeof(int32_t) + accessRecordBg.size() * sizeof(int64_t);
    size += sizeof(int32_t) + rejectRecordBg.size() * sizeof(int64_t);
    return size;
}

nlohmann::json PermissionUsedRecord::to_json(const PermissionUsedRecord &record)
{
    nlohmann::json jsonObj = nlohmann::json{{"permissionName", record.permissionName},
        {"accessCountFg", record.accessCountFg},
        {"rejectCountFg", record.rejectCountFg},
        {"accessCountBg", record.accessCountBg},
        {"rejectCountBg", record.rejectCountBg},
        {"lastAccessTime", record.lastAccessTime},
        {"lastRejectTime", record.lastRejectTime},
        {"accessRecordFg", record.accessRecordFg},
        {"rejectRecordFg", record.rejectRecordFg},
        {"accessRecordBg", record.accessRecordBg},
        {"rejectRecordBg", record.rejectRecordBg}};
    return jsonObj;
}

void PermissionUsedRecord::from_json(const nlohmann::json &jsonObj, PermissionUsedRecord &record)
{
    if (jsonObj.find("permissionName") != jsonObj.end() && jsonObj.at("permissionName").is_string()) {
        jsonObj.at("permissionName").get_to(record.permissionName);
    }
    if (jsonObj.find("accessCountFg") != jsonObj.end() && jsonObj.at("accessCountFg").is_number()) {
        jsonObj.at("accessCountFg").get_to(record.accessCountFg);
    }
    if (jsonObj.find("rejectCountFg") != jsonObj.end() && jsonObj.at("rejectCountFg").is_number()) {
        jsonObj.at("rejectCountFg").get_to(record.rejectCountFg);
    }
    if (jsonObj.find("accessCountBg") != jsonObj.end() && jsonObj.at("accessCountBg").is_number()) {
        jsonObj.at("accessCountBg").get_to(record.accessCountBg);
    }
    if (jsonObj.find("rejectCountBg") != jsonObj.end() && jsonObj.at("rejectCountBg").is_number()) {
        jsonObj.at("rejectCountBg").get_to(record.rejectCountBg);
    }
    if (jsonObj.find("lastAccessTime") != jsonObj.end() && jsonObj.at("lastAccessTime").is_number()) {
        jsonObj.at("lastAccessTime").get_to(record.lastAccessTime);
    }
    if (jsonObj.find("lastRejectTime") != jsonObj.end() && jsonObj.at("lastRejectTime").is_number()) {
        jsonObj.at("lastRejectTime").get_to(record.lastRejectTime);
    }
    if (jsonObj.find("accessRecordFg") != jsonObj.end() && jsonObj.at("accessRecordFg").is_array()) {
        jsonObj.at("accessRecordFg").get_to(record.accessRecordFg);
    }
    if (jsonObj.find("rejectRecordFg") != jsonObj.end() && jsonObj.at("rejectRecordFg").is_array()) {
        jsonObj.at("rejectRecordFg").get_to(record.rejectRecordFg);
    }
    if (jsonObj.find("accessRecordBg") != jsonObj.end() && jsonObj.at("accessRecordBg").is_array()) {
        jsonObj.at("accessRecordBg").get_to(record.accessRecordBg);
    }
    if (jsonObj.find("rejectRecordBg") != jsonObj.end() && jsonObj.at("rejectRecordBg").is_array()) {
        jsonObj.at("rejectRecordBg").get_to(record.rejectRecordBg);
    }
}

void PermissionUsedRecord::updateRecord(const PermissionUsedRecord &record)
{
    this->accessCountFg += record.accessCountFg;
    this->rejectCountFg += record.rejectCountFg;
    this->accessCountBg += record.accessCountBg;
    this->rejectCountBg += record.rejectCountBg;
    this->lastAccessTime = this->lastAccessTime > record.lastAccessTime ? this->lastAccessTime : record.lastAccessTime;
    this->lastRejectTime = this->lastRejectTime > record.lastRejectTime ? this->lastRejectTime : record.lastRejectTime;
}

void PermissionUsedRecord::updateRecordWithTime(const PermissionUsedRecord &record)
{
    updateRecord(record);
    if (record.accessCountFg > 0 || record.rejectCountFg > 0) {
        if (record.lastAccessTime > 0 && this->accessRecordFg.size() <= Constant::MAX_DETAIL_RECORDS) {
            this->accessRecordFg.emplace_back(record.lastAccessTime);
        } else if (record.lastRejectTime > 0 && this->rejectRecordFg.size() <= Constant::MAX_DETAIL_RECORDS) {
            this->rejectRecordFg.emplace_back(record.lastRejectTime);
        }
    } else {
        if (record.lastAccessTime > 0 && this->accessRecordBg.size() <= Constant::MAX_DETAIL_RECORDS) {
            this->accessRecordBg.emplace_back(record.lastAccessTime);
        } else if (record.lastRejectTime > 0 && this->rejectRecordBg.size() <= Constant::MAX_DETAIL_RECORDS) {
            this->rejectRecordBg.emplace_back(record.lastRejectTime);
        }
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS