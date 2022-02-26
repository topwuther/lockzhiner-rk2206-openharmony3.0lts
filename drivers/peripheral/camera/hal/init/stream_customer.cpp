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

#include "stream_customer.h"

namespace OHOS::Camera {
StreamCustomer::StreamCustomer() {}
StreamCustomer::~StreamCustomer() {}

void StreamCustomer::CamFrame(const std::function<void(void*, uint32_t)> callback)
{
    OHOS::Rect damage;
    int32_t flushFence = 0;
    int64_t timestamp = 0;
    constexpr uint32_t delayTime = 12000;

    CAMERA_LOGD("demo test:enter CamFrame thread ++ \n");

    do {
        OHOS::sptr<OHOS::SurfaceBuffer> buff = nullptr;
        consumer_->AcquireBuffer(buff, flushFence, timestamp, damage);
        if (buff != nullptr) {
            void* addr = buff->GetVirAddr();
            int32_t size = buff->GetSize();
            if (callback != nullptr) {
                int32_t gotSize = 0;
                int32_t frameNum = 0;
                int isKey = 0;
                int64_t timestamp;
                buff->ExtraGet("dataSize", gotSize);
                buff->ExtraGet("isKeyFrame", isKey);
                buff->ExtraGet("timeStamp", timestamp);
                buff->ExtraGet("frameNum", frameNum);
                CAMERA_LOGE("demo test:CamFrame callback +++++++ Size == %d frameNum = %d timestamp == %lld\n",
                    gotSize, frameNum, timestamp);
                callback(addr, gotSize);
            }
            consumer_->ReleaseBuffer(buff, -1);
        }
        usleep(delayTime);
    } while (camFrameExit_ == 0);

    CAMERA_LOGD("demo test:Exiting CamFrame thread -- \n");
}

sptr<OHOS::IBufferProducer> StreamCustomer::CreateProducer()
{
    consumer_ = OHOS::Surface::CreateSurfaceAsConsumer();
    if (consumer_ == nullptr) {
        return nullptr;
    }
    sptr<IBufferConsumerListener> listener = new TestBuffersConsumerListener();
    consumer_->RegisterConsumerListener(listener);

    auto producer = consumer_->GetProducer();
    if (producer == nullptr) {
        return nullptr;
    }

    CAMERA_LOGI("demo test, create a buffer queue producer %{public}p", producer.GetRefPtr());
    return producer;
}

RetCode StreamCustomer::ReceiveFrameOn(const std::function<void(void*, uint32_t)> callback)
{
    CAMERA_LOGD("demo test:ReceiveFrameOn enter");

    if (camFrameExit_ == 1) {
        camFrameExit_ = 0;
        previewThreadId_ = new (std::nothrow) std::thread(&StreamCustomer::CamFrame, this, callback);
        if (previewThreadId_ == nullptr) {
            CAMERA_LOGE("demo test:ReceiveFrameOn failed\n");
            return OHOS::Camera::RC_ERROR;
        }
    } else {
        CAMERA_LOGD("demo test:ReceiveFrameOn loop thread is running\n");
    }
    CAMERA_LOGD("demo test:ReceiveFrameOn exit");

    return RC_OK;
}

void StreamCustomer::ReceiveFrameOff()
{
    CAMERA_LOGD("demo test:ReceiveFrameOff enter\n");

    if (camFrameExit_ == 0) {
        camFrameExit_ = 1;
        if (previewThreadId_ != nullptr) {
            previewThreadId_->join();
            delete previewThreadId_;
            previewThreadId_ = nullptr;
        }
    }

    CAMERA_LOGD("demo test:ReceiveFrameOff exit\n");
}
}
