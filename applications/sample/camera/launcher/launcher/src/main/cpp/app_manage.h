/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef OHOS_APP_MANAGE_H
#define OHOS_APP_MANAGE_H

#include <ability_info.h>
#include <ability_slice.h>
#include <bundle_manager.h>
#include <element_name.h>
#include <module_info.h>
#include <want.h>

#include "app_info.h"
#include "ui_config.h"
#include "view_group_page.h"

namespace OHOS {
class AppManage {
public:
    AppManage() = default;
    ~AppManage();
    // start all app ,get all app info

    bool LauncherApp(BundleInfo** info, int& count);
    bool InstallApp(AppInfo* app);
    bool UnInstallApp(AppInfo* app);
    bool StartApp(AppInfo* app);
    static void SetViewGroup(funcClick click, funcLongPress press, ViewGroupPage* arrPage[MAX_VIEWGROUP], int size);

private:
    static void MyBundleStateCallback(const uint8_t installType, const uint8_t resultCode, const void* resultMessage,
        const char* bundleName, void* data);
    static void MyBundleOwnCallback(const uint8_t resultCode, const void* resultMessage);
    static bool GetAailityInfosByBundleName(const char* bundleName, AppInfo* pApp);
    static bool GetAppInstallInfo(const char* bundleName);

private:
    BundleStatusCallback callBackParam_ { nullptr };
    static ViewGroupPage* viewPage_[MAX_VIEWGROUP];
    static int size_;
    static funcClick installFuncclick_;
    static funcLongPress installFunclPress_;
};
} // namespace OHOS
#endif