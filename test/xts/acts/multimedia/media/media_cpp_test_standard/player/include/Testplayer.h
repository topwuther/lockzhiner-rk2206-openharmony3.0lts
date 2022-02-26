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

#ifndef PLAYER_TEST_H
#define PLAYER_TEST_H

#include "securec.h"
#include "TestParamsConfig.h"
#include "window_manager.h"
#include "surface_type.h"
#include "display_type.h"

namespace OHOS {
namespace Media {
static const std::string SURFACE_STRIDE_ALIGNMENT = "SURFACE_STRIDE_ALIGNMENT";
static const std::string SURFACE_FORMAT = "SURFACE_FORMAT";
static WindowConfig g_config = {
    1920,
    1080,
    PIXEL_FMT_RGBA_8888,
    0,
    0,
    WINDOW_TYPE_NORMAL
};
static WindowConfig g_sub_config = {
    720,
    480,
    PIXEL_FMT_YCRCB_420_SP,
    0,
    0,
    WINDOW_TYPE_VIDEO,
};

class PlayerSignal {
public:
    PlayerStates state_ = PLAYER_IDLE;
    int32_t seekPositon_;
    bool seekDoneFlag_;
    bool mutexFlag_ = true;
    std::mutex mutexSeek_;
    std::mutex mutexReset_;
    std::mutex mutexPrepare_;
    std::mutex mutexPlay_;
    std::mutex mutexPause_;
    std::mutex mutexStop_;
    std::condition_variable condVarSeek_;
    std::condition_variable condVarReset_;
    std::condition_variable condVarPrepare_;
    std::condition_variable condVarPlay_;
    std::condition_variable condVarPause_;
    std::condition_variable condVarStop_;

    void SetState(PlayerStates state);
    void SetSeekResult(bool seekDoneFlag);
};
class TestPlayer {
public:
    std::shared_ptr<Player> player_ = nullptr;
    sptr<Window> window_ = nullptr;
    explicit TestPlayer(std::shared_ptr<PlayerSignal> test);
    virtual ~TestPlayer();
    bool CreatePlayer();
    int32_t SetSource(const std::string &uri);
    int32_t SetDataSrc(const std::string &uri, bool seekable);
    int32_t Play();
    int32_t Prepare();
    int32_t PrepareAsync();
    int32_t Pause();
    int32_t Stop();
    int32_t Reset();
    int32_t Release();
    int32_t SetVolume(float leftVolume, float rightVolume);
    int32_t Seek(int32_t mseconds, PlayerSeekMode mode);
    int32_t GetCurrentTime(int32_t &currentTime);
    int32_t GetDuration(int32_t &duration);
    int32_t SetPlaybackSpeed(PlaybackRateMode mode);
    int32_t GetPlaybackSpeed(PlaybackRateMode &mode);
    sptr<Surface> GetVideoSurface(WindowConfig g_sub_config);
    int32_t SetVideoSurface(sptr<Surface> surface);
    bool IsPlaying();
    bool IsLooping();
    int32_t SetLooping(bool loop);
    int32_t SetPlayerCallback(const std::shared_ptr<PlayerCallback> &callback);
private:
    std::shared_ptr<PlayerSignal> test_;
};
class TestPlayerCallback : public PlayerCallback {
public:
    int errorNum_ = 0;
    PlayerStates state_ = PLAYER_STATE_ERROR;
    explicit TestPlayerCallback(std::shared_ptr<PlayerSignal> test);
    virtual ~TestPlayerCallback();
    void OnError(PlayerErrorType errorType, int32_t errorCode);
    int WaitForSeekDone(int32_t currentPositon);
    void OnInfo(PlayerOnInfoType type, int32_t extra, const Format &infoBody = {});
    int WaitForState(PlayerStates state);
private:
    void PrintState(PlayerStates state);

    PlayerErrorType errorType_ = PLAYER_ERROR_UNKNOWN;
    int32_t errorCode_ = -1;
    bool seekDoneFlag_ = false;
    int32_t postion_ = 0;
    std::shared_ptr<PlayerSignal> test_;
};
}
}
#endif