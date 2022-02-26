/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef RECODER_VIDEO_TEST_H
#define RECODER_VIDEO_TEST_H

#include "gtest/gtest.h"
#include "recorder.h"

namespace OHOS {
namespace Media {
struct VideoRecorderConfig {
    int32_t audioSourceId;
    int32_t videoSourceId;
    int32_t audioEncodingBitRate;
    int32_t channelCount;
    int32_t duration;
    int32_t width;
    int32_t height;
    int32_t frameRate;
    int32_t videoEncodingBitRate;
    int32_t sampleRate;
    double captureFps;
    std::string outPath;
    AudioCodecFormat audioFormat;
    AudioSourceType aSource;
    OutputFormatType outPutFormat;
    VideoSourceType vSource;
    VideoCodecFormat videoFormat;
};

class ActsVideoRecorderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    void ConfigvideoRecorder(std::shared_ptr<Recorder> recorder, VideoRecorderConfig videoConfig, bool isAudio);
    void VideoRecorderStart(std::shared_ptr<Recorder> recorder);
};

class TestVideoRecorderCallback : public RecorderCallback {
public:
    void OnError(int32_t errorType, int32_t errorCode);
    void OnInfo(int32_t type, int32_t extra);
};
}
}
#endif