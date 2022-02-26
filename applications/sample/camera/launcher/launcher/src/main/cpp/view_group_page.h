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

#ifndef OHOS_VIEWGROUP_PAGE_H
#define OHOS_VIEWGROUP_PAGE_H

#include <components/ui_label.h>
#include <components/ui_label_button.h>
#include <components/ui_view_group.h>
#include "gfx_utils/list.h"

#include "ui_config.h"
#include "app_info.h"
#include "native_base.h"

namespace OHOS {
class ViewGroupPage {
public:
    ViewGroupPage() = delete;
    explicit ViewGroupPage(UIViewGroup* viewGroup);
    virtual ~ViewGroupPage();
    void SetMatrix(int16_t rows, int16_t cols);
    bool AddApp(AppInfo* pAppInfo);
    bool RemoveApp(const char* appName);
    bool FindApp(AppInfo* pAppInfo);
    void SetScale(double scale);

protected:
    void SetPosion(int16_t width, int16_t height, int16_t x = 0, int16_t y = 0);
    void SetStyle(Style sty);
    bool IsFull(int16_t& row, int16_t& col);
    void SetUpApp(AppInfo* pAppInfo);
    void CalculateAppPosition(AppInfo* pAppInfo, int16_t row, int16_t col);

private:
    List<AppInfo*> appInfo_;
    UIViewGroup* viewGroup_ { nullptr };
    bool** row_col_ { nullptr };
    int16_t row_ { 0 };
    int16_t col_ { 0 };
    double scale_ { 0.0 };
};
} // namespace OHOS
#endif