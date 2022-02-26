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

#ifndef HOS_CAMERA_STREAM_CUSTOMER_H
#define HOS_CAMERA_STREAM_CUSTOMER_H

#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <hdf_log.h>
#include <surface.h>
#include <display_type.h>
#include "camera_metadata_info.h"
#include "ibuffer.h"
#include "if_system_ability_manager.h"
#include "ioffline_stream_operator.h"
#include "iservice_registry.h"
#include "camera.h"

namespace OHOS::Camera {
class StreamCustomer {
public:
    StreamCustomer();
    ~StreamCustomer();

    sptr<OHOS::IBufferProducer> CreateProducer();
    void CamFrame(const std::function<void(void*, const uint32_t)> callback);

    RetCode ReceiveFrameOn(const std::function<void(void*, const uint32_t)> callback);
    void ReceiveFrameOff();

    class TestBuffersConsumerListener: public IBufferConsumerListener {
    public:
        TestBuffersConsumerListener()
        {
        }

        ~TestBuffersConsumerListener()
        {
        }

        void OnBufferAvailable()
        {
        }
    };

private:
    unsigned int camFrameExit_ = 1;

    sptr<OHOS::Surface> consumer_ = nullptr;
    std::thread* previewThreadId_ = nullptr;
};
} // namespace OHOS::Camera
#endif // HOS_CAMERA_STREAM_CUSTOMER_H
