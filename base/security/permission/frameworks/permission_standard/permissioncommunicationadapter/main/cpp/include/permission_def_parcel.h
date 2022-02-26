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

#ifndef PERMISSION_DEF_PARCEL_H
#define PERMISSION_DEF_PARCEL_H

#include "permission/permission_def.h"

#include "parcel.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct PermissionDefParcel final : public Parcelable {
    PermissionDefParcel() = default;

    ~PermissionDefParcel() override = default;

    bool Marshalling(Parcel &out) const override;

    static PermissionDefParcel *Unmarshalling(Parcel &in);

    PermissionDef permissionDef;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_DEF_PARCEL_H
