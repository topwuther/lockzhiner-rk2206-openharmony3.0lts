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
#include "permission_reminder_info.h"
#include <codecvt>
#include <string>
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionReminderInfo"};
}
using namespace std;
bool PermissionReminderInfo::Marshalling(Parcel& out) const
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);
    if (!out.WriteString16(this->deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "WriteString16 deviceId error!");
        return false;
    }
    if (!out.WriteString16(this->deviceLabel)) {
        PERMISSION_LOG_ERROR(LABEL, "WriteString16 deviceLabel error!");
        return false;
    }
    if (!out.WriteString16(this->bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "WriteString16 bundleName error!");
        return false;
    }
    if (!out.WriteString16(this->bundleLabel)) {
        PERMISSION_LOG_ERROR(LABEL, "WriteString16 bundleLabel error!");
        return false;
    }
    if (!out.WriteString16(this->permName)) {
        PERMISSION_LOG_ERROR(LABEL, "WriteString16 permName error!");
        return false;
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s done", __func__);
    return true;
}

PermissionReminderInfo* PermissionReminderInfo::Unmarshalling(Parcel& in)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);
    PermissionReminderInfo* permReminderInfo = new PermissionReminderInfo();
    if (permReminderInfo == nullptr) {
        delete permReminderInfo;
        permReminderInfo = nullptr;
    } else {
        permReminderInfo->deviceId = in.ReadString16();
        permReminderInfo->deviceLabel = in.ReadString16();
        permReminderInfo->bundleName = in.ReadString16();
        permReminderInfo->bundleLabel = in.ReadString16();
        permReminderInfo->permName = in.ReadString16();
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s done", __func__);
    return permReminderInfo;
}

std::string PermissionReminderInfo::ToString() const
{
    std::ostringstream oss;
    oss << "PermissionReminderInfo{deviceId= " << Str16ToStr8(deviceId) << ", deviceLabel=";
    oss << Str16ToStr8(deviceLabel) << ", bundleName=" << Str16ToStr8(bundleName) << ", bundleLabel=";
    oss << Str16ToStr8(bundleLabel) << ", permName=" << Str16ToStr8(permName) << "}";
    return oss.str();
}

void PermissionReminderInfo::Init()
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);
    std::string strNull = "";
    SetDeviceId(strNull);
    SetDeviceLabel(strNull);
    SetBundleName(strNull);
    SetBundleLabel(strNull);
    SetPermName(strNull);
    PERMISSION_LOG_INFO(LABEL, "%{public}s done", __func__);

    return;
}

void PermissionReminderInfo::SetDeviceId(std::string strParam)
{
    deviceId = Str8ToStr16(strParam);
}

void PermissionReminderInfo::SetDeviceLabel(std::string strParam)
{
    deviceLabel = Str8ToStr16(strParam);
}

void PermissionReminderInfo::SetBundleName(std::string strParam)
{
    bundleName = Str8ToStr16(strParam);
}

void PermissionReminderInfo::SetBundleLabel(std::string strParam)
{
    bundleLabel = Str8ToStr16(strParam);
}
void PermissionReminderInfo::SetPermName(std::string strParam)
{
    permName = Str8ToStr16(strParam);
}
} // namespace Permission
} // namespace Security
} // namespace OHOS