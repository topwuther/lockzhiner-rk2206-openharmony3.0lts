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

#include <ctime>
#include <iostream>
#include <thread>
#include <string>
#include "audio_capturer.h"
#include "media_errors.h"
#include "securec.h"

using namespace OHOS;
using namespace OHOS::Audio;
using namespace OHOS::Media;
using namespace std;

struct AudioSourceInput {
    size_t framesize;
    uint8_t *buffer;
    AudioCapturer *audioCap;
    AudioCodecFormat audioFormat;
    std::thread processThread;
    bool bThreadRun;
};

enum AppState {
    AV_ON,
    AV_OFF
};

static AudioSourceInput g_audioSourceProcessInput;

static char *GernerateFileName(AudioCodecFormat format)
{
    time_t stCurrent;
    char aszDatetime[0x100];
    (void)time(&stCurrent);
    struct tm *pstCurrentTime = localtime(&(stCurrent));
    if (pstCurrentTime == nullptr) {
        return nullptr;
    }
    if (strftime(aszDatetime, 0x100, "%Y-%m-%d-%H-%M-%S", pstCurrentTime) > 0) {
        std::cout << "Current Time: " << aszDatetime << std::endl;
    }

    string postfix;
    switch (format) {
        case PCM:
            postfix = "pcm";
            break;
        case AAC_LC:
            postfix = "aac";
            break;
        case G711A:
            postfix = "g711a";
            break;
        case G711U:
            postfix = "g711u";
            break;
        case G726:
            postfix = "g726";
            break;
        default:
            return nullptr;
    }

    const int32_t size = 0x180;
    char *name = static_cast<char *>(malloc(size));
    if (name == nullptr) {
        return nullptr;
    }
    (void)memset_s(name, size, 0, size);
    /* create file for save stream */
    if (snprintf_s(name, size, size - 1, "/userdata/audio_%s.%s", aszDatetime, postfix.c_str()) < 0) {
        std::cout << "snprintf_s failed " << std::endl;
        free(name);
        return nullptr;
    }
    return name;
}

static void AudioInputSourceProcess(AudioSourceInput *audioSourceInput)
{
    std::cout << "audioSourceInput: " << audioSourceInput << std::endl;
    if (audioSourceInput == nullptr) {
        return;
    }

    char *fileName = GernerateFileName(audioSourceInput->audioFormat);
    if (fileName == nullptr) {
        return;
    }

    FILE *pfd = fopen(fileName, "w+");
    if (pfd == nullptr) {
        std::cout << "open file  failed " << fileName << std::endl;
        free(fileName);
        return;
    }
    std::cout << "Open  SUCCESS " << fileName << std::endl;
    int readCnt = 0;
    while (audioSourceInput->bThreadRun) {
        int ret = audioSourceInput->audioCap->Read(audioSourceInput->buffer,
            audioSourceInput->framesize, false);
        if (ret == -1) {
            std::cout << "audioCap Read failed ret:" << ret << std::endl;
            continue;
        }
        if (fwrite(audioSourceInput->buffer, 1, ret, pfd) != ret) {
            std::cout << "fwrite failed errno:"<< errno << std::endl;
            break;
        }
        readCnt++;
        std::cout << "audioCap Read readCnt:  "<< readCnt << " size: " << ret << std::endl;
    }
    (void)fclose(pfd);
    free(fileName);
}

struct CapturerInfo {
    AudioCodecFormat audioFormat;
    int32_t sampleRate;
    int32_t bitRate;
};

static AudioCodecFormat GetAudioFormat(void)
{
    std::cout << "*******************************************" << endl;
    std::cout << "SetCapturerInfo (PCM:1, AAC_LC:2, G711A:7, G711U:8, G726:9)" << endl;
    std::cout << "*******************************************" << endl;
    int32_t audioFormat;
    std::cin >> audioFormat;
    cout << "input  audioFormat:" << audioFormat << endl;
    if (audioFormat != 1 && audioFormat != 0x2 && audioFormat != 0x7 &&
        audioFormat != 0x8 && audioFormat != 0x9) {
        std::cout << "Can't support input format:" << static_cast<int32_t> (audioFormat) << std::endl;
        return FORMAT_BUTT;
    }
    return static_cast<AudioCodecFormat> (audioFormat);
}

static void GetDefaultSampleRateAndRateBaseFormat(AudioCodecFormat format, int32_t &sr, int32_t &rate)
{
    const CapturerInfo audioCapturerInfo[] = {
        {PCM, 16000, 128000},
        {AAC_LC, 48000, 128000},
        {G711A, 8000, 64000},
        {G711U, 8000, 64000},
        {G726, 8000, 24000}
    };

    int validCapInfoNum = sizeof(audioCapturerInfo) / sizeof(audioCapturerInfo[0]);
    for (int i = 0; i < validCapInfoNum; i++) {
        if (format == audioCapturerInfo[i].audioFormat) {
            sr = audioCapturerInfo[i].sampleRate;
            rate = audioCapturerInfo[i].bitRate;
        }
    }
}

static int32_t GetChannelCount(void)
{
    std::cout << "*******************************************" << endl;
    std::cout << "SetCapturerInfo (channelCount:1, channelCount:2)" << endl;
    std::cout << "*******************************************" << endl;
    int32_t channelCount;
    std::cin >> channelCount;
    if (channelCount != 1 && channelCount != 0x2) {
        std::cout << "Can't support input channelCount:" << channelCount << std::endl;
        return -1;
    }
    return channelCount;
}

static void ShowCmdInfo(void)
{
    cout << "*******************************************" << endl;
    cout << "Select the behavior of audio capturer." << endl;
    cout << "s or S-----start audio capturer" << endl;
    cout << "p or P-----stop audio capturer" << endl;
    cout << "q or Q-----quit audio capturer" << endl;
    cout << "*******************************************" << endl;
}

static void TaskQuit(AudioCapturer &audioCap, AppState &state)
{
    if (state == AV_ON) {
        g_audioSourceProcessInput.bThreadRun = false;
        g_audioSourceProcessInput.processThread.join();
        if (!audioCap.Stop()) {
            std::cout << "Stop audioCap failed, quit record\n" << endl;
        }
        state = AV_OFF;
    }
}

static int32_t TaskStop(AudioCapturer &audioCap, AppState &state)
{
    if (state == AV_ON) {
        g_audioSourceProcessInput.bThreadRun = false;
        g_audioSourceProcessInput.processThread.join();
        if (!audioCap.Stop()) {
            std::cout << "Stop audioCap fialed, stop record " << endl;
            return -1;
        }
        state = AV_OFF;
    } else {
        std::cout << "Start recorder first." << endl;
    }
    return 0;
}

static int32_t TaskStart(AudioCapturer &audioCap, AppState &state)
{
    if (state == AV_ON) {
        return 0;
    }

    if (!audioCap.Start()) {
        std::cout << "Can't Start..." << endl;
        delete g_audioSourceProcessInput.buffer;
        return -1;
    }
    g_audioSourceProcessInput.audioCap = &audioCap;
    g_audioSourceProcessInput.bThreadRun = true;
    g_audioSourceProcessInput.processThread = std::thread(AudioInputSourceProcess,
        &g_audioSourceProcessInput);
    state = AV_ON;
    std::cout << "Recording..." << endl;
    return 0;
}

static void RumCmd(AudioCapturer &audioCap)
{
    ShowCmdInfo();
    char input;
    AppState state = AV_OFF;
    while (std::cin >> input) {
        switch (input) {
            case 's':
            case 'S':
                if (TaskStart(audioCap, state) != 0) {
                    return;
                }
                break;
            case 'p':
            case 'P':
                if (TaskStop(audioCap, state) != 0) {
                    return;
                }
                break;
            case 'q':
            case 'Q':
                TaskQuit(audioCap, state);
                return;
            default:
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    std::cout << "audio_capture_sample " << std::endl;
    int ret = 0;
    size_t frameCount;
    AudioCapturer audioCap;

    AudioCapturerInfo info;
    info.inputSource = AUDIO_MIC;
    info.bitWidth = BIT_WIDTH_16;
    AudioCodecFormat audioFormat = GetAudioFormat();
    if (audioFormat == FORMAT_BUTT) {
        return -1;
    }
    info.audioFormat = audioFormat;
    g_audioSourceProcessInput.audioFormat = audioFormat;
    GetDefaultSampleRateAndRateBaseFormat(audioFormat, info.sampleRate, info.bitRate);

    info.channelCount = GetChannelCount();
    if (info.channelCount == -1) {
        return -1;
    }

    std::cout << " SetCapturerInfo" << std::endl;
    if ((ret = audioCap.SetCapturerInfo(info)) != 0) {
        std::cout << "Can't SetCapturerInfo " << std::endl;
        delete g_audioSourceProcessInput.buffer;
        return -1;
    }
    frameCount = audioCap.GetFrameCount();
    std::cout << "GetFrameCount  " << frameCount << std::endl;
    g_audioSourceProcessInput.framesize = frameCount * 0x400;
    g_audioSourceProcessInput.buffer = new uint8_t[g_audioSourceProcessInput.framesize];

    RumCmd(audioCap);
EXIT:
    audioCap.Release();
    delete g_audioSourceProcessInput.buffer;
    g_audioSourceProcessInput.buffer = nullptr;
    return 0;
}
