/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file types.h
 *
 * @brief Declares data types
 * used by the Hardware Driver Interfaces (HDIs) of this module.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef CAMERA_DEVICE_DRIVER_TYPES_H
#define CAMERA_DEVICE_DRIVER_TYPES_H

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <surface.h>
#include "camera_metadata_info.h"

namespace OHOS::Camera {
using CameraAbility = CameraStandard::CameraMetadata;
using CameraSetting = CameraStandard::CameraMetadata;

/**
 * @brief buffer key information of ExtraGet() and ExtraSet().
 */
const std::string dataSize = "dataSize";     // int32_t
const std::string timeStamp = "timeStamp";   // int64_t
const std::string isKeyFrame = "isKeyFrame"; // int32_t 1:true 0:false

/**
 * @brief Enumerates return values of the HDIs.
 */
using CamRetCode = enum _CamRetCode : int32_t {
    /**
     * Successful call.
     */
    NO_ERROR = 0,

    /**
     * The camera device is busy.
     */
    CAMERA_BUSY = 1,

    /**
     * Insufficient resources.
     */
    INSUFFICIENT_RESOURCES = 2,

    /**
     * Invalid parameters.
     */
    INVALID_ARGUMENT = 3,

    /**
     * Unsupported function.
     */
    METHOD_NOT_SUPPORTED = 4,

    /**
     * The camera device is closed.
     */
    CAMERA_CLOSED = 5,

    /**
     * A critical error occurs at the driver layer.
     */
    DEVICE_ERROR = 6,
};

/**
 * @brief Enumerates metadata reporting modes.
 */
using ResultCallbackMode = enum _ResultCallbackMode : int32_t {
    /**
     * Frame-by-frame reporting
     */
    PER_FRAME,

    /**
     * Reporting upon device status change
     */
    ON_CHANGED
};

/**
 * @brief Enumerates stream operation modes.
 */
using OperationMode = enum _OperationMode : int32_t {
    /**
     * Normal
     */
    NORMAL = 0,
};

/**
 * @brief Enumerates stream types.
 */
using StreamIntent = enum _StreamIntent : int32_t {
    /**
     * Preview streams, indicating that stream data is used for preview
     */
    PREVIEW = 0,

    /**
     * Video streams, indicating that stream data is used to encode and generate videos
     */
    VIDEO = 1,

    /**
     * Photographing streams, indicating that stream data is used to encode and generate images
     */
    STILL_CAPTURE = 2,

    /**
     * Stream data that is used to store thumbnails
     */
    POST_VIEW = 3,

    /**
     * Stream data that is used for image analysis
     */
    ANALYZE = 4,

    /**
     * Custom type
     */
    CUSTOM = 5,
};

/**
 * @brief Enumerates encoding types of stream data.
 */
using EncodeType = enum _EncodeType : int32_t {
    /**
     * Unspecified
     */
    ENCODE_TYPE_NULL = 0,

    /**
     * H.264
     */
    ENCODE_TYPE_H264 = 1,

    /**
     * H.265
     */
    ENCODE_TYPE_H265 = 2,

    /**
     * JPEG
     */
    ENCODE_TYPE_JPEG = 3,
};

/**
 * @brief Defines the stream information, which is used to pass configuration parameters during stream creation.
 */
using StreamInfo = struct _StreamInfo {
    /**
     * Stream ID, which uniquely identifies a stream on a camera device.
     */
    int streamId_;

    /**
     * Image width.
     */
    int width_;

    /**
     * Image height.
     */
    int height_;

    /**
     * Image format.
     */
    int format_;

    /**
     * Image color space.
     */
    int datasapce_;

    /**
     * Stream type.
     */
    StreamIntent intent_;

    /**
     * Tunnel mode. The value <b>true</b> means that the tunnel mode is enabled, and <b>false</b> means the opposite.
     *
     * After the tunnel mode is enabled,
     * the hardware abstraction layer (HAL) does not directly interact with the upper layer.
     * Instead, it uses the producer handle provided by the graphics layer to transfer frame data.
     * You need to disable the tunnel mode for IoT devices that do not support
     * or require image data caching and forwarding of preview streams.
     */
    bool tunneledMode_;

    /**
     * Producer handle provided by the graphics layer.
     */
    OHOS::sptr<OHOS::IBufferProducer> bufferQueue_;

    /**
     * Minimum frame interval.
     */
    int minFrameDuration_;

    /**
     * Encoding type.
     */
    EncodeType encodeType_;
};

/**
 * @brief Enumerates the support types of the stream. For details about the application scenario,
 * see {@link IsStreamsSupported}.
 */
using StreamSupportType = enum _StreamSupportType : int32_t {
    /**
     * The stream can be dynamically created, and the corresponding stream parameters take effect directly.
     */
    DYNAMIC_SUPPORTED,

    /**
     * The stream cannot be dynamically created,
     * and the corresponding parameters take effect only after the existing stream is stopped and reconfigured.
     */
    RE_CONFIGURED_REQUIRED,

    /**
     * The stream cannot be dynamically created.
     */
    NOT_SUPPORTED,
};

/**
 * @brief Defines the stream attributes.
 */
using StreamAttribute = struct _StreamAttribute {
    /**
     * Stream ID, which uniquely identifies a stream on a camera device.
     */
    int streamId_;

    /**
     * Image width.
     */
    int width_;

    /**
     * Image height.
     */
    int height_;

    /**
     * New image format.
     */
    int overrideFormat_;

    /**
     * New image color space.
     */
    int overrideDatasapce_;

    /**
     * New procedure usage.
     */
    int producerUsage_;

    /**
     * New number of produce buffers.
     */
    int producerBufferCount_;

    /**
     * Maximum number of frames that can be captured in a continuous capture.
     */
    int maxBatchCaptureCount_;

    /**
     * Maximum number of concurrent capture requests. The default value is <b>1</b>.
    */
    int maxCaptureCount_;
};

/**
 * @brief Defines the information about a capture request.
 */
using CaptureInfo = struct _CaptureInfo {
    /**
     * IDs of captured streams.
    */
    std::vector<int> streamIds_;

    /**
     * Captured configuration information.
     */
    std::shared_ptr<CameraStandard::CameraMetadata> captureSetting_;

    /**
     * Whether to enable callback for each capture. If enabled, {@link OnFrameShutter} is called upon each capture.
    */
    bool enableShutterCallback_;
};

/**
 * @brief Enumerates camera device statuses.
 */
using CameraStatus = enum _CameraStatus {
    /**
     * The camera device is not in position or is unavailable.
     */
    UN_AVAILABLE = 0,

    /**
     * The camera device is available.
     */
    AVAILABLE = 1,
};

/**
 * @brief Enumerates flash statuses.
 */
using FlashlightStatus = enum _FlashlightStatus : uint32_t {
    /**
     * The flash is off.
     */
    FLASHLIGHT_OFF = 0,

    /**
     * The flash is on.
     */
    FLASHLIGHT_ON = 1,

    /**
     * The flash is unavailable.
     */
    FLASHLIGHT_UNAVAILABLE = 2,
};

/**
 * @brief Enumerates camera device error types, which are used by {@link OnError}.
 */
using ErrorType = enum _ErrorTyp : uint32_t {
    /**
     * A critical error occurs. The camera device needs to be closed.
     */
    FATAL_ERROR = 0,

    /**
     * A request timeout occurs. The camera device needs to be closed.
     */
    REQUEST_TIMEOUT = 1,
};

/**
 * @brief Defines the information about the end of packet capture, which is used by {@link OnCaptureEnded}.
 */
using CaptureEndedInfo = struct _CaptureEndedInfo {
    /**
     * ID of a captured stream.
     */
    int streamId_;

    /**
     * Number of frames that have been captured when the capture ends.
     */
    int frameCount_;
};

/**
 * @brief Enumerates stream error types, which are used by {@link CaptureErrorInfo}.
 */
using StreamError = enum _StreamError {
    /**
     * Unknown error
     */
    UNKNOWN_ERROR = 0,

    /**
     * Packet loss
     */
    BUFFER_LOST = 1,
};

/**
 * @brief Defines the stream error information, which is used by {@link OnCaptureError}.
 */
using CaptureErrorInfo = struct _CaptureErrorInfo {
    /**
     * Stream ID
     */
    int streamId_;

    /**
     * Error type
     */
    StreamError error_;
};

using MetaType = int32_t;

class IBuffer;
}
#endif /* CAMERA_DEVICE_DRIVER_TYPES_H */
