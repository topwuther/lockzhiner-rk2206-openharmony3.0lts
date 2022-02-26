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

#ifndef OHOS_MAIN_ABILITY_SLICE_H
#define OHOS_MAIN_ABILITY_SLICE_H

#include <ability_loader.h>
#include <functional>
#include <utility>
#include <securec.h>
#include <common/task.h>
#include <stdio.h>
#include <components/ui_label_button.h>
#include <components/ui_label.h>
#include <components/ui_checkbox.h>
#include <components/ui_image_view.h>
#include <components/ui_scroll_view.h>
#include <components/ui_surface_view.h>
#include <components/ui_slider.h>
#include <animator/animator.h>

#include "event_listener.h"
#include "camera_manager.h"

namespace OHOS {
class TaskView : public Task {
public:
    TaskView() = delete;
    TaskView(UILabel* tmLabel):timeLabel_(tmLabel)
    {
        runEnable_ = false;
        gTimeCount_ = 0;
        Task::Init();
    }

    virtual ~TaskView(){}
    void TaskStart(void)
    {
        Task::SetPeriod(1000);    /* 1000=1s */
        Task::TaskExecute();
    }

    void SetStart(void)
    {
        runEnable_ = true;
        gTimeCount_ = 0;
    }

    void SetPause(void)
    {
        runEnable_ = false;
    }

    void SetResume(void)
    {
        runEnable_ = true;
    }

    void SetStop(void)
    {
        gTimeCount_ = 0;
        runEnable_ = false;
        UpdateTimeLabel(gTimeCount_);
    }

    void Callback() override
    {
        if (runEnable_)
            UpdateTimeLabel(gTimeCount_++);
    }
private:
    UILabel* timeLabel_;
    bool runEnable_;
    uint32_t gTimeCount_;

    void UpdateTimeLabel(int ss)
    {
        char buff[20] = { 0 };

        if (timeLabel_ == nullptr) return;

        sprintf_s(buff, sizeof(buff), "%02d : %02d", ss / 60, ss % 60);     /* 60=1s */
        timeLabel_->SetText(buff);
    }
};

class CameraAbilitySlice : public AbilitySlice {
public:
    CameraAbilitySlice() = default;
    ~CameraAbilitySlice() override;

protected:
    void OnStart(const Want &want) override;
    void OnInactive() override;
    void OnActive(const Want &want) override;
    void OnBackground() override;
    void OnStop() override;

private:
    static constexpr int BUTTON_NUMS = 4;
    static constexpr int FONT_SIZE = 28;

    EventListener *buttonListener_ { nullptr };
    SampleCameraManager *cam_manager;
    UIImageView *background_;
    UISurfaceView* surfaceView;
    UIImageView *backBttn;
    UIImageView *backIcon;
    UILabel *txtMsgLabel;
    UIImageView *recordImage;
    UILabel *tmLabel;

    UIImageView* bttnLeft;
    UIImageView* bttnMidle;
    UIImageView* bttnRight;
    UIImageView* bttnRecord;
    UIScrollView* scroll;
    UISlider *slider;
    Animator *animator_;
    TaskView *gTaskView_;
    UIView::OnClickListener *bttnImageClick[BUTTON_NUMS];

    void SetHead();
    void SetBottom();
};
}

#endif // OHOS_MAIN_ABILITY_SLICE_H
