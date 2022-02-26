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

#include "setting_about_ability_slice.h"
#include "gfx_utils/style.h"

namespace OHOS {
REGISTER_AS(SettingAboutAbilitySlice)

SettingAboutAbilitySlice::~SettingAboutAbilitySlice()
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
    for (int count = 0; count < SCROLL_ITEM_NUM; count++) {
        if (!itemInfo_[count][1]) {
            itemInfo_[count][1] = nullptr;
        }
    }
}

void SettingAboutAbilitySlice::SetButtonListener()
{
    auto onClick = [this](UIView& view, const Event& event) -> bool {
        Terminate();
        return true;
    };
    buttonBackListener_ = new EventListener(onClick, nullptr);
}

void SettingAboutAbilitySlice::SetItemInfo()
{
    itemInfo_[0][0] = (char*) "设备名称"; // 0
    itemInfo_[0][1] = GetDeviceType(); // 0
    itemInfo_[1][0] = (char*) "厂家信息"; // 1
    itemInfo_[1][1] = GetManufacture(); // 1
    itemInfo_[2][0] = (char*) "品牌信息"; // 2
    itemInfo_[2][1] = GetBrand(); // 2
    itemInfo_[3][0] = (char*) "硬件版本号"; // 3
    itemInfo_[3][1] = GetHardwareModel(); // 3
    itemInfo_[4][0] = (char*) "设备序列号"; // 4
    itemInfo_[4][1] = GetSerial(); // 4
    itemInfo_[5][0] = (char*) "操作系统名"; // 5
    itemInfo_[5][1] = GetOSFullName(); // 5
    itemInfo_[6][0] = (char*) "软件版本号"; // 6
    itemInfo_[6][1] = GetDisplayVersion(); // 6
    itemInfo_[7][0] = (char*) "BootLoader版本号"; // 7
    itemInfo_[7][1] = GetBootloaderVersion(); // 7
    itemInfo_[8][0] = (char*) "构建时间"; // 8
    itemInfo_[8][1] = GetBuildTime(); // 8
}

void SettingAboutAbilitySlice::SetHead()
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
    lablelFont->SetText("关于");
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void SettingAboutAbilitySlice::SetScrollItem(int count)
{
    int myPositonY = count * DE_ITEM_INTERVAL;

    UIViewGroup* itemView = new UIViewGroup();
    itemView->SetPosition(ITEM_X, myPositonY, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    itemView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    itemView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    scrollView_->Add(itemView);

    UILabel* lablelFontName = new UILabel();
    lablelFontName->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFontName->SetText(itemInfo_[count][0]);
    lablelFontName->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    lablelFontName->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    itemView->Add(lablelFontName);

    UILabel* lablelFontInfo = new UILabel();
    lablelFontInfo->SetPosition(ITEM_INFO_X, ITEM_INFO_Y, DE_SUBTITLE_TEXT_WIDTH, DE_SUBTITLE_TEXT_HEIGHT);
    lablelFontInfo->SetText(itemInfo_[count][1]);
    lablelFontInfo->SetFont(DE_FONT_OTF, DE_SUBTITLE_TEXT_SIZE);
    lablelFontInfo->SetAlign(TEXT_ALIGNMENT_RIGHT);
    lablelFontInfo->SetStyle(STYLE_TEXT_COLOR, DE_SUBTITLE_TEXT_COLOR);
    itemView->Add(lablelFontInfo);
}

void SettingAboutAbilitySlice::SetScroll()
{
    scrollView_ = new UIScrollView();
    scrollView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_SCROLL_COLOR);
    scrollView_->SetPosition(DE_SCROLL_X, DE_SCROLL_Y, DE_SCROLL_WIDTH, DE_SCROLL_HEIGHT);
    scrollView_->SetXScrollBarVisible(false);
    scrollView_->SetYScrollBarVisible(true);
    rootView_->Add(scrollView_);
    for (int count = 0; count < SCROLL_ITEM_NUM; count++) {
        SetScrollItem(count);
    }
}

void SettingAboutAbilitySlice::OnStart(const Want& want)
{
    AbilitySlice::OnStart(want);
    SetButtonListener();
    SetItemInfo();

    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);
    SetHead();
    SetScroll();

    SetUIContent(rootView_);
}

void SettingAboutAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void SettingAboutAbilitySlice::OnActive(const Want& want)
{
    AbilitySlice::OnActive(want);
}

void SettingAboutAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void SettingAboutAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
} // namespace OHOS
