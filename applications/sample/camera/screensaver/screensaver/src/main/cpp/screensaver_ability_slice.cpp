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

#include "screensaver_ability_slice.h"
#include "common/screen.h"

namespace OHOS {
REGISTER_AS(ScreensaverAbilitySlice)
namespace {
    int16_t screenWidth = static_cast<int16_t>(Screen::GetInstance().GetWidth());
    int16_t screenHeight = static_cast<int16_t>(Screen::GetInstance().GetHeight());
}
static ImageAnimatorInfo g_imageAnimatorInfo[IMAGE_TOTEL_NUM] = {
    {IMG_DEFAULT_001_PATH, {0, 0}, screenWidth, screenHeight},
    {IMG_DEFAULT_002_PATH, {0, 0}, screenWidth, screenHeight},
    {IMG_DEFAULT_003_PATH, {0, 0}, screenWidth, screenHeight},
    {IMG_DEFAULT_004_PATH, {0, 0}, screenWidth, screenHeight},
    {IMG_DEFAULT_005_PATH, {0, 0}, screenWidth, screenHeight},
};

ScreensaverAbilitySlice::~ScreensaverAbilitySlice()
{
    if (imageAnimator_ != nullptr) {
        delete imageAnimator_;
        imageAnimator_ = nullptr;
    }

    if (exitListener_ != nullptr) {
        delete exitListener_;
        exitListener_ = nullptr;
    }
}

void ScreensaverAbilitySlice::SetCyclePlayView()
{
    imageAnimator_ = new UIImageAnimatorView();
    imageAnimator_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight());
    imageAnimator_->SetImageAnimatorSrc(g_imageAnimatorInfo, IMAGE_TOTEL_NUM, IMAGE_ANIMATOR_TIME_S);
    rootView_->Add(imageAnimator_);

    auto onClick = [this] (UIView& view, const Event& event) -> bool {
        TerminateAbility();
        return true;
    };
    exitListener_ = new EventListener(onClick, nullptr);
    imageAnimator_->SetOnClickListener(exitListener_);
    imageAnimator_->SetTouchable(true);

    imageAnimator_->Start();
}

void ScreensaverAbilitySlice::OnStart(const Want &want)
{
    rootView_ = RootView::GetWindowRootView();
    rootView_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight());
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);

    SetCyclePlayView();
    SetUIContent(rootView_);
    rootView_->Invalidate();
    AbilitySlice::OnStart(want);
}

void ScreensaverAbilitySlice::OnInactive()
{
    printf("ScreensaverAbilitySlice::OnInactive\n");
    AbilitySlice::OnInactive();
}

void ScreensaverAbilitySlice::OnActive(const Want &want)
{
    printf("ScreensaverAbilitySlice::OnActive\n");
    AbilitySlice::OnActive(want);
}

void ScreensaverAbilitySlice::OnBackground()
{
    printf("ScreensaverAbilitySlice::OnBackground\n");
    AbilitySlice::OnBackground();
}

void ScreensaverAbilitySlice::OnStop()
{
    printf("ScreensaverAbilitySlice::OnStop\n");
    AbilitySlice::OnStop();
}
} // namespace OHOS
