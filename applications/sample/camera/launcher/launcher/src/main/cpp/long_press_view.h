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

#ifndef OHOS_LISTENTER_H
#define OHOS_LISTENTER_H

#include "app_info.h"
#include "view_group_page.h"
#include "app_manage.h"

namespace OHOS {
class LongPressView : public UIView::OnClickListener, public NativeBase {
public:
    explicit LongPressView(UninstallApp uninstall);
    virtual ~LongPressView();
    bool OnClick(UIView& view, const ClickEvent& event) override;
    void RemoveLview();
    void Show(UIViewGroup* viewParent, AppInfo* pApp);
    void SetStatus(bool status)
    {
        bStatus_ = status;
    }

    bool GetStatus() const
    {
        return bStatus_;
    }

private:
    UIViewGroup* viewParent_ { nullptr };
    UIViewGroup* viewGroup_ { nullptr };
    UILabelButton* buttUninstall_ { nullptr };
    UILabelButton* buttCancle_ { nullptr };
    UninstallApp uninstall_ { nullptr };
    AppInfo* app_ { nullptr };
    bool bStatus_ { false };
};
} // namespace OHOS
#endif