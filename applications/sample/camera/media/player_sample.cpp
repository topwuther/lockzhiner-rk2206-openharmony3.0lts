/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include "source.h"
#include "player.h"
#include "format.h"
#include <string>
#include <sys/prctl.h>
#include "securec.h"

using OHOS::Surface;

using OHOS::Media::Player;
using OHOS::Media::PlayerSeekMode;
using OHOS::Media::Source;
using OHOS::Media::Format;
using OHOS::Media::StreamSource;
using OHOS::Media::StreamCallback;
using namespace OHOS::Media;

static int64_t GetCurTimeUs()
{
    const int32_t  SS2US = 1000000;
    struct timeval ts;
    ts.tv_sec = 0;
    ts.tv_usec = 0;
    gettimeofday(&ts, nullptr);
    return (static_cast<int64_t> (ts.tv_sec) * SS2US) + static_cast<int64_t> (ts.tv_usec);
}

static int64_t GetCurTimeMs()
{
    const int32_t US2MS = 1000;
    int64_t curTimeUs = GetCurTimeUs();
    return static_cast<int64_t>(curTimeUs / US2MS);
}

class StreamSourceSample;
struct TestSample {
    std::shared_ptr<Player> adapter;
    pthread_t process;
    pthread_mutex_t mutex;
    bool isThreadRunning;
    int32_t sourceType;
    char filePath[PATH_MAX + 1];
    int64_t fileSize;
    int32_t pauseAfterPlay;
    Surface *surface;
    std::shared_ptr<StreamSourceSample> streamSample;
    std::shared_ptr<PlayerCallback> cb;
    int64_t newPlayerCost;
    int64_t setSourceCost;
    int64_t prepareCost;
    int64_t setSurfaceCost;
    int64_t playCost;
    int64_t RewindCost;
    int64_t RewindBegin;
    int64_t stopCost;
    int64_t releaseCost;
};

struct IdleBuffer {
    size_t idx;
    size_t offset;
    size_t size;
};

class StreamSourceSample : public StreamSource {
public:
    StreamSourceSample(void);
    virtual ~StreamSourceSample(void);
    void OnBufferAvailable(size_t index, size_t offset, size_t size);
    void SetStreamCallback(const std::shared_ptr<StreamCallback> &callback);
    uint8_t *GetBufferAddress(size_t idx);
    void QueueBuffer(size_t index, size_t offset, size_t size, int64_t timestampUs, uint32_t flags);
    int GetAvailableBuffer(IdleBuffer *buffer);

private:
    std::weak_ptr<StreamCallback> m_callBack;
    std::vector<IdleBuffer> aviableBuffer;
    pthread_mutex_t m_mutex;
};

StreamSourceSample::StreamSourceSample(void)
{
    aviableBuffer.clear();
    pthread_mutex_init(&m_mutex, nullptr);
}

StreamSourceSample::~StreamSourceSample(void)
{
    aviableBuffer.clear();
    pthread_mutex_destroy(&m_mutex);
}

void StreamSourceSample::SetStreamCallback(const std::shared_ptr<StreamCallback> &callback)
{
    m_callBack = callback;
}

uint8_t *StreamSourceSample::GetBufferAddress(size_t idx)
{
    std::shared_ptr<StreamCallback> callback = m_callBack.lock();
    if (callback == nullptr) {
        return nullptr;
    }
    return callback->GetBuffer(idx);
}

void StreamSourceSample::QueueBuffer(size_t index, size_t offset, size_t size, int64_t timestampUs, uint32_t flags)
{
    std::shared_ptr<StreamCallback> callback = m_callBack.lock();
    if (callback == nullptr) {
        return;
    }
    callback->QueueBuffer(index, offset, size, timestampUs, flags);
}

void StreamSourceSample::OnBufferAvailable(size_t index, size_t offset, size_t size)
{
    IdleBuffer buffer;
    pthread_mutex_lock(&m_mutex);
    buffer.idx = index;
    buffer.offset = offset;
    buffer.size = size;
    aviableBuffer.push_back(buffer);
    pthread_mutex_unlock(&m_mutex);
}

int StreamSourceSample::GetAvailableBuffer(IdleBuffer* buffer)
{
    if (buffer == nullptr) {
        printf("buffer is nullptr error %s %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    pthread_mutex_lock(&m_mutex);
    if (aviableBuffer.empty()) {
        pthread_mutex_unlock(&m_mutex);
        return -1;
    }
    *buffer = aviableBuffer[0];
    aviableBuffer.erase(aviableBuffer.begin());
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

class PlayerCallbackImpl : public PlayerCallback {
public:
    PlayerCallbackImpl(TestSample &in);
    ~PlayerCallbackImpl();
    void OnPlaybackComplete();
    void OnError(int32_t errorType, int32_t errorCode);
    void OnInfo(int type, int extra);
    void OnVideoSizeChanged(int width, int height);
    void OnRewindToComplete();

private:
    TestSample *sample;
};

PlayerCallbackImpl::PlayerCallbackImpl(TestSample &in)
{
    sample = &in;
}

PlayerCallbackImpl::~PlayerCallbackImpl()
{
    printf("dtor\n");
}

void PlayerCallbackImpl::OnPlaybackComplete(void)
{
    printf("OnPlaybackComplete test\n");
}

void PlayerCallbackImpl::OnError(int32_t errorType, int32_t errorCode)
{
    printf("OnError test, errorType:%d, errorCode:%d\n", errorType, errorCode);
}

void PlayerCallbackImpl::OnInfo(int type, int extra)
{
    printf("OnInfo test, type:%d, extra:%d\n", type, extra);
}

void PlayerCallbackImpl::OnVideoSizeChanged(int width, int height)
{
    printf("OnVideoSizeChanged test width:%d, height:%d\n", width, height);
}

void PlayerCallbackImpl::OnRewindToComplete(void)
{
    sample->RewindCost = GetCurTimeMs() - sample->RewindBegin;
    printf("OnRewindToComplete test\n");
}

static void CostTestPrintf(TestSample &sample)
{
    printf("\n\n\n##################player cost info begin\n");

    printf("start cost total:%lld\n", sample.setSourceCost + sample.prepareCost + sample.setSurfaceCost +
        sample.playCost);
    printf("new player:%lld,\n setSourceCost:%lld,\n prepareCost:%lld,\n, setSurfaceCost:%lld,\n playCost:%lld,\n",
        sample.newPlayerCost, sample.setSourceCost, sample.prepareCost, sample.setSurfaceCost, sample.playCost);

    printf("seek cost total:%lld\n", sample.RewindCost);

    printf("destory cost total:%lld\n", sample.stopCost + sample.releaseCost);
    printf("stopCost:%lld,\n releaseCost:%lld,\n", sample.stopCost, sample.releaseCost);

    printf("##################player cost info end\n\n\n");
}

static void *streamProcess(void *arg)
{
    const int32_t STEAM_PROCESS_SLEEP_TIME_US = 20000;
    uint8_t *data = nullptr;
    size_t readLen;
    size_t bufferSize;
    TestSample *sample = (TestSample *)arg;
    FILE* pFile = fopen(sample->filePath, "rb");
    if (pFile == nullptr) {
        return nullptr;
    }
    prctl(PR_SET_NAME, "StreamProc", 0, 0, 0);
    printf("[%s,%d] file:%s\n", __func__, __LINE__, sample->filePath);
    while (true) {
        pthread_mutex_lock(&sample->mutex);
        if (!sample->isThreadRunning) {
            printf("[%s,%d] thread should exit\n", __func__, __LINE__);
            pthread_mutex_unlock(&sample->mutex);
            break;
        }
        pthread_mutex_unlock(&sample->mutex);
        data = sample->streamSample->GetSharedBuffer(bufferSize);
        if (data == nullptr) {
            usleep(STEAM_PROCESS_SLEEP_TIME_US);
            continue;
        }
        readLen = fread(data, 1, bufferSize, pFile);
        if (readLen > 0) {
            sample->streamSample->QueueSharedBuffer(data, readLen);
        } else {
            sample->streamSample->QueueSharedBuffer(data, 0);
            break;
        }
    }
    fclose(pFile);
    printf("[%s,%d]\n", __func__, __LINE__);
    pthread_mutex_lock(&sample->mutex);
    sample->isThreadRunning = false;
    pthread_mutex_unlock(&sample->mutex);
    return nullptr;
}

static void GetFileSize(TestSample &sample)
{
    sample.fileSize = -1;
    if (sample.sourceType != 1 && sample.sourceType != 0) {
        return;
    }

    FILE *fileTmp = fopen(sample.filePath, "rb");
    if (fileTmp != nullptr) {
        fseek(fileTmp, 0, SEEK_END);
        sample.fileSize = ftell(fileTmp);
        fclose(fileTmp);
        fileTmp = nullptr;
    }
}

static int SetSurface(TestSample &sample)
{
    if (sample.surface != nullptr) {
        return 0;
    }

    int64_t begin = GetCurTimeMs();

    sample.surface = Surface::CreateSurface();
    if (sample.surface == nullptr) {
        printf("[%s,%d] CreateSurface failed\n",  __func__, __LINE__);
        return -1;
    }
    sample.surface->SetUserData("region_position_x", "0");
    sample.surface->SetUserData("region_position_y", "0");
    sample.surface->SetUserData("region_width", "480");
    sample.surface->SetUserData("region_height", "480");
    int32_t ret = sample.adapter->SetVideoSurface(sample.surface);
    sample.setSurfaceCost = GetCurTimeMs() - begin;
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    return 0;
}

static void DestorySurface(TestSample &sample)
{
    if (sample.surface != nullptr) {
        delete(sample.surface);
        sample.surface = nullptr;
    }
}

static int SetCallback(TestSample &sample)
{
    if (sample.cb == nullptr) {
        sample.cb = std::make_shared<PlayerCallbackImpl>(sample);
        sample.adapter->SetPlayerCallback(sample.cb);
    }
    return 0;
}

static void DestoryCallback(TestSample &sample)
{
    if (sample.cb != nullptr) {
        sample.cb.reset();
        sample.cb = nullptr;
    }
}

static int SetStreamSourceProcess(TestSample &sample)
{
    sample.streamSample = std::make_shared<StreamSourceSample>();
    Format formats;
    formats.PutStringValue(CODEC_MIME, MIME_AUDIO_AAC);
    Source source(sample.streamSample, formats);
    sample.adapter->SetSource(source);
    sample.isThreadRunning = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 0x10000);
    int32_t ret = pthread_create(&sample.process, &attr, streamProcess, &sample);
    if (ret != 0) {
        printf("pthread_create failed %d\n", ret);
        sample.isThreadRunning = false;
        return -1;
    }
    return 0;
}

static int SetSourceProcess(TestSample &sample)
{
    int64_t begin = GetCurTimeMs();
    if (sample.sourceType == 1) {
        std::string uri(sample.filePath);
        std::map<std::string, std::string> header;
        Source source(uri, header);
        sample.adapter->SetSource(source);
    } else {
        struct stat stFileState = {0};
        if (lstat(sample.filePath, &stFileState) != 0) {
            printf("lstat %s failed, please check the file exist, errno:%d\n", sample.filePath, errno);
            return -1;
        }
        if (SetStreamSourceProcess(sample) != 0) {
            printf("SetStreamSourceProcess failed\n");
            return -1;
        }
    }
    sample.setSourceCost = GetCurTimeMs() - begin;
    return 0;
}

static int PrepareProcess(TestSample &sample)
{
    int64_t begin = GetCurTimeMs();

    if (sample.pauseAfterPlay != 0) {
        Format formatSetPauseAfterPlay;
        formatSetPauseAfterPlay.PutIntValue(PAUSE_AFTER_PLAY, true);
        sample.adapter->SetParameter(formatSetPauseAfterPlay);
    }
    sample.adapter->Prepare();
    sample.prepareCost = GetCurTimeMs() - begin;

    SetCallback(sample);
    SetSurface(sample);
    return 0;
}

static int PlayProcess(TestSample &sample)
{
    int32_t ret;
    int64_t begin = GetCurTimeMs();

    if (sample.pauseAfterPlay != 0) {
        ret = sample.adapter->Pause();
    } else {
        ret = sample.adapter->Play();
    }
    sample.playCost = GetCurTimeMs() - begin;
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    return 0;
}

static int ReleaseProcess(TestSample &sample)
{
    int32_t ret;
    int64_t begin = GetCurTimeMs();

    if (sample.pauseAfterPlay != 0) {
        ret = sample.adapter->Pause();
    } else {
        ret = sample.adapter->Play();
    }
    sample.playCost = GetCurTimeMs() - begin;
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    return 0;
}

static void EndStreamSourceThread(TestSample &sample)
{
    if (sample.sourceType == 0x2 && sample.process != -1) {
        pthread_mutex_lock(&sample.mutex);
        sample.isThreadRunning = false;
        pthread_mutex_unlock(&sample.mutex);
        pthread_join(sample.process, nullptr);
        sample.process = -1;
    }
}

static int CreatePlayerProcess(TestSample &sample)
{
    SetSourceProcess(sample);
    PrepareProcess(sample);
    PlayProcess(sample);
    return 0;
}

static int ReplayProcess(TestSample &sample)
{
    EndStreamSourceThread(sample);
    sample.adapter->Stop();

    sample.adapter->Release();
    sample.adapter.reset();

    DestorySurface(sample);
    DestoryCallback(sample);
    if (sample.sourceType == 0x2) {
        sample.streamSample.reset();
    }

    sample.adapter = std::make_shared<Player>();

    return CreatePlayerProcess(sample);
}

static int PlayNextProcess(TestSample &sample)
{
    EndStreamSourceThread(sample);
    sample.adapter->Stop();
    sample.adapter->Reset();
    DestorySurface(sample);
    if (sample.sourceType == 0x2) {
        sample.streamSample.reset();
    }

    return CreatePlayerProcess(sample);
}

static int CostTest(TestSample &sample)
{
    if (sample.sourceType >= 0x2) {
        return 0;
    }

    int64_t begin;
    int64_t end;

    EndStreamSourceThread(sample);
    begin = GetCurTimeMs();
    sample.adapter->Stop();
    end = GetCurTimeMs();
    sample.stopCost = end - begin;

    begin = end;
    sample.adapter->Release();
    sample.adapter.reset();
    sample.releaseCost = GetCurTimeMs() - begin;

    DestorySurface(sample);
    DestoryCallback(sample);
    if (sample.sourceType == 0x2) {
        sample.streamSample.reset();
    }
    CostTestPrintf(sample);

    begin = GetCurTimeMs();
    sample.adapter = std::make_shared<Player>();
    sample.newPlayerCost = GetCurTimeMs() - begin;

    CreatePlayerProcess(sample);

    sleep(0xa); // sleep 10s and seek
    sample.RewindBegin = GetCurTimeMs();
    sample.adapter->Rewind(0, PlayerSeekMode::PLAYER_SEEK_PREVIOUS_SYNC);
    sleep(0x5); // sleep 5s after seek
    return 0;
}

static void RunCmdShow(TestSample &sample)
{
    printf("now show player info\n");

    printf("player IsPlaying:%d IsSingleLooping:%d\n", sample.adapter->IsPlaying(),
        sample.adapter->IsSingleLooping());

    int64_t duration;
    int32_t ret = sample.adapter->GetDuration(duration);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }

    int64_t currentPosition;
    ret = sample.adapter->GetCurrentTime(currentPosition);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    printf("player duration:%lld GetCurrentTime:%lld\n", duration, currentPosition);

    int32_t videoWidth;
    int32_t videoHeight;
    ret = sample.adapter->GetVideoWidth(videoWidth);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    ret = sample.adapter->GetVideoHeight(videoHeight);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    printf("player videoWidth:%d videoHeight:%d\n", videoWidth, videoHeight);
    int32_t state;
    ret = sample.adapter->GetPlayerState(state);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    printf("player current state:%d\n", state);
}

static void RunCmdSetVolume(TestSample &sample, const char cmd[])
{
    float lvolume;
    float rvolume;

    if (sscanf_s(cmd, "volume %f %f", &lvolume, &rvolume) != 0x2) {
        printf("ERR: not input volume, example: volume 50 50!\n");
        return;
    }
    printf("try set volume %f %f\n", lvolume, rvolume);
    int32_t ret = sample.adapter->SetVolume(lvolume, rvolume);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
}

static void RunCmdNext(TestSample &sample, const char cmd[])
{
    int32_t cnt;

    if (sscanf_s(cmd, "next %d", &cnt) != 1) {
        printf("ERR: not input next play count, example: next 10000!\n");
        return;
    }

    while (cnt > 0) {
        if (PlayNextProcess(sample) != 0) {
            printf("PlayNextProcess process failed\n");
            return;
        }
        sleep(0x5); // sleep 5s and loop
        cnt--;
    };
}

static void RunCmdPressurePupl(TestSample &sample, const char cmd[])
{
    int32_t cnt;
    int ret;

    if (sscanf_s(cmd, "pressure_pupl %d", &cnt) != 1) {
        printf("ERR: not input replay count, example: pressure 10000!\n");
        return;
    }
    while (cnt > 0) {
        ret = sample.adapter->Pause();
        sleep(1);
        ret = sample.adapter->Play();
        sleep(1);
        cnt--;
    }
}

static void RunCmdPressureSeek(TestSample &sample, const char cmd[])
{
    int32_t cnt;
    int ret;

    if (sscanf_s(cmd, "pressure_seek %d", &cnt) != 1) {
        printf("ERR: not input replay count, example: pressure 10000!\n");
        return;
    }
    if (sample.sourceType == 0x2) {
        printf("ERR: streamsource not support pressure_seek\n");
        return;
    }

    const int32_t rewindPos = 5000;
    int64_t currentPosition;
    while (cnt > 0) {
        sample.adapter->Rewind(rewindPos, PlayerSeekMode::PLAYER_SEEK_PREVIOUS_SYNC);
        ret = sample.adapter->GetCurrentTime(currentPosition);
        printf("##############player GetCurrentTime, ret:%d, time:%lld\n", ret, currentPosition);
        sleep(0x2);
        cnt--;
    }
}

static void RunCmdLoop(TestSample &sample, const char cmd[])
{
    int32_t loop;

    if (sscanf_s(cmd, "loop %d", &loop) != 1) {
        printf("ERR: not input loop flag, example: loop 1!\n");
        return;
    }

    printf("try set loop %d\n", loop);
    int32_t ret = sample.adapter->EnableSingleLooping(loop == 1);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
    printf("Set loop %d\n", loop);
}

static void RunCmdSpeed(TestSample &sample, const char cmd[])
{
    float speed = 1.0;
    if (sscanf_s(cmd, "speed %f", &speed) != 1) {
        printf("ERR: not input seek time, example: speed 1.0!\n");
        return;
    }

    int32_t ret = sample.adapter->SetPlaybackSpeed(speed);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
}

static void RunCmdSeek(TestSample &sample, const char cmd[])
{
    int64_t timeMs = 0;
    if (sscanf_s(cmd, "seek %d", &timeMs) != 1) {
        printf("ERR: not input seek time, example: seek 1000!\n");
        return;
    }

    int32_t ret = sample.adapter->Rewind(timeMs, PlayerSeekMode::PLAYER_SEEK_PREVIOUS_SYNC);
    if (ret != 0) {
        printf("[%s: %d] ret:%d\n", __func__, __LINE__, ret);
    }
}

static void RunCmdSleep(const char cmd[])
{
    int64_t timeS = 0;
    if (sscanf_s(cmd, "sleep %d", &timeS) != 1) {
        printf("ERR: not input replay count, example: sleep 100!\n");
        return;
    }
    sleep(timeS);
}

static void RunCmdPlay(TestSample &sample)
{
    int32_t ret = sample.adapter->Play();
    if (ret != 0) {
        printf("[%s: %d] Play failed ret:%d\n", __func__, __LINE__, ret);
    }
}

static void ShowCmdHelp(void)
{
    printf("Input CMD: quit, show, pause(pu), play(pl), seek [ms], volume [lvol] [rvol], loop [bool], speed [float]"
        "replay, next [cnt], pressure_pupl [cnt], pressure_seek [cnt], sleep [s] cost_test\n");
}

static void SampleCmd(TestSample &sample)
{
    char cmd[0x40];   /* 64: Array length */
    int ret;
    char *retValue = nullptr;

    ShowCmdHelp();
    while ((retValue = fgets(cmd, 0x20, stdin)) != nullptr) {  /* 10: use array length */
        cmd[0x20] = '\0';  /* 10: end of string */
        if (strncmp(cmd, "quit", 0x4) == 0) {
            printf("quit\n");
            break;
        } else if (strncmp(cmd, "pause", 0x4) == 0 || strncmp(cmd, "pu", 0x2) == 0) {
            ret = sample.adapter->Pause();
        } else if (strncmp(cmd, "replay", 0x6) == 0) {
            if (ReplayProcess(sample) != 0) {
                printf("ReplayProcess process failed\n");
                break;
            }
        } else if (strncmp(cmd, "play", 0x4) == 0 || strncmp(cmd, "pl", 0x2) == 0) {
            RunCmdPlay(sample);
        } else if (strncmp(cmd, "speed", 0x5) == 0) {
            RunCmdSpeed(sample, cmd);
        } else if (strncmp(cmd, "seek", 0x4) == 0) {
            RunCmdSeek(sample, cmd);
        } else if (strncmp(cmd, "volume", 0x6) == 0) {
            RunCmdSetVolume(sample, cmd);
        } else if (strncmp(cmd, "show", 0x4) == 0) {
            RunCmdShow(sample);
        } else if (strncmp(cmd, "loop", 0x4) == 0) {
            RunCmdLoop(sample, cmd);
        } else if (strncmp(cmd, "next", 0x4) == 0) {
            RunCmdNext(sample, cmd);
        } else if (strncmp(cmd, "pressure_pupl", 0xd) == 0) {
            RunCmdPressurePupl(sample, cmd);
        } else if (strncmp(cmd, "pressure_seek", 0xd) == 0) {
            RunCmdPressureSeek(sample, cmd);
        } else if (strncmp(cmd, "sleep", 0x5) == 0) {
            RunCmdSleep(cmd);
        } else if (strncmp(cmd, "cost_test", 0x9) == 0) {
            CostTest(sample);
        } else {
            ShowCmdHelp();
        }
    }

    printf("cmd test end\n");
}

static int32_t ParseInputParam(int argc, char **argv, TestSample &sample)
{
    int32_t sourceType = 1;
    int32_t pauseAfterPlay = 0;

    if (argv == nullptr) {
        printf("argv is nullptr error %s %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if (argc < 0x2) {  /* 2: Minimum number of inputs */
        printf("usage: %s filename source_type(1:uri, 2:stream-source) pause-after-play(1:yes, ohter:no)\n", argv[0]);
        printf("eg: %s file 1 0\n", argv[0]);
        return -1;
    }
    if (argc >= 0x3) {  /* 4:  sourceType */
        sourceType = atoi(argv[0x2]);  /*  */
        if ((sourceType != 1) && (sourceType != 0x2)) {
            printf("unsuport sourceType:%d\n", sourceType);
            return -1;
        }
    }
    if (argc >= 0x4) {
        pauseAfterPlay = atoi(argv[0x3]);
        if (pauseAfterPlay != 1) {
            printf("unsuport pauseAfterPlay:%d, not enable pause after play mode\n", sourceType);
            pauseAfterPlay = 0;
        }
    }

    pthread_mutex_init(&sample.mutex, nullptr);
    sample.pauseAfterPlay = pauseAfterPlay;
    sample.sourceType = sourceType;

    if ((sourceType == 0 || sourceType == 0x2) && (realpath(argv[1], sample.filePath) == nullptr)) {
        printf("realpath input file failed, errno: %d!\n", errno);
        return -1;
    } else if (strlen(argv[1]) < PATH_MAX) {
        if (memcpy_s(sample.filePath, PATH_MAX, argv[1], strlen(argv[1]) + 1) != 0) {
            printf("memcpy_s failed!\n");
        }
    } else {
        printf("input path too long!\n");
        return -1;
    }

    printf("[%s,%d] sourceType:%d, path:%s, pauseAfterPlay:%d\n", __func__, __LINE__, sourceType, sample.filePath,
        pauseAfterPlay);
    return 0;
}
int main(int argc, char **argv)
{
    prctl(PR_SET_NAME, "mainProc", 0, 0, 0);
    TestSample sample = {0};
    sample.process = -1;

    if (ParseInputParam(argc, argv, sample) != 0) {
        return -1;
    }

    GetFileSize(sample);
    sample.adapter = std::make_shared<Player>();
    SetSourceProcess(sample);

    if (sample.pauseAfterPlay != 0) {
        Format formatSetPauseAfterPlay;
        formatSetPauseAfterPlay.PutIntValue(PAUSE_AFTER_PLAY, true);
        sample.adapter->SetParameter(formatSetPauseAfterPlay);
    }
    sample.adapter->Prepare();

    SetCallback(sample);
    SetSurface(sample);
    PlayProcess(sample);

    sleep(1);

    SampleCmd(sample);
    EndStreamSourceThread(sample);
    int32_t ret = sample.adapter->Stop();
    printf("Stop, ret:%d\n", ret);

    ret = sample.adapter->Release();
    printf("Release, ret:%d\n", ret);
    DestorySurface(sample);
    DestoryCallback(sample);
    pthread_mutex_destroy(&sample.mutex);

    printf("[%s, %d] main end\n", __func__, __LINE__);
    return 0;
}
