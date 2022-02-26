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
#include <iostream>
#include "request_permissions_info.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RequestPermissionsInfo"};
}
RequestPermissionsInfo::RequestPermissionsInfo(const std::vector<std::string> permissions,
    const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
    int32_t reasonResId)
    : permissions_(permissions),
      callback_(callback),
      nodeId_(nodeId),
      bundleName_(bundleName),
      reasonResId_(reasonResId)
{
    pid_ = IPCSkeleton::GetCallingPid();
    uid_ = IPCSkeleton::GetCallingUid();
    // deviceId is unique
    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    requestId_ = deviceId;
    deviceId_ = deviceId;
}
bool RequestPermissionsInfo::NeedStopProcess()
{
    if (permissions_.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "requestPermissionsFromRemote permissions is null");
        return true;
    }
    return false;
}
bool RequestPermissionsInfo::CanRequestPermissions()
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId_)) {
        PERMISSION_LOG_ERROR(LABEL, "prepareRequest device id is empty or not online");
        return false;
    }
    if (!InitObjectLocaleReason()) {
        PERMISSION_LOG_ERROR(LABEL, "prepareRequest reason string can not get");
        return false;
    }
    PERMISSION_LOG_INFO(LABEL,
        "current request %{public}s should request sensitive permissions from object device",
        requestId_.c_str());
    return true;
}
bool RequestPermissionsInfo::InitObjectLocaleReason()
{
    if (bundleName_.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "prepareRequest bundleName is empty");
        return false;
    }

    if (reasonResId_ < 0) {
        PERMISSION_LOG_ERROR(LABEL, "prepareRequest reasonResId is not correct");
        return false;
    }
    // need check resource  GetStringResources
    reasonString_ = "reasonString";
    return !reasonString_.empty();
}
std::string RequestPermissionsInfo::ToString()
{
    std::ostringstream oss;
    oss << "RequestPermissionsInfo{"
        << "requestId='" << requestId_ << '\'' << ", permissions=";

    std::vector<std::string>::iterator it;
    for (it = permissions_.begin(); it != permissions_.end(); it++) {
        oss << *it;
    }
    oss << ", nodeId='" << nodeId_ << ", deviceId='" << deviceId_ << '\'' << ", bundleName='" << bundleName_ << '\''
        << ", reasonResId=" << reasonResId_ << ", uid=" << std::to_string(uid_) << '}';
    return oss.str();
};
std::vector<int32_t> RequestPermissionsInfo::GetGrantResult()
{
    std::vector<int32_t> grantResults;
    for (std::vector<std::string>::iterator it = permissions_.begin(); it < permissions_.end(); it++) {
        grantResults.push_back(
            ObjectDevicePermissionManager::GetInstance().VerifyPermissionFromRemote((*it), deviceId_, pid_, uid_));
    }

    return grantResults;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS