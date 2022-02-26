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

#include "ability_info.h"
#include "application_info.h"
#include "mock_bundle_manager.h"

namespace OHOS {
namespace AppExecFwk {
void MockBundleMgrService::MockSetIsSystemApp(bool isSystemApp)
{
    isSystemAppMock_ = true;
    isSystemApp_ = isSystemApp;
}

bool MockBundleMgrService::CheckIsSystemAppByUid(const int uid)
{
    HILOG_ERROR("MockBundleMgrService::CheckIsSystemAppByUid beagin**********");
    if (isSystemAppMock_) {
        return isSystemApp_;
    }
    return (uid < 1000) ? false : true;
}

int MockBundleMgrService::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    if (!bundleName.compare("hello")) {
        return 0;
    } else {
        return -1;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS