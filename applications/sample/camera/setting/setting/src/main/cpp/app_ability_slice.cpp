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

#include "app_ability_slice.h"
#include "gfx_utils/style.h"

namespace OHOS {
REGISTER_AS(AppAbilitySlice)

AppAbilitySlice::~AppAbilitySlice()
{
    if (scrollView_) {
        DeleteChildren(scrollView_);
        scrollView_ = nullptr;
    }
    if (headView_) {
        DeleteChildren(headView_);
        headView_ = nullptr;
    }
    if (buttonBackListener_) {
        delete buttonBackListener_;
        buttonBackListener_ = nullptr;
    }
    if (buttonAppInfoListener_) {
        delete buttonAppInfoListener_;
        buttonAppInfoListener_ = nullptr;
    }
    if (pBundleInfos_) {
        ClearBundleInfo(pBundleInfos_);
    }
}

void AppAbilitySlice::SetButtonListener(void)
{
    auto onClick1 = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("MainAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingWifiAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonBackListener_ = new EventListener(onClick1, nullptr);
}

void AppAbilitySlice::SetAppButtonListener(const char* appName)
{
    auto onClick2 = [this, appName](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        bool ret = SetWantData(&want1, appName, strlen(appName) + 1);
        if (ret != true) {
            return false;
        }
        StartAbility(want1);
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("AppInfoAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingWifiAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        ClearWant(&want1);
        return true;
    };
    buttonAppInfoListener_ = new EventListener(onClick2, nullptr);
}

void AppAbilitySlice::SetHead()
{
    headView_ = new UIViewGroup();
    rootView_->Add(headView_);
    headView_->SetPosition(DE_HEAD_X, DE_HEAD_Y, DE_HEAD_WIDTH, DE_HEAD_HEIGHT);
    headView_->SetStyle(STYLE_BACKGROUND_OPA, 0);
    headView_->SetTouchable(true);
    headView_->SetOnClickListener(buttonBackListener_);

    UIImageView* imageView = new UIImageView();
    headView_->Add(imageView);
    imageView->SetPosition(DE_HEAD_IMAGE_X, DE_HEAD_IMAGE_Y, DE_HEAD_IMAGE_WIDTH, DE_HEAD_IMAGE_HEIGHT);
    imageView->SetSrc(DE_IMAGE_BACK);

    UILabel* lablelFont = new UILabel();
    lablelFont->SetPosition(DE_HEAD_TEXT_X, DE_HEAD_TEXT_Y, DE_HEAD_TEXT_WIDTH, DE_HEAD_TEXT_HEIGHT);
    lablelFont->SetText("应用");
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void AppAbilitySlice::SetAnAppInfo(const int count, BundleInfo& pBundleInfo)
{
    UIViewGroup* itemView = new UIViewGroup();
    char buff[64] = {0};
    int useX = 0;
    int useY = count * DE_ITEM_INTERVAL;
    itemView->SetPosition(useX, useY, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    itemView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    itemView->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    itemView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    itemView->SetTouchable(true);

    int err = strcpy_s(buff, sizeof(buff), pBundleInfo.bundleName);
    if (err != EOK) {
        printf("[ERROR]strcpy_s pBundleInfo.bundleName failed, err = %d\n", err);
        return;
    }
    for (size_t i = 0; i < strlen(pBundleInfo.bundleName); i++) {
        buff[i] = pBundleInfo.bundleName[i];
    }
    SetAppButtonListener(pBundleInfo.bundleName);
    itemView->SetOnClickListener(buttonAppInfoListener_);
    scrollView_->Add(itemView);

    UIImageView* imageIdView = new UIImageView();
    imageIdView->SetPosition(APP_IMAGE_X, APP_IMAGE_Y, APP_IMAGE_WIDTH, APP_IMAGE_HEIGHT);
    imageIdView->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    imageIdView->SetSrc(DE_IMAGE_APP);
    itemView->Add(imageIdView);

    int bundleNameOffset = 11;
    UILabel* name = new UILabel();
    name->SetPosition(APP_NAME_X, APP_NAME_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    name->SetText(pBundleInfo.bundleName + bundleNameOffset);
    name->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    name->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    itemView->Add(name);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(DE_FORWARD_IMG_X, DE_FORWARD_IMG_Y, DE_FORWARD_IMG_WIDTH, DE_FORWARD_IMG_HEIGHT);
    imageView->SetSrc(DE_IMAGE_FORWORD);
    itemView->Add(imageView);
}


void AppAbilitySlice::SetScrollView()
{
    scrollView_ = new UIScrollView();
    scrollView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_SCROLL_COLOR);
    scrollView_->SetPosition(DE_SCROLL_X, DE_SCROLL_Y, DE_SCROLL_WIDTH, DE_SCROLL_HEIGHT);
    scrollView_->SetXScrollBarVisible(false);
    scrollView_->SetYScrollBarVisible(true);
    rootView_->Add(scrollView_);

    uint8_t ret = -1;
    int num = 0;
    ret = GetBundleInfos(1, &pBundleInfos_, &num);
    if (ret == 0) {
        BundleInfo* pBundleInfo = pBundleInfos_;
        for (int count = 0; count < num; count++, pBundleInfo++) {
            printf("[LOG]pBundleInfo.bundleName->%s versionName->%s \n",
                pBundleInfo->bundleName, pBundleInfo->versionName);
            if (pBundleInfo->isSystemApp == false) {
                SetAnAppInfo(count, *pBundleInfo);
            }
        }
    }
}

void AppAbilitySlice::OnStart(const Want& want)
{
    AbilitySlice::OnStart(want);

    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);
    SetButtonListener();
    SetHead();
    SetScrollView();
    SetUIContent(rootView_);
}

void AppAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void AppAbilitySlice::OnActive(const Want &want)
{
    AbilitySlice::OnActive(want);
}

void AppAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void AppAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
} // namespace OHOS
