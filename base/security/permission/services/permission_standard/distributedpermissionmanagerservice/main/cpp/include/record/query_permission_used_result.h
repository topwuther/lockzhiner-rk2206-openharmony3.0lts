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

#ifndef QUERY_PERMISSION_USED_RESULT_H
#define QUERY_PERMISSION_USED_RESULT_H

#include <string>
#include <vector>

#include "bundle_permission_used_record.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct QueryPermissionUsedResult : public Parcelable {
    QueryPermissionUsedResult() = default;

    ~QueryPermissionUsedResult() override = default;

    bool Marshalling(Parcel &out) const override;

    static QueryPermissionUsedResult *Unmarshalling(Parcel &in);

    /**
     * Warning: If you change any property to this class, please change GetDataSize function together.
     */
    size_t GetDataSize() const;

    int32_t code = 0;
    int64_t beginTimeMillis = 0;
    int64_t endTimeMillis = 0;
    std::vector<BundlePermissionUsedRecord> bundlePermissionUsedRecords;

    nlohmann::json to_json(const QueryPermissionUsedResult &result) const;

    void from_json(const nlohmann::json &jsonObj, QueryPermissionUsedResult &result);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // QUERY_PERMISSION_USED_RESULT_H
