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

#include "main_ability_slice.h"
#include "ability_loader.h"
#include "ability_slice.h"
#include "ability_info.h"
#include "ability_manager.h"
#include "event_listener.h"
#include "bundle_manager.h"
#include "module_info.h"
#include "element_name.h"
#include "wpa_work.h"
#include "gfx_utils/style.h"
#include <cstdint>
#include <ctime>

namespace OHOS {
REGISTER_AS(MainAbilitySlice)

MainAbilitySlice::~MainAbilitySlice()
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

    if (buttonWifiListener_) {
        delete buttonWifiListener_;
        buttonWifiListener_ = nullptr;
    }

    if (buttonDisplayListener_) {
        delete buttonDisplayListener_;
        buttonDisplayListener_ = nullptr;
    }

    if (buttonAppListener_) {
        delete buttonAppListener_;
        buttonAppListener_ = nullptr;
    }

    if (buttonAboutListener_) {
        delete buttonAboutListener_;
        buttonAboutListener_ = nullptr;
    }
}

void MainAbilitySlice::SetButtonListenerWifi(void)
{
    auto onClick1 = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("SettingWifiAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingWifiAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonWifiListener_ = new EventListener(onClick1, nullptr);
}

void MainAbilitySlice::SetButtonListenerApp(void)
{
    auto onClick2 = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("AppAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined AppInfoAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonAppListener_ = new EventListener(onClick2, nullptr);
}

void MainAbilitySlice::SetButtonListenerDisplay(void)
{
    auto onClick3 = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("SettingDisplayAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingDisplayAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonDisplayListener_ = new EventListener(onClick3, nullptr);
}


void MainAbilitySlice::SetButtonListenerAbout(void)
{
    auto onClick4 = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("SettingAboutAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined SettingAboutAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonAboutListener_ = new EventListener(onClick4, nullptr);
}

void MainAbilitySlice::SetHead(void)
{
    auto toLaunher = [this] (UIView &view, const Event &event) -> bool {
        TerminateAbility();
        return true;
    };
    buttonBackListener_ = new EventListener(toLaunher, nullptr);

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
    lablelFont->SetText("设置");
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void MainAbilitySlice::SetWifiButtonView(void)
{
    UIViewGroup* buttonView = new UIViewGroup();
    buttonView->SetPosition(WIFI_BUTTON_X, WIFI_BUTTON_Y, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    buttonView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    buttonView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    buttonView->SetTouchable(true);
    buttonView->SetOnClickListener(buttonWifiListener_);
    scrollView_->Add(buttonView);

    UILabel* lablelFontWifi = new UILabel();
    lablelFontWifi->SetPosition(DE_TITLE_TEXT_X, WIFI_BUTTON_TEXT_WIFI_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFontWifi->SetText("WiFi");
    lablelFontWifi->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    lablelFontWifi->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    buttonView->Add(lablelFontWifi);

    char buff[64] = {0};
    int myX = WIFI_BUTTON_TEXT_SSID_X;
    int myY = WIFI_BUTTON_TEXT_SSID_Y;
    int ret = GetCurrentConnInfo(buff, sizeof(buff));
    lablelFontSsid_ = new UILabel();
    lablelFontSsid_->SetPosition(myX, myY, DE_SUBTITLE_TEXT_WIDTH, DE_SUBTITLE_TEXT_HEIGHT);
    if (ret == 0) {
        lablelFontSsid_->SetText(buff);
    } else {
        lablelFontSsid_->SetText("未连接");
    }

    lablelFontSsid_->SetFont(DE_FONT_OTF, DE_SUBTITLE_TEXT_SIZE);
    lablelFontSsid_->SetStyle(STYLE_TEXT_COLOR, DE_SUBTITLE_TEXT_COLOR);
    buttonView->Add(lablelFontSsid_);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(DE_FORWARD_IMG_X, DE_FORWARD_IMG_Y, DE_FORWARD_IMG_WIDTH, DE_FORWARD_IMG_HEIGHT);
    imageView->SetSrc(DE_IMAGE_FORWORD);
    buttonView->Add(imageView);
}

void MainAbilitySlice::SetAppButtonView(void)
{
    UIViewGroup* buttonView = new UIViewGroup();
    buttonView->SetPosition(APP_BUTTON_X, APP_BUTTON_Y, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    buttonView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    buttonView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    buttonView->SetTouchable(true);
    buttonView->SetOnClickListener(buttonAppListener_);
    scrollView_->Add(buttonView);

    UILabel* lablelFont = new UILabel();
    lablelFont->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFont->SetText("应用");
    lablelFont->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);

    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    buttonView->Add(lablelFont);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(DE_FORWARD_IMG_X, DE_FORWARD_IMG_Y, DE_FORWARD_IMG_WIDTH, DE_FORWARD_IMG_HEIGHT);
    imageView->SetSrc(DE_IMAGE_FORWORD);
    buttonView->Add(imageView);
}

void MainAbilitySlice::SetDisplayButtonView(void)
{
    UIViewGroup* buttonView = new UIViewGroup();
    buttonView->SetPosition(DISPALY_BUTTON_X, DISPALY_BUTTON_Y, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    buttonView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    buttonView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    buttonView->SetTouchable(true);
    buttonView->SetOnClickListener(buttonDisplayListener_);
    scrollView_->Add(buttonView);

    UILabel* lablelFont = new UILabel();
    lablelFont->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFont->SetText("显示");
    lablelFont->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);

    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    buttonView->Add(lablelFont);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(DE_FORWARD_IMG_X, DE_FORWARD_IMG_Y, DE_FORWARD_IMG_WIDTH, DE_FORWARD_IMG_HEIGHT);
    imageView->SetSrc(DE_IMAGE_FORWORD);
    buttonView->Add(imageView);
}

static void setAboutTest(UIViewGroup *buttonView, int positionX, int positionY, const char *setText)
{
    UILabel* lablelFontSystem = new UILabel();
    lablelFontSystem->SetPosition(positionX, positionY, DE_SUBTITLE_TEXT_WIDTH, DE_SUBTITLE_TEXT_HEIGHT);
    lablelFontSystem->SetText(setText);
    lablelFontSystem->SetFont(DE_FONT_OTF, DE_SUBTITLE_TEXT_SIZE);
    lablelFontSystem->SetStyle(STYLE_TEXT_COLOR, DE_SUBTITLE_TEXT_COLOR);
    buttonView->Add(lablelFontSystem);
}

void MainAbilitySlice::SetAboutButtonView(void)
{
    UIViewGroup* buttonView = new UIViewGroup();
    buttonView->SetPosition(ABOUT_BUTTON_X, ABOUT_BUTTON_Y, DE_BUTTON_WIDTH, ABOUT_BUTTON_HEIGHT);
    buttonView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    buttonView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    buttonView->SetTouchable(true);
    buttonView->SetOnClickListener(buttonAboutListener_);
    scrollView_->Add(buttonView);

    UILabel* lablelFontAbout = new UILabel();
    lablelFontAbout->SetPosition(DE_TITLE_TEXT_X, ABOUT_BUTTON_TEXT_ABOUT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFontAbout->SetText("关于");
    lablelFontAbout->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    lablelFontAbout->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    buttonView->Add(lablelFontAbout);

    char buff[62];
    const char* gDV = GetDisplayVersion();
    int err = sprintf_s(buff, sizeof(buff), "系统版本: %s", gDV);
    if (err < 0) {
        printf("[ERROR]sprintf_s failed, err = %d\n", err);
        gDV = nullptr;
        return;
    }
    setAboutTest(buttonView, ABOUT_BUTTON_TEXT_SYSTEM_X, ABOUT_BUTTON_TEXT_SYSTEM_Y, buff);

    err = memset_s(buff, sizeof(buff), 0, sizeof(buff));
    if (err < EOK) {
        printf("[ERROR]memset_s failed, err = %d\n", err);
        return;
    }
    const char* gPT = GetDeviceType();
    err = sprintf_s(buff, sizeof(buff), "设备名称: %s", gPT);
    if (err < 0) {
        printf("[ERROR]sprintf_s failed, err = %d\n", err);
        gPT = nullptr;
        return;
    }
    setAboutTest(buttonView, ABOUT_BUTTON_TEXT_DEVICE_X, ABOUT_BUTTON_TEXT_DEVICE_Y, buff);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(DE_FORWARD_IMG_X, ABOUT_BUTTON_IMAGE_Y, DE_FORWARD_IMG_WIDTH, DE_FORWARD_IMG_HEIGHT);
    imageView->SetSrc(DE_IMAGE_FORWORD);
    buttonView->Add(imageView);
}

void MainAbilitySlice::SetScrollView()
{
    scrollView_ = new UIScrollView();
    scrollView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_SCROLL_COLOR);
    scrollView_->SetPosition(DE_SCROLL_X, DE_SCROLL_Y, DE_SCROLL_WIDTH, DE_SCROLL_HEIGHT);
    scrollView_->SetXScrollBarVisible(false);
    scrollView_->SetYScrollBarVisible(false);
    rootView_->Add(scrollView_);
    SetWifiButtonView();
    SetAppButtonView();
    SetDisplayButtonView();
    SetAboutButtonView();
}

void MainAbilitySlice::OnStart(const Want& want)
{
    AbilitySlice::OnStart(want);
    SetButtonListenerWifi();
    SetButtonListenerApp();
    SetButtonListenerDisplay();
    SetButtonListenerAbout();
    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);

    SetHead();
    SetScrollView();
    SetUIContent(rootView_);
}

void MainAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void MainAbilitySlice::OnActive(const Want& want)
{
    char buff[64] = {0};
    int ret = GetCurrentConnInfo(buff, sizeof(buff));
    if (ret == 0) {
        printf("##### SetText -> %s \n", buff);
        lablelFontSsid_->SetText(buff);
    } else {
        lablelFontSsid_->SetText("未连接");
    }
    AbilitySlice::OnActive(want);
}

void MainAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void MainAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
} // namespace OHOS