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

#ifndef __UI_CONFIG_H__
#define __UI_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_CH_55 0
#define LCD_CH_70 1
#define LCD_CH_SEL LCD_CH_55

#define LCD_MODE_HORIZONTAL 0
#define LCD_MODE_VERTICAL 1
#define LCD_MODE_SELECT LCD_MODE_HORIZONTAL

#if (LCD_CH_SEL==LCD_CH_55)
#if (LCD_MODE_SELECT==LCD_MODE_HORIZONTAL)
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 480
#else
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 960
#endif
#elif (LCD_CH_SEL==LCD_CH_70)
#if (LCD_MODE_SELECT==LCD_MODE_HORIZONTAL)
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600
#else
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 1024
#endif
#else
#error "no such type LCD SIZE select!!!"
// exit (1)
#endif

#define START_X 0
#define START_Y 0
#define BUTTON_ICON_SW 60
#define BUTTON_ICON_SH 60
#define BUTTON_ICON_BW 114
#define BUTTON_ICON_BH 114

#define TITLE_HEIGHT 80

#define ICON_B_WIDTH 50
#define ICON_B_HEIGHT 50
#define ICON_M_WIDTH 36
#define ICON_M_HEIGHT 36
#define ICON_S_WIDTH 12
#define ICON_S_HEIGHT 12

// coord define
#define V_GROUP_X START_X
#define V_GROUP_Y START_Y
#define V_GROUP_W SCREEN_WIDTH
#define V_GROUP_H SCREEN_HEIGHT

#if (LCD_MODE_SELECT==LCD_MODE_HORIZONTAL)
#define BACK_LABEL_X (START_X + 30)
#define BACK_LABEL_Y ((TITLE_HEIGHT - ICON_M_HEIGHT) / 2)
#define BACK_LABEL_W ICON_M_WIDTH
#define BACK_LABEL_H ICON_M_HEIGHT

#define TXT_LABEL_X (BACK_LABEL_X + BUTTON_ICON_SW)
#define TXT_LABEL_Y (START_Y + (TITLE_HEIGHT - ICON_B_HEIGHT) / 2)
#define TXT_LABEL_W (ICON_B_WIDTH * 3)
#define TXT_LABEL_H ICON_B_HEIGHT

#define RECORD_IMAGE_X (SCREEN_WIDTH / 2 - BUTTON_ICON_SW)
#define RECORD_IMAGE_Y (START_Y + (TITLE_HEIGHT - ICON_S_HEIGHT) / 2)
#define RECORD_IMAGE_W ICON_S_WIDTH
#define RECORD_IMAGE_H ICON_S_HEIGHT

#define TIME_LABEL_X (RECORD_IMAGE_X + RECORD_IMAGE_W + 14)
#define TIME_LABEL_Y (START_Y + (TITLE_HEIGHT - ICON_B_HEIGHT) / 2)
#define TIME_LABEL_W (SCREEN_WIDTH-(RECORD_IMAGE_X + RECORD_IMAGE_W + 14))
#define TIME_LABEL_H ICON_B_HEIGHT

#define SCROLL_VIEW_X START_X
#define SCROLL_VIEW_Y ((SCREEN_HEIGHT - BUTTON_ICON_BH) - 30)
#define SCROLL_VIEW_W SCREEN_WIDTH
#define SCROLL_VIEW_H (BUTTON_ICON_BH + 6)

#define LEFT_BUTTON_X (SCREEN_WIDTH / 3)
#define LEFT_BUTTON_Y ((SCROLL_VIEW_H - BUTTON_ICON_SH) / 2)
#define LEFT_BUTTON_W BUTTON_ICON_SW
#define LEFT_BUTTON_H BUTTON_ICON_SH
#define MID_BUTTON_X (((SCREEN_WIDTH / 3) - BUTTON_ICON_BW) / 2 + 1 * (SCREEN_WIDTH / 3))
#define MID_BUTTON_Y ((SCROLL_VIEW_H - BUTTON_ICON_BH) / 2)
#define MID_BUTTON_W BUTTON_ICON_BW
#define MID_BUTTON_H BUTTON_ICON_BH
#define RIGHT_BUTTON_X (2 * (SCREEN_WIDTH / 3) - BUTTON_ICON_SW)
#define RIGHT_BUTTON_Y ((SCROLL_VIEW_H - BUTTON_ICON_SH) / 2)
#define RIGHT_BUTTON_W BUTTON_ICON_SW
#define RIGHT_BUTTON_H BUTTON_ICON_SH

#else
#define BACK_LABEL_X (START_X + 30)
#define BACK_LABEL_Y ((BUTTON_ICON_SH - ICON_M_HEIGHT) / 2)
#define BACK_LABEL_W ICON_M_WIDTH
#define BACK_LABEL_H ICON_M_HEIGHT

#define TXT_LABEL_X (BACK_LABEL_X + BUTTON_ICON_SW)
#define TXT_LABEL_Y (START_Y + (BUTTON_ICON_SH - ICON_B_HEIGHT) / 2)
#define TXT_LABEL_W (ICON_B_WIDTH * 3)
#define TXT_LABEL_H ICON_B_HEIGHT

#define RECORD_IMAGE_X (SCREEN_WIDTH / 2 - BUTTON_ICON_SW)
#define RECORD_IMAGE_Y (START_Y + (BUTTON_ICON_SH - ICON_S_HEIGHT) / 2)
#define RECORD_IMAGE_W ICON_S_WIDTH
#define RECORD_IMAGE_H ICON_S_HEIGHT

#define TIME_LABEL_X (RECORD_IMAGE_X + RECORD_IMAGE_W + 14)
#define TIME_LABEL_Y (START_Y + (BUTTON_ICON_SH - ICON_B_HEIGHT) / 2)
#define TIME_LABEL_W (BUTTON_ICON_SW * 2)
#define TIME_LABEL_H ICON_B_HEIGHT

#define SCROLL_VIEW_X START_X
#define SCROLL_VIEW_Y ((SCREEN_HEIGHT - BUTTON_ICON_BH) - 10)
#define SCROLL_VIEW_W SCREEN_WIDTH
#define SCROLL_VIEW_H (BUTTON_ICON_BH + 6)

#define LEFT_BUTTON_X (SCREEN_WIDTH / 3)
#define LEFT_BUTTON_Y ((SCROLL_VIEW_H - BUTTON_ICON_SH) / 2)
#define LEFT_BUTTON_W BUTTON_ICON_SW
#define LEFT_BUTTON_H BUTTON_ICON_SH
#define MID_BUTTON_X (((SCREEN_WIDTH / 3) - BUTTON_ICON_BW) / 2 + 1 * (SCREEN_WIDTH / 3))
#define MID_BUTTON_Y ((SCROLL_VIEW_H - BUTTON_ICON_SH) / 2)
#define MID_BUTTON_W BUTTON_ICON_SW
#define MID_BUTTON_H BUTTON_ICON_SH
#define RIGHT_BUTTON_X (2 * (SCREEN_WIDTH / 3) - BUTTON_ICON_SW)
#define RIGHT_BUTTON_Y ((SCROLL_VIEW_H - BUTTON_ICON_SH) / 2)
#define RIGHT_BUTTON_W BUTTON_ICON_SW
#define RIGHT_BUTTON_H BUTTON_ICON_SH
#endif

#define UI_IMAGE_PATH \
    "/storage/app/run/com.huawei.camera/cameraApp/assets/cameraApp/resources/base/media/"
#define TTF_PATH "SourceHanSansSC-Regular.otf"

#define PHOTO_PATH "/userdata/photo/"
#define VIDEO_PATH "/userdata/video/"
#define THUMB_PATH "/userdata/thumb/"

#define MAX_NAME_LEN 256
#define PAGE_SIZE 1024

#define IMAGE_WIDTH 1920
#define IMAGE_HEIGHT 1080

#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 1
#define AUDIO_ENCODING_BITRATE SAMPLE_RATE
#define FRAME_RATE 30
#define BIT_RATE 4096

#define FILE_MODE 0777

#ifdef __cplusplus
}
#endif

#endif
