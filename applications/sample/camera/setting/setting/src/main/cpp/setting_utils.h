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

#ifndef OHOS_SETTING_UTILS_H
#define OHOS_SETTING_UTILS_H

#include "components/ui_view_group.h"

namespace OHOS {
#define DE_IMAGE_BACK "/storage/app/run/com.huawei.setting/setting/assets/setting/resources/base/media/back.png"
#define DE_IMAGE_FORWORD \
    "/storage/app/run/com.huawei.setting/setting/assets/setting/resources/base/media/forward.png"
#define DE_IMAGE_ENTER "/storage/app/run/com.huawei.setting/setting/assets/setting/resources/base/media/enter.png"
#define DE_IMAGE_APP "/storage/app/run/com.huawei.setting/setting/assets/setting/resources/base/media/app.png"
#define DE_FONT_OTF "SourceHanSansSC-Regular.otf"

#define DE_ROOT_X 0
#define DE_ROOT_Y 0
#define DE_ROOT_WIDTH 960
#define DE_ROOT_HEIGHT 480
#define DE_ROOT_BACKGROUND_COLOR Color::ColorTo32(Color::Black())

#define DE_SCROLL_X 36
#define DE_SCROLL_Y 72
#define DE_SCROLL_WIDTH 888
#define DE_SCROLL_HEIGHT 408
#define DE_SCROLL_COLOR Color::ColorTo32(Color::Black())

#define DE_HEAD_X  0
#define DE_HEAD_Y  0
#define DE_HEAD_WIDTH 300
#define DE_HEAD_HEIGHT 72

#define DE_HEAD_IMAGE_X 39
#define DE_HEAD_IMAGE_Y 16
#define DE_HEAD_IMAGE_WIDTH 30
#define DE_HEAD_IMAGE_HEIGHT 30

#define DE_HEAD_TEXT_X 100
#define DE_HEAD_TEXT_Y 15
#define DE_HEAD_TEXT_WIDTH 180
#define DE_HEAD_TEXT_HEIGHT 60
#define DE_HEAD_TEXT_SIZE 32
#define DE_HEAD_TEXT_COLOR Color::ColorTo32(Color::White())

#define DE_BUTTON_WIDTH 888
#define DE_BUTTON_HEIGHT 89
#define DE_BUTTON_BACKGROUND_COLOR Color::ColorTo32(Color::GetColorFromRGB(0x33, 0x33, 0x33))
#define DE_BUTTON_RADIUS 16

#define DE_TITLE_TEXT_X 18
#define DE_TITLE_TEXT_Y 28
#define DE_TITLE_TEXT_WIDTH 400
#define DE_TITLE_TEXT_HEIGHT 50
#define DE_TITLE_TEXT_SIZE 26
#define DE_TITLE_TEXT_COLOR Color::ColorTo32(Color::White())

#define DE_SUBTITLE_TEXT_WIDTH 400
#define DE_SUBTITLE_TEXT_HEIGHT 40
#define DE_SUBTITLE_TEXT_COLOR Color::ColorTo32(Color::GetColorFromRGB(0x9F, 0x9F, 0x9F))
#define DE_SUBTITLE_TEXT_SIZE 24

#define DE_FORWARD_IMG_X 855
#define DE_FORWARD_IMG_Y 24
#define DE_FORWARD_IMG_WIDTH 12
#define DE_FORWARD_IMG_HEIGHT 12

#define DE_TOGGLE_BUTTON_X 816
#define DE_TOGGLE_BUTTON_Y 14

#define DE_ITEM_INTERVAL 95
#define DE_CONTENT_FONT_SIZE 28
#define DE_OPACITY_ALL 255

void DeleteChildren(UIView *view);
}
#endif
