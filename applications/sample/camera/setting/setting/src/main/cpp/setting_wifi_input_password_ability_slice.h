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

#ifndef OHOS_SETTING_WIFI_INPUT_PASSWORD_ABILITY_SLICE_H
#define OHOS_SETTING_WIFI_INPUT_PASSWORD_ABILITY_SLICE_H

#include "ability_loader.h"
#include "common/task.h"
#include "components/ui_checkbox.h"
#include "components/ui_image_view.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_list.h"
#include "components/ui_scroll_view.h"
#include "components/ui_view_group.h"
#include "event_listener.h"
#include "parameter.h"
#include "setting_utils.h"
#include "setting_wifi_input_password_ability_slice.h"
#include "wpa_work.h"

namespace OHOS {
class SettingWifiInputPasswordAbilitySlice : public AbilitySlice {
public:
    SettingWifiInputPasswordAbilitySlice() : headView_(nullptr), inputView_(nullptr), lablelCursorText_(nullptr),
                                             scrollView_(nullptr), rootView_(nullptr), lablelInputText_(nullptr),
                                             buttonBackListener_(nullptr) {}
    virtual ~SettingWifiInputPasswordAbilitySlice();

protected:
    void OnStart(const Want& want) override;
    void OnInactive() override;
    void OnActive(const Want& want) override;
    void OnBackground() override;
    void OnStop() override;

private:
    void SetHead();
    void SetInput();
    void AddInputKeyBoardZero();
    void SetScrollView();
    void SetButtonListener();
    UIViewGroup* headView_;
    UIViewGroup* inputView_;
    UILabel* lablelCursorText_;
    UIScrollView* scrollView_;
    RootView* rootView_;
    UILabel* lablelInputText_;
    EventListener* buttonBackListener_;

    constexpr static int CURSOR_POSITION_OFFSET = 16;
    constexpr static int RECT_RADIUS = 6;
    constexpr static int INPUT_X = 36;
    constexpr static int INPUT_Y = 108;
    constexpr static int INPUT_WIDTH = 888;
    constexpr static int INPUT_HEIGHT = 54;
    constexpr static int INPUT_RADIUS = 32;
    constexpr static int INPUT_CURSOR_X = 20;
    constexpr static int INPUT_CURSOR_Y = 10;
    constexpr static int INPUT_CURSOR_WIDTH = 2;
    constexpr static int INPUT_CURSOR_HEIGHT = 36;
    constexpr static int INPUT_TEXT_X = 24;
    constexpr static int INPUT_TEXT_Y = 11;
    constexpr static int INPUT_TEXT_WIDTH = 480;
    constexpr static int INPUT_TEXT_HEIGHT = 38;
    constexpr static int INPUT_FONT_SIZE = 28;
    constexpr static int INPUT_ENTER_X = 824;
    constexpr static int INPUT_ENTER_Y = 6;
    constexpr static int INPUT_ENTER_WIDTH = 58;
    constexpr static int INPUT_ENTER_HEIGHT = 42;
    constexpr static int INPUT_ENTER_RADIUS = 32;

    constexpr static int INPUT_IMAGE_X = 10;
    constexpr static int INPUT_IMAGE_Y = 2;
    constexpr static int INPUT_IMAGE_WIDTH = 38;
    constexpr static int INPUT_IMAGE_HEIGHT = 38;

    constexpr static int BUTTON_NUM = 3;

    constexpr static int BUTTON_INTERVAL_X = 172;
    constexpr static int BUTTON_INTERVAL_Y = 66;
    constexpr static int BUTTON_WIDTH = 160;
    constexpr static int BUTTON_HEIGHT = 54;

    constexpr static int SCROLL_WIFI_INPUT_X = 228;
    constexpr static int SCROLL_WIFI_INPUT_Y = 198;
    constexpr static int SCROLL_WIFI_INPUT_WIDTH = 530;
    constexpr static int SCROLL_WIFI_INPUT_HEIGHT = 252;
};

} // namespace OHOS
#endif
