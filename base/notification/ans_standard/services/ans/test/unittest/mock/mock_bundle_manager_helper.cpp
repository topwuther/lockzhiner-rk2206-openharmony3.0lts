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

#include "bundle_manager_helper.h"

#include "ans_ut_constant.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Notification {

BundleManagerHelper::BundleManagerHelper()
{}

BundleManagerHelper::~BundleManagerHelper()
{}

void BundleManagerHelper::OnRemoteDied(const wptr<IRemoteObject> &object)
{}

std::string BundleManagerHelper::GetBundleNameByUid(int uid)
{
    return (uid == NON_BUNDLE_NAME_UID) ? "" : "bundleName";
}

int BundleManagerHelper::GetDefaultUidByBundleName(const std::string &bundle)
{
    return NON_SYSTEM_APP_UID;
}

bool BundleManagerHelper::IsSystemApp(int uid)
{
    return (uid == SYSTEM_APP_UID);
}

void BundleManagerHelper::Connect()
{}

void BundleManagerHelper::Disconnect()
{}

}  // namespace Notification
}  // namespace OHOS