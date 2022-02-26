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

#include "view_group_page.h"
#include "ui_config.h"

namespace OHOS {
ViewGroupPage::ViewGroupPage(UIViewGroup* viewGroup)
{
    viewGroup_ = viewGroup;
}

ViewGroupPage::~ViewGroupPage()
{
    ListNode<AppInfo*>* app = appInfo_.Begin();
    while (app != appInfo_.End()) {
        delete app->data_;
        app = app->next_;
    }
    appInfo_.Clear();
    if (row_col_) {
        delete[] row_col_;
    }
}

bool ViewGroupPage::IsFull(int16_t& row, int16_t& col)
{
    for (int16_t i = 0; i < row_; i++) {
        for (int16_t j = 0; j < col_; j++) {
            if (row_col_[i][j] == false) {
                row = i;
                col = j;
                return false;
            }
        }
    }
    return true;
}

void ViewGroupPage::SetStyle(Style sty)
{
    viewGroup_->SetStyle(sty);
    viewGroup_->Invalidate();
}

void ViewGroupPage::SetPosion(int16_t width, int16_t height, int16_t x, int16_t y)
{
    viewGroup_->SetPosition(x, y, width, height);
}

void ViewGroupPage::SetScale(double scale)
{
    scale_ = scale;
}

void ViewGroupPage::SetMatrix(int16_t rows, int16_t cols)
{
    row_col_ = new bool* [rows];
    for (int i = 0; i < rows; i++) {
        row_col_[i] = new bool[cols]();
    }
    row_ = rows;
    col_ = cols;
}

void ViewGroupPage::CalculateAppPosition(AppInfo* pAppInfo, int16_t row, int16_t col)
{
    int16_t w = viewGroup_->GetWidth();

    const double scale = scale_;
    const int16_t blank1 = 10;
    const int16_t blank2 = 30;
    const int16_t labelH = 2;
    int16_t width = static_cast<int16_t>(static_cast<double>(w) / static_cast<double>(scale * col_ + col_ + scale));
    int16_t heightB = width;
    int16_t heightL = heightB / labelH;
    int16_t xB = scale * width + (scale + 1) * width * col;
    int16_t yB = blank1 + (blank2 + heightL + heightB) * row;
    int16_t xL = xB;
    int16_t yL = yB + heightB + blank1;

    pAppInfo->buttonXY_.x = xB;
    pAppInfo->buttonXY_.y = yB;
    pAppInfo->buttonHV_.x = width;
    pAppInfo->buttonHV_.y = heightB;

    pAppInfo->lableXY_.x = xL;
    pAppInfo->lableXY_.y = yL;
    pAppInfo->lableHV_.x = width;
    pAppInfo->lableHV_.y = heightL;
}

void ViewGroupPage::SetUpApp(AppInfo *pAppInfo)
{
    UILabelButton *button = new UILabelButton();
    UILabel *lable = new UILabel();
    lable->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Red()));
    lable->SetStyle(STYLE_BACKGROUND_OPA, UN_OPACITY);
    pAppInfo->SetButton(button);
    pAppInfo->SetLable(lable);

    pAppInfo->SetListener(pAppInfo);
    viewGroup_->Add(button);
    viewGroup_->Add(lable);
    viewGroup_->Invalidate();
}

bool ViewGroupPage::AddApp(AppInfo* pAppInfo)
{
    int16_t row = 0;
    int16_t col = 0;

    if (FindApp(pAppInfo)) {
        return true;
    }

    if (IsFull(row, col)) {
        return false;
    }

    pAppInfo->SetLocation(row, col);
    CalculateAppPosition(pAppInfo, row, col);

    SetUpApp(pAppInfo);
    appInfo_.PushBack(pAppInfo);
    row_col_[row][col] = true;
    return true;
}

bool ViewGroupPage::FindApp(AppInfo* pApp)
{
    ListNode<AppInfo*>* app = appInfo_.Begin();
    while (app != appInfo_.End()) {
        if (memcmp(app->data_->appName_, pApp->appName_, strlen(pApp->appName_)) == 0) {
            return true;
        }
        app = app->next_;
    }
    return false;
}

bool ViewGroupPage::RemoveApp(const char* pAppName)
{
    ListNode<AppInfo*>* app = appInfo_.Begin();
    while (app != appInfo_.End()) {
        if (memcmp(app->data_->appName_, pAppName, strlen(pAppName)) == 0) {
            row_col_[app->data_->row_col_.x][app->data_->row_col_.y] = false;
            viewGroup_->Remove(app->data_->button_);
            viewGroup_->Remove(app->data_->lable_);
            viewGroup_->Invalidate();
            appInfo_.Remove(app);
            return true;
        }
        app = app->next_;
    }
    return false;
}
} // namespace OHOS
