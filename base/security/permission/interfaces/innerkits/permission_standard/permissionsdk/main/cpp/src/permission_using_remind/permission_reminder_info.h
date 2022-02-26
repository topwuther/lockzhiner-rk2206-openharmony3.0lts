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

#ifndef PERMISSION_REMINDER_INFO_H
#define PERMISSION_REMINDER_INFO_H

#include <sstream>

#include "parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct PermissionReminderInfo : public Parcelable {
    PermissionReminderInfo() = default;

    ~PermissionReminderInfo() override = default;

    bool Marshalling(Parcel& out) const override;

    static PermissionReminderInfo* Unmarshalling(Parcel& in);

    std::string ToString() const;

    std::u16string deviceId;
    std::u16string deviceLabel;
    std::u16string bundleName;
    std::u16string bundleLabel;
    std::u16string permName;
    std::string appName;

    void Init();
    void SetDeviceId(std::string strParam);
    void SetDeviceLabel(std::string strParam);
    void SetBundleName(std::string strParam);
    void SetBundleLabel(std::string strParam);
    void SetPermName(std::string strParam);
};
} // namespace Permission
} // namespace Security
} // namespace OHOS

#endif // PERMISSION_REMINDER_INFO_H
