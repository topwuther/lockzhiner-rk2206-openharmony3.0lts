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

#include <cstdint>
#include <cstdio>

#include "kws_manager.h"

using namespace OHOS::AI;
using namespace OHOS::Audio;
using namespace OHOS::Media;

namespace KWS {
namespace {
    const int32_t CONFIDENCE = 2662;
    const int32_t NUM_SLIDE_WINDOW = 10;
    const int32_t MAX_CACHE_SIZE = 16000;
    const int32_t WINDOW_SIZE = 480;  // times of per window: 30ms
    const int32_t STRIDE_SIZE = 320;  // times of per stride: 20ms
    const int32_t SUCCESS = 0;
    const int32_t CHANNEL_COUNT = 1;

    using Capturer = std::shared_ptr<AudioCapturer>;
    using Cache = std::shared_ptr<AudioCache>;
    using Plugin = std::shared_ptr<KWSSdk>;

    // Can not create KwsManager twice
    std::shared_ptr<std::thread> g_producer = nullptr;
    std::shared_ptr<std::thread> g_consumer = nullptr;
    const auto DELETE_ARRAY = [](uint8_t *array) {
        if (array != nullptr) {
            delete[] array;
        }
    };
}

class MyKwsCallback : public KWSCallback {
public:
    MyKwsCallback() {}
    ~MyKwsCallback() {}

    void OnError(int32_t errorCode)
    {
        printf("Executing error, error code: %d\n", errorCode);
    }

    void OnResult(const Array<int32_t> &result)
    {
        if (result.size != WORD_CONTENT.size()) {
            return;
        }
        for (size_t i = 0; i < result.size; ++i) {
            if (result.data[i] > CONFIDENCE) {
                printf("[%s]\n", WORD_CONTENT[i].c_str());
            }
        }
    }
};

void ThreadTask::AudioProducer(KwsManager *kwsManager)
{
    kwsManager->ProduceSamples();
}

void ThreadTask::AudioConsumer(KwsManager *kwsManager)
{
    kwsManager->ConsumeSamples();
}

KwsManager::KwsManager(int32_t sampleRate, int32_t bitRate)
    : sampleRate_(sampleRate), bitRate_(bitRate)
{
    status_ = IDLE;
}

KwsManager::~KwsManager()
{
    OnStop();
    status_ = IDLE;
}

void KwsManager::ProduceSamples()
{
    printf("[KwsManager]ProduceSamples start\n");
    if (capturer_ == nullptr || cache_ == nullptr || aacHandler_ == -1) {
        printf("[KwsManager]Produce AudioSample failed for nullptr\n");
        return;
    }
    int32_t readLen = 0;
    int32_t retCode = SUCCESS;
    size_t frameSize = capturer_->GetFrameCount() * sizeof(uint16_t);
    if (frameSize <= 0) {
        printf("[KwsManager]Capturer get frame count failed.\n");
        return;
    }
    CoderStream aacStream = {
        .buffer = new (std::nothrow) uint8_t[frameSize],
        .size = frameSize
    };
    CoderStream pcmStream = {
        .buffer = new (std::nothrow) uint8_t[frameSize],
        .size = frameSize
    };
    if (aacStream.buffer == nullptr || pcmStream.buffer == nullptr) {
        printf("[KwsManager]Allocate buffer for aacStream and pcmStream failed.\n");
        DELETE_ARRAY(aacStream.buffer);
        DELETE_ARRAY(pcmStream.buffer);
        return;
    }
    while (status_ == RUNNING) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (capturer_ == nullptr) {
                break;
            }
            readLen = capturer_->Read(aacStream.buffer, frameSize, false);
        }
        if (readLen <= 0 || readLen > static_cast<int32_t>(frameSize)) {
            continue;
        }
        aacStream.size = readLen;
        retCode = AudioWrapper::GetInstance().Convert(aacHandler_, aacStream, pcmStream);
        if (retCode != SUCCESS) {
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (cache_ == nullptr) {
                break;
            }
            if (!cache_->AppendBuffer(pcmStream.size, pcmStream.buffer)) {
                printf("[KwsManager]Fail to append pcm into cache.\n");
            }
        }
    }
    DELETE_ARRAY(aacStream.buffer);
    DELETE_ARRAY(pcmStream.buffer);
}

void KwsManager::ConsumeSamples()
{
    uintptr_t sampleAddr = 0;
    size_t sampleSize = 0;
    int32_t retCode = SUCCESS;
    while (status_ == RUNNING) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (cache_ == nullptr) {
                printf("[KwsManager]cache_ is nullptr\n");
                break;
            }
            sampleSize = cache_->GetCapturedBuffer(sampleAddr);
        }
        if (sampleSize == 0 || sampleAddr == 0) {
            continue;
        }
        Array<int16_t> input = {
            .data = (int16_t *)sampleAddr,
            .size = sampleSize >> 1
        };
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (plugin_ == nullptr) {
                printf("[KwsManager]cache_ is nullptr\n");
                break;
            }
            if ((retCode = plugin_->SyncExecute(input)) != SUCCESS) {
                printf("[KwsManager]SyncExecute KWS failed with retcode = [%d]\n", retCode);
                continue;
            }
        }
    }
}

bool KwsManager::Prepare()
{
    printf("[KwsManager]Prepare.\n");
    if (status_ != IDLE) {
        printf("[KwsManager]Already prepared.\n");
        return false;
    }
    if (!PreparedAudioCapturer()) {
        printf("[KwsManager]Fail to prepare AudioCapturer!\n");
        OnStop();
        return false;
    }
    if (!PreparedAudioWrapper()) {
        printf("[KwsManager]Fail to prepare AudioWrapper!\n");
        OnStop();
        return false;
    }
    if (!PreparedInference()) {
        printf("[KwsManager]Fail to prepare Inference!\n");
        OnStop();
        return false;
    }
    status_ = PREPARED;
    return true;
}

void KwsManager::Start()
{
    printf("[KwsManager]Start.\n");
    if (status_ == RUNNING) {
        printf("[KwsManager]Already running.\n");
        return;
    }
    if (status_ != PREPARED && !Prepare()) {
        printf("[KwsManager]Fail to prepare KwsManager!\n");
        return;
    }
    OnStart();
}

void KwsManager::Stop()
{
    printf("[KwsManager]Stop.\n");
    if (status_ == IDLE) {
        printf("[KwsManager]Is already stopped.\n");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    status_ = IDLE;
    OnStop();
}

void KwsManager::OnStart()
{
    if (!capturer_->Start()) {
        printf("[KwsManager]Fail to start audioCapturer\n");
        OnStop();
        return;
    }
    if (aacHandler_ == -1) {
        printf("[KwsManager]Fail to start producer for the illegal aac-to-pcm handler\n");
        OnStop();
        return;
    }
    if (cache_ == nullptr) {
        printf("[KwsManager]Fail to start producer for the nullptr cache\n");
        OnStop();
        return;
    }
    if (plugin_ == nullptr) {
        printf("[KwsManager]Fail to start producer for the nullptr plugin\n");
        OnStop();
        return;
    }
    status_ = RUNNING;
    g_producer = std::make_shared<std::thread>(ThreadTask::AudioProducer, this);
    g_consumer = std::make_shared<std::thread>(ThreadTask::AudioConsumer, this);
}

void KwsManager::StopAudioCapturer()
{
    printf("[KwsManager]StopAudioCapturer\n");
    if (capturer_ != nullptr) {
        capturer_->Stop();
        capturer_ = nullptr;
    }
}

void KwsManager::StopAudioWrapper()
{
    printf("[KwsManager]StopAudioWrapper\n");
    AudioWrapper::GetInstance().Deinit(aacHandler_);
}

void KwsManager::StopInference()
{
    printf("[KwsManager]StopInference\n");
    if (plugin_ != nullptr) {
        int32_t ret = plugin_->Destroy();
        if (ret != SUCCESS) {
            printf("[KwsManager]plugin_ destroy failed.\n");
        }
        plugin_ = nullptr;
    }
}

void KwsManager::OnStop()
{
    if (capturer_ != nullptr) {
        StopAudioCapturer();
    }
    StopAudioWrapper();
    if (plugin_ != nullptr) {
        StopInference();
    }
    if (g_producer != nullptr) {
        g_producer->join();
    }
    if (g_consumer != nullptr) {
        g_consumer->join();
    }
}

bool KwsManager::PreparedAudioCapturer()
{
    // Set audio wrapper mode before build audio capturer
    AudioWrapper::GetInstance().SetCodecMode(false);
    if (capturer_ != nullptr) {
        printf("[KwsManager]Stop created AudioCapturer at first\n");
        StopAudioCapturer();
    }
    capturer_ = std::make_shared<AudioCapturer>();
    if (capturer_ == nullptr || !ConfiguredAudioCapturer()) {
        printf("[KwsManager]Fail to create AudioCapturer.\n");
        OnStop();
        return false;
    }
    return true;
}

bool KwsManager::PreparedAudioWrapper()
{
    cache_ = std::make_shared<AudioCache>();
    if (cache_ == nullptr) {
        printf("[KwsManager]Failed to create AudioCache\n");
        return false;
    }
    if (!cache_->Init(MAX_CACHE_SIZE)) {
        printf("[KwsManager]Failed to init AudioCache\n");
        return false;
    }
    if (!ConfiguredAudioWrapper()) {
        printf("[KwsManager]Failed to prepared AudioWrapper.\n");
        OnStop();
        return false;
    }
    return true;
}

bool KwsManager::PreparedInference()
{
    if (capturer_ == nullptr) {
        printf("[KwsManager]Only load plugin after AudioCapturer ready.\n");
        return false;
    }
    if (plugin_ != nullptr) {
        printf("[KwsManager]Stop created Inference Plugin at first.\n");
        StopInference();
    }
    plugin_ = std::make_shared<KWSSdk>();
    if (plugin_ == nullptr) {
        printf("[KwsManager]Failed to create InferencePlugin.\n");
        return false;
    }
    if (plugin_->Create() != SUCCESS) {
        printf("[KwsManager]Failed to create KWSSDK.\n");
        return false;
    }
    std::shared_ptr<KWSCallback> callback = std::make_shared<MyKwsCallback>();
    if (callback == nullptr) {
        printf("[KwsManager]Create callback failed.\n");
        return false;
    }
    plugin_->SetCallback(callback);
    return true;
}

bool KwsManager::ConfiguredAudioCapturer()
{
    AudioCapturerInfo audioConfig;
    audioConfig.inputSource = AUDIO_MIC;
    audioConfig.audioFormat = AAC_LC;
    audioConfig.sampleRate = AUDIO_SAMPLE_RATE;
    audioConfig.channelCount = CHANNEL_COUNT;
    audioConfig.bitRate = AUDIO_CODEC_BITRATE;
    audioConfig.streamType = TYPE_MEDIA;
    audioConfig.bitWidth = BIT_WIDTH_16;
    int32_t ret = capturer_->SetCapturerInfo(audioConfig);
    if (ret != SUCCESS) {
        printf("[KwsManager]ConfiguredAudioCapturer fail with ret = 0x%.8x\n", ret);
        return false;
    }
    return true;
}

bool KwsManager::ConfiguredAudioWrapper()
{
    ConvertType typo = CONVERT_AAC_TO_PCM;
    CoderConfig codecConfig;
    codecConfig.audioFormat = AAC_LC;
    codecConfig.bitRate = bitRate_;
    codecConfig.sampleRate = sampleRate_;
    codecConfig.channelCount = CHANNEL_COUNT;
    codecConfig.bitWidth = BIT_WIDTH_16;
    if (AudioWrapper::GetInstance().Init(typo, aacHandler_) != SUCCESS) {
        return false;
    }
    if (AudioWrapper::GetInstance().SetConfig(aacHandler_, codecConfig) != SUCCESS) {
        return false;
    }
    if (AudioWrapper::GetInstance().StartCodec(aacHandler_) != SUCCESS) {
        return false;
    }
    return true;
}
}