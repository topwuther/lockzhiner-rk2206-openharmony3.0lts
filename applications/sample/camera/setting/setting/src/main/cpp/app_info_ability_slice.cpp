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

#include "app_info_ability_slice.h"
#include "gfx_utils/style.h"

namespace OHOS {
REGISTER_AS(AppInfoAbilitySlice)

AppInfoAbilitySlice::~AppInfoAbilitySlice()
{
    if (scrollView_) {
        DeleteChildren(scrollView_);
        scrollView_ = nullptr;
    }

    if (headView_) {
        DeleteChildren(headView_);
        headView_ = nullptr;
    }

    if (!buttonBackListener_) {
        delete buttonBackListener_;
        buttonBackListener_ = nullptr;
    }

    if (permissions_) {
        free(permissions_);
    }
    ListNode<ToggBtnOnListener*>* node = listListener_.Begin();
    while (node != listListener_.End()) {
        delete node->data_;
        node = node->next_;
    }
    listListener_.Clear();
}

void AppInfoAbilitySlice::SetButtonListener(void)
{
    auto onClick = [this](UIView& view, const Event& event) -> bool {
        Terminate();
        return true;
    };
    buttonBackListener_ = new EventListener(onClick, nullptr);
}

void AppInfoAbilitySlice::SetHead()
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

    printf("[LOG] bundleName_-> %s +11->%s \n", bundleName_, bundleName_ + 11); // 11
    UILabel* lablelFont = new UILabel();
    lablelFont->SetPosition(DE_HEAD_TEXT_X, DE_HEAD_TEXT_Y, DE_HEAD_TEXT_WIDTH, DE_HEAD_TEXT_HEIGHT);
    lablelFont->SetText(bundleName_ + 11); // use 11
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void AppInfoAbilitySlice::SetAppPermissionInfo(int index, PermissionSaved& permissions)
{
    UIViewGroup* itemView = new UIViewGroup();
    int useX = 0;
    int useY = index * DE_ITEM_INTERVAL;
    itemView->SetPosition(useX, useY, DE_BUTTON_WIDTH, DE_BUTTON_HEIGHT);
    itemView->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    itemView->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    itemView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    scrollView_->Add(itemView);

    UILabel* nameLabel = new UILabel();
    nameLabel->SetPosition(DE_TITLE_TEXT_X, DE_TITLE_TEXT_Y, DE_TITLE_TEXT_WIDTH, DE_TITLE_TEXT_HEIGHT);
    nameLabel->SetText(permissions.name + 16); // 16 is get offset name
    nameLabel->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    nameLabel->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_CENTER);
    nameLabel->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    itemView->Add(nameLabel);
    UIToggleButton* togglebutton = new UIToggleButton();
    togglebutton->SetPosition(DE_TOGGLE_BUTTON_X, DE_TOGGLE_BUTTON_Y);
    if (permissions.granted == 0) {
        togglebutton->SetState(false);
    } else {
        togglebutton->SetState(true);
    }
    ToggBtnOnListener* listener = new ToggBtnOnListener(togglebutton);
    listener->SetPermissionName(permissions.name, strlen(permissions.name));
    listener->SetBundleName(bundleName_, strlen(bundleName_));
    togglebutton->SetOnClickListener(listener);
    listListener_.PushBack(listener);
    itemView->Add(togglebutton);
}

void AppInfoAbilitySlice::PermissionInfoList()
{
    int permNum = 0;
    scrollView_ = new UIScrollView();
    scrollView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_SCROLL_COLOR);
    scrollView_->SetPosition(DE_SCROLL_X, DE_SCROLL_Y, DE_SCROLL_WIDTH, DE_SCROLL_HEIGHT);
    scrollView_->SetXScrollBarVisible(false);
    scrollView_->SetYScrollBarVisible(true);
    rootView_->Add(scrollView_);
    int ret = QueryPermission(bundleName_, &permissions_, &permNum);
    if (ret == 0) {
        printf("[LOG]PermissionInfoList bundleName_ -> %s ,permNum->%d\n", bundleName_, permNum);
        for (int i = 0; i < permNum; i++) {
            if (permissions_ != nullptr) {
                printf("[LOG]PermissionInfoList xxx -> name %s \n", permissions_->name);
                SetAppPermissionInfo(i, permissions_[i]);
            }
        }
    }
}

void AppInfoAbilitySlice::OnStart(const Want& want)
{
    int ret;
    printf("[LOG]receive the data -> %s\n", static_cast<char*>(want.data));
    AbilitySlice::OnStart(want);

    ret = memcpy_s(bundleName_, sizeof(bundleName_), want.data, want.dataLength);
    if (ret != EOK) {
        return;
    }
    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);
    SetButtonListener();
    SetHead();
    PermissionInfoList();
    SetUIContent(rootView_);
}

void AppInfoAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void AppInfoAbilitySlice::OnActive(const Want &want)
{
    AbilitySlice::OnActive(want);
}

void AppInfoAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void AppInfoAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
} // namespace OHOS
