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

#include "v4l2_buffer.h"

namespace OHOS::Camera {
HosV4L2Buffers::HosV4L2Buffers(enum v4l2_memory memType, enum v4l2_buf_type bufferType)
    : memoryType_(memType), bufferType_(bufferType)
{
}

HosV4L2Buffers::~HosV4L2Buffers() {}

RetCode HosV4L2Buffers::V4L2ReqBuffers(int fd, int unsigned buffCont)
{
    struct v4l2_requestbuffers req = {};

    CAMERA_LOGD("V4L2ReqBuffers buffCont %d\n", buffCont);

    req.count = buffCont;
    req.type = bufferType_;
    req.memory = memoryType_;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        CAMERA_LOGE("does not support memory mapping %s\n", strerror(errno));
        return RC_ERROR;
    }

    if (req.count != buffCont) {
        CAMERA_LOGE("error Insufficient buffer memory on \n");

        req.count = 0;
        req.type = bufferType_;
        req.memory = memoryType_;
        if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
            CAMERA_LOGE("V4L2ReqBuffers does not release buffer	%s\n", strerror(errno));
            return RC_ERROR;
        }

        return RC_ERROR;
    }

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2QueueBuffer(int fd, const std::shared_ptr<FrameSpec>& frameSpec)
{
    struct v4l2_buffer buf = {};

    if (frameSpec == nullptr) {
        CAMERA_LOGE("V4L2QueueBuffer: frameSpec is NULL\n");
        return RC_ERROR;
    }

    buf.index = (uint32_t)frameSpec->buffer_->GetIndex();
    buf.type = bufferType_;
    buf.memory = memoryType_;

    buf.m.userptr = (unsigned long)frameSpec->buffer_->GetVirAddress();
    buf.length = frameSpec->buffer_->GetSize();

    CAMERA_LOGD("V4L2QueueBuffer buf.index = %d, buf.length = %d, buf.m.userptr = %p\n",
        buf.index, buf.length, (void*)buf.m.userptr);

    int rc = ioctl(fd, VIDIOC_QBUF, &buf);
    if (rc < 0) {
        CAMERA_LOGE("ioctl VIDIOC_QBUF failed: %s\n", strerror(errno));
        return RC_ERROR;
    }

    auto itr = queueBuffers_.find(fd);
    if (itr != queueBuffers_.end()) {
        std::lock_guard<std::mutex> l(bufferLock_);
        itr->second[buf.index] = frameSpec;
        CAMERA_LOGD("insert frameMap fd = %d buf.index = %d\n", fd, buf.index);
    } else {
        FrameMap frameMap;
        std::lock_guard<std::mutex> l(bufferLock_);
        frameMap.insert(std::make_pair(buf.index, frameSpec));
        queueBuffers_.insert(std::make_pair(fd, frameMap));
        CAMERA_LOGD("insert fd = %d buf.index = %d\n", fd, buf.index);
    }

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2DqueueBuffer(int fd)
{
    struct v4l2_buffer buf = {};

    buf.type = bufferType_;
    buf.memory = memoryType_;

    int rc = ioctl(fd, VIDIOC_DQBUF, &buf);
    if (rc < 0) {
        CAMERA_LOGE("ioctl VIDIOC_DQBUF failed: %s\n", strerror(errno));
        return RC_ERROR;
    }
    CAMERA_LOGD("V4L2DqueueBuffer index = %d buf.m.ptr = %u\n", buf.index, buf.m.userptr);

    auto IterMap = queueBuffers_.find(fd);
    if (IterMap == queueBuffers_.end()) {
        CAMERA_LOGE("std::map queueBuffers_ no fd\n");
        return RC_ERROR;
    }
    auto& bufferMap = IterMap->second;

    auto Iter = bufferMap.find(buf.index);
    if (Iter == bufferMap.end()) {
        CAMERA_LOGE("V4L2DqueueBuffer buf.index == %d is not find in FrameMap\n", buf.index);
        return RC_ERROR;
    }

    if (dequeueBuffer_ == nullptr) {
        CAMERA_LOGE("V4L2DqueueBuffer buf.index == %d no callback\n", buf.index);
        std::lock_guard<std::mutex> l(bufferLock_);
        bufferMap.erase(Iter);
        return RC_ERROR;
    }

    // callback to up
    dequeueBuffer_(Iter->second);
    std::lock_guard<std::mutex> l(bufferLock_);
    bufferMap.erase(Iter);

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2AllocBuffer(int fd, const std::shared_ptr<FrameSpec>& frameSpec)
{
    struct v4l2_buffer buf = {};
    CAMERA_LOGD("V4L2AllocBuffer\n");

    if (frameSpec == nullptr) {
        CAMERA_LOGE("V4L2AllocBuffer frameSpec is NULL\n");
        return RC_ERROR;
    }

    switch (memoryType_) {
        case V4L2_MEMORY_MMAP:
            // to do something
            break;
        case V4L2_MEMORY_USERPTR:
            buf.type = bufferType_;
            buf.memory = memoryType_;
            buf.index = (uint32_t)frameSpec->buffer_->GetIndex();
            CAMERA_LOGD("V4L2_MEMORY_USERPTR Print the cnt: %d\n", buf.index);

            if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
                CAMERA_LOGE("error: ioctl VIDIOC_QUERYBUF failed: %s\n", strerror(errno));
                return RC_ERROR;
            }

            CAMERA_LOGD("buf.length = %d frameSpec->buffer_->GetSize() = %d\n", buf.length,
                        frameSpec->buffer_->GetSize());

            if (buf.length > frameSpec->buffer_->GetSize()) {
                CAMERA_LOGE("ERROR:user buff < V4L2 buf.length\n");
                return RC_ERROR;
            }

            break;
        case V4L2_MEMORY_OVERLAY:
            // to do something
            break;

        case V4L2_MEMORY_DMABUF:
            // to do something
            break;

        default:
            CAMERA_LOGE("It can not be happening - incorrect memory type\n");
            return RC_ERROR;
    }

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2ReleaseBuffers(int fd)
{
    std::lock_guard<std::mutex> l(bufferLock_);
    queueBuffers_.erase(fd);

    return V4L2ReqBuffers(fd, 0);
}

void HosV4L2Buffers::SetCallback(BufCallback cb)
{
    CAMERA_LOGD("HosV4L2Buffers::SetCallback OK.");
    dequeueBuffer_ = cb;
}
} // namespace OHOS::Camera
