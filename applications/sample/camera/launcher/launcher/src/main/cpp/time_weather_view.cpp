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

#include <ctime>
#include <securec.h>

#include "time_weather_view.h"

namespace OHOS {
static constexpr int16_t DISPLATE_PICESE = 2;
static constexpr int16_t BLANK_H = 5;
static constexpr int16_t BLANK_TW = 15;
static constexpr int16_t BLANK_W = 100;
static constexpr int16_t BIGLABEL_H = 100;
static constexpr int16_t SMALLLABEL_H = 35;
static constexpr int16_t IMAGE_H = 40;
static constexpr int16_t IMAGE_W = 40;
const char* g_weekDate[WEEK_DAY_MAX] = {"星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};

TimeWeatherView::TimeWeatherView(UIViewGroup* viewGroup)
{
    viewGroup_ = viewGroup;
}
TimeWeatherView::~TimeWeatherView()
{
    // todo other release
}

void TimeWeatherView::SetStyle(Style sty)
{
    viewGroup_->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    viewGroup_->Invalidate();
}

void TimeWeatherView::SetPosion(int16_t x, int16_t y, int16_t height, int16_t width)
{
    viewGroup_->SetPosition(x, y, width, height);
}

void TimeWeatherView::SetUpView()
{
    SetUpTimeView();
    SetUpWeatherView();
    viewGroup_->Invalidate();
}

void TimeWeatherView::SetUpTimeView()
{
    char hour_min[TMP_BUF_SIZE] = { 0 };
    char mont_day[TMP_BUF_SIZE] = { 0 };
    char week_day[TMP_BUF_SIZE] = { 0 };
    char date[TMP_BUF_SIZE] = { 0 };
    const int16_t january = 1;
    const int16_t commonYear = 1970;
    time_t t = time(nullptr);
    struct tm* st = localtime(&t);
    if (st == nullptr) {
        return;
    }
    int ret = sprintf_s(hour_min, sizeof(hour_min), "%02d : %02d", st->tm_hour, st->tm_min);
    if (ret == LAUNCHER_PARAMERROR) {
        return;
    }
    ret = sprintf_s(mont_day, sizeof(mont_day), "%02d月%02d日", st->tm_mon + january, st->tm_mday);
    if (ret == LAUNCHER_PARAMERROR) {
        return;
    }
    GetWeekdayByYearday(st->tm_year + commonYear, st->tm_mon + january, st->tm_mday, week_day, sizeof(week_day));
    ret = sprintf_s(date, sizeof(date), "%s %s", mont_day, week_day);
    if (ret == LAUNCHER_PARAMERROR) {
        return;
    }
    if (viewTime_ == nullptr) {
        viewTime_ = new UIViewGroup();
        viewTime_->SetPosition(BLANK_TW, BLANK_H, viewGroup_->GetWidth() - BLANK_W,
            viewGroup_->GetHeight() / DISPLATE_PICESE - SMALLLABEL_H);
        viewTime_->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
        UILabel* lable = new UILabel();
        lable->SetPosition(BLANK_TW, BLANK_H, viewTime_->GetWidth(), BIGLABEL_H);
        lable->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_BOTTOM);
        lable->SetText(hour_min);
        lable->SetFont(FOND_PATH, BIGLAUNCHER_FOND_ID);
        lable->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
        lable->SetStyle(STYLE_BORDER_RADIUS, LABLE_RADIUS);
        lable->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
        lable->SetViewId("labletime");

        UILabel* lable2 = new UILabel();
        lable2->SetPosition(BLANK_TW, BLANK_H + BIGLABEL_H + BLANK_H, viewTime_->GetWidth(), SMALLLABEL_H);
        lable2->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_TOP);
        lable2->SetText(date);
        lable2->SetFont(FOND_PATH, LAUNCHER_FOND_ID);

        lable2->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
        lable2->SetStyle(STYLE_BORDER_RADIUS, LABLE_RADIUS);
        lable2->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
        lable2->SetViewId("labledate");

        viewTime_->Add(lable);
        viewTime_->Add(lable2);
        viewGroup_->Add(viewTime_);
    } else {
        UILabel* label = nullptr;
        label = static_cast<UILabel*>(viewTime_->GetChildById("labletime"));
        if (label) {
            label->SetText(hour_min);
        }
        label = static_cast<UILabel*>(viewTime_->GetChildById("labledate"));
        if (label) {
            label->SetText(date);
        }
        viewTime_->Invalidate();
    }
}

void TimeWeatherView::GetWeekdayByYearday(int iY, int iM, int iD, char* date, int size)
{
    if (date == nullptr) {
        return;
    }
    const int16_t months = 12;
    const int16_t january = 1;
    const int16_t february = 2;
    const int16_t oneHundred = 100;
    const int16_t fourHundred = 400;
    int iWeekDay = -1;
    if (january == iM || february == iM) {
        iM += months;
        iY--;
    }
    // 1 : MONDAY_LAUNCHER, 2 : TUESDAY_LAUNCHER, 3 : WEDNESDAY_LAUNCHER, 4 : , 5 : ect
    iWeekDay = (iD + 1 + 2 * iM + 3 * (iM + 1) / 5 + iY + iY / 4 - iY / oneHundred + iY / fourHundred) % WEEKEND_LAUNCHER;
    for (int i = 0; i < WEEK_DAY_MAX; i++) {
        if (iWeekDay == i) {
            if (memcpy_s(date, size, g_weekDate[i], strlen(g_weekDate[i])) == LAUNCHER_SUCCESS) {
                date[strlen(g_weekDate[i])] = 0;
                break;
            }
        }
    }
    return;
}

void TimeWeatherView::SetUpWeatherView()
{
    const int16_t countTimes = 6;
    viewweather_ = new UIViewGroup();
    viewweather_->SetPosition(BLANK_W, viewGroup_->GetHeight() / DISPLATE_PICESE - SMALLLABEL_H,
        viewGroup_->GetWidth() / DISPLATE_PICESE + BLANK_TW, DISPLATE_PICESE * (BLANK_H + SMALLLABEL_H) + BLANK_H);
    viewweather_->SetStyle(STYLE_BACKGROUND_OPA, HALF_OPACITY);
    viewweather_->SetStyle(STYLE_BORDER_RADIUS, GROUP_VIEW_RADIUS);
    viewweather_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Gray()));

    UIImageView* uiImageView = new UIImageView();
    uiImageView->SetPosition(BLANK_TW, BLANK_H * countTimes, IMAGE_W, IMAGE_H);
    uiImageView->SetSrc(RES_WEATHER);
    uiImageView->SetStyle(STYLE_BACKGROUND_OPA, UN_OPACITY);

    UILabel* lable = new UILabel();
    lable->SetPosition(BLANK_TW + IMAGE_W, BLANK_H,
        viewweather_->GetWidth() - IMAGE_W - BLANK_TW - BLANK_TW - BLANK_TW, SMALLLABEL_H);
    lable->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_CENTER);
    lable->SetText("室内温度 26℃");
    lable->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    lable->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    lable->SetStyle(STYLE_BORDER_RADIUS, LABLE_RADIUS);
    lable->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);

    UILabel* lable2 = new UILabel();
    lable2->SetPosition(BLANK_TW + IMAGE_W, SMALLLABEL_H + BLANK_H + BLANK_H,
        viewweather_->GetWidth() - IMAGE_W - BLANK_TW - BLANK_TW - BLANK_TW, SMALLLABEL_H);
    lable2->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_CENTER);
    lable2->SetText("空气污染指数 136");
    lable2->SetFont(FOND_PATH, LAUNCHER_FOND_ID);
    lable2->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    lable2->SetStyle(STYLE_BORDER_RADIUS, LABLE_RADIUS);
    lable2->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);

    viewweather_->Add(uiImageView);
    viewweather_->Add(lable);
    viewweather_->Add(lable2);
    viewGroup_->Add(viewweather_);
}
} // namespace OHOS
