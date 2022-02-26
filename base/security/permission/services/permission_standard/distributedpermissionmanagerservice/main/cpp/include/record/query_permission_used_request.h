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

#ifndef QUERY_PERMISSION_USED_REQUEST_H
#define QUERY_PERMISSION_USED_REQUEST_H

#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "parcel.h"
#include "query_permission_used_result.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct QueryPermissionUsedRequest : public Parcelable {
    enum { FLAG_PERMISSION_USAGE_SUMMARY = 0, FLAG_PERMISSION_USAGE_DETAIL = 1 };

    QueryPermissionUsedRequest() = default;

    ~QueryPermissionUsedRequest() override = default;

    bool Marshalling(Parcel &out) const override;

    static QueryPermissionUsedRequest *Unmarshalling(Parcel &in);

    std::string deviceLabel;
    std::string bundleName;
    std::vector<std::string> permissionNames;
    int64_t beginTimeMillis = 0;
    int64_t endTimeMillis = 0;
    int flag = FLAG_PERMISSION_USAGE_SUMMARY;

    nlohmann::json to_json(const QueryPermissionUsedRequest &request) const;

    void from_json(const nlohmann::json &jsonObj, QueryPermissionUsedRequest &request);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // QUERY_PERMISSION_USED_REQUEST_H
