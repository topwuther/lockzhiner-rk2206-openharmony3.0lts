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

#include "setting_display_ability_slice.h"
#include <iostream>
#include <power_screen_saver.h>
#include <securec.h>
#include "gfx_utils/style.h"

namespace OHOS {
REGISTER_AS(SettingDisplayAbilitySlice)

#define COM_SET_ON 0
#define COM_SET_OFF 1
#define COM_GET_STATUS 2

SettingDisplayAbilitySlice::~SettingDisplayAbilitySlice()
{
    if (toggleButtonView_) {
        DeleteChildren(toggleButtonView_);
        toggleButtonView_ = nullptr;
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
}

static int Callback(IOwner owner, int code, IpcIo *reply)
{
    size_t src = IpcIoPopInt32(reply);
    printf("[setting]IpcIoPopInt32 src -> %d\n", src);

    int ret = memcpy_s(owner, sizeof(size_t), &src, sizeof(size_t));
    if (ret < 0) {
        printf("memcpy_s Error\n");
        return -1;
    }
    printf("[setting]owner -> %d\n", *(int*)owner);
    return 0;
}

void SettingDisplayAbilitySlice::SetButtonListener(void)
{
    auto onClick = [this](UIView& view, const Event& event) -> bool {
        Want want1 = { nullptr };
        AbilitySlice* nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("MainAbilitySlice");
        if (nextSlice == nullptr) {
            printf("[warning]undefined MainAbilitySlice\n");
        } else {
            Present(*nextSlice, want1);
        }
        return true;
    };
    buttonBackListener_ = new EventListener(onClick, nullptr);
}

void SettingDisplayAbilitySlice::SetHead(void)
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
    lablelFont->SetText("显示");
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void SettingDisplayAbilitySlice::SetToggleButton(void)
{
    toggleButtonView_ = new UIViewGroup();
    toggleButtonView_->SetPosition(TOGGLE_X, TOGGLE_Y, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    toggleButtonView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    toggleButtonView_->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    toggleButtonView_->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    rootView_->Add(toggleButtonView_);

    auto lablelFont = new UILabel();
    lablelFont->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    lablelFont->SetText("屏保");
    lablelFont->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    toggleButtonView_->Add(lablelFont);

    SetScreenSaverState(TRUE);
    UIToggleButton* togglebutton = new UIToggleButton();
    togglebutton->SetState(true);
    changeListener_ = new DisBtnOnStateChangeListener(togglebutton);
    togglebutton->SetOnClickListener(changeListener_);
    togglebutton->SetPosition(DE_TOGGLE_BUTTON_X, DE_TOGGLE_BUTTON_Y);
    toggleButtonView_->Add(togglebutton);
}

void SettingDisplayAbilitySlice::OnStart(const Want& want)
{
    AbilitySlice::OnStart(want);

    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);
    SetButtonListener();
    SetHead();
    SetToggleButton();
    SetUIContent(rootView_);
}

void SettingDisplayAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void SettingDisplayAbilitySlice::OnActive(const Want& want)
{
    AbilitySlice::OnActive(want);
}

void SettingDisplayAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void SettingDisplayAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
} // namespace OHOS
