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

#include "camera_ability_slice.h"
#include <algorithm>
#include <meta_data.h>
#include <window/window.h>

#include "ability_manager.h"
#include "gfx_utils/color.h"
#include "ui_config.h"
#include "securec.h"

namespace OHOS {
REGISTER_AS(CameraAbilitySlice)

static constexpr const char *VIEWIMAGE = "/userdata/photo/tmp.jpg";
// button idx
static uint8_t g_curButtonIdx = 1;
// video info
static int g_isRecording = 0;    // 0--stop recording    1--start/resume recording  2--pause recording

class SliderAnimator : public Animator, public AnimatorCallback {
public:
    explicit SliderAnimator(UISlider *slider, UIImageView *backview, UISurfaceView *surface,
    SampleCameraManager *cManager, uint16_t duration)
        : Animator(this, slider, duration, true), backgroundView_(backview), mSurfaceview(surface),
        camManager(cManager), ss(0), runType_(0), camRestart(false) {}
    virtual ~SliderAnimator() {}

    void Callback(UIView *view) override
    {
        if (runType_ == 1) {    /* 1 is normal photo */
            if (camManager->SampleCameraCaptrueIsFinish()) {
                if (ss == 0) {    /* 0 first record times */
                    BackViewSetImage(VIEWIMAGE);
                    backgroundView_->SetVisible(true);
                    backgroundView_->Invalidate();
                    ss = GetRunTime();
                } else {
                    if ((GetRunTime()-ss) > 1000) {    /* 1000 = 1s */
                        backgroundView_->SetVisible(false);
                        backgroundView_->Invalidate();
                        Stop();
                    }
                }
            }
        } else if (runType_ == 2) {    /* 2 is record mode */
            int ms = GetRunTime();
            if (ss == 0) {
                ss = GetRunTime();
                return;
            }
            if ((ms - ss) > 1000 && camRestart == false) {    /* 1000 = 1s */
                camManager->SampleCameraStopRecord();
                printf("after stop record!! \n");
                printf("before SampleCameraStart!! \n");
                camManager->SampleCameraStart(mSurfaceview->GetSurface());
                printf("after SampleCameraStart!! \n");
                camRestart = true;
            } else if ((ms - ss) > 2000) {    /* 2000 = 2s */
                backgroundView_->SetVisible(false);
                backgroundView_->Invalidate();
                Stop();
            }
        }
    }

    void OnStop(UIView &view) override
    {
        runType_ = 0;
        ss = 0;
        camRestart = false;
    }

    void SetStart(int type)
    {
        runType_ = type;
        camRestart = false;
    }
private:
    UIImageView *backgroundView_;
    UISurfaceView *mSurfaceview;
    SampleCameraManager *camManager;
    uint32_t ss;
    uint32_t runType_;
    bool camRestart;

    void BackViewSetImage(const char *image)
    {
        backgroundView_->SetSrc(image);
        int16_t imageWidth = backgroundView_->GetWidth();
        int16_t imageHeight = backgroundView_->GetHeight();
        if (imageWidth > SCREEN_WIDTH || imageHeight > SCREEN_HEIGHT) {
            TransformMap transMap(backgroundView_->GetOrigRect());
            float scaleWidth = 1.0;
            float scaleHeight = 1.0;
            if (imageWidth > SCREEN_WIDTH)
                scaleWidth = static_cast<float>(SCREEN_WIDTH) / imageWidth;
            if (imageHeight > SCREEN_HEIGHT)
                scaleHeight = static_cast<float>(SCREEN_HEIGHT) / imageHeight;
            float scale = (scaleWidth < scaleHeight) ? scaleWidth : scaleHeight;

            transMap.Scale(Vector2<float>(scale, scale), Vector2<float>(0, 0));
            backgroundView_->SetTransformMap(transMap);
            backgroundView_->SetTransformAlgorithm(TransformAlgorithm::NEAREST_NEIGHBOR);
            imageWidth = imageWidth * scale;
            imageHeight = imageHeight * scale;
        }
        int16_t imagePosX = (SCREEN_WIDTH - imageWidth) / 2;    /* 2 half */
        int16_t imagePosY = (SCREEN_HEIGHT - imageHeight) / 2;    /* 2 half */
        backgroundView_->SetPosition(imagePosX, imagePosY);
    }
};

class CameraImageButtonOnClickListener : public UIView::OnClickListener {
public:
    CameraImageButtonOnClickListener(UIView *uiView,  UISurfaceView *surface,
        UIImageView *iamgeview, TaskView *taskView, SliderAnimator *animator) : uiView_(uiView),
        backgroundView_(iamgeview), mSurfaceview(surface), animator_(animator), gTaskView_(taskView)
        {
            cManager_ = nullptr;
            bttnLeft = nullptr;
            bttnRight = nullptr;
            bttnMidle = nullptr;
            bttnRecord = nullptr;
            recordImage = nullptr;
            tmLabel = nullptr;
            bttnIdx_ = 0;
        }
    virtual ~CameraImageButtonOnClickListener(){}

    bool OnClick(UIView &view, const ClickEvent &event) override
    {
        if (cManager_ == nullptr || tmLabel == nullptr || bttnLeft == nullptr || bttnMidle == nullptr ||
            bttnRight == nullptr || bttnRecord == nullptr || recordImage == nullptr) {
            printf("nullptr point!! \n");
            return false;
        }
        switch (bttnIdx_) {
            case 0: {     /* 0 thumb */
                StartGallery();
                return true;
            }
                break;
            case 1:    /* 1 photo */
                StartTakePhoto();
                break;
            case 2:    /* 2 record */
                RecorderButtonDeal();
                break;
            case 3:    /* 3 pause/resume */
                if (g_isRecording == 1) {    /* 1 start */
                    g_isRecording = 2;    /* 2 pause */
                    gTaskView_->SetPause();
                    bttnRecord->SetSrc(UI_IMAGE_PATH"ic_camera_record_continue.png");
                } else {
                    bttnRecord->SetSrc(UI_IMAGE_PATH"ic_camera_record_pause.png");
                    g_isRecording = 1;    /* 1 start */
                    gTaskView_->SetResume();
                }
                bttnRecord->Invalidate();
                return true;
            default:
                return true;
        }

        if (g_curButtonIdx != 2) {    /* 2 is record button */
            recordImage->SetVisible(false);
            recordImage->Invalidate();

            tmLabel->SetVisible(false);
            tmLabel->Invalidate();

            bttnRecord->SetVisible(false);
            bttnRecord->Invalidate();
        } else {
            tmLabel->SetVisible(true);
            tmLabel->Invalidate();
        }

        uiView_->Invalidate();

        return true;
    }

    // idx  0--left, 1--midle, 2--right, 3--record, 4--rview
    void SetImageView(UIImageView *imgButton, int idx)
    {
        if (idx == 0) bttnLeft = imgButton;    /* 0 thumb */
        if (idx == 1) bttnMidle = imgButton;    /* 1 photo */
        if (idx == 2) bttnRight = imgButton;    /* 2 record */
        if (idx == 3) bttnRecord = imgButton;    /* 3 pause */
        if (idx == 4) recordImage = imgButton;    /* 4 icon */
    }

    void SetButtonIdxCamera(SampleCameraManager *cManager, int idx)
    {
        cManager_ = cManager;
        bttnIdx_ = idx;
    }

    void SetLabel(UILabel *l)
    {
        tmLabel = l;
    }

private:
    UIView *uiView_;
    UIImageView *backgroundView_;
    UISurfaceView *mSurfaceview;
    SliderAnimator *animator_;
    TaskView *gTaskView_;
    int16_t bttnIdx_;

    SampleCameraManager *cManager_;
    UIImageView *bttnLeft;
    UIImageView *bttnRight;
    UIImageView *bttnMidle;
    UIImageView *bttnRecord;
    UIImageView *recordImage;
    UILabel *tmLabel;
    void StartGallery(void)
    {
        Want want1 = { nullptr };
        ElementName element = { nullptr };
        SetElementBundleName(&element, "com.huawei.gallery");
        SetElementAbilityName(&element, "GalleryAbility");
        SetWantElement(&want1, element);
        SetWantData(&want1, "WantData", strlen("WantData") + 1);
        StartAbility(&want1);
    }

    void StartTakePhoto(void)
    {
        if (g_curButtonIdx != 1) {    /* 1 photo */
            if (g_isRecording) {
                return;
            }
            g_curButtonIdx = 1;
            bttnLeft->SetPosition(LEFT_BUTTON_X, LEFT_BUTTON_Y, LEFT_BUTTON_W, LEFT_BUTTON_H);
            bttnMidle->SetPosition(MID_BUTTON_X, MID_BUTTON_Y, MID_BUTTON_W, MID_BUTTON_H);
            bttnRight->SetPosition(RIGHT_BUTTON_X, RIGHT_BUTTON_Y, RIGHT_BUTTON_W, RIGHT_BUTTON_H);
            bttnMidle->SetSrc(UI_IMAGE_PATH"ic_camera_shutter.png");
            bttnRight->SetSrc(UI_IMAGE_PATH"ic_camera_video.png");
        } else {
            cManager_->SampleCameraCaptrue(0);
            animator_->SetStart(1);
            animator_->Start();
        }
    }

    void RecorderButtonDeal(void)
    {
        if (g_curButtonIdx != 2) {    /* 2 record */
            g_curButtonIdx = 2;    /* 2 record */
            bttnRight->SetPosition(MID_BUTTON_X, MID_BUTTON_Y, MID_BUTTON_W, MID_BUTTON_H);
            bttnMidle->SetPosition(RIGHT_BUTTON_X, RIGHT_BUTTON_Y, RIGHT_BUTTON_W, RIGHT_BUTTON_H);

            bttnMidle->SetSrc(UI_IMAGE_PATH"ic_camera_record_camra.png");
            bttnRight->SetSrc(UI_IMAGE_PATH"ic_camera_record.png");
        } else {
            if (g_isRecording) {
                g_isRecording = 0;    /* 0 stop */
                animator_->Stop();

                cManager_->SampleCameraStopRecord();
                printf("after stop record!! \n");
                printf("before SampleCameraStart!! \n");
                cManager_->SampleCameraStart(mSurfaceview->GetSurface());
                printf("after SampleCameraStart!! \n");

                gTaskView_->SetStop();
                recordImage->SetVisible(false);
                recordImage->Invalidate();

                bttnRecord->SetVisible(false);
                bttnRecord->Invalidate();
                bttnMidle->SetVisible(true);
                bttnMidle->Invalidate();
                bttnLeft->SetVisible(true);
                bttnLeft->Invalidate();

                bttnRight->SetSrc(UI_IMAGE_PATH"ic_camera_record.png");
            } else {
                g_isRecording = 1;    /* 1 start */
                cManager_->SampleCameraCaptrue(1);    /* 1 start */
                cManager_->SampleCameraStartRecord(mSurfaceview->GetSurface());

                recordImage->SetVisible(true);
                recordImage->Invalidate();
                bttnMidle->SetVisible(false);
                bttnMidle->Invalidate();
                gTaskView_->SetStart();
                bttnLeft->SetVisible(false);
                bttnLeft->Invalidate();

                bttnRight->SetSrc(UI_IMAGE_PATH"ic_camera_record_stop.png");
            }
            bttnRight->Invalidate();
        }
    }

    void BackViewSetImage(const char *image)
    {
        backgroundView_->SetSrc(image);
        int16_t imageWidth = backgroundView_->GetWidth();
        int16_t imageHeight = backgroundView_->GetHeight();
        if (imageWidth > SCREEN_WIDTH || imageHeight > SCREEN_HEIGHT) {
            TransformMap transMap(backgroundView_->GetOrigRect());
            float scaleWidth = 1.0;
            float scaleHeight = 1.0;
            if (imageWidth > SCREEN_WIDTH)
                scaleWidth = static_cast<float>(SCREEN_WIDTH) / imageWidth;
            if (imageHeight > SCREEN_HEIGHT)
                scaleHeight = static_cast<float>(SCREEN_HEIGHT) / imageHeight;
            float scale = (scaleWidth < scaleHeight) ? scaleWidth : scaleHeight;

            transMap.Scale(Vector2<float>(scale, scale), Vector2<float>(0, 0));
            backgroundView_->SetTransformMap(transMap);
            backgroundView_->SetTransformAlgorithm(TransformAlgorithm::NEAREST_NEIGHBOR);
            imageWidth = imageWidth * scale;
            imageHeight = imageHeight * scale;
        }
        int16_t imagePosX = (SCREEN_WIDTH - imageWidth) / 2;    /* 2 half */
        int16_t imagePosY = (SCREEN_HEIGHT - imageHeight) / 2;    /* 2 half */
        backgroundView_->SetPosition(imagePosX, imagePosY);
    }
};

CameraAbilitySlice::~CameraAbilitySlice()
{
    gTaskView_->SetPause();
    delete gTaskView_;
    for (int i = 0; i < BUTTON_NUMS; i++)
        delete bttnImageClick[i];
    delete cam_manager;
    delete backBttn;
    delete backIcon;
    delete txtMsgLabel;
    delete recordImage;
    delete tmLabel;
    delete bttnLeft;
    delete bttnMidle;
    delete bttnRight;
    delete bttnRecord;
    delete scroll;
    animator_->Stop();
    delete animator_;
    delete slider;
    delete buttonListener_;
    delete surfaceView;
    delete background_;
}

void CameraAbilitySlice::SetHead(void)
{
    backIcon = new UIImageView();
    backIcon->SetTouchable(true);
    backIcon->SetSrc(UI_IMAGE_PATH"ic_back.png");
    backIcon->SetPosition(BACK_LABEL_X, BACK_LABEL_Y, BACK_LABEL_W, BACK_LABEL_H);

    backBttn = new UIImageView();
    backBttn->SetTouchable(true);
    backBttn->SetPosition(0, 0, BACK_LABEL_W * 4, BACK_LABEL_H * 4);    /* 4 cups of icon size */
    backBttn->SetStyle(STYLE_BACKGROUND_OPA, 0);
    auto backBttnonClick = [this](UIView &view, const Event &event) -> bool {
        printf("############  from launcher enter  #############\n");
        TerminateAbility();
        printf("############  to launcher  #############\n");
        return true;
    };

    buttonListener_ = new EventListener(backBttnonClick, nullptr);
    backBttn->SetOnClickListener(buttonListener_);
    backIcon->SetOnClickListener(buttonListener_);

    txtMsgLabel = new UILabel();
    txtMsgLabel->SetPosition(TXT_LABEL_X, TXT_LABEL_Y, TXT_LABEL_W, TXT_LABEL_H);
    txtMsgLabel->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT, UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);

    txtMsgLabel->SetFont((const char *)TTF_PATH, FONT_SIZE);

    txtMsgLabel->SetAlign(TEXT_ALIGNMENT_LEFT);
    txtMsgLabel->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    txtMsgLabel->SetStyle(STYLE_BACKGROUND_OPA, 0);
    txtMsgLabel->SetText((char *)"相机");

    recordImage = new UIImageView();
    recordImage->SetTouchable(false);
    recordImage->SetSrc(UI_IMAGE_PATH"ic_timer.png");
    recordImage->SetPosition(RECORD_IMAGE_X, RECORD_IMAGE_Y, RECORD_IMAGE_W, RECORD_IMAGE_H);
    recordImage->SetStyle(STYLE_BACKGROUND_OPA, 0);

    tmLabel = new UILabel();
    tmLabel->SetPosition(TIME_LABEL_X, TIME_LABEL_Y, TIME_LABEL_W, TIME_LABEL_H);
    tmLabel->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT, UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    tmLabel->SetText("00:00");

    tmLabel->SetFont((const char *)TTF_PATH, FONT_SIZE);

    tmLabel->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    tmLabel->SetStyle(STYLE_BACKGROUND_OPA, 0);

    recordImage->SetVisible(false);
    recordImage->Invalidate();
    tmLabel->SetVisible(false);
    tmLabel->Invalidate();
}

void CameraAbilitySlice::SetBottom(void)
{
    scroll = new UIScrollView();
    scroll->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::White()));
    scroll->SetStyle(STYLE_BACKGROUND_OPA, 0);
    scroll->SetPosition(SCROLL_VIEW_X, SCROLL_VIEW_Y, SCROLL_VIEW_W, SCROLL_VIEW_H);
    scroll->SetHorizontalScrollState(true);
    scroll->SetVerticalScrollState(false);
    scroll->SetXScrollBarVisible(false);
    scroll->SetYScrollBarVisible(false);

    bttnLeft = new UIImageView();
    bttnLeft->SetTouchable(true);
    bttnLeft->SetPosition(LEFT_BUTTON_X, LEFT_BUTTON_Y, LEFT_BUTTON_W, LEFT_BUTTON_H);
    bttnLeft->SetSrc(UI_IMAGE_PATH"ic_camera_photo.png");
    bttnLeft->SetStyle(STYLE_BACKGROUND_OPA, 0);

    bttnMidle = new UIImageView();
    bttnMidle->SetTouchable(true);
    bttnMidle->SetSrc(UI_IMAGE_PATH"ic_camera_shutter.png");
    bttnMidle->SetPosition(MID_BUTTON_X, MID_BUTTON_Y, MID_BUTTON_W, MID_BUTTON_H);
    bttnMidle->SetStyle(STYLE_BACKGROUND_OPA, 0);

    bttnRight = new UIImageView();
    bttnRight->SetTouchable(true);
    bttnRight->SetPosition(RIGHT_BUTTON_X, RIGHT_BUTTON_Y, RIGHT_BUTTON_W, RIGHT_BUTTON_H);
    bttnRight->SetSrc(UI_IMAGE_PATH"ic_camera_video.png");
    bttnRight->SetStyle(STYLE_BACKGROUND_OPA, 0);

    bttnRecord = new UIImageView();
    bttnRecord->SetTouchable(true);
    bttnRecord->SetPosition(RIGHT_BUTTON_X, RIGHT_BUTTON_Y, RIGHT_BUTTON_W, RIGHT_BUTTON_H);
    bttnRecord->SetSrc(UI_IMAGE_PATH"ic_camera_record_pause.png");
    bttnRecord->SetStyle(STYLE_BACKGROUND_OPA, 0);
    bttnRecord->SetVisible(false);
    bttnRecord->Invalidate();

    slider = new UISlider();
    slider->SetPosition(-1, -1, 1, 1);

    gTaskView_ = new TaskView(tmLabel);
    gTaskView_->TaskStart();

    animator_ = new SliderAnimator(slider, background_, surfaceView, cam_manager, 10000);    /* 10000 = 10s */

    UIImageView *imageV[BUTTON_NUMS] = {bttnLeft, bttnMidle, bttnRight, bttnRecord};
    for (int i = 0; i < BUTTON_NUMS; i++) {
        bttnImageClick[i] =
            new CameraImageButtonOnClickListener((UIView *)scroll, surfaceView, background_, gTaskView_,
            (SliderAnimator*)animator_);
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetButtonIdxCamera(cam_manager, i);
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetLabel(tmLabel);
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetImageView(bttnLeft, 0);    /* 0 */
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetImageView(bttnMidle, 1);    /* 1 */
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetImageView(bttnRight, 2);    /* 2 */
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetImageView(bttnRecord, 3);    /* 3 */
        ((CameraImageButtonOnClickListener *)bttnImageClick[i])->SetImageView(recordImage, 4);    /* 4 */
        imageV[i]->SetOnClickListener(bttnImageClick[i]);
    }
}

void CameraAbilitySlice::OnStart(const Want &want)
{
    AbilitySlice::OnStart(want);
    printf("CameraAbilitySlice onstart \n");

    surfaceView = new UISurfaceView();
    surfaceView->SetPosition(V_GROUP_X, V_GROUP_Y, V_GROUP_W, V_GROUP_H);
    surfaceView->GetSurface()->SetWidthAndHeight(IMAGE_WIDTH, IMAGE_HEIGHT);

    background_ = new UIImageView();
    background_->SetTouchable(false);
    background_->SetSrc("/userdata/tmp.jpg");
    background_->SetPosition(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    background_->SetVisible(true);
    background_->Invalidate();

    cam_manager = new SampleCameraManager();
    cam_manager->SampleCameraCreate();

    SetHead();

    SetBottom();
    scroll->Add(bttnLeft);
    scroll->Add(bttnMidle);
    scroll->Add(bttnRight);
    scroll->Add(bttnRecord);

    RootView *rootView = RootView::GetWindowRootView();
    rootView->SetPosition(0, 0);
    rootView->Resize(SCREEN_WIDTH, SCREEN_HEIGHT);
    rootView->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Black()));

    rootView->Add(surfaceView);
    rootView->Add(background_);
    rootView->Add(backBttn);
    rootView->Add(backIcon);
    rootView->Add(txtMsgLabel);
    rootView->Add(recordImage);
    rootView->Add(tmLabel);
    rootView->Add(scroll);
    rootView->Add(slider);

    int timecnt = 0;
    while (1) {
        if (++timecnt > 5) {    /* 5s timeout */
            printf("wait camera timeout!! \n");
            break;
        }
        if (cam_manager->SampleCameraIsReady()) break;
        sleep(1);
    }
    SetUIContent(rootView);
}

void CameraAbilitySlice::OnInactive()
{
    printf("CameraAbilitySlice::OnInactive\n");
    AbilitySlice::OnInactive();
}

void CameraAbilitySlice::OnActive(const Want &want)
{
    printf("CameraAbilitySlice::OnActive\n");
    AbilitySlice::OnActive(want);

    if (cam_manager) {
        cam_manager->SampleCameraStart(surfaceView->GetSurface());
        if (background_) {
            background_->SetVisible(false);
            background_->Invalidate();
        }
    }
}

void CameraAbilitySlice::OnBackground()
{
    printf("CameraAbilitySlice::OnBackground\n");
    AbilitySlice::OnBackground();
    if (background_) {
        background_->SetSrc("/userdata/tmp.jpg");
        background_->SetVisible(true);
        background_->Invalidate();
    }
    if (cam_manager)
        cam_manager->SampleCameraStop();
}

void CameraAbilitySlice::OnStop()
{
    printf("CameraAbilitySlice::OnStop\n");
    AbilitySlice::OnStop();
}
}
