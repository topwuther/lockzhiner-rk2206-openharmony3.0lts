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
#ifndef RECODER_DEMO_TEST_H
#define RECODER_DEMO_TEST_H

#include "gtest/gtest.h"
#include "recorder.h"

namespace OHOS {
namespace Media {
struct AudioRecorderConfig {
    int32_t audioSourceId;
    int32_t audioEncodingBitRate;
    int32_t channelCount;
    int32_t duration;
    std::string outPath;
    int32_t sampleRate;
    AudioCodecFormat audioFormat;
    AudioSourceType inputSource;
    OutputFormatType outPutFormat;
};

class ActsAudioRecorderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    void ConfigAudioRecorder(std::shared_ptr<Recorder> recorder, AudioRecorderConfig audioConfig);
    void AudioRecorderStart(std::shared_ptr<Recorder> recorder);
};

class TestAudioRecorderCallback : public RecorderCallback {
public:
    void OnError(int32_t errorType, int32_t errorCode);
    void OnInfo(int32_t type, int32_t extra);
};
}
}
#endif