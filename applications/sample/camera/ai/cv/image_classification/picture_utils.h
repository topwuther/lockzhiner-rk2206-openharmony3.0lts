/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef PICTURE_UTILS_H
#define PICTURE_UTILS_H

#include <csetjmp>
#include <cstdio>
#include <memory>
#include <string>

#include "jpeglib.h"

namespace IC {
struct MyErrorMgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

using MyErrorPtr = MyErrorMgr *;

// The index order of BGR format
enum BgrIndex {
    BGR_BLUE = 0,
    BGR_GREEN,
    BGR_RED,
};

// The index order of RGB format
enum RgbIndex {
    RGB_RED = 0,
    RGB_GREEN,
    RGB_BLUE,
};

// Information about picture
struct PicInfo {
    int widthSrc;
    int heightSrc;
    int widthDest;
    int heightDest;
};

// Change this to your own settings
const std::string JPEG_SRC_PATH = "/storage/data/image_classification_demo.jpg";
const int WIDTH_DEST = 224;
const int HEIGHT_DEST = 224;
const int NUM_CHANNELS = 3;

int WriteJpegFile(const std::string &filename, int quality, uint8_t *srcBuffer, int srcWidth, int srcHeight);
int WriteBgrFile(const std::string &filename, uint8_t *dataBuffer, int bufferSize);
uint8_t *ConvertToCaffeInput(uint8_t *dataBuffer, int maxSize);
uint8_t *ReadJpegFile(const std::string &filename, int &srcWidth, int &srcHeight);
uint8_t *Resize(const int widthDest, const int heightDest, uint8_t *src, int widthSrc, int heightSrc);
}  // namespace IC
#endif  // PICTURE_UTILS_H
