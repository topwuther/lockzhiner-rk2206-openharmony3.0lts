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

#ifndef OHOS_UI_CONFIG_H
#define OHOS_UI_CONFIG_H

#include <cstdint>
#include <securec.h>

namespace OHOS {
static constexpr int16_t LABLE_TITLE_HEIGHT = 30; // tail lable height
static constexpr int16_t LABLE_TAIL_HEIGHT = 30;
static constexpr int16_t APP_WIDTH_COUNT = 7;       // blank + app + blank + app + blank
static constexpr int16_t APP_HEIGHT_COUNT = 4;      // app + lable + app + lable
static constexpr int16_t MAX_VIEWGROUP = 3;         // swipe window count
static constexpr int16_t APP_ROW_COUNT = 2;         // a swipe view app count in row
static constexpr int16_t APP_COL_COUNT = 3;         // a sswipe view app count in col
static constexpr int16_t LAUNCHER_FOND_ID = 16;     // other view fond id
static constexpr int16_t APP_FOND_ID = 16;          // app name fond id
static constexpr int16_t BIGLAUNCHER_FOND_ID = 48;  // time big fond id
static constexpr int16_t TOTAL_OPACITY = 0;         // transparent
static constexpr int16_t HALF_OPACITY = 50;         // diaphanous
static constexpr int16_t UN_OPACITY = 255;          // opaque
static constexpr int16_t BUTTON_RADIUS = 20;        // app icon radius
static constexpr int16_t LABLE_RADIUS = 0;          // lable icon radius
static constexpr int16_t TITLE_LABLE_OPACITY = 255; // translucent
static constexpr int16_t GROUP_VIEW_RADIUS = 20;    // view radius

#ifndef TMP_BUF_SIZE
#define TMP_BUF_SIZE 128
#endif

#define LAUNCHER_BUNDLE_NAME "com.huawei.launcher"
#define SCREENSAVER_BUNDLE_NAME "com.huawei.screensaver"
#define TABLE_BACKGROUND \
    "/storage/app/run/com.huawei.launcher/launcher/assets/launcher/resources/base/media/background.png"
#define RES_WEATHER "/storage/app/run/com.huawei.launcher/launcher/assets/launcher/resources/base/media/weather.png"
#define FOND_PATH "SourceHanSansSC-Regular.otf"

#ifndef LAUNCHER_SUCCESS
#define LAUNCHER_SUCCESS 0
#endif

#ifndef LAUNCHER_PARAMERROR
#define LAUNCHER_PARAMERROR (-1)
#endif

#ifndef WEEK_DAY_MAX
#define WEEK_DAY_MAX 7
#endif
} // namespace OHOS
#endif
