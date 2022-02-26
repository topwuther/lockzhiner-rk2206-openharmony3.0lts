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

#ifndef KWS_MANAGER_H
#define KWS_MANAGER_H

#include <mutex>
#include <thread>
#include <unistd.h>
#include <vector>

#include "ai_datatype.h"
#include "audio_cache.h"
#include "audio_capturer.h"
#include "audio_wrapper.h"
#include "kws_callback.h"
#include "kws_sdk.h"
#include "media_errors.h"
#include "media_info.h"

namespace KWS {
const int32_t AUDIO_SAMPLE_RATE = 16000; // 16kHz
const int32_t AUDIO_CODEC_BITRATE = 32000; // 32kHz

const std::vector<std::string> WORD_CONTENT = {
    "Hi Xiaowen",
    "Nihao Wenwen",
    "Unknown"
};

enum KwsStatus {
    IDLE = 1000,
    PREPARED,
    RUNNING,
};

class KwsManager {
public:
    KwsManager(int32_t sampleRate, int32_t bitRate);
    ~KwsManager();
    void Start();
    void Stop();
    bool Prepare();
    void ProduceSamples();
    void ConsumeSamples();

private:
    void OnStart();
    void OnStop();
    bool PreparedAudioCapturer();
    bool PreparedAudioWrapper();
    bool PreparedInference();
    bool ConfiguredAudioCapturer();
    bool ConfiguredAudioWrapper();
    void StopAudioCapturer();
    void StopAudioWrapper();
    void StopInference();

private:
    std::shared_ptr<AudioCache> cache_ = nullptr;
    std::shared_ptr<OHOS::AI::KWSSdk> plugin_ = nullptr;
    std::shared_ptr<OHOS::Audio::AudioCapturer> capturer_ = nullptr;
    int32_t sampleRate_ = AUDIO_SAMPLE_RATE;
    int32_t bitRate_ = AUDIO_CODEC_BITRATE;
    KwsStatus status_ = IDLE;
    intptr_t aacHandler_ = -1;
    std::mutex mutex_;
};

class ThreadTask {
public:
    static void AudioProducer(KwsManager *kwsManager);
    static void AudioConsumer(KwsManager *kwsManager);
};
}  // namespace KWS
#endif