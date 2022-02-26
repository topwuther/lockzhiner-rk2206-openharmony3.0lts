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

#ifndef OHOS_SETTING_ABOUT_ABILITY_SLICE_H
#define OHOS_SETTING_ABOUT_ABILITY_SLICE_H

#include "ability_info.h"
#include "ability_loader.h"
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
#include "setting_utils.h"
#include "want.h"

namespace OHOS {
class SettingAboutAbilitySlice : public AbilitySlice {
public:
    SettingAboutAbilitySlice()
        : headView_(nullptr), scrollView_(nullptr), rootView_(nullptr), buttonBackListener_(nullptr) {}
    virtual ~SettingAboutAbilitySlice();
protected:
    void OnStart(const Want& want) override;
    void OnInactive() override;
    void OnActive(const Want& want) override;
    void OnBackground() override;
    void OnStop() override;
private:
    void SetItemInfo();
    void SetButtonListener();
    void SetScrollItem(int count);
    void SetScroll();
    void SetHead();

    UIViewGroup* headView_;
    UIScrollView* scrollView_;
    RootView* rootView_;
    EventListener *buttonBackListener_;
    constexpr static int SCROLL_ITEM_NUM = 9;
    const char *itemInfo_[SCROLL_ITEM_NUM][2];
    constexpr static int ITEM_X = 0;
    constexpr static int ITEM_INFO_X = 465;
    constexpr static int ITEM_INFO_Y = 36;
};
} // namespace OHOS
#endif
