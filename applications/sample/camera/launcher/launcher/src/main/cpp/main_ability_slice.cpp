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

#include <cstdio>
#include <common/screen.h>
#include <components/ui_label.h>
#include <components/ui_label_button.h>

#include "main_ability_slice.h"
#include "ability_manager.h"

namespace OHOS {
REGISTER_AS(MainAbilitySlice)

MainAbilitySlice::~MainAbilitySlice()
{
    ReleaseView();
}

void MainAbilitySlice::ReleaseView()
{
    if (swipeView_) {
        delete swipeView_;
        swipeView_ = nullptr;
    }
    if (uiImageView_) {
        delete uiImageView_;
        uiImageView_ = nullptr;
    }
    if (lableHead_) {
        delete lableHead_;
        lableHead_ = nullptr;
    }
    if (lableTail_) {
        delete lableTail_;
        lableTail_ = nullptr;
    }
}

void MainAbilitySlice::SetHead()
{
    char tmp[TMP_BUF_SIZE] = { 0 };
    time_t t = time(nullptr);
    struct tm* st = nullptr;
    st = localtime(&t);
    sprintf_s(tmp, sizeof(tmp), "%02d : %02d", st->tm_hour, st->tm_min);
    UILabel* label = new UILabel();
    rootview_->Add(label);
    label->SetPosition(0, 0, Screen::GetInstance().GetWidth(), LABLE_TITLE_HEIGHT);
    label->SetText(tmp);
    label->SetAlign(TEXT_ALIGNMENT_RIGHT, TEXT_ALIGNMENT_TOP);
    label->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    label->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    label->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);

    lableHead_ = label;
}

void MainAbilitySlice::SetTail()
{
    UILabel* label = new UILabel();
    rootview_->Add(label);
    label->SetPosition(0, Screen::GetInstance().GetHeight() - LABLE_TAIL_HEIGHT,
                       Screen::GetInstance().GetWidth(), LABLE_TAIL_HEIGHT);
    char buf[TMP_BUF_SIZE] = { 0 };
    sprintf_s(buf, sizeof(buf), ".%d.", 1);
    label->SetText(buf);
    label->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_CENTER);
    label->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    label->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    label->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);

    lableTail_ = label;
}

void MainAbilitySlice::SetImageView()
{
    uiImageView_ = new UIImageView();
    // modify image view height

    uiImageView_->SetPosition(0, 0, Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight());
    uiImageView_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::White()));
    uiImageView_->SetSrc(TABLE_BACKGROUND);
    uiImageView_->SetStyle(STYLE_BACKGROUND_OPA, UN_OPACITY);
    rootview_->Add(uiImageView_);
}

void MainAbilitySlice::SetSwipe()
{
    swipeView_ = new SwipeView(lableHead_, lableTail_);
    swipeView_->OnSetUpView();
    rootview_->Add(swipeView_->GetSwipeView());
}

void MainAbilitySlice::OnStart(const Want& want)
{
    AbilitySlice::OnStart(want);
    rootview_ = RootView::GetWindowRootView();
    rootview_->SetPosition(0, 0);
    rootview_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight());
    rootview_->SetStyle(STYLE_BACKGROUND_OPA, UN_OPACITY);
    rootview_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::GetColorFromRGB(0x30, 0x30, 0x30)));

    SetHead();
    SetTail();
    SetSwipe();
    SetUIContent(rootview_);
    rootview_->Invalidate();
}

void MainAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void MainAbilitySlice::OnActive(const Want& want)
{
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
