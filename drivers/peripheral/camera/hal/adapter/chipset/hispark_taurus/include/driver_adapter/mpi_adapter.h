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

#ifndef HOS_CAMERA_MPI_ADAPTER_H
#define HOS_CAMERA_MPI_ADAPTER_H

#include <vector>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <map>
#include "camera.h"
#include "ibuffer.h"
#include "stream.h"

namespace OHOS::Camera {
#define INIT_PARAM_KEY_MAX_LEN 128

using DeviceFormat = struct _DeviceFormat {
   uint32_t portId;
   int32_t width;
   int32_t height;
   int32_t framerate;
   uint64_t pixFormat;
   int32_t mirror;
   int32_t flip;
   int32_t rotation;
};

using PortConfig = struct _PortConfig {
    int32_t streamId_;
    int32_t bufferCount_;
    int32_t width_;
    int32_t height_;
    int32_t framerate_;
    uint32_t format_;
    int32_t srcPortId_;
    int32_t dstPortId_;
    uint64_t usage_;
    bool isExtPool_;
    CameraEncodeType encodeType_;
};

enum ThreadState : uint32_t {
    THREAD_STOP = 0,
    THREAD_RUNNING,
    THREAD_DESTROY,
    THREAD_NEED_CREATE,
};

enum PoolState : uint32_t {
    POOL_INACTIVE = 0,
    POOL_ACTIVE = 1,
};

using PoolSpec = struct _PoolSpec {
    uint32_t poolId;
    int32_t streamId_;
    uint32_t statue;
    uint32_t isCreated;
    int32_t  portNum;
    uint32_t curCount;
    uint32_t totalCount;
    uint64_t frameNumber;
};
constexpr int32_t MPI_SUCCESS = 0;

using BufCallback = std::function<void(std::shared_ptr<FrameSpec>)>;
using DeviceStatusCb = std::function<void()>;
}
#endif //HOS_CAMERA_MPI_ADAPTER_H
