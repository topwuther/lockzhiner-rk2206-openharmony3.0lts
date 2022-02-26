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

#ifndef OHOS_PLAYER_ABILITY_SLICE_H
#define OHOS_PLAYER_ABILITY_SLICE_H

#include "ability_loader.h"
#include "animator/animator.h"
#include "animator/easing_equation.h"
#include "components/root_view.h"
#include "components/ui_label.h"
#include "components/ui_slider.h"
#include "components/ui_surface_view.h"
#include "components/ui_toggle_button.h"
#include "event_listener.h"
#include "gallery_config.h"
#include "player.h"
#include "securec.h"
#include "source.h"

namespace OHOS {
using OHOS::Media::Player;
using OHOS::Media::Source;
using namespace OHOS::Media;
struct PlayerAdapter {
    std::shared_ptr<Player> adapter;
    int32_t sourceType;
    char filePath[MAX_PATH_LENGTH];
};

class ToggleBtnListener : public UIView::OnClickListener {
public:
    explicit ToggleBtnListener(UIToggleButton* btn,
                               PlayerAdapter* sample,
                               Animator* animator,
                               UISurfaceView* surfaceView)
        : button_(btn),
          videoPlayer_(sample),
          animator_(animator) {}

    virtual ~ToggleBtnListener() {}

    bool OnClick(UIView &view, const ClickEvent& event) override;

    void SetCompleteFlag(bool state)
    {
        completeFlag_ = state;
    }

private:
    UIToggleButton* button_;
    PlayerAdapter* videoPlayer_;
    Animator* animator_;
    bool completeFlag_ { false };
};

class SliderAnimator : public Animator, public AnimatorCallback {
public:
    explicit SliderAnimator(PlayerAdapter *sample,
                            UISlider *slider,
                            UILabel *label,
                            int64_t duration,
                            UISurfaceView* surfaceView)
        : Animator(this, slider, duration, true),
          videoPlayer_(sample),
          slider_(slider),
          timeLabel_(label),
          duration_(duration),
          surfaceView_(surfaceView),
          needRefreshPlayer_(false) {}

    virtual ~SliderAnimator() {}

    void Callback(UIView* view) override;

    void SetToggleButton(UIToggleButton* toggleButton)
    {
        toggleButton_ = toggleButton;
    }

    void SetToggleBtnListener(ToggleBtnListener* listener)
    {
        listener_ = listener;
    }

private:
    PlayerAdapter* videoPlayer_;
    UISlider* slider_;
    UILabel* timeLabel_;
    int64_t duration_;
    UISurfaceView* surfaceView_;
    UIToggleButton* toggleButton_ { nullptr };
    ToggleBtnListener* listener_ { nullptr };
    bool needRefreshPlayer_;
};

class PlayerAbilitySlice : public AbilitySlice {
public:
    PlayerAbilitySlice() = default;
    ~PlayerAbilitySlice() override;
    static std::shared_ptr<Player> CreatePlayer();
protected:
    void OnStart(const Want &want) override;
    void OnInactive() override;
    void OnActive(const Want &want) override;
    void OnBackground() override;
    void OnStop() override;

private:
    void Clear();
    void ShowErrorTips();
    void SetUpRootView();
    void SetUpBackArea(const char* pathHeader);
    void SetUpVideoPlayer(const Want &want);
    bool SetUpSurfaceView();
    void SetUpProgress(int64_t duration);
    void SetUpAnimatorGroup(const char* pathHeader);
    void SetUpToggleButton(const char* pathHeader);

    PlayerAdapter* videoPlayer_ { nullptr };
    SliderAnimator* animator_ { nullptr };
    EventListener* backIconListener_ { nullptr };
    ToggleBtnListener* onClickListener_ { nullptr };

    RootView* rootView_ { nullptr };
    UIViewGroup* backArea_ { nullptr };
    UIImageView* backIcon_ { nullptr };
    UISurfaceView* surfaceView_ { nullptr };
    UIViewGroup* animatorGroup_ { nullptr };
    UIToggleButton* toggleButton_ { nullptr };
    UIViewGroup* toggleButtonArea_ { nullptr };
    UILabel* currentTimeLabel_ { nullptr };
    UISlider* slider_ { nullptr };
    UILabel* totalTimeLabel_ { nullptr };
    UILabel* errorTips_ { nullptr };
    char backIconAbsolutePath[MAX_PATH_LENGTH] = { 0 };
    char videoPlayAbsolutePath[MAX_PATH_LENGTH] = { 0 };
    char videoPauseAbsolutePath[MAX_PATH_LENGTH] = { 0 };
};
} // namespace OHOS
#endif // OHOS_PLAYER_ABILITY_SLICE_H
