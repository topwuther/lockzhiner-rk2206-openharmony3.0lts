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

#include "picture_utils.h"

#include <fstream>
#include <iostream>

#include "ic_retcode.h"
#include "resize_computer.h"
#include "securec.h"

using namespace std;

namespace IC {
int WriteJpegFile(const string &filename, int quality,
    uint8_t *srcBuffer, int srcWidth, int srcHeight)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW rowPointer[1];
    int rowStride = 0;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    if (srcBuffer == nullptr) {
        printf("WriteJpegFile: srcBuffer is nullptr\n");
        return IC_RETCODE_FAILURE;
    }
    FILE *outfile;
    if ((outfile = fopen(filename.c_str(), "wb")) == nullptr) {
        printf("WriteJpegFile: can't open %s\n", filename.c_str());
        return IC_RETCODE_FAILURE;
    }
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = srcWidth;
    cinfo.image_height = srcHeight;
    cinfo.input_components = NUM_CHANNELS;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    rowStride = srcWidth * NUM_CHANNELS;

    while (cinfo.next_scanline < cinfo.image_height) {
        rowPointer[0] = &srcBuffer[cinfo.next_scanline * rowStride];
        (void)jpeg_write_scanlines(&cinfo, rowPointer, 1);
    }
    jpeg_finish_compress(&cinfo);

    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    return IC_RETCODE_SUCCESS;
}

int WriteBgrFile(const string &filename, uint8_t *dataBuffer, int bufferSize)
{
    if (dataBuffer == nullptr || bufferSize <= 0) {
        printf("WriteBgrFile: dataBuffer is nullptr.\n");
        return IC_RETCODE_FAILURE;
    }
    ofstream outfile(filename.c_str(), ofstream::out | ofstream::trunc);
    if (!outfile.is_open()) {
        printf("WriteBgrFile: Error writing file from BGR dataBuffer\n");
        return IC_RETCODE_FAILURE;
    }
    outfile.write((const char*)dataBuffer, bufferSize);
    outfile.close();
    return IC_RETCODE_SUCCESS;
}

uint8_t *ConvertToCaffeInput(uint8_t *dataBuffer, int maxSize)
{
    if (dataBuffer == nullptr) {
        return nullptr;
    }
    if (maxSize % NUM_CHANNELS != 0) {
        return nullptr;
    }
    uint8_t *input = new (std::nothrow) uint8_t[maxSize];
    if (input == nullptr) {
        return nullptr;
    }
    int numPreChannel = maxSize / NUM_CHANNELS;
    for (int i = 0; i < maxSize; i++) {
        input[BGR_RED * numPreChannel + i / NUM_CHANNELS] = dataBuffer[i + RGB_RED];
        input[BGR_BLUE * numPreChannel + i / NUM_CHANNELS] = dataBuffer[i + RGB_BLUE];
        input[BGR_GREEN * numPreChannel + i / NUM_CHANNELS] = dataBuffer[i + RGB_GREEN];
    }
    return input;
}

uint8_t *ReadJpegFile(const string &filename, int &srcWidth, int &srcHeight)
{
    struct jpeg_decompress_struct cinfo;
    struct MyErrorMgr jerr;
    FILE *infile;
    if ((infile = fopen(filename.c_str(), "rb")) == nullptr) {
        printf("ReadJpegFile: can't open %s\n", filename.c_str());
        return nullptr;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);
    srcHeight = cinfo.output_height;
    srcWidth = cinfo.output_width;
    int dataSize = srcHeight * srcWidth * cinfo.output_components;
    uint8_t *buffer = new (std::nothrow) uint8_t[dataSize];
    if (buffer == nullptr) {
        printf("ReadJpegFile: error to alloc buffer.\n");
        (void)jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return nullptr;
    }
    uint8_t *rowptr = nullptr;
    while (cinfo.output_scanline < srcHeight) {
        rowptr = buffer + cinfo.output_scanline * srcWidth * cinfo.output_components;
        (void)jpeg_read_scanlines(&cinfo, &rowptr, 1);
    }

    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return buffer;
}

uint8_t *Resize(
    const int widthDest, const int heightDest, uint8_t *src, int widthSrc, int heightSrc)
{
    if (src == nullptr) {
        printf("Resize: src is nullptr.\n");
        return nullptr;
    }
    if (widthDest <= 0 || heightDest <= 0 || widthSrc <= 0 || heightSrc <= 0) {
        printf("Resize: dimension below zero.\n");
        return nullptr;
    }
    int bufferSize = widthDest * heightDest * NUM_CHANNELS;
    uint8_t *pDest = new (std::nothrow) uint8_t[bufferSize];
    PicInfo picInfo = {
        .widthSrc = widthSrc,
        .heightSrc = heightSrc,
        .widthDest = widthDest,
        .heightDest = heightDest
    };
    ResizeComputer resizer(picInfo);
    if (pDest == nullptr) {
        printf("Resize: pDest alloc failed.\n");
        return nullptr;
    }
    if (widthDest == widthSrc && heightDest == heightSrc) {
        if (memcpy_s(pDest, bufferSize, src, bufferSize) != EOK) {
            printf("Resize: memcpy_s failed.\n");
            delete[] pDest;
            return nullptr;
        } else {
            return pDest;
        }
    }
    resizer.Compute(pDest, src, bufferSize, widthSrc * heightSrc * NUM_CHANNELS);
    return pDest;
}
}  // namespace IC