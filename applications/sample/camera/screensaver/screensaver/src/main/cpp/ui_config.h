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

namespace OHOS {
static constexpr uint16_t IMAGE_ANIMATOR_TIME_S = 2 * 1000; // Cyclic image playback interval
static constexpr uint8_t IMAGE_TOTEL_NUM = 5;      // Number of images

static const char* const IMG_DEFAULT_001_PATH =
    "/storage/app/run/com.huawei.screensaver/screensaver/assets/screensaver/resources/base/media/img_default_1.png";
static const char* const IMG_DEFAULT_002_PATH =
    "/storage/app/run/com.huawei.screensaver/screensaver/assets/screensaver/resources/base/media/img_default_2.png";
static const char* const IMG_DEFAULT_003_PATH =
    "/storage/app/run/com.huawei.screensaver/screensaver/assets/screensaver/resources/base/media/img_default_3.png";
static const char* const IMG_DEFAULT_004_PATH =
    "/storage/app/run/com.huawei.screensaver/screensaver/assets/screensaver/resources/base/media/img_default_4.png";
static const char* const IMG_DEFAULT_005_PATH =
    "/storage/app/run/com.huawei.screensaver/screensaver/assets/screensaver/resources/base/media/img_default_5.png";
} // namespace OHOS
#endif
