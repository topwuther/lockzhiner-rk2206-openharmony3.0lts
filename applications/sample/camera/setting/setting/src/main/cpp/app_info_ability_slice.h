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

#ifndef OHOS_APP_INFO_ABILITY_SLICE_H
#define OHOS_APP_INFO_ABILITY_SLICE_H

#include "ability_info.h"
#include "ability_loader.h"
#include "ability_manager.h"
#include "ability_slice.h"
#include "bundle_manager.h"
#include "components/ui_image_view.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_list.h"
#include "components/ui_scroll_view.h"
#include "components/ui_toggle_button.h"
#include "element_name.h"
#include "event_listener.h"
#include "gfx_utils/list.h"
#include "module_info.h"
#include "setting_utils.h"
#include "want.h"
#include "pms_interface.h"

#include <cstdio>
#include <securec.h>

namespace OHOS {

class ToggBtnOnListener : public UIView::OnClickListener {
public:

    explicit ToggBtnOnListener(UIToggleButton* togglebutton)
    {
        status_ = false;
        togglebutton_ = togglebutton;
    }

    virtual ~ToggBtnOnListener(){}

    void SetPermissionName(const char* permissionsName, int nameLenght)
    {
        int ret;
        ret = memcpy_s(name_, sizeof(name_), permissionsName, nameLenght);
        if (ret != EOK) {
            printf("[ERR] memcpy_s func[SetToggleButton]\n");
            return;
        }
        name_[nameLenght] = 0;
    }

    void SetBundleName(const char* bundleName, int nameLength)
    {
        int ret;
        ret = memcpy_s(bundleName_, sizeof(bundleName_), bundleName, nameLength);
        if (ret != EOK) {
            printf("[ERR] memcpy_s func[SetBundleName]\n");
            return;
        }
        bundleName_[nameLength] = 0;
    }

    bool OnClick(UIView& view, const ClickEvent& event) override
    {
        int ret;
        if (status_) {
            ret = RevokePermission(bundleName_, name_);
            if (ret == 0) {
                status_ = false;
                togglebutton_->SetState(false);
            }
        } else {
            ret = GrantPermission(bundleName_, name_);
            if (ret == 0) {
                status_ = true;
                togglebutton_->SetState(true);
            }
        }
        return true;
    }
private:
    char name_[128] = {0};
    char bundleName_[128] = {0};
    bool status_ = false;
    UIToggleButton* togglebutton_ = nullptr;
};

class AppInfoAbilitySlice : public AbilitySlice {
public:
    AppInfoAbilitySlice()
        : headView_(nullptr), scrollView_(nullptr), rootView_(nullptr), permissions_(nullptr),
          buttonBackListener_(nullptr) {}
    virtual ~AppInfoAbilitySlice();
protected:
    void OnStart(const Want& want) override;
    void OnInactive() override;
    void OnActive(const Want& want) override;
    void OnBackground() override;
    void OnStop() override;
private:
    void SetAppButtonListener(const char* appName);
    void SetButtonListener(void);
    void SetHead(void);
    UIViewGroup* headView_;
    UIScrollView* scrollView_;
    RootView* rootView_;
    PermissionSaved* permissions_;
    EventListener* buttonBackListener_;
    List<ToggBtnOnListener*> listListener_;

    void PermissionInfoList();
    void SetAppPermissionInfo(int index, PermissionSaved& permissions);
    char bundleName_[128];

};
} // namespace OHOS
#endif
