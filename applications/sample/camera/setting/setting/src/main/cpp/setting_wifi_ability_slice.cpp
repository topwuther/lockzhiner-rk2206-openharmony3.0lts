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

#include "setting_wifi_ability_slice.h"
#include <iostream>
#include <thread>
#include "gfx_utils/style.h"

namespace OHOS {
REGISTER_AS(SettingWifiAbilitySlice)
static int g_wifiStatus = 0;

SettingWifiAbilitySlice::SettingWifiAbilitySlice()
    : headView_(nullptr), toggleButtonView_(nullptr), scrollView_(nullptr), rootView_(nullptr),
      changeListener_(nullptr), buttonBackListener_(nullptr), buttonInputListener_(nullptr), wpaCount_(0)
{
    int taskPeriod = 5000;
    Task::Init();
    SetPeriod(taskPeriod);
}

SettingWifiAbilitySlice::~SettingWifiAbilitySlice()
{
    if (wpaCount_ == 1) {
        printf("%s:%d call ExitWpa, this %p\n", __FUNCTION__, __LINE__, this);
        ExitWpa();
        ExitWpaScan();
        wpaCount_ = 0;
    }

    if (toggleButtonView_) {
        DeleteChildren(toggleButtonView_);
        toggleButtonView_ = nullptr;
    }

    if (scrollView_) {
        DeleteChildren(scrollView_);
        scrollView_ = nullptr;
    }

    if (headView_) {
        DeleteChildren(headView_);
        headView_ = nullptr;
    }

    if (changeListener_) {
        delete changeListener_;
        changeListener_ = nullptr;
    }

    if (buttonBackListener_) {
        delete buttonBackListener_;
        buttonBackListener_ = nullptr;
    }

    if (buttonInputListener_) {
        delete buttonInputListener_;
        buttonInputListener_ = nullptr;
    }
}

void SettingWifiAbilitySlice::SetButtonListener(void)
{
    auto onClick = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("MainAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingWifiAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonBackListener_ = new EventListener(onClick, nullptr);
}

void SettingWifiAbilitySlice::SetWifiButtonListener(char* ssid)
{
    auto onClick2 = [this, ssid](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        bool ret = SetWantData(&want1, ssid, strlen(ssid) + 1);
        if (ret != true) {
            return false;
        }
        StartAbility(want1);
        AbilitySlice* nextSlice =
            AbilityLoader::GetInstance().GetAbilitySliceByName("SettingWifiInputPasswordAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingWifiAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonInputListener_ = new EventListener(onClick2, nullptr);
}

void SettingWifiAbilitySlice::SetHead(void)
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
    lablelFont->SetText("WiFi");
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void SettingWifiAbilitySlice::SetToggleButton(void)
{
    toggleButtonView_ = new UIViewGroup();
    toggleButtonView_->SetPosition(TOGGLE_X, TOGGLE_Y, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    toggleButtonView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    toggleButtonView_->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    toggleButtonView_->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    rootView_->Add(toggleButtonView_);

    auto lablelFont = new UILabel();
    lablelFont->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFont->SetText("WiFi");
    lablelFont->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    toggleButtonView_->Add(lablelFont);

    UIToggleButton* togglebutton = new UIToggleButton();
    changeListener_ = new TestBtnOnStateChangeListener(reinterpret_cast<UIView*>(scrollView_));
    togglebutton->SetOnClickListener(changeListener_);
    togglebutton->SetPosition(DE_TOGGLE_BUTTON_X, DE_TOGGLE_BUTTON_Y);
    togglebutton->SetState(true);
    scrollView_->SetVisible(true);

    toggleButtonView_->Add(togglebutton);
}

void SettingWifiAbilitySlice::SetUseWifi(void)
{
    UILabel* lablelFont = new UILabel();
    lablelFont->SetPosition(USE_WIFI_FONT_X, USE_WIFI_FONT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFont->SetText("可用WiFi列表");
    lablelFont->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_SUBTITLE_TEXT_COLOR);
    rootView_->Add(lablelFont);
}

void SettingWifiAbilitySlice::AddWifi(void)
{
    int ssidIndex, ssidCount;

    ssidCount = GetIdNum();
    if (ssidCount == 0) {
        printf("[LOG]SettingWifiAbilitySlice::AddWifi ssidCount == 0 \n");
        return;
    }
    if (scrollView_ == nullptr) {
        printf("[LOG]SettingWifiAbilitySlice::AddWifi scrollView_ == nullptr \n");
        return;
    }
    if (g_wifiStatus == 1) {
        printf("[LOG]SettingWifiAbilitySlice::AddWifi wifiStatus == 1 \n");
        return;
    }
    for (ssidIndex = 0; ssidIndex < ssidCount; ssidIndex++) {
        UIViewGroup *useWifiView = new UIViewGroup();
        useWifiView->SetPosition(ADD_WIFI_X, DE_ITEM_INTERVAL * ssidIndex, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
        useWifiView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
        useWifiView->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
        useWifiView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
        useWifiView->SetTouchable(true);
        char* buff = GetSsid(ssidIndex); // GetSsid need return point
        SetWifiButtonListener(buff);
        useWifiView->SetOnClickListener(buttonInputListener_);
        scrollView_->Add(useWifiView);

        auto lablelFont = new UILabel();
        lablelFont->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
        lablelFont->SetText(buff);
        lablelFont->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
        lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
        useWifiView->Add(lablelFont);
        g_wifiStatus = 1;
    }

    scrollView_->Invalidate();
}

void SettingWifiAbilitySlice::SetScrollWifi(void)
{
    scrollView_ = new UIScrollView();
    scrollView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_SCROLL_COLOR);
    scrollView_->SetPosition(DE_SCROLL_X, SCROLL_WIFI_Y, DE_SCROLL_WIDTH, SCROLL_WIFI_HEIGHT);
    scrollView_->SetXScrollBarVisible(false);
    scrollView_->SetYScrollBarVisible(true);
    rootView_->Add(scrollView_);
    g_wifiStatus = 0;
    AddWifi();
}

void SettingWifiAbilitySlice::Callback()
{
    if (GetAndResetScanStat() == 1) {
        LockWifiData();
        AddWifi();
        UnLockWifiData();
    }
}

void SettingWifiAbilitySlice::OnStart(const Want& want)
{
    if (wpaCount_ == 0) {
        printf("%s:%d call WpaClientStart, this %p\n", __FUNCTION__, __LINE__, this);
        WpaClientStart();
        WpaScanReconnect(nullptr, nullptr, HIDDEN_CLOSE);
        wpaCount_ = 1;
    }
    AbilitySlice::OnStart(want);

    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);
    SetButtonListener();
    SetHead();
    SetUseWifi();
    SetScrollWifi();
    SetToggleButton();

    TaskExecute();
    SetUIContent(rootView_);
}

void SettingWifiAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void SettingWifiAbilitySlice::OnActive(const Want& want)
{
    AbilitySlice::OnActive(want);
}

void SettingWifiAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void SettingWifiAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
    if (wpaCount_ == 1) {
        printf("%s:%d call ExitWpa, this %p\n", __FUNCTION__, __LINE__, this);
        ExitWpa();
        ExitWpaScan();
        wpaCount_ = 0;
    }
}
} // namespace OHOS
