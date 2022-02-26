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

#ifndef HOS_CAMERA_TEMP_H
#define HOS_CAMERA_TEMP_H

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace OHOS::Camera {
class IBuffer {
public:
    IBuffer() {}
    ~IBuffer(){};

    int32_t GetIndex()
    {
        return index_;
    }
    uint32_t GetSize()
    {
        return size_;
    }
    void* GetVirAddress()
    {
        return virAddr_;
    }

    uint64_t GetUsage()
    {
        return usage_;
    }

    void SetIndex(const uint32_t index)
    {
        index_ = index;
        return;
    }

    void SetSize(const uint32_t size)
    {
        size_ = size;
        return;
    }

    void SetVirAddress(void* addr)
    {
        virAddr_ = addr;
        return;
    }

    void SetUsage(const uint64_t usage)
    {
        usage_ = usage;
        return;
    }

private:
    int32_t index_ = -1;
    uint32_t size_ = 0;
    void* virAddr_ = nullptr;
    uint64_t usage_ = 0;
};

struct FrameSpec {
    int64_t bufferPoolId_;
    std::shared_ptr<IBuffer> buffer_;
};

using FrameSpec = struct FrameSpec;

enum AdapterCmd : uint32_t {
    CMD_AE_EXPO,
    CMD_AWB_MODE,
    CMD_AE_EXPOTIME,
    CMD_AWB_COLORGAINS
};

#ifdef DISABLE_LOGD
#define CAMERA_LOGD(...)
#else
#define CAMERA_LOGD(fmt, ...)                    \
    do {                                         \
        printf("INFO:" fmt "\n", ##__VA_ARGS__); \
    } while (0);
#endif

#define CAMERA_LOGE(fmt, ...)                     \
    do {                                          \
        printf("ERROR:" fmt "\n", ##__VA_ARGS__); \
    } while (0);

enum RetCode {
    RC_OK = 0,
    RC_ERROR,
};
} // namespace OHOS::Camera
#endif // HOS_CAMERA_TEMP_H
