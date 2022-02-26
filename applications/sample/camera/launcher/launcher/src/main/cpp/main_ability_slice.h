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

#include <ability_info.h>
#include <ability_loader.h>
#include <bundle_info.h>
#include <components/root_view.h>
#include <components/ui_image_view.h>
#include <components/ui_label.h>
#include <components/ui_label_button.h>
#include <components/ui_swipe_view.h>

#include "event_listener.h"
#include "gfx_utils/list.h"
#include "swipe_view.h"
#include "ui_config.h"

namespace OHOS {
class MainAbilitySlice : public AbilitySlice {
public:
    MainAbilitySlice() = default;
    ~MainAbilitySlice() override;

protected:
    void OnStart(const Want& want) override;
    void OnInactive() override;
    void OnActive(const Want& want) override;
    void OnBackground() override;
    void OnStop() override;
    void SetHead();
    void SetTail();
    void SetSwipe();
    void ReleaseView();
    void SetImageView();

private:
    SwipeView* swipeView_ { nullptr };
    UIImageView* uiImageView_ { nullptr };
    UILabel* lableHead_ { nullptr };
    UILabel* lableTail_ { nullptr };
    RootView *rootview_ { nullptr };
};
} // namespace OHOS
#endif
