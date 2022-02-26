/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "player_ability_slice.h"
#include <algorithm>
#include <vector>

#include "ability_env.h"
#include "ability_manager.h"
#include "components/ui_image_view.h"
#include "gfx_utils/file.h"

using OHOS::Media::Player;
using OHOS::Media::Source;
using namespace OHOS::Media;

namespace OHOS {
REGISTER_AS(PlayerAbilitySlice)


PlayerAbilitySlice::~PlayerAbilitySlice()
{
    printf("################ ~PlayerAbilitySlice enter\n");

    /** released in DestoryPlayer(). */

    printf("################ ~PlayerAbilitySlice exit\n");
}

std::shared_ptr<Player> PlayerAbilitySlice::CreatePlayer()
{
    static std::shared_ptr<Player> instance_ = nullptr;
    if (instance_ == nullptr) {
        instance_ = std::make_shared<Player>();
    }
    return instance_;
}

void PlayerAbilitySlice::Clear()
{
    printf("PlayerAbilitySlice::Clear | enter\n");
    if (backIconListener_ != nullptr) {
        delete backIconListener_;
        backIconListener_ = nullptr;
    }
    if (onClickListener_ != nullptr) {
        delete onClickListener_;
        onClickListener_ = nullptr;
    }
    if (surfaceView_ != nullptr) {
        delete surfaceView_;
        surfaceView_ = nullptr;
    }
    if (animatorGroup_ != nullptr) {
        delete animatorGroup_;
        animatorGroup_ = nullptr;
    }
    if (backIcon_ != nullptr) {
        delete backIcon_;
        backIcon_ = nullptr;
    }
    if (backArea_ != nullptr) {
        delete backArea_;
        backArea_ = nullptr;
    }
    if (errorTips_ != nullptr) {
        delete errorTips_;
        errorTips_ = nullptr;
    }
    if (totalTimeLabel_ != nullptr) {
        delete totalTimeLabel_;
        totalTimeLabel_ = nullptr;
    }
    if (currentTimeLabel_ != nullptr) {
        delete currentTimeLabel_;
        currentTimeLabel_ = nullptr;
    }
    if (slider_ != nullptr) {
        delete slider_;
        slider_ = nullptr;
    }
    if (toggleButton_ != nullptr) {
        delete toggleButton_;
        toggleButton_ = nullptr;
    }
    if (toggleButtonArea_ != nullptr) {
        delete toggleButtonArea_;
        toggleButtonArea_ = nullptr;
    }
    if (rootView_ != nullptr) {
        RootView::DestoryWindowRootView(rootView_);
        rootView_ = nullptr;
    }
    printf("PlayerAbilitySlice::Clear() | end\n");
}

void PlayerAbilitySlice::ShowErrorTips()
{
    errorTips_ = new UILabel();
    errorTips_->SetPosition(ROOT_VIEW_POSITION_X, ROOT_VIEW_POSITION_Y, ROOT_VIEW_WIDTH, ROOT_VIEW_HEIGHT);
    errorTips_->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER,
        UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    errorTips_->SetFont(FONT_NAME, GALLERY_FONT_SIZE);
    errorTips_->SetText("视频播放错误");

    rootView_->Add(backArea_);
    rootView_->Add(errorTips_);
    rootView_->Add(backIcon_);
    SetUIContent(rootView_);
}

void PlayerAbilitySlice::SetUpRootView()
{
    if (rootView_ != nullptr) {
        return;
    }
    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(ROOT_VIEW_POSITION_X, ROOT_VIEW_POSITION_Y);
    rootView_->Resize(ROOT_VIEW_WIDTH, ROOT_VIEW_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);
}

void PlayerAbilitySlice::SetUpBackArea(const char* pathHeader)
{
    auto onClick = [this] (UIView &view, const Event &event) -> bool {
        printf("############  PlayerAbilitySlice terminate AS enter   #############\n");
        Terminate();
        printf("############  PlayerAbilitySlice terminate AS exit   #############\n");
        return true;
    };
    backIcon_ = new UIImageView();
    backIcon_->SetPosition(BACK_ICON_POSITION_X, BACK_ICON_POSITION_Y);

    if (sprintf_s(backIconAbsolutePath, MAX_PATH_LENGTH, "%s%s", pathHeader, BACK_ICON_PATH) < 0) {
        printf("PlayerAbilitySlice::OnStart | backIconAbsolutePath | %s\n", pathHeader);
        return;
    }
    backIcon_->SetSrc(backIconAbsolutePath);
    backIcon_->SetTouchable(true);
    backIconListener_ = new EventListener(onClick, nullptr);
    backIcon_->SetOnClickListener(backIconListener_);

    backArea_ = new UIViewGroup();
    backArea_->SetPosition(0, 0, LABEL_POSITION_X, LABEL_HEIGHT);
    backArea_->SetStyle(STYLE_BACKGROUND_OPA, 0);
    backArea_->SetTouchable(true);
    backArea_->SetOnClickListener(backIconListener_);

    rootView_->Add(backArea_);
    rootView_->Add(backIcon_);
}

void PlayerAbilitySlice::SetUpVideoPlayer(const Want &want)
{
    if (videoPlayer_ == nullptr) {
        videoPlayer_ = new PlayerAdapter();
    }
    videoPlayer_->sourceType = 1;

    uint16_t videoPathLen = strlen(VIDEO_SOURCE_DIRECTORY) + strlen(reinterpret_cast<char*>(want.data)) + 1;
    int8_t ret = sprintf_s(videoPlayer_->filePath, videoPathLen + 1, "%s/%s", VIDEO_SOURCE_DIRECTORY,
        reinterpret_cast<char*>(want.data));
    if (ret < 0) {
        printf("PlayerAbilitySlice::OnStart | videoPlayer_->filePath | %s\n", reinterpret_cast<char*>(want.data));
        return;
    }
    ret = sprintf_s(&videoPlayer_->filePath[videoPathLen - strlen(AVAILABEL_SOURCE_TYPE)],
                    strlen(AVAILABEL_SOURCE_TYPE) + 1, "%s", AVAILABEL_SOURCE_TYPE);
    if (ret < 0) {
        printf("PlayerAbilitySlice::OnStart | videoPlayer_->filePath \n");
        return;
    }
    printf("------########### mp4 file path | %s\n", videoPlayer_->filePath);

    videoPlayer_->adapter = PlayerAbilitySlice::CreatePlayer();
    std::string uri(videoPlayer_->filePath);
    std::map<std::string, std::string> header;
    Source source(uri, header);
    videoPlayer_->adapter->SetSource(source);
}

bool PlayerAbilitySlice::SetUpSurfaceView()
{
    if (surfaceView_ != nullptr) {
        return true;
    }
    int32_t width = 0;
    int32_t height = 0;
    videoPlayer_->adapter->GetVideoWidth(width);
    printf("[%s,%d] width:%d\n", __func__, __LINE__, width);
    videoPlayer_->adapter->GetVideoHeight(height);
    printf("[%s,%d] height:%d\n", __func__, __LINE__, height);

    if (width <= 0 || height <= 0) {
        videoPlayer_->adapter->Release();
        delete videoPlayer_;
        videoPlayer_ = nullptr;
        printf("******** width <= 0 || height <= 0 | return \n");
        ShowErrorTips();
        return false;
    }
    float ratio_x = static_cast<float>(width) / ROOT_VIEW_WIDTH;
    float ratio_y = static_cast<float>(height) / ROOT_VIEW_HEIGHT;
    uint16_t surfaceViewWidth;
    uint16_t surfaceViewHeight;
    uint16_t surfaceViewPositionX = 0;
    uint16_t surfaceViewPositionY = 0;
    if (ratio_x > ratio_y) {
        surfaceViewWidth = ROOT_VIEW_WIDTH;
        surfaceViewHeight = height / ratio_x;
        surfaceViewPositionY = (ROOT_VIEW_HEIGHT - surfaceViewHeight) / 2; // 2: half
    } else {
        surfaceViewWidth = width / ratio_y;
        surfaceViewHeight = ROOT_VIEW_HEIGHT;
        surfaceViewPositionX = (ROOT_VIEW_WIDTH - surfaceViewWidth) / 2; // 2: half
    }

    surfaceView_ = new UISurfaceView();
    surfaceView_->SetPosition(surfaceViewPositionX, surfaceViewPositionY);
    surfaceView_->SetWidth(surfaceViewWidth - 1);
    surfaceView_->SetHeight(surfaceViewHeight);
    videoPlayer_->adapter->SetVideoSurface(surfaceView_->GetSurface());

    rootView_->Add(surfaceView_);

    return true;
}

void PlayerAbilitySlice::SetUpProgress(int64_t duration)
{
    slider_ = new UISlider();
    slider_->SetPosition(SLIDER_X, SLIDER_Y, SLIDER_WIDTH, STATUS_BAR_GROUP_HEIGHT);
    slider_->SetValidHeight(SLIDER_HEIGHT);
    slider_->SetValidWidth(SLIDER_WIDTH - KNOB_WIDTH);
    slider_->SetRange(SLIDER_WIDTH, 0);
    slider_->SetValue(0);
    slider_->SetKnobWidth(KNOB_WIDTH);
    slider_->SetSliderRadius(SLIDER_HEIGHT, SLIDER_HEIGHT);
    slider_->SetKnobRadius(KNOB_WIDTH / 2); // 2: half
    slider_->SetKnobStyle(STYLE_BACKGROUND_COLOR, Color::White().full);
    slider_->SetBackgroundStyle(STYLE_BACKGROUND_COLOR, 0x1A888888);
    slider_->SetBackgroundStyle(STYLE_BACKGROUND_OPA, 90); // 90: opacity is 90
    slider_->SetDirection(UISlider::Direction::DIR_LEFT_TO_RIGHT);
    slider_->SetTouchable(false);
    animatorGroup_->Add(slider_);

    animator_ = new SliderAnimator(videoPlayer_, slider_, currentTimeLabel_, duration, surfaceView_);
}

void PlayerAbilitySlice::SetUpAnimatorGroup(const char* pathHeader)
{
    int64_t duration = 0;
    videoPlayer_->adapter->GetDuration(duration);
    printf("[%s,%d] GetDuration:%lld\n", __func__, __LINE__, duration);

    animatorGroup_ = new UIViewGroup();
    animatorGroup_->SetPosition(0, ROOT_VIEW_HEIGHT - STATUS_BAR_GROUP_HEIGHT,
                                ROOT_VIEW_WIDTH, STATUS_BAR_GROUP_HEIGHT);
    animatorGroup_->SetStyle(STYLE_BACKGROUND_OPA, 0);

    totalTimeLabel_ = new UILabel();
    totalTimeLabel_->SetPosition(TOTAL_TIME_LABEL_X, TOTAL_TIME_LABEL_Y,
        TOTAL_TIME_LABEL_WIDTH, TOTAL_TIME_LABEL_HEIGHT);
    totalTimeLabel_->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT,
        UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    totalTimeLabel_->SetFont(FONT_NAME, PLAYER_FONT_SIZE);
    int64_t second = duration / 1000; // 1000: 1s = 1000ms
    char timer[6]; // 6: length of time label
    if (sprintf_s(timer, sizeof(timer), "%02lld:%02lld", second / 60, second % 60) < 0) { // 60: 1minute = 60s
        return;
    }

    totalTimeLabel_->SetText(timer);
    totalTimeLabel_->SetTextColor(Color::White());
    animatorGroup_->Add(totalTimeLabel_);

    currentTimeLabel_ = new UILabel();
    currentTimeLabel_->SetPosition(CURRENT_TIME_LABEL_X, CURRENT_TIME_LABEL_Y,
        CURRENT_TIME_LABEL_WIDTH, CURRENT_TIME_LABEL_HEIGHT);
    currentTimeLabel_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Red().full);
    currentTimeLabel_->SetFont(FONT_NAME, PLAYER_FONT_SIZE);
    currentTimeLabel_->SetText("00:00");
    currentTimeLabel_->SetTextColor(Color::White());
    currentTimeLabel_->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT,
        UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    animatorGroup_->Add(currentTimeLabel_);

    SetUpProgress(duration);

    SetUpToggleButton(pathHeader);

    rootView_->Add(animatorGroup_);
}

void PlayerAbilitySlice::SetUpToggleButton(const char* pathHeader)
{
    toggleButton_ = new UIToggleButton();
    toggleButton_->SetTouchable(false);
    toggleButton_->SetPosition(TOGGLE_BUTTON_OFFSET_X, TOGGLE_BUTTON_OFFSET_Y,
        TOGGLE_BUTTON_WIDTH, TOGGLE_BUTTON_HEIGHT);
    toggleButton_->SetState(true);

    if (sprintf_s(videoPlayAbsolutePath, MAX_PATH_LENGTH, "%s%s", pathHeader, VIDEO_PALY_PATH) < 0) {
        printf("PlayerAbilitySlice::OnStart | videoPlayAbsolutePath\n");
        return;
    }

    if (sprintf_s(videoPauseAbsolutePath, MAX_PATH_LENGTH, "%s%s", pathHeader, VIDEO_PAUSE_PATH) < 0) {
        printf("PlayerAbilitySlice::OnStart | videoPauseAbsolutePath\n");
        return;
    }
    toggleButton_->SetImages(videoPauseAbsolutePath, videoPlayAbsolutePath);
    onClickListener_ = new ToggleBtnListener(toggleButton_, videoPlayer_, animator_, surfaceView_);

    toggleButtonArea_ = new UIViewGroup();
    toggleButtonArea_->SetPosition(0, 0, TOGGLE_BUTTON_OFFSET_X + TOGGLE_BUTTON_WIDTH, STATUS_BAR_GROUP_HEIGHT);
    toggleButtonArea_->SetTouchable(true);
    toggleButtonArea_->SetOnClickListener(onClickListener_);
    toggleButtonArea_->Add(toggleButton_);

    animatorGroup_->Add(toggleButtonArea_);
}

void PlayerAbilitySlice::OnStart(const Want &want)
{
    printf("@@@@@ PlayerAbilitySlice::OnStart\n");
    AbilitySlice::OnStart(want);

    SetUpRootView();
    const char* pathHeader = GetSrcPath();
    SetUpVideoPlayer(want);

    videoPlayer_->adapter->Prepare();

    if (!SetUpSurfaceView()) {
        return;
    }
    SetUpBackArea(pathHeader);
    SetUpAnimatorGroup(pathHeader);

    SetUIContent(rootView_);

    videoPlayer_->adapter->Play();
    animator_->SetToggleButton(toggleButton_);
    animator_->SetToggleBtnListener(onClickListener_);
    animator_->Start();

    printf("## @@@@@ PlayerAbilitySlice::OnStart | end \n");
}

void PlayerAbilitySlice::OnInactive()
{
    printf("PlayerAbilitySlice::OnInactive\n");
    AbilitySlice::OnInactive();
}

void PlayerAbilitySlice::OnActive(const Want &want)
{
    printf("PlayerAbilitySlice::OnActive\n");
    AbilitySlice::OnActive(want);
}

void PlayerAbilitySlice::OnBackground()
{
    printf("PlayerAbilitySlice::OnBackground\n");
    AbilitySlice::OnBackground();
}

void PlayerAbilitySlice::OnStop()
{
    if (animator_ != nullptr) {
        animator_->Stop();
        delete animator_;
        animator_ = nullptr;
    }

    if (videoPlayer_ != nullptr && videoPlayer_->adapter.get() != nullptr) {
        videoPlayer_->adapter->Stop();
        videoPlayer_->adapter->Reset();
        delete videoPlayer_;
        videoPlayer_ = nullptr;
    }
    Clear();
    printf("PlayerAbilitySlice::OnStop\n");
    AbilitySlice::OnStop();
}

void SliderAnimator::Callback(UIView* view)
{
    if (needRefreshPlayer_) {
        videoPlayer_->adapter->Stop();
        videoPlayer_->adapter->Reset();

        videoPlayer_->adapter = PlayerAbilitySlice::CreatePlayer();
        std::string uri(videoPlayer_->filePath);
        std::map<std::string, std::string> header;
        Source source(uri, header);
        videoPlayer_->adapter->SetSource(source);
        videoPlayer_->adapter->Prepare();
        videoPlayer_->adapter->SetVideoSurface(surfaceView_->GetSurface());
        videoPlayer_->adapter->Play();
        needRefreshPlayer_ = false;
    }

    int64_t currentTime = 0;
    videoPlayer_->adapter->GetCurrentTime(currentTime);
    int64_t currentSecond = currentTime / 1000; // 1000: 1s = 1000ms

    char time[6]; // 6: length of time label
    sprintf_s(time, sizeof(time), "%02lld:%02lld", currentSecond / 60, currentSecond % 60); // 60: 1minute = 60s
    timeLabel_->SetText(time);
    timeLabel_->Invalidate();

    int64_t curPosition = currentTime * slider_->GetRangeMax() / duration_;
    slider_->SetValue(curPosition);
    slider_->Invalidate();

    if (currentTime >= duration_) {
        listener_->SetCompleteFlag(true);
        toggleButton_->SetState(false);
        needRefreshPlayer_ = true;
        Stop();
    }
}

bool ToggleBtnListener::OnClick(UIView &view, const ClickEvent& event)
{
    button_->OnClickEvent(event);
    if (completeFlag_) {
        animator_->Start();
        button_->Invalidate();
        completeFlag_ = false;
        return true;
    }

    if (button_->GetState()) {
        videoPlayer_->adapter->Play();
        animator_->Resume();
        printf("ToggleBtnListener::OnClick | play\n");
    } else {
        videoPlayer_->adapter->Pause();
        animator_->Pause();
        printf("ToggleBtnListener::OnClick | pause\n");
    }
    button_->Invalidate();
    return true;
}
}
