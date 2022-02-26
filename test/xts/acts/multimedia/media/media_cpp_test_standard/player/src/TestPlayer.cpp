
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

#include "TestPlayer.h"
#include "parameter.h"
#include "media_errors.h"

using namespace OHOS;
using namespace OHOS::Media;

TestPlayer::TestPlayer(std::shared_ptr<PlayerSignal> test)
    : test_(test)
{
}

TestPlayer::~TestPlayer()
{
}

bool TestPlayer::CreatePlayer()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    player_ = PlayerFactory::CreatePlayer();
    if (player_ == nullptr) {
        return false;
    }
    return true;
}

int32_t TestPlayer::SetSource(const std::string &uri)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->SetSource(uri);
}

int32_t TestPlayer::Play()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    int32_t ret = player_->Play();
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->state_ != PLAYER_STARTED) {
        std::unique_lock<std::mutex> lockPlay(test_->mutexPlay_);
        test_->condVarPlay_.wait_for(lockPlay, std::chrono::seconds(WAITSECOND));
        if (test_->state_ != PLAYER_STARTED) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::Prepare()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    int32_t ret = player_->Prepare();
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->state_ != PLAYER_PREPARED) {
        std::unique_lock<std::mutex> lockPrepare(test_->mutexPrepare_);
        test_->condVarPrepare_.wait_for(lockPrepare, std::chrono::seconds(WAITSECOND));
        if (test_->state_ != PLAYER_PREPARED) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::PrepareAsync()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    int32_t ret = player_->PrepareAsync();
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->state_ != PLAYER_PREPARED) {
        std::unique_lock<std::mutex> lockPrepare(test_->mutexPrepare_);
        test_->condVarPrepare_.wait_for(lockPrepare, std::chrono::seconds(WAITSECOND));
        if (test_->state_ != PLAYER_PREPARED) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::Pause()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    int32_t ret = player_->Pause();
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->state_ != PLAYER_PAUSED) {
        std::unique_lock<std::mutex> lockPause(test_->mutexPause_);
        test_->condVarPause_.wait_for(lockPause, std::chrono::seconds(WAITSECOND));
        if (test_->state_ != PLAYER_PAUSED) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::Stop()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    int32_t ret = player_->Stop();
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->state_ != PLAYER_STOPPED) {
        std::unique_lock<std::mutex> lockStop(test_->mutexStop_);
        test_->condVarStop_.wait_for(lockStop, std::chrono::seconds(WAITSECOND));
        if (test_->state_ != PLAYER_STOPPED) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::Reset()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    int32_t ret = player_->Reset();
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->state_ != PLAYER_IDLE) {
        std::unique_lock<std::mutex> lockReset(test_->mutexReset_);
        test_->condVarReset_.wait_for(lockReset, std::chrono::seconds(WAITSECOND));
        if (test_->state_ != PLAYER_IDLE) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::Release()
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->Release();
}

int32_t TestPlayer::SetVolume(float leftVolume, float rightVolume)
{
    MEDIA_INFO_LOG("%s", __FUNCTION__);
    return player_->SetVolume(leftVolume, rightVolume);
}

int32_t TestPlayer::Seek(int32_t mseconds, PlayerSeekMode mode)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    test_->seekDoneFlag_ = false;
    test_->seekPositon_ = mseconds;
    int32_t ret = player_->Seek(mseconds, mode);
    if (ret == RET_OK && test_->mutexFlag_ == true && test_->seekDoneFlag_ == false) {
        std::unique_lock<std::mutex> lockSeek(test_->mutexSeek_);
        test_->condVarSeek_.wait_for(lockSeek, std::chrono::seconds(WAITSECOND));
        if (test_->seekDoneFlag_ != true) {
            return -1;
        }
    }
    return ret;
}

int32_t TestPlayer::GetCurrentTime(int32_t &currentTime)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->GetCurrentTime(currentTime);
}

int32_t TestPlayer::GetDuration(int32_t &duration)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->GetDuration(duration);
}

int32_t TestPlayer::SetPlaybackSpeed(PlaybackRateMode mode)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->SetPlaybackSpeed(mode);
}

int32_t TestPlayer::GetPlaybackSpeed(PlaybackRateMode &mode)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->GetPlaybackSpeed(mode);
}

sptr<Surface> TestPlayer::GetVideoSurface(WindowConfig sub_config)
{
    char surface[PARA_MAX_LEN] = "window";
    GetParameter("sys.media.test.surface", "window", &surface[0], PARA_MAX_LEN);
    sptr<Surface> videoSurface = nullptr;
    if (strcmp(surface, "null") == 0) {
        return videoSurface;
    }
    if (strcmp(surface, "window") == 0) {
        sptr<WindowManager> wmi = WindowManager::GetInstance();
        if (wmi == nullptr) {
            return nullptr;
        }
        (void)wmi->Init();
        sptr<WindowOption> option = WindowOption::Get();
        if (option == nullptr) {
            return nullptr;
        }
        (void)option->SetWindowType(WINDOW_TYPE_NORMAL);
        (void)wmi->CreateWindow(window_, option);
        if (window_ == nullptr) {
            return nullptr;
        }
        videoSurface = window_->GetSurface();
        if (videoSurface == nullptr) {
            return nullptr;
        }
        (void)videoSurface->SetUserData(SURFACE_FORMAT, std::to_string(PIXEL_FMT_RGBA_8888));
    }
    return videoSurface;
}

int32_t TestPlayer::SetVideoSurface(const sptr<Surface> surface)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    char parameter[PARA_MAX_LEN] = "window";
    GetParameter("sys.media.test.surface", "window", &parameter[0], PARA_MAX_LEN);
    if (strcmp(parameter, "null") == 0) {
        MEDIA_INFO_LOG("sys.media.test.surface null");
        return 0;
    }
    return player_->SetVideoSurface(surface);
}

bool TestPlayer::IsPlaying()
{
    return player_->IsPlaying();
}

bool TestPlayer::IsLooping()
{
    return player_->IsLooping();
}

int32_t TestPlayer::SetLooping(bool loop)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->SetLooping(loop);
}

int32_t TestPlayer::SetPlayerCallback(const std::shared_ptr<PlayerCallback> &callback)
{
    MEDIA_DEBUG_LOG("%s", __FUNCTION__);
    return player_->SetPlayerCallback(callback);
}

TestPlayerCallback::TestPlayerCallback(std::shared_ptr<PlayerSignal> test)
    : test_(test)
{
}

TestPlayerCallback::~TestPlayerCallback()
{
}

void TestPlayerCallback::OnError(PlayerErrorType errorType, int32_t errorCode)
{
    errorNum_++;
    errorType_ = errorType;
    errorCode_ = errorCode;
    std::string errorTypeMsg = PlayerErrorTypeToString(errorType);
    std::string errorCodeMsg = MSErrorToString(static_cast<MediaServiceErrCode>(errorCode));
    MEDIA_ERROR_LOG("TestPlayerCallback: OnError errorType is %s, errorCode is %s",
        errorTypeMsg.c_str(), errorCodeMsg.c_str());
}

void TestPlayerCallback::OnInfo(PlayerOnInfoType type, int32_t extra, const Format &infoBody)
{
    switch (type) {
        case INFO_TYPE_SEEKDONE:
            seekDoneFlag_ = true;
            test_->SetSeekResult(true);
            MEDIA_INFO_LOG("TestPlayerCallback: OnSeekDone currentPositon is %d", extra);
            if (abs(test_->seekPositon_ - extra) <= DELTA_TIME) {
                test_->condVarSeek_.notify_all();
            } else {
                test_->SetSeekResult(false);
            }
            break;
        case INFO_TYPE_EOS:
            MEDIA_INFO_LOG("TestPlayerCallback: OnEndOfStream isLooping is %d", extra);
            break;
        case INFO_TYPE_STATE_CHANGE:
            state_ = static_cast<PlayerStates>(extra);
            PrintState(state_);
            break;
        case INFO_TYPE_POSITION_UPDATE:
            postion_ = extra;
            break;
        case INFO_TYPE_MESSAGE:
            MEDIA_INFO_LOG("TestPlayerCallback: OnMessage type is %d", extra);
            break;
        default:
            break;
    }
}

int TestPlayerCallback::WaitForSeekDone(int32_t currentPositon)
{
    int64_t waitTime = 0;
    seekDoneFlag_ = false;
    while (seekDoneFlag_ != true && waitTime < WAITSECOND * TIME_SEC2MS) {
        (void)usleep(WAIT_TIME);
        waitTime += 1;
    }
    seekDoneFlag_ = false;
    if (waitTime >= WAITSECOND * TIME_SEC2MS) {
        MEDIA_ERROR_LOG("Failed to seek [%d]s ", currentPositon);
        return -1;
    }
    return 0;
}

int TestPlayerCallback::WaitForState(PlayerStates state)
{
    int64_t waitTime = 0;
    while (state_ != state && waitTime < WAITSECOND * TIME_SEC2MS) {
        (void)usleep(WAIT_TIME);
        waitTime += 1;
    }
    if (waitTime >= WAITSECOND * TIME_SEC2MS) {
        MEDIA_ERROR_LOG("Failed to wait for state[%d] down", state);
        return -1;
    }
    return 0;
}

void TestPlayerCallback::PrintState(PlayerStates state)
{
    switch (state) {
        case PLAYER_STATE_ERROR:
            MEDIA_INFO_LOG("State: Error");
            break;
        case PLAYER_IDLE:
            MEDIA_INFO_LOG("State: IDLE");
            test_->SetState(state);
            test_->condVarReset_.notify_all();
            break;
        case PLAYER_INITIALIZED:
            MEDIA_INFO_LOG("State: Initialized");
            break;
        case PLAYER_PREPARED:
            MEDIA_INFO_LOG("State: Prepared");
            test_->SetState(state);
            test_->condVarPrepare_.notify_all();
            break;
        case PLAYER_STARTED:
            MEDIA_INFO_LOG("State: Started");
            test_->SetState(state);
            test_->condVarPlay_.notify_all();
            break;
        case PLAYER_PAUSED:
            MEDIA_INFO_LOG("State: Paused");
            test_->SetState(state);
            test_->condVarPause_.notify_all();
            break;
        case PLAYER_STOPPED:
            MEDIA_INFO_LOG("State: Stopped");
            test_->SetState(state);
            test_->condVarStop_.notify_all();
            break;
        case PLAYER_PLAYBACK_COMPLETE:
            MEDIA_INFO_LOG("State: Complete");
            break;
        default:
            MEDIA_INFO_LOG("Invalid state");
            break;
    }
}

void PlayerSignal::SetState(PlayerStates state)
{
    state_ = state;
}

void PlayerSignal::SetSeekResult(bool seekDoneFlag)
{
    seekDoneFlag_ = seekDoneFlag;
}
