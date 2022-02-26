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

#ifndef BUNDLE_PERMISSION_USED_RECORD_H
#define BUNDLE_PERMISSION_USED_RECORD_H

#include <string>
#include <vector>

#include "permission_used_record.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct BundlePermissionUsedRecord : public Parcelable {
    BundlePermissionUsedRecord() = default;

    ~BundlePermissionUsedRecord() override = default;

    bool Marshalling(Parcel &out) const override;

    static BundlePermissionUsedRecord *Unmarshalling(Parcel &in);

    /**
     * Warning: If you change any property to this class, please change GetDataSize function together.
     */
    size_t GetDataSize() const;

    std::string deviceId;
    std::string deviceLabel;
    std::string bundleName;
    std::string bundleLabel;
    int32_t applicationIconId;
    int32_t labelId;
    std::vector<PermissionUsedRecord> permissionUsedRecords;

    nlohmann::json to_json(const BundlePermissionUsedRecord &bundle) const;

    void from_json(const nlohmann::json &jsonObj, BundlePermissionUsedRecord &bundle);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // BUNDLE_PERMISSION_USED_RECORD_H
