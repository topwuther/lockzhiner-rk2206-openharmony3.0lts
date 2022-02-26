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

#include "app_info.h"
#include "ui_config.h"

namespace OHOS {
AppInfo::AppInfo()
{
    ReSet();
}
void AppInfo::Release()
{
    if (button_) {
        delete button_;
        button_ = nullptr;
    }
    if (lable_) {
        delete lable_;
        lable_ = nullptr;
    }
    if (appClickListener_) {
        delete appClickListener_;
        appClickListener_ = nullptr;
    }
    if (appLpListener_) {
        delete appLpListener_;
        appLpListener_ = nullptr;
    }
}

AppInfo::~AppInfo()
{
    Release();
}

void AppInfo::ReSet()
{
    button_ = nullptr;
    lable_ = nullptr;
    appClickListener_ = nullptr;
    appLpListener_ = nullptr;
}

void AppInfo::SetButton(UILabelButton* button)
{
    button->SetPosition(buttonXY_.x, buttonXY_.y, buttonHV_.x, buttonHV_.y);
    button->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    button->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_RADIUS, UIButton::PRESSED);
    button->SetStyleForState(STYLE_BACKGROUND_OPA, TOTAL_OPACITY, UIButton::PRESSED);
    button->SetStyleForState(STYLE_BORDER_OPA, TOTAL_OPACITY, UIButton::PRESSED);
    button->SetStyle(STYLE_BORDER_RADIUS, BUTTON_RADIUS);
    button->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    button->SetStyle(STYLE_BORDER_OPA, TOTAL_OPACITY);
    button->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    button->SetImageSrc(appIconDir_, appIconDir_);
    button_ = button;
}

void AppInfo::SetLable(UILabel* lable)
{
    lable->SetPosition(lableXY_.x, lableXY_.y, lableHV_.x, lableHV_.y);
    char* p = strrchr(appName_, static_cast<int>('.'));
    if (p != nullptr) {
        lable->SetText(p + 1);
    } else {
        lable->SetText(appName_);
    }
    lable->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_TOP);
    lable->SetFont(FOND_PATH, APP_FOND_ID);
    lable->SetStyle(STYLE_BORDER_RADIUS, LABLE_RADIUS);
    lable->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    lable_ = lable;
}

void AppInfo::SetListener(AppInfo* app)
{
    AppClickListener* clickListener = new AppClickListener(app->funcclick_, this);
    button_->SetOnClickListener(clickListener);
    appClickListener_ = static_cast<UIView::OnClickListener*>(clickListener);
    AppLongPressListener* lPListener = new AppLongPressListener(app->funclPress_, this);
    button_->SetOnLongPressListener(lPListener);
    appLpListener_ = static_cast<UIView::OnLongPressListener*>(lPListener);
    button_->SetDraggable(true);
}

void AppInfo::SetLocation(int16_t r, int16_t c)
{
    row_col_.x = r;
    row_col_.y = c;
}
} // namespace OHOS
