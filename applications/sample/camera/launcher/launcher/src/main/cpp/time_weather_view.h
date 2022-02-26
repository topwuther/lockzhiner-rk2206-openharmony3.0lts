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

#ifndef OHOS_TIME_WEATHER_H
#define OHOS_TIME_WEATHER_H

#include <components/ui_label.h>
#include <components/ui_label_button.h>
#include <components/ui_view_group.h>
#include <components/ui_image_view.h>

#include "app_info.h"
#include "native_base.h"
#include "ui_config.h"

namespace OHOS {
enum DATE_LAUNCHER {
    SUNDAY_LAUNCHER = 0,
    MONDAY_LAUNCHER,
    TUESDAY_LAUNCHER,
    WEDNESDAY_LAUNCHER,
    THURSDAY_LAUNCHER,
    FRIDAY_LAUNCHER,
    STAURDAY_LAUNCHER,
    WEEKEND_LAUNCHER
};

class TimeWeatherView : public NativeBase {
public:
    TimeWeatherView() = delete;
    explicit TimeWeatherView(UIViewGroup* viewGroup);
    virtual ~TimeWeatherView();
    void SetStyle(Style sty);
    void SetPosion(int16_t width, int16_t height, int16_t x, int16_t y);
    void SetUpView();
    void SetUpTimeView();

protected:
    void SetUpWeatherView();
    void GetWeekdayByYearday(int iY, int iM, int iD, char* date, int size);

private:
    UIViewGroup* viewTime_ { nullptr };
    UIViewGroup* viewweather_ { nullptr };
    UIViewGroup* viewGroup_ { nullptr };
};
} // namespace OHOS
#endif