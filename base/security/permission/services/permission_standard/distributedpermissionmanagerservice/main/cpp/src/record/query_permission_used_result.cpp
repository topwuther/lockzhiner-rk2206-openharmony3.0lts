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

#include "query_permission_used_result.h"

namespace OHOS {
namespace Security {
namespace Permission {
using namespace std;
bool QueryPermissionUsedResult::Marshalling(Parcel &out) const
{
    if (!out.WriteInt32(this->code)) {
        return false;
    }
    if (!out.WriteInt64(this->beginTimeMillis)) {
        return false;
    }
    if (!out.WriteInt64(this->endTimeMillis)) {
        return false;
    }
    if (!out.WriteInt32(this->bundlePermissionUsedRecords.size())) {
        return false;
    }
    for (const auto &record : this->bundlePermissionUsedRecords) {
        if (!out.WriteParcelable(&record)) {
            return false;
        }
    }
    return true;
}
QueryPermissionUsedResult *QueryPermissionUsedResult::Unmarshalling(Parcel &in)
{
    auto *result = new (nothrow) QueryPermissionUsedResult();
    if (result == nullptr) {
        delete result;
        result = nullptr;
        return result;
    }
    if (!in.ReadInt32(result->code)) {
        delete result;
        result = nullptr;
        return result;
    }
    if (!in.ReadInt64(result->beginTimeMillis)) {
        delete result;
        result = nullptr;
        return result;
    }
    if (!in.ReadInt64(result->endTimeMillis)) {
        delete result;
        result = nullptr;
        return result;
    }
    int size = 0;
    if (!in.ReadInt32(size)) {
        delete result;
        result = nullptr;
        return result;
    }
    for (int i = 0; i < size; i++) {
        sptr<BundlePermissionUsedRecord> record = in.ReadParcelable<BundlePermissionUsedRecord>();
        if (record == nullptr) {
            delete result;
            result = nullptr;
            return result;
        }
        result->bundlePermissionUsedRecords.push_back(*record);
    }
    return result;
}
size_t QueryPermissionUsedResult::GetDataSize() const
{
    size_t size = sizeof(int32_t);
    size += sizeof(int64_t);
    size += sizeof(int64_t);
    size += sizeof(int32_t);
    for (const auto &bundleRecord : bundlePermissionUsedRecords) {
        size += bundleRecord.GetDataSize();
    }
    return size;
}

nlohmann::json QueryPermissionUsedResult::to_json(const QueryPermissionUsedResult &result) const
{
    nlohmann::json jsonBundle;
    for (auto bundle : result.bundlePermissionUsedRecords) {
        jsonBundle.emplace_back(bundle.to_json(bundle));
    }
    nlohmann::json jsonObj = nlohmann::json{{"code", result.code},
        {"beginTimeMillis", result.beginTimeMillis},
        {"endTimeMillis", result.endTimeMillis},
        {"bundlePermissionUsedRecords", jsonBundle}};
    return jsonObj;
}

void QueryPermissionUsedResult::from_json(const nlohmann::json &jsonObj, QueryPermissionUsedResult &result)
{
    jsonObj.at("code").get_to(result.code);
    jsonObj.at("beginTimeMillis").get_to(result.beginTimeMillis);
    jsonObj.at("endTimeMillis").get_to(result.endTimeMillis);
    std::vector<BundlePermissionUsedRecord> bundleVector;
    nlohmann::json jsonBundle = jsonObj.at("bundlePermissionUsedRecords").get<nlohmann::json>();
    for (auto bundle : jsonBundle) {
        BundlePermissionUsedRecord tempBundle;
        tempBundle.from_json(bundle, tempBundle);
        bundleVector.emplace_back(tempBundle);
    }
    result.bundlePermissionUsedRecords = bundleVector;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
