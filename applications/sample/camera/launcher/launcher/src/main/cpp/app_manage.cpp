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

#include <cstdio>
#include <securec.h>

#include "app_manage.h"

namespace OHOS {
ViewGroupPage* AppManage::viewPage_[MAX_VIEWGROUP] = { nullptr };
funcClick AppManage::installFuncclick_ = { nullptr };
funcLongPress AppManage::installFunclPress_ = { nullptr };
int AppManage::size_ = 0;

AppManage::~AppManage()
{
    UnregisterCallback();
}

bool AppManage::GetAailityInfosByBundleName(const char* bundleName, AppInfo* pApp)
{
    if (bundleName == nullptr || pApp == nullptr) {
        return false;
    }
    uint8_t ret = -1;
    BundleInfo* pBundleInfos = nullptr;
    int count = 0;
    ret = GetBundleInfos(1, &pBundleInfos, &count);
    if (ret == 0) {
        BundleInfo* pBundleInfo = pBundleInfos;
        for (int i = 0; i < count; i++, pBundleInfo++) {
            if (memcmp(bundleName, pBundleInfo->bundleName, strlen(pBundleInfo->bundleName)) == 0) {
                memcpy_s(
                    pApp->appName_, sizeof(pApp->appName_), pBundleInfo->bundleName, strlen(pBundleInfo->bundleName));
                pApp->appName_[strlen(pBundleInfo->bundleName)] = 0;
                if (pBundleInfo->abilityInfos[0].name) {
                    memcpy_s(pApp->abilityName_, sizeof(pApp->abilityName_), pBundleInfo->abilityInfos[0].name,
                        strlen(pBundleInfo->abilityInfos[0].name));
                    pApp->abilityName_[strlen(pBundleInfo->abilityInfos[0].name)] = 0;
                }
                if (pBundleInfo->bigIconPath) {
                    memcpy_s(pApp->appIconDir_, sizeof(pApp->appIconDir_), pBundleInfo->bigIconPath,
                        strlen(pBundleInfo->bigIconPath));
                    pApp->appIconDir_[strlen(pBundleInfo->bigIconPath)] = 0;
                }
                return true;
            }
        }
    }
    return false;
}

bool AppManage::GetAppInstallInfo(const char* bundleName)
{
    if (bundleName == nullptr) {
        return false;
    }
    AppInfo* pApp = new AppInfo();
    pApp->funcclick_ = installFuncclick_;
    pApp->funclPress_ = installFunclPress_;
    if (GetAailityInfosByBundleName(bundleName, pApp) == true) {
        int i = 0;
        for (; i < size_; i++) {
            if (viewPage_[i]->FindApp(pApp)) {
                break;
            }
        }
        if (i == size_) {
            for (i = 0; i < size_; i++) {
                if (viewPage_[i]->AddApp(pApp)) {
                    return true;
                }
            }
        }
    }
    delete pApp;
    pApp = nullptr;
    return false;
}

void AppManage::MyBundleStateCallback(
    const uint8_t installType, const uint8_t resultCode, const void* resultMessage, const char* bundleName, void* data)
{
    if (installType == 0) { // install update
        if (resultCode == 0 && bundleName != nullptr) {
            char tmpName[TMP_BUF_SIZE] = {0};
            if (memcpy_s(tmpName, sizeof(tmpName), bundleName, strlen(bundleName)) == LAUNCHER_SUCCESS) {
                tmpName[strlen(bundleName)] = 0;
                GetAppInstallInfo(tmpName);
            }
        }
    }
}

void AppManage::MyBundleOwnCallback(const uint8_t resultCode, const void* resultMessage)
{
    // todo uninstall callback
}

bool AppManage::LauncherApp(BundleInfo** info, int& count)
{
    callBackParam_.bundleName = nullptr;
    callBackParam_.data = nullptr;
    callBackParam_.callBack = MyBundleStateCallback;
    RegisterCallback(&callBackParam_);

    BundleInfo* pBundleInfos = nullptr;
    uint8_t ret = GetBundleInfos(1, &pBundleInfos, &count);
    if (ret == 0) {
        *info = pBundleInfos;
        return true;
    } else {
        *info = nullptr;
        return false;
    }
}

bool AppManage::InstallApp(AppInfo* app)
{
    return true;
}

bool AppManage::UnInstallApp(AppInfo* app)
{
    return Uninstall(app->appName_, nullptr, MyBundleOwnCallback);
}

bool AppManage::StartApp(AppInfo* app)
{
    Want want1 = { nullptr };
    ElementName element = { nullptr };
    SetElementBundleName(&element, app->appName_);
    SetElementAbilityName(&element, app->abilityName_);
    SetWantElement(&want1, element);
    SetWantData(&want1, "WantData", strlen("WantData") + 1);
    StartAbility(&want1);
    ClearElement(&element);
    ClearWant(&want1);
    return true;
}

void AppManage::SetViewGroup(funcClick click, funcLongPress press, ViewGroupPage* viewPage[MAX_VIEWGROUP], int size)
{
    if (click == nullptr || press == nullptr || viewPage == nullptr) {
        return;
    }
    for (int i = 0; i < size; i++) {
        viewPage_[i] = viewPage[i];
    }
    size_ = size;
    installFuncclick_ = click;
    installFunclPress_ = press;
}
} // namespace OHOS
