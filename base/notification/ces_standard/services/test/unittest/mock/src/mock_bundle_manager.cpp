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
    static int num1 = 0;
    static int num2 = 0;

    if (!bundleName.compare("case1")) {
        if (!permission.compare("ohos.permission.RECEIVER_STARTUP_COMPLETED") ||
            !permission.compare("ohos.permission.MANAGE_SECURE_SETTINGS") ||
            !permission.compare("ohos.permission.LISTEN_BUNDLE_CHANGE") ||
            !permission.compare("ohos.permission.GET_APP_ACCOUNTS") ||
            !permission.compare("ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS")) {
            return 0;
        }
    }

    if (!bundleName.compare("case2")) {
        if (!permission.compare("ohos.permission.MANAGE_USERS") ||
            !permission.compare("ohos.permission.INTERACT_ACROSS_USERS") ||
            !permission.compare("ohos.permission.LOCATION") || !permission.compare("ohos.permission.GET_WIFI_INFO") ||
            !permission.compare("ohos.permission.MPLINK_CHANGE_STATE") ||
            !permission.compare("ohos.permission.USE_BLUETOOTH") ||
            !permission.compare("ohos.permission.DISCOVER_BLUETOOTH") ||
            !permission.compare("ohos.permission.USE_BLUETOOTH permission")) {
            return 0;
        }
    }

    if (!bundleName.compare("case3")) {  // "ohos.permission.GET_WIFI_INFO" and "ohos.permission.LOCATION"  ||
                                         // "ohos.permission.USE_BLUETOOTH" and "ohos.permission.LOCATION"
        if (num1 == 0) {
            if (!permission.compare("ohos.permission.GET_WIFI_INFO") ||
                !permission.compare("ohos.permission.USE_BLUETOOTH")) {
                num1++;
                return 0;
            }
        }
    }

    if (!bundleName.compare("case3")) {
        if (num1 == 1) {
            num1 = 0;
            if (!permission.compare("ohos.permission.LOCATION")) {
                return 0;
            }
        }
    }

    if (!bundleName.compare("case4")) {
        if (num2 == 0) {
            if (!permission.compare("ohos.permission.WRITE_USER_STORAGE")) {
                num2++;
                return -1;
            }
        }
    }

    if (!bundleName.compare("case4")) {
        if (num2 == 1) {
            num2 = 0;
            if (!permission.compare("ohos.permission.READ_USER_STORAGE")) {
                return 0;
            }
        }
    }

    if (!bundleName.compare("hello")) {
        return 0;
    }

    return -1;
}

}  // namespace AppExecFwk
}  // namespace OHOS
