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

#include <common.h>
#include "camera.h"

namespace OHOS::Camera {
uint64_t Test::GetCurrentLocalTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    return static_cast<uint64_t>(tmp.count());
}

int32_t Test::SaveYUV(const char* type, const void* buffer, int32_t size)
{
    if (strncmp(type, "preview", strlen(type)) == 0) {
        previewBufCnt += 1;
        if (previewBufCnt % 8 != 0) {
            std::cout << "receive preview buffer not save" << std::endl;
            return 0;
        }
    }
    char path[PATH_MAX] = {0};
    if (strncmp(type, "preview", strlen(type)) == 0) {
        system("mkdir -p /data/camera/preview");
        if (sprintf_s(path, sizeof(path) / sizeof(path[0]), "/data/camera/preview/%s_%lld.yuv",
            type, GetCurrentLocalTimeStamp()) < 0) {
            CAMERA_LOGE("%s: sprintf path failed", __func__);
            return 0;
        }
    } else {
        system("mkdir -p /data/camera/capture");
        if (sprintf_s(path, sizeof(path) / sizeof(path[0]), "/data/camera/capture/%s_%lld.jpg",
            type, GetCurrentLocalTimeStamp()) < 0) {
            CAMERA_LOGE("%s: sprintf path failed", __func__);
            return 0;
        }
    }
    std::cout << "save yuv to file:" << path << std::endl;

    int imgFd = open(path, O_RDWR | O_CREAT, 00766); // 00766:file jurisdiction
    if (imgFd == -1) {
        std::cout << "open file failed, errno = " << strerror(errno) << std::endl;
        return -1;
    }

    int ret = write(imgFd, buffer, size);
    if (ret == -1) {
        std::cout << "write file failed, error = " << strerror(errno) << std::endl;
        close(imgFd);
        return -1;
    }
    close(imgFd);
    return 0;
}

int32_t Test::SaveVideoFile(const char* type, const void* buffer, int32_t size, int32_t operationMode)
{
    if (operationMode == 0) {
        char path[PATH_MAX] = {0};
        system("mkdir -p /data/camera/video");
        if (sprintf_s(path, sizeof(path) / sizeof(path[0]), "/data/camera/video/%s_%lld.h265",
            type, GetCurrentLocalTimeStamp()) < 0) {
            CAMERA_LOGE("%s: sprintf path failed", __func__);
            return 0;
        }
        CAMERA_LOGI("%{public}s, save yuv to file %{public}s", __FUNCTION__, path);
        videoFd = open(path, O_RDWR | O_CREAT, 00766); // 00766:file jurisdiction
        if (videoFd == -1) {
            std::cout << "open file failed, errno = " << strerror(errno) << std::endl;
            return -1;
        }
    } else if (operationMode == 1 && videoFd != -1) {
        int32_t ret = write(videoFd, buffer, size);
        if (ret == -1) {
            std::cout << "write file failed, error = " << strerror(errno) << std::endl;
            close(videoFd);
            return -1;
        }
    } else {
        if (videoFd != -1) {
            close(videoFd);
        }
    }
    return 0;
}

void Test::Init()
{
    if (service == nullptr) {
        service = ICameraHost::Get("camera_service");
        if (service == nullptr) {
            std::cout << "==========[test log]ICameraHost get failed."<< std::endl;
        } else {
            std::cout << "==========[test log]ICameraHost get success."<< std::endl;
        }
        ASSERT_TRUE(service != nullptr);
    }
    hostCallback = new CameraHostCallback();
    service->SetCallback(hostCallback);
}

void Test::GetCameraAbility()
{
    if (cameraDevice == nullptr) {
        rc = service->GetCameraIds(cameraIds);
        if (rc != Camera::NO_ERROR) {
            std::cout << "==========[test log]GetCameraIds failed." << std::endl;
            return;
        } else {
            std::cout << "==========[test log]GetCameraIds success." << std::endl;
        }
        GetCameraMetadata();
    }
}

void Test::GetCameraMetadata()
{
    rc = service->GetCameraAbility(cameraIds.front(), ability);
    if (rc != Camera::NO_ERROR) {
        std::cout << "==========[test log]GetCameraAbility failed, rc = " << rc << std::endl;
    }
    common_metadata_header_t* data = ability->get();
    int32_t expo = 0;
    camera_metadata_item_t entry;
    int ret = find_camera_metadata_item(data, OHOS_CONTROL_AE_AVAILABLE_MODES, &entry);
    if (ret == 0) {
        std::cout << "==========[test log] get OHOS_CONTROL_AE_AVAILABLE_MODES success" << std::endl;
    }
}

void Test::Open()
{
    if (cameraDevice == nullptr) {
        service->GetCameraIds(cameraIds);
        GetCameraMetadata();
        deviceCallback = new CameraDeviceCallback();
        rc = service->OpenCamera(cameraIds.front(), deviceCallback, cameraDevice);
        if (rc != Camera::NO_ERROR || cameraDevice == nullptr) {
            std::cout << "==========[test log]OpenCamera failed, rc = " << rc << std::endl;
            return;
        }
        std::cout << "==========[test log]OpenCamera success." << std::endl;
    }
}

void Test::Close()
{
    if (cameraDevice != nullptr) {
        cameraDevice->Close();
        std::cout << "cameraDevice->Close" << std::endl;
        cameraDevice = nullptr;
    }
}

void Test::StartStream(std::vector<Camera::StreamIntent> intents)
{
    EXPECT_EQ(true, cameraDevice != nullptr);
    streamOperatorCallback = new StreamOperatorCallback();
    rc = cameraDevice->GetStreamOperator(streamOperatorCallback, streamOperator);
    EXPECT_EQ(true, rc == Camera::NO_ERROR);
    if (rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]GetStreamOperator success." << std::endl;
    } else {
        std::cout << "==========[test log]GetStreamOperator fail, rc = " << rc << std::endl;
    }
    streamInfo_pre = std::make_shared<Camera::StreamInfo>();
    streamInfo_video = std::make_shared<Camera::StreamInfo>();
    streamInfo_capture = std::make_shared<Camera::StreamInfo>();
    for (auto& intent : intents) {
        if (intent == 0) {
            streamInfo_pre->streamId_ = streamId_preview;
            streamInfo_pre->width_ = 640; // 640:width of stream
            streamInfo_pre->height_ = 480; // 480: height of stream
            streamInfo_pre->format_ = PIXEL_FMT_YCRCB_420_SP;
            streamInfo_pre->datasapce_ = 8; // 8:datasapce of stream
            streamInfo_pre->intent_ = intent;
            streamInfo_pre->tunneledMode_ = 5; // 5:tunneledMode of stream
            std::shared_ptr<StreamConsumer> consumer_pre = std::make_shared<StreamConsumer>();
            std::cout << "==========[test log]received a preview buffer ... 0" << std::endl;
            streamInfo_pre->bufferQueue_ = consumer_pre->CreateProducer([this](void* addr, uint32_t size) {
                SaveYUV("preview", addr, size);
            });
            streamInfo_pre->bufferQueue_->SetQueueSize(8); // 8:bufferqueue size
            consumerMap_[intent] = consumer_pre;
            streamInfos.push_back(streamInfo_pre);
        } else if (intent == 1) {
            streamInfo_video->streamId_ = streamId_video;
            streamInfo_video->width_ = 1280; // 1280:width of stream
            streamInfo_video->height_ = 960; // 960: height of stream
            streamInfo_video->format_ = PIXEL_FMT_YCRCB_420_SP;
            streamInfo_video->datasapce_ = 8; // 8:datasapce of stream
            streamInfo_video->intent_ = intent;
            streamInfo_video->encodeType_ = ENCODE_TYPE_H265;
            streamInfo_video->tunneledMode_ = 5; // 5:tunneledMode of stream
            std::shared_ptr<StreamConsumer> consumer_video = std::make_shared<StreamConsumer>();
            std::cout << "==========[test log]received a video buffer ... 1" << std::endl;
            SaveVideoFile("video", nullptr, 0, 0);
            streamInfo_video->bufferQueue_ = consumer_video->CreateProducer([this](void* addr, uint32_t size) {
                SaveVideoFile("video", addr, size, 1);
            });
            streamInfo_video->bufferQueue_->SetQueueSize(8); // 8:bufferqueue size
            consumerMap_[intent] = consumer_video;
            streamInfos.push_back(streamInfo_video);
        } else {
            streamInfo_capture->streamId_ = streamId_capture;
            streamInfo_capture->width_ = 1280; // 1280:width of stream
            streamInfo_capture->height_ = 960; // 960: height of stream
            streamInfo_capture->format_ = PIXEL_FMT_YCRCB_420_SP;
            streamInfo_capture->datasapce_ = 8; // 8:datasapce of stream
            streamInfo_capture->intent_ = intent;
            streamInfo_capture->encodeType_ = ENCODE_TYPE_JPEG;
            streamInfo_capture->tunneledMode_ = 5; // 5:tunneledMode of stream
            std::shared_ptr<StreamConsumer> consumer_capture = std::make_shared<StreamConsumer>();
            std::cout << "==========[test log]received a capture buffer ... 2" << std::endl;
            streamInfo_capture->bufferQueue_ = consumer_capture->CreateProducer([this](void* addr, uint32_t size) {
                SaveYUV("capture", addr, size);
            });
            streamInfo_capture->bufferQueue_->SetQueueSize(8); // 8:bufferqueue size
            consumerMap_[intent] = consumer_capture;
            streamInfos.push_back(streamInfo_capture);
        }
    }
    rc = streamOperator->CreateStreams(streamInfos);
    EXPECT_EQ(false, rc != Camera::NO_ERROR);
    if (rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]CreateStreams success." << std::endl;
    } else {
        std::cout << "==========[test log]CreateStreams fail, rc = " << rc << std::endl;
    }
    rc = streamOperator->CommitStreams(Camera::NORMAL, ability);
    EXPECT_EQ(false, rc != Camera::NO_ERROR);
    if (rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]CommitStreams success." << std::endl;
    } else {
        std::cout << "==========[test log]CommitStreams fail, rc = " << rc << std::endl;
    }
    sleep(2); // 2:The program waits two seconds
    std::vector<std::shared_ptr<Camera::StreamInfo>>().swap(streamInfos);
}

void Test::StartCapture(int streamId, int captureId, bool shutterCallback, bool isStreaming)
{
    captureInfo = std::make_shared<Camera::CaptureInfo>();
    captureInfo->streamIds_ = {streamId};
    captureInfo->captureSetting_ = ability;
    captureInfo->enableShutterCallback_ = shutterCallback;
    rc = streamOperator->Capture(captureId, captureInfo, isStreaming);
    EXPECT_EQ(true, rc == Camera::NO_ERROR);
    if (rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]check Capture: Capture success, " << captureId << std::endl;
    } else {
        std::cout << "==========[test log]check Capture: Capture fail, rc = " << rc << std::endl;
    }
    sleep(5); // 5:Wait 5 seconds for the program to run
}

void Test::StopStream(std::vector<int>& captureIds, std::vector<int>& streamIds)
{
    if (captureIds.size() > 0) {
        for (auto &captureId : captureIds) {
            rc = streamOperator->CancelCapture(captureId);
            EXPECT_EQ(true, rc == Camera::NO_ERROR);
            if (rc == Camera::NO_ERROR) {
                std::cout << "==========[test log]check Capture: CancelCapture success," << captureId << std::endl;
            } else {
                std::cout << "==========[test log]check Capture: CancelCapture fail, rc = " << rc;
                std::cout << "captureId = " << captureId << std::endl;
            }
        }
    }
    SaveVideoFile("video", nullptr, 0, 2); // 2:Operation Mode
    if (streamIds.size() > 0) {
        rc = streamOperator->ReleaseStreams(streamIds);
        EXPECT_EQ(true, rc == Camera::NO_ERROR);
        if (rc == Camera::NO_ERROR) {
            std::cout << "==========[test log]check Capture: ReleaseStreams success." << std::endl;
        } else {
            std::cout << "==========[test log]check Capture: ReleaseStreams fail, rc = " << rc << std::endl;
        }
    }
}

void Test::StopOfflineStream(int captureId)
{
    rc = offlineStreamOperator->CancelCapture(captureId);
    EXPECT_EQ(rc, Camera::NO_ERROR);
    if (rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]check offline: CancelCapture success," << captureId << std::endl;
    } else {
        std::cout << "==========[test log]check offline: CancelCapture fail, rc = " << rc;
        std::cout << "captureId = " << captureId << std::endl;
    }
    rc = offlineStreamOperator->Release();
    EXPECT_EQ(rc, Camera::NO_ERROR);
    if (rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]Check offline stream: offline Release success." << std::endl;
    } else {
        std::cout << "==========[test log]Check offline stream: offline Release fail, rc = " << rc << std::endl;
    }
}

OHOS::sptr<OHOS::IBufferProducer> Test::StreamConsumer::CreateProducer(std::function<void(void*, uint32_t)> callback)
{
    consumer_ = OHOS::Surface::CreateSurfaceAsConsumer();
    if (consumer_ == nullptr) {
        return nullptr;
    }
    sptr<IBufferConsumerListener> listener = new TestBufferConsumerListener();
    consumer_->RegisterConsumerListener(listener);
    auto producer = consumer_->GetProducer();
    std::cout << "create a buffer queue producer:" << producer.GetRefPtr() << std::endl;
    if (producer == nullptr) {
        return nullptr;
    }
    callback_ = callback;
    consumerThread_ = new std::thread([this] {
        int32_t flushFence = 0;
        int64_t timestamp = 0;
        OHOS::Rect damage;
        OHOS::BufferRequestConfig config;
        while (running_ == true) {
            OHOS::sptr<OHOS::SurfaceBuffer> buffer = nullptr;
            consumer_->AcquireBuffer(buffer, flushFence, timestamp, damage);
            if (buffer != nullptr) {
                void* addr = buffer->GetVirAddr();
                uint32_t size = buffer->GetSize();
                uint64_t pa = buffer->GetPhyAddr();
                CAMERA_LOGI("consumer receive buffer add = %{public}llu", pa);
                if (callback_ != nullptr) {
                    callback_(addr, size);
                }
                consumer_->ReleaseBuffer(buffer, -1);
                CAMERA_LOGI("consumer release buffer add = %{public}llu", pa);
                shotCount_--;
                if (shotCount_ == 0) {
                    std::unique_lock<std::mutex> l(l_);
                    cv_.notify_one();
                }
            }
            if (running_ == false) {
                break;
            }
        }
    });
    return producer;
}
}
