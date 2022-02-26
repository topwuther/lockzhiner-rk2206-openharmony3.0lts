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

#ifndef OHOS_SWIPE_VIEW_H
#define OHOS_SWIPE_VIEW_H

#include <cstdio>
#include <securec.h>
#include <components/ui_label_button.h>
#include <components/ui_label.h>
#include <components/ui_view_group.h>
#include <components/ui_swipe_view.h>
#include <components/root_view.h>
#include <common/task.h>

#include "ui_config.h"
#include "app_info.h"
#include "view_group_page.h"
#include "native_base.h"
#include "long_press_view.h"
#include "app_manage.h"
#include "time_weather_view.h"

namespace OHOS {
class ViewPageListener : public UIView::OnClickListener {
public:
    explicit ViewPageListener(LongPressView* view) : view_(view) {}
    virtual ~ViewPageListener() {}
    bool OnClick(UIView& view, const ClickEvent& event) override
    {
        view_->RemoveLview();
        return true;
    }

private:
    LongPressView *view_ { nullptr };
};

class SwipeLisstener : public UISwipeView::OnSwipeListener {
public:
    SwipeLisstener(LongPressView* view, UISwipeView* swipe, UILabel* lable)
        : view_(view), swipe_(swipe), lable_(lable) {};
    ~SwipeLisstener() {};
    virtual void OnSwipe(UISwipeView& view) override
    {
        char buf[TMP_BUF_SIZE] = { 0 };
        sprintf_s(buf, sizeof(buf), ".%d.", swipe_->GetCurrentPage() + 1);
        lable_->SetText(buf);
        view_->RemoveLview();
    }

private:
    LongPressView* view_ { nullptr };
    UISwipeView* swipe_ { nullptr };
    UILabel* lable_ { nullptr };
};

class SwipeView : public Task, public NativeBase {
public:
    SwipeView() = delete;
    SwipeView(UILabel* titlellable, UILabel* taillable);
    virtual ~SwipeView();
    void OnSetUpView();
    void StartApp(AppInfo* app);
    void ShowLongPressView(AppInfo* app);
    void UninstallApp(AppInfo* app);
    void InstallApp(AppInfo* app);
    void Callback() override
    {
        char tmp[TMP_BUF_SIZE] = { 0 };
        time_t t = time(nullptr);
        struct tm* st = localtime(&t);
        if (st != nullptr) {
            int ret = sprintf_s(tmp, sizeof(tmp), "%02d : %02d", st->tm_hour, st->tm_min);
            if (ret != LAUNCHER_PARAMERROR) {
                lableTitle_->SetText(tmp);
                timeWeatherView_->SetUpTimeView();
            }
        }
    }

    UISwipeView* GetSwipeView() const
    {
        return swipe_;
    }

private:
    void OnStop();
    void SetUpSwipe();
    UIViewGroup* AddViewGroup();
    UIViewGroup* AddFirstViewGroup();
    ViewGroupPage* arrPage_[MAX_VIEWGROUP] { nullptr };
    UISwipeView* swipe_ { nullptr };
    UILabel* lableTitle_ { nullptr }; // view title time label
    UILabel* lableTail_ { nullptr };
    int groupCount_ { 0 };
    ViewPageListener* arrViewListener_ { nullptr };
    SwipeLisstener* swipeLisstener_ { nullptr };
    AppManage* appManage_ { nullptr };
    LongPressView* lpView_ { nullptr };
    TimeWeatherView* timeWeatherView_ {nullptr};
};

class AppEvent {
public:
    static AppEvent* GetInstance(SwipeView* nativeView)
    {
        if (appEvent_ == nullptr) {
            appEvent_ = new AppEvent();
            nativeView_ = nativeView;
        }
        return appEvent_;
    }
    // app click

    static bool ClickEvent(AppInfo* app)
    {
        nativeView_->StartApp(app);
        return true;
    }
    // app long press show window

    static bool LongPressEvent(AppInfo* app)
    {
        nativeView_->ShowLongPressView(app);
        return true;
    }
    // app uninstall click

    static bool UninstallApp(AppInfo* app)
    {
        nativeView_->UninstallApp(app);
        return true;
    }
    // none install app used appmanage::InstallApp, this function is invailate

    static bool InstallApp(AppInfo* app)
    {
        nativeView_->InstallApp(app);
        return true;
    }

private:
    AppEvent() {}
    ~AppEvent()
    {
        if (appEvent_) {
            delete appEvent_;
            appEvent_ = nullptr;
        }
    }

private:
    static AppEvent* appEvent_;
    static SwipeView* nativeView_;
};
} // namespace OHOS
#endif