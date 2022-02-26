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

#ifndef OHOS_APP_ABILITY_SLICE_H
#define OHOS_APP_ABILITY_SLICE_H

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
#include "setting_utils.h"
#include "want.h"
#include "securec.h"

namespace OHOS {
class AppAbilitySlice : public AbilitySlice {
public:
    AppAbilitySlice()
        : headView_(nullptr), scrollView_(nullptr), rootView_(nullptr), pBundleInfos_(nullptr),
          buttonBackListener_(nullptr), buttonAppInfoListener_(nullptr) {}
    virtual ~AppAbilitySlice();
protected:
    void OnStart(const Want& want) override;
    void OnInactive() override;
    void OnActive(const Want& want) override;
    void OnBackground() override;
    void OnStop() override;

private:
    void SetButtonListener();
    void SetAppButtonListener(const char* appName);
    void SetHead();
    void SetScrollView();
    void SetAnAppInfo(int count, BundleInfo& pBundleInfo);
    UIViewGroup* headView_;
    UIScrollView* scrollView_;
    RootView* rootView_;
    BundleInfo* pBundleInfos_;
    EventListener* buttonBackListener_;
    EventListener* buttonAppInfoListener_;

    constexpr static int APP_IMAGE_X = 12;
    constexpr static int APP_IMAGE_Y = 12;
    constexpr static int APP_IMAGE_WIDTH = 64;
    constexpr static int APP_IMAGE_HEIGHT = 64;
    constexpr static int APP_NAME_X = 94;
    constexpr static int APP_NAME_Y = 28;
};
} // namespace OHOS
#endif
