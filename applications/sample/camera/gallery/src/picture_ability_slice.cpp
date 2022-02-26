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

#include "picture_ability_slice.h"
#include "ability_env.h"
#include "gallery_config.h"
#include "gfx_utils/file.h"
#include "securec.h"

namespace OHOS {
REGISTER_AS(PictureAbilitySlice)

PictureAbilitySlice::~PictureAbilitySlice()
{
    Clear();
}

void PictureAbilitySlice::Clear()
{
    printf("PictureAbilitySlice::Clear() | start \n");
    if (backIcon_ != nullptr) {
        delete backIcon_;
        backIcon_ = nullptr;
    }
    if (backIconListener_ != nullptr) {
        delete backIconListener_;
        backIconListener_ = nullptr;
    }
    if (backArea_ != nullptr) {
        delete backArea_;
        backArea_ = nullptr;
    }
    if (picture_ != nullptr) {
        delete picture_;
        picture_ = nullptr;
    }
    if (rootView_ != nullptr) {
        RootView::DestoryWindowRootView(rootView_);
        rootView_ = nullptr;
    }
    printf("PictureAbilitySlice::Clear() | end \n");
}

void PictureAbilitySlice::InitTitle()
{
    printf("PictureAbilitySlice::InitTitle | start \n");
    backIcon_ = new UIImageView();
    backIcon_->SetPosition(BACK_ICON_POSITION_X, BACK_ICON_POSITION_Y);
    backIcon_->SetSrc(backIconAbsolutePath);
    backIcon_->SetTouchable(true);

    backArea_ = new UIViewGroup();
    backArea_->SetPosition(0, 0, LABEL_POSITION_X, LABEL_HEIGHT);
    backArea_->SetStyle(STYLE_BACKGROUND_OPA, 0);
    backArea_->SetTouchable(true);

    auto onClick = [this] (UIView &view, const Event &event) -> bool {
        printf("############  terminate AS enter  #############\n");
        Terminate();
        printf("############  terminate AS exit  #############\n");
        return true;
    };
    backIconListener_ = new EventListener(onClick, nullptr);
    backIcon_->SetOnClickListener(backIconListener_);
    backArea_->SetOnClickListener(backIconListener_);

    backArea_->Add(backIcon_);
    rootView_->Add(backArea_);
}

void PictureAbilitySlice::InitPicture(const char* path)
{
    printf("PictureAbilitySlice::InitPicture | start | %s\n", path);
    picture_ = new UIImageView();
    picture_->SetSrc(path);
    int16_t imageWidth = picture_->GetWidth();
    int16_t imageHeight = picture_->GetHeight();
    if (imageWidth > ROOT_VIEW_WIDTH || imageHeight > ROOT_VIEW_HEIGHT) {
        TransformMap transMap(picture_->GetOrigRect());
        float scaleWidth = 1.0;
        float scaleHeight = 1.0;
        if (imageWidth > ROOT_VIEW_WIDTH) {
            scaleWidth = static_cast<float>(ROOT_VIEW_WIDTH) / imageWidth;
            printf("########## scaleWidth: %f \n", scaleWidth);
        }
        if (imageHeight > ROOT_VIEW_HEIGHT) {
            scaleHeight = static_cast<float>(ROOT_VIEW_HEIGHT) / imageHeight;
            printf("########## scaleHeight: %f \n", scaleHeight);
        }
        float scale = (scaleWidth < scaleHeight) ? scaleWidth : scaleHeight;
        printf("########## scale: %f \n", scale);
        transMap.Scale(Vector2<float>(scale, scale), Vector2<float>(0, 0));
        picture_->SetTransformMap(transMap);
        picture_->SetTransformAlgorithm(TransformAlgorithm::NEAREST_NEIGHBOR);
        imageWidth = imageWidth * scale;
        imageHeight = imageHeight * scale;
    }
    int16_t imagePosX = (ROOT_VIEW_WIDTH - imageWidth) / 2; // 2: half
    int16_t imagePosY = (ROOT_VIEW_HEIGHT - imageHeight) / 2; // 2: half
    printf("########## image pos x: %d  | y: %d \n", imagePosX, imagePosY);
    picture_->SetPosition(imagePosX, imagePosY);

    rootView_->Add(picture_);
}

void PictureAbilitySlice::OnStart(const Want &want)
{
    printf("######### PictureAbilitySlice::OnStart\n");
    printf("receive the data -> %s\n", reinterpret_cast<char*>(want.data));
    AbilitySlice::OnStart(want);

    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(ROOT_VIEW_POSITION_X, ROOT_VIEW_POSITION_Y);
    rootView_->Resize(ROOT_VIEW_WIDTH, ROOT_VIEW_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);

    uint16_t imagePathLen = strlen(PHOTO_DIRECTORY) + strlen(reinterpret_cast<char*>(want.data)) + 1;
    if (imagePathLen > MAX_PATH_LENGTH) {
        printf("---- imagePathLen > MAX_PATH_LENGTH | %d", imagePathLen);
        return;
    }
    char* imagePath = new char[imagePathLen + 1]();
    if (sprintf_s(imagePath, imagePathLen + 1, "%s/%s", PHOTO_DIRECTORY, reinterpret_cast<char*>(want.data)) < 0) {
        printf("PictureAbilitySlice::OnStart | imagePath\n");
        delete[] imagePath;
        imagePath = nullptr;
        return;
    }

    const char* pathHeader = GetSrcPath();
    if (sprintf_s(backIconAbsolutePath, MAX_PATH_LENGTH, "%s%s", pathHeader, BACK_ICON_PATH) < 0) {
        printf("PictureAbilitySlice::OnStart | backIconAbsolutePath\n");
        delete[] imagePath;
        imagePath = nullptr;
        return;
    }

    InitPicture(imagePath);
    InitTitle();
    delete[] imagePath;

    SetUIContent(rootView_);
}

void PictureAbilitySlice::OnInactive()
{
    printf("PictureAbilitySlice::OnInactive\n");
    AbilitySlice::OnInactive();
}

void PictureAbilitySlice::OnActive(const Want &want)
{
    printf("PictureAbilitySlice::OnActive\n");
    AbilitySlice::OnActive(want);
}

void PictureAbilitySlice::OnBackground()
{
    printf("PictureAbilitySlice::OnBackground\n");
    AbilitySlice::OnBackground();
}

void PictureAbilitySlice::OnStop()
{
    printf("PictureAbilitySlice::OnStop\n");
    AbilitySlice::OnStop();
    Clear();
}
}
