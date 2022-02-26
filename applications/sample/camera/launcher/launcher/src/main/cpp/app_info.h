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

#ifndef OHOS_APP_INFO_H
#define OHOS_APP_INFO_H

#include <components/ui_label_button.h>
#include <components/ui_label.h>
#include <components/ui_view_group.h>

#include "native_base.h"
#include "ui_config.h"

namespace OHOS {

class AppInfo;
using funcLongPress = bool (*)(AppInfo *app);
using funcClick  = bool (*)(AppInfo *app);
using UninstallApp = bool (*)(AppInfo *app);
using AddApp = bool (*)(AppInfo *app);

struct MyPoint {
    int16_t x; // the x coordinate of the point
    int16_t y; // the y coordinate of the point
};

class AppInfo {
public:
    AppInfo();
    virtual ~AppInfo();
    void Release();
    void ReSet();
    void SetButton(UILabelButton *button);
    void SetLable(UILabel *lable);
    void SetListener(AppInfo *app);
    void SetLocation(int16_t r, int16_t c);

    UILabelButton* button_ { nullptr };
    UILabel* lable_ { nullptr };

    UIView::OnLongPressListener* appLpListener_ { nullptr };
    UIView::OnClickListener* appClickListener_ { nullptr };

    funcClick funcclick_ { nullptr };
    funcLongPress funclPress_ { nullptr };
    MyPoint lableXY_ { 0 };
    MyPoint lableHV_ { 0 };
    MyPoint buttonXY_ { 0 };
    MyPoint buttonHV_ { 0 };
    MyPoint row_col_ { 0 };
    char appName_[TMP_BUF_SIZE] = { 0 };
    char abilityName_[TMP_BUF_SIZE] = { 0 };
    char appIconDir_[TMP_BUF_SIZE] = { 0 };
};

class AppClickListener : public UIView::OnClickListener {
public:
    AppClickListener(funcClick func, AppInfo* app) : funcClick_(func), appInfo_(app) {}
    virtual ~AppClickListener() {}
    bool OnClick(UIView& view, const ClickEvent& event) override
    {
        funcClick_(appInfo_);
        return true;
    }

private:
    funcClick funcClick_ { nullptr };
    AppInfo *appInfo_ { nullptr };
};

class AppLongPressListener : public UIView::OnLongPressListener {
public:
    AppLongPressListener(funcClick func, AppInfo* app) : appInfo_(app), funcLongPress_(func) {}
    virtual ~AppLongPressListener() {}
    bool OnLongPress(UIView& view, const LongPressEvent& event) override
    {
        funcLongPress_(appInfo_);
        return true;
    }

private:
    AppInfo* appInfo_ { nullptr };
    funcLongPress funcLongPress_ { nullptr };
};
} // namespace OHOS
#endif
