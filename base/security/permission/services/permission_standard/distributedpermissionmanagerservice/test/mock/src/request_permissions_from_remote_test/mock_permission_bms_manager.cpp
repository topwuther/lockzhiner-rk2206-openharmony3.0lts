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
#include "permission_bms_manager.h"
#include "monitor_manager.h"
#include "distributed_data_validator.h"
namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionBmsManager"};
}
namespace {
static const std::string TAG = "PERMISSION_BMS_MANAGER";
}
PermissionBmsManager::PermissionBmsManager()
    : iBundleManager_(nullptr), iPermissionManager_(nullptr), permissionFetcher_(nullptr), appInfoFetcher_(nullptr)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionBmsManager()");
}
PermissionBmsManager::~PermissionBmsManager()
{
    PERMISSION_LOG_DEBUG(LABEL, "~PermissionBmsManager()");
}
PermissionBmsManager &PermissionBmsManager::GetInstance()
{
    static PermissionBmsManager instance;
    return instance;
}
int PermissionBmsManager::Init()
{
    return Constant::SUCCESS;
}
int32_t PermissionBmsManager::GetPermissions(int32_t uid, UidBundleBo &info)
{
    return Constant::SUCCESS;
}
int32_t PermissionBmsManager::GetRegrantedPermissions(UidBundleBo &pInfo, UidBundleBo &rInfo)
{
    return Constant::FAILURE;
}
bool PermissionBmsManager::IsSystemSignatureUid(const int32_t &uid)
{
    if (uid == 100111) {
        return true;
    }
    return false;
}
int PermissionBmsManager::InitSystemDefinedPermissions()
{
    return Constant::SUCCESS;
}
void PermissionBmsManager::ReGrantDuidPermissions(UidBundleBo &uidBundlePermInfo)
{}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
