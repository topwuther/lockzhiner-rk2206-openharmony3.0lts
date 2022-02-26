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

#ifndef OHOS_GALLERY_CONFIG_H
#define OHOS_GALLERY_CONFIG_H

namespace OHOS {
/** icon resource file path */
static const char* const BACK_ICON_PATH = "/gallery/assets/gallery/resources/drawable/ic_back.png";
static const char* const VIDEO_TAG_ICON_PATH = "/gallery/assets/gallery/resources/drawable/ic_gallery_video_tag.png";
static const char* const VIDEO_PALY_PATH = "/gallery/assets/gallery/resources/drawable/ic_gallery_play.png";
static const char* const VIDEO_PAUSE_PATH = "/gallery/assets/gallery/resources/drawable/ic_gallery_pause.png";

static constexpr int16_t MAX_PICTURE_COUNT = 256;
static constexpr uint16_t MAX_PATH_LENGTH = 512;

/** thumb, photo, and video folder path */
static const char* const THUMBNAIL_DIRECTORY = "/userdata/thumb";
static const char* const PHOTO_DIRECTORY = "/userdata/photo";
static const char* const VIDEO_SOURCE_DIRECTORY = "/userdata/video";

/** general page configuration */
static constexpr int ROOT_VIEW_POSITION_X = 0;
static constexpr int ROOT_VIEW_POSITION_Y = 0;
static constexpr int ROOT_VIEW_WIDTH = 960;
static constexpr int ROOT_VIEW_HEIGHT = 480;
static constexpr uint16_t ROOT_VIEW_OPACITY = 255;

static const char* const FONT_NAME = "SourceHanSansSC-Regular.otf";

/** back icon 36 x 36 */
static constexpr int16_t BACK_ICON_POSITION_X = 38;
static constexpr int16_t BACK_ICON_POSITION_Y = 17;

/** THUMBNAIL */
static constexpr int16_t THUMBNAIL_RESOLUTION_X = 120;
static constexpr int16_t THUMBNAIL_RESOLUTION_Y = 120;
static constexpr int16_t THUMBNAIL_SPACE = 4;
static constexpr int16_t THUMBNAIL_COLUMN = 3;

static constexpr int16_t VIDEO_TAG_POSITION_X = 10;
static constexpr int16_t VIDEO_TAG_POSITION_Y = THUMBNAIL_RESOLUTION_Y - 37;

/** title */
static constexpr int16_t LABEL_POSITION_X = BACK_ICON_POSITION_X + 60;
static constexpr int16_t LABEL_POSITION_Y = 0;
static constexpr int16_t LABEL_WIDTH = 100;
static constexpr int16_t LABEL_HEIGHT = 70;
static constexpr uint16_t GALLERY_FONT_SIZE = 25;
static constexpr uint16_t GALLERY_DELETE_FONT_SIZE = 22;
static constexpr int16_t DELETE_LABEL_WIDTH = 150;

/** prefix and File Type */
static const char* const PHOTO_PREFIX = "photo";
static const char* const AVAILABEL_SOURCE_TYPE = ".mp4";
static const char* const AVAILABEL_SOURCE_TYPE_MP4 = ".MP4";

/** playback status bar */
static constexpr uint16_t STATUS_BAR_GROUP_HEIGHT = 96;
static constexpr uint16_t TOGGLE_BUTTON_OFFSET_X = 36;
static constexpr uint16_t TOGGLE_BUTTON_OFFSET_Y = 18;
static constexpr uint16_t TOGGLE_BUTTON_WIDTH = 60;
static constexpr uint16_t TOGGLE_BUTTON_HEIGHT = 60;

static constexpr uint16_t CURRENT_TIME_LABEL_X = TOGGLE_BUTTON_OFFSET_X + TOGGLE_BUTTON_WIDTH + TOGGLE_BUTTON_OFFSET_Y;
static constexpr uint16_t CURRENT_TIME_LABEL_Y = 0;
static constexpr uint16_t CURRENT_TIME_LABEL_WIDTH = 60;
static constexpr uint16_t CURRENT_TIME_LABEL_HEIGHT = STATUS_BAR_GROUP_HEIGHT;

static constexpr uint16_t TOTAL_TIME_LABEL_WIDTH = 90;
static constexpr uint16_t TOTAL_TIME_LABEL_HEIGHT = STATUS_BAR_GROUP_HEIGHT;
static constexpr uint16_t TOTAL_TIME_LABEL_X = ROOT_VIEW_WIDTH - TOTAL_TIME_LABEL_WIDTH;
static constexpr uint16_t TOTAL_TIME_LABEL_Y = 0;

static constexpr uint16_t SLIDER_X = CURRENT_TIME_LABEL_X + CURRENT_TIME_LABEL_WIDTH;
static constexpr uint16_t SLIDER_Y = 2;
static constexpr uint16_t SLIDER_HEIGHT = 3;
static constexpr uint16_t SLIDER_WIDTH = ROOT_VIEW_WIDTH - SLIDER_X - TOTAL_TIME_LABEL_WIDTH - 20;
static constexpr uint16_t KNOB_WIDTH = 25;

static constexpr uint16_t PLAYER_FONT_SIZE = 18;
} // namespace OHOS
#endif // OHOS_GALLERY_CONFIG_H