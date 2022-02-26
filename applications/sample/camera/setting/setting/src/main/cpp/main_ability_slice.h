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

#ifndef OHOS_MAIN_ABILITY_SLICE_H
#define OHOS_MAIN_ABILITY_SLICE_H

#include <cstdio>

#include "ability_info.h"
#include "ability_loader.h"
#include "ability_manager.h"
#include "ability_slice.h"
#include "bundle_manager.h"
#include "components/ui_image_view.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_list.h"
#include "components/ui_scroll_view.h"
#include "components/ui_toggle_button.h"
#include "element_name.h"
#include "event_listener.h"
#include "gfx_utils/list.h"
#include "module_info.h"
#include "parameter.h"
#include "pthread.h"
#include "setting_utils.h"
#include "want.h"
#include "wpa_work.h"

namespace OHOS {
class MainAbilitySlice : public AbilitySlice {
public:
    MainAbilitySlice()
        : headView_(nullptr), scrollView_(nullptr), rootView_(nullptr), lablelFontSsid_(nullptr),
          buttonWifiListener_(nullptr), buttonAppListener_(nullptr), buttonDisplayListener_(nullptr),
          buttonAboutListener_(nullptr), buttonBackListener_(nullptr) {}
    virtual ~MainAbilitySlice();

protected:
    void OnStart(const Want &want) override;
    void OnInactive() override;
    void OnActive(const Want &want) override;
    void OnBackground() override;
    void OnStop() override;

private:
    void SetButtonListenerWifi();
    void SetButtonListenerApp();
    void SetButtonListenerDisplay();
    void SetButtonListenerAbout();
    void SetAboutButtonView();
    void SetAppButtonView();
    void SetDisplayButtonView();
    void SetWifiButtonView();
    void SetScrollView();
    void SetHead();

    UIViewGroup* headView_;
    UIScrollView* scrollView_;
    RootView* rootView_;
    UILabel* lablelFontSsid_;
    EventListener* buttonWifiListener_;
    EventListener* buttonAppListener_;
    EventListener* buttonDisplayListener_;
    EventListener* buttonAboutListener_;
    EventListener* buttonBackListener_;

    constexpr static int WIFI_BUTTON_X = 0;
    constexpr static int WIFI_BUTTON_Y = 0;
    constexpr static int WIFI_BUTTON_TEXT_WIFI_Y = 13;
    constexpr static int WIFI_BUTTON_TEXT_SSID_X = 18;
    constexpr static int WIFI_BUTTON_TEXT_SSID_Y = 45;

    constexpr static int APP_BUTTON_X = 0;
    constexpr static int APP_BUTTON_Y = 95;

    constexpr static int DISPALY_BUTTON_X = 0;
    constexpr static int DISPALY_BUTTON_Y = 190;

    constexpr static int ABOUT_BUTTON_X = 0;
    constexpr static int ABOUT_BUTTON_Y = 190 + DE_BUTTON_HEIGHT + 6;
    constexpr static int ABOUT_BUTTON_HEIGHT = 113;
    constexpr static int ABOUT_BUTTON_TEXT_ABOUT_Y = 5;
    constexpr static int ABOUT_BUTTON_TEXT_SYSTEM_X = 18;
    constexpr static int ABOUT_BUTTON_TEXT_SYSTEM_Y = 39;
    constexpr static int ABOUT_BUTTON_TEXT_DEVICE_X = 18;
    constexpr static int ABOUT_BUTTON_TEXT_DEVICE_Y = 72;
    constexpr static int ABOUT_BUTTON_IMAGE_Y = 34;
};
}

#endif // OHOS_MAIN_ABILITY_SLICE_H
