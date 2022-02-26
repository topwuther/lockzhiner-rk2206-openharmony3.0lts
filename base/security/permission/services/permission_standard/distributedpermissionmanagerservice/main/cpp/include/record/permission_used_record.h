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

#ifndef PERMISSION_USED_RECORD_H
#define PERMISSION_USED_RECORD_H

#include <string>
#include <vector>

#include "constant.h"
#include "nlohmann/json.hpp"
#include "parcel.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct PermissionUsedRecord : public Parcelable {
    PermissionUsedRecord() = default;

    ~PermissionUsedRecord() override = default;

    bool Marshalling(Parcel &out) const override;

    static PermissionUsedRecord *Unmarshalling(Parcel &in);

    /**
     * Warning: If you change any property to this class, please change GetDataSize function together.
     */
    size_t GetDataSize() const;

    std::string permissionName;
    int32_t accessCountFg = 0;
    int32_t rejectCountFg = 0;
    int32_t accessCountBg = 0;
    int32_t rejectCountBg = 0;
    int64_t lastAccessTime = -1L;
    int64_t lastRejectTime = -1L;
    std::vector<int64_t> accessRecordFg;
    std::vector<int64_t> rejectRecordFg;
    std::vector<int64_t> accessRecordBg;
    std::vector<int64_t> rejectRecordBg;

    nlohmann::json to_json(const PermissionUsedRecord &result);

    void from_json(const nlohmann::json &jsonObj, PermissionUsedRecord &result);

    void updateRecord(const PermissionUsedRecord &record);

    void updateRecordWithTime(const PermissionUsedRecord &record);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // PERMISSION_USED_RECORD_H
