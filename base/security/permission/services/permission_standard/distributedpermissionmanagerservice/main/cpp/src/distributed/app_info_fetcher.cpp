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
#include <map>

#include "app_info_fetcher.h"

#include "constant.h"
#include "application_info.h"
#include "permission_log.h"
#include "distributed_data_validator.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "AppInfoFetcher"};
}
AppInfoFetcher &AppInfoFetcher::GetInstance()
{
    static AppInfoFetcher instance;
    return instance;
}

/**
 * Get application attribute. Any combination of {@link #APP_ATTRIBUTE_PRESET}, {@link #APP_ATTRIBUTE_PRIVILEGED}
 * and {@link #APP_ATTRIBUTE_SIGNED_WITH_PLATFORM_KEY} in normal case.
 *
 * @param uid The application uid.
 * @param bundleInfo The BundleInfo.
 * @return Application attribute.
 */
int32_t AppInfoFetcher::GetAppAttribute(int32_t uid, AppExecFwk::BundleInfo &bundleInfo)
{
    PERMISSION_LOG_DEBUG(LABEL, "GetAppAttribute begin, uid: %{public}d", uid);

    int32_t appAttribute = 0;
    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "getAppAttribute invalid uid: %{public}d", uid);
        return Constant::FAILURE;
    }

    if (attributeBuffer_.count(uid) > 0) {
        appAttribute = attributeBuffer_[uid];
        return appAttribute;
    }

    AppExecFwk::ApplicationInfo appInfo = bundleInfo.applicationInfo;
    if (appInfo.isSystemApp) {
        appAttribute = static_cast<int32_t>(AppAttribute::APP_ATTRIBUTE_PRESET);
    }
    if (appInfo.isLauncherApp) {
        appAttribute = static_cast<int32_t>(AppAttribute::APP_ATTRIBUTE_PRIVILEGED);
    }
    if (appInfo.isLauncherApp && appInfo.signatureKey.empty()) {
        appAttribute = static_cast<int32_t>(AppAttribute::APP_ATTRIBUTE_SIGNED_WITH_PLATFORM_KEY);
    }
    attributeBuffer_[uid] = appAttribute;

    PERMISSION_LOG_DEBUG(LABEL, "getAppAttribute end, appAttribute: %{public}d", appAttribute);

    return appAttribute;
}

void AppInfoFetcher::RemoveAttributeBuffer(int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL, "begin, uid: %{public}d", uid);

    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "invalid uid: %{public}d", uid);
    }

    for (auto iter = attributeBuffer_.begin(); iter != attributeBuffer_.end(); ++iter) {
        if (iter->first == uid) {
            attributeBuffer_.erase(iter);
        }
    }
    PERMISSION_LOG_DEBUG(LABEL, "end, uid: %{public}d", uid);
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS