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

#include <common/screen.h>

#include "swipe_view.h"

namespace OHOS {
AppEvent* AppEvent::appEvent_ = { nullptr };
SwipeView* AppEvent::nativeView_ = { nullptr };
static constexpr int32_t PERIOD_TIME = 60 * 1000; // 60 seconds

SwipeView::SwipeView(UILabel* titlellable, UILabel* taillable)
{
    lableTitle_ = titlellable;
    lableTail_ = taillable;
    groupCount_ = 0;
    for (int i = 0; i < MAX_VIEWGROUP; i++) {
        arrPage_[i] = nullptr;
    }
    arrViewListener_ = nullptr;
    swipeLisstener_ = nullptr;
    appManage_ = new AppManage();
    lpView_ = new LongPressView(AppEvent::UninstallApp);
    Task::Init();
    Task::SetPeriod(PERIOD_TIME);
}

SwipeView::~SwipeView()
{
    OnStop();
}

void SwipeView::SetUpSwipe()
{
    swipe_ = new UISwipeView();
    swipe_->SetPosition(0, LABLE_TITLE_HEIGHT, Screen::GetInstance().GetWidth(),
        Screen::GetInstance().GetHeight() - LABLE_TITLE_HEIGHT - LABLE_TAIL_HEIGHT);
    swipe_->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    swipe_->SetLoopState(true);
    swipe_->SetAnimatorTime(20); // set swipe view animator time 20s
}

UIViewGroup* SwipeView::AddViewGroup()
{
    if (groupCount_ >= MAX_VIEWGROUP) {
        return nullptr;
    }
    UIViewGroup* viewGroup = new UIViewGroup();
    viewGroup->SetPosition(0, LABLE_TITLE_HEIGHT, Screen::GetInstance().GetWidth(),
        Screen::GetInstance().GetHeight() - LABLE_TITLE_HEIGHT - LABLE_TAIL_HEIGHT);
    viewGroup->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    groupCount_++;
    ViewGroupPage* page = new ViewGroupPage(viewGroup);
    arrPage_[groupCount_ - 1] = page;
    swipe_->Add(viewGroup);
    return viewGroup;
}

UIViewGroup* SwipeView::AddFirstViewGroup()
{
    UIViewGroup* firstView = new UIViewGroup();
    firstView->SetPosition(0, LABLE_TITLE_HEIGHT, Screen::GetInstance().GetWidth(),
        Screen::GetInstance().GetHeight() - LABLE_TITLE_HEIGHT - LABLE_TAIL_HEIGHT);
    firstView->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);

    UIViewGroup* viewTimeWeather = new UIViewGroup();
    // 2: set first view to 2 piece
    viewTimeWeather->SetPosition(0, 0, firstView->GetWidth() / 2, firstView->GetHeight());
    viewTimeWeather->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    timeWeatherView_ = new TimeWeatherView(viewTimeWeather);
    timeWeatherView_->SetUpView();
    firstView->Add(viewTimeWeather);

    UIViewGroup* viewGroup = new UIViewGroup();
    // 2 : get left && right view width and height
    viewGroup->SetPosition(firstView->GetWidth() / 2, 0, firstView->GetWidth() / 2, firstView->GetHeight());
    viewGroup->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    firstView->Add(viewGroup);
    groupCount_++;
    ViewGroupPage *page = new ViewGroupPage(viewGroup);
    arrPage_[groupCount_ - 1] = page;

    swipe_->Add(firstView);
    firstView->Invalidate();
    Task::TaskExecute();
    return firstView;
}

void SwipeView::OnSetUpView()
{
    SetUpSwipe();
    swipeLisstener_ = new SwipeLisstener(lpView_, swipe_, lableTail_);
    swipe_->SetOnSwipeListener(swipeLisstener_);
    arrViewListener_ = new ViewPageListener(lpView_);
    swipe_->SetOnClickListener(arrViewListener_);

    AddFirstViewGroup();
    AddViewGroup();
    AddViewGroup();
    // Reserved. Touch and hold to add a page.
    arrPage_[0]->SetMatrix(APP_ROW_COUNT, APP_COL_COUNT);
    arrPage_[0]->SetScale(0.6);    // 0.6 blank/icon width
    for (int16_t i = 1; i < groupCount_; i++) {
        arrPage_[i]->SetMatrix(APP_ROW_COUNT, 2 * APP_COL_COUNT); // 2 scale of first view's col count
        arrPage_[i]->SetScale(0.69);    // 0.69 blank/icon width
    }
    AppEvent::GetInstance(this);
    AppManage::SetViewGroup(AppEvent::ClickEvent, AppEvent::LongPressEvent, arrPage_, groupCount_);
    BundleInfo* pBundleInfos = nullptr;
    int count = 0;
    if (appManage_->LauncherApp(&pBundleInfos, count)) {
        for (int j = 0; j < count; j++) {
            for (int i = 0; i < groupCount_; i++) {
                if (memcmp(LAUNCHER_BUNDLE_NAME, pBundleInfos[j].bundleName, strlen(pBundleInfos[j].bundleName)) == 0) {
                    break;
                }
                if (memcmp(SCREENSAVER_BUNDLE_NAME, pBundleInfos[j].bundleName, strlen(pBundleInfos[j].bundleName)) == 0) {
                    break;
                }

                AppInfo* app = new AppInfo();
                app->funcclick_ = AppEvent::ClickEvent;
                app->funclPress_ = AppEvent::LongPressEvent;
                if (pBundleInfos[j].bundleName) {
                    memcpy_s(app->appName_, sizeof(app->appName_), pBundleInfos[j].bundleName,
                        strlen(pBundleInfos[j].bundleName));
                    app->appName_[strlen(pBundleInfos[j].bundleName)] = 0;
                }
                if (pBundleInfos[j].abilityInfos[0].name) {
                    memcpy_s(app->abilityName_, sizeof(app->abilityName_), pBundleInfos[j].abilityInfos[0].name,
                        strlen(pBundleInfos[j].abilityInfos[0].name));
                    app->abilityName_[strlen(pBundleInfos[j].abilityInfos[0].name)] = 0;
                }
                if (pBundleInfos[j].bigIconPath) {
                    memcpy_s(app->appIconDir_, sizeof(app->appIconDir_), pBundleInfos[j].bigIconPath,
                        strlen(pBundleInfos[j].bigIconPath));
                    app->appIconDir_[strlen(pBundleInfos[j].bigIconPath)] = 0;
                }
                if (arrPage_[i]->AddApp(app)) {
                    break;
                }
            }
        }
    }
    swipe_->SetCurrentPage(0);
}

void SwipeView::StartApp(AppInfo* app)
{
    if (lpView_->GetStatus() == true) {
        lpView_->RemoveLview();
        return;
    }
    appManage_->StartApp(app);
}

void SwipeView::ShowLongPressView(AppInfo* app)
{
    lpView_->Show(static_cast<UIViewGroup*>(app->button_->GetParent()), app);
}

void SwipeView::UninstallApp(AppInfo* app)
{
    if (appManage_->UnInstallApp(app)) {
        for (int16_t i = 0; i < groupCount_; i++) {
            if (arrPage_[i]) {
                if (arrPage_[i]->RemoveApp(app->appName_)) {
                    swipe_->Invalidate();
                    return;
                }
            }
        }
    }
}

void SwipeView::InstallApp(AppInfo* app)
{
    appManage_->InstallApp(app);
    AppInfo* pApp = new AppInfo();
    if (pApp == nullptr) {
        return;
    }
    app->funcclick_ = AppEvent::ClickEvent;
    app->funclPress_ = AppEvent::LongPressEvent;
    int16_t i;
    for (i = 0; i < groupCount_; i++) {
        if (arrPage_[i]->AddApp(pApp)) {
            break;
        }
    }
    if (i == groupCount_) {
        delete pApp;
        pApp = nullptr;
    }
}

void SwipeView::OnStop()
{
    if (lpView_) {
        delete lpView_;
        lpView_ = nullptr;
    }
    if (appManage_) {
        delete appManage_;
        appManage_ = nullptr;
    }
    if (arrViewListener_) {
        delete arrViewListener_;
        arrViewListener_ = nullptr;
    }
    if (swipeLisstener_) {
        delete swipeLisstener_;
        swipeLisstener_ = nullptr;
    }
    if (timeWeatherView_) {
        delete timeWeatherView_;
        timeWeatherView_ = nullptr;
    }
    for (int i = 0; i < MAX_VIEWGROUP; i++) {
        if (arrPage_[i]) {
            delete arrPage_[i];
            arrPage_[i] = nullptr;
        }
    }
    DeleteChildren(swipe_);
}
} // namespace OHOS
