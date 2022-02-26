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

#include "long_press_view.h"
#include "ui_config.h"

namespace OHOS {
LongPressView::LongPressView(UninstallApp uninstall)
{
    bStatus_ = false;
    uninstall_ = uninstall;
    viewGroup_ = new UIViewGroup();
    viewGroup_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Black()));
    viewGroup_->SetStyle(STYLE_BORDER_RADIUS, GROUP_VIEW_RADIUS);
    viewGroup_->SetStyle(STYLE_BACKGROUND_OPA, UN_OPACITY);

    buttUninstall_ = new UILabelButton();
    buttUninstall_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Gray()));
    buttUninstall_->SetStyle(STYLE_BORDER_RADIUS, BUTTON_RADIUS);
    buttUninstall_->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    buttUninstall_->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_RADIUS, UIButton::PRESSED);
    buttUninstall_->SetStyleForState(STYLE_BACKGROUND_OPA, HALF_OPACITY, UIButton::PRESSED);
    buttUninstall_->SetText("卸载");
    buttUninstall_->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    buttUninstall_->SetOnClickListener(this);

    buttCancle_ = new UILabelButton();
    buttCancle_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Gray()));
    buttCancle_->SetStyle(STYLE_BORDER_RADIUS, BUTTON_RADIUS);
    buttCancle_->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    buttCancle_->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_RADIUS, UIButton::PRESSED);
    buttCancle_->SetStyleForState(STYLE_BACKGROUND_OPA, HALF_OPACITY, UIButton::PRESSED);
    buttCancle_->SetText("取消");
    buttCancle_->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    buttCancle_->SetOnClickListener(this);

    viewGroup_->Add(buttUninstall_);
    viewGroup_->Add(buttCancle_);
    viewGroup_->SetVisible(false);
}

LongPressView::~LongPressView()
{
    DeleteChildren(viewGroup_);
}

void LongPressView::RemoveLview()
{
    if (bStatus_ == false) {
        return;
    }
    viewParent_->Remove(viewGroup_);
    viewGroup_->SetVisible(false);
    viewParent_->Invalidate();
    bStatus_ = false;
}

void LongPressView::Show(UIViewGroup* viewParent, AppInfo* pApp)
{
    const int16_t HEIGHT_DISCOUNT = 3;
    const int16_t WIDTH_DISCOUNT = 2;
    bStatus_ = true;
    viewParent_ = viewParent;
    app_ = pApp;
    viewGroup_->SetPosition(pApp->buttonXY_.x / WIDTH_DISCOUNT + pApp->button_->GetWidth(),
        pApp->buttonXY_.y / WIDTH_DISCOUNT + pApp->button_->GetHeight(), pApp->button_->GetWidth(),
        (pApp->button_->GetHeight() * WIDTH_DISCOUNT) / HEIGHT_DISCOUNT + pApp->button_->GetHeight() / WIDTH_DISCOUNT);
    buttUninstall_->SetPosition(0, 0,
        pApp->button_->GetWidth(), pApp->button_->GetHeight() / WIDTH_DISCOUNT);
    buttCancle_->SetPosition(0, (pApp->button_->GetHeight() * WIDTH_DISCOUNT) / HEIGHT_DISCOUNT,
        pApp->button_->GetWidth(), pApp->button_->GetHeight() / WIDTH_DISCOUNT);
    viewGroup_->SetVisible(true);
    viewParent_->Add(viewGroup_);
    viewParent_->Invalidate();
}

bool LongPressView::OnClick(UIView& view, const ClickEvent& event)
{
    UIView *currentView = &view;
    if (currentView == nullptr) {
        return false;
    }
    UILabelButton* lbutt = nullptr;
    lbutt = static_cast<UILabelButton*>(currentView);
    RemoveLview();
    if (currentView == buttUninstall_) {
        uninstall_(app_);
    }
    return true;
}
} // namespace OHOS
