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

#include "gallery_ability_slice.h"
#include "ability_env.h"
#include "ability_manager.h"
#include "picture_ability_slice.h"

#include "gfx_utils/file.h"
#include "securec.h"

namespace OHOS {
REGISTER_AS(GalleryAbilitySlice)

GalleryAbilitySlice::~GalleryAbilitySlice()
{
    printf("~GalleryAbilitySlice() | start \n");
    Clear();
    printf("~GalleryAbilitySlice() | end \n");
}

void GalleryAbilitySlice::Clear()
{
    printf("GalleryAbilitySlice::Clear() | start \n");
    if (backIcon_ != nullptr) {
        delete backIcon_;
        backIcon_ = nullptr;
    }
    if (backIconListener_ != nullptr) {
        delete backIconListener_;
        backIconListener_ = nullptr;
    }
    if (deleteClickListener_ != nullptr) {
        delete deleteClickListener_;
        deleteClickListener_ = nullptr;
    }
    if (backArea_ != nullptr) {
        delete backArea_;
        backArea_ = nullptr;
    }
    if (titleLabel_ != nullptr) {
        delete titleLabel_;
        titleLabel_ = nullptr;
    }
    if (deleteLabel_ != nullptr) {
        delete deleteLabel_;
        deleteLabel_ = nullptr;
    }

    ClearThumb();

    if (rootView_ != nullptr) {
        RootView::DestoryWindowRootView(rootView_);
        rootView_ = nullptr;
    }
    printf("GalleryAbilitySlice::Clear() | end \n");
}

void GalleryAbilitySlice::ClearThumb()
{
    printf("GalleryAbilitySlice::ClearThumb() | start \n");
    if (picContainer_ != nullptr) {
        delete picContainer_;
        picContainer_ = nullptr;
    }
    if (picList_ != nullptr) {
        ClearPictureList(picList_);
        delete picList_;
        picList_ = nullptr;
    }
    for (uint16_t i = 0; i < pictureCount_; i++) {
        if (pictureName_[i] != nullptr) {
            delete[] pictureName_[i];
            pictureName_[i] = nullptr;
        }
    }
    for (uint16_t i = 0; i < pictureOnClickListenerCount_; i++) {
        if (pictureOnClickListener_[i] != nullptr) {
            delete pictureOnClickListener_[i];
            pictureOnClickListener_[i] = nullptr;
        }
    }
    pictureCount_ = 0;
    pictureOnClickListenerCount_ = 0;
    printf("GalleryAbilitySlice::ClearThumb() | end \n");
}

void GalleryAbilitySlice::ClearPictureList(const UIView* view)
{
    printf("GalleryAbilitySlice::ClearPictureList() | start \n");
    if (view == nullptr || !(view->IsViewGroup())) {
        return;
    }
    UIView* child = static_cast<const UIViewGroup*>(view)->GetChildrenHead();
    UIView* childNext = nullptr;
    while (child != nullptr) {
        childNext = child->GetNextSibling();
        if (child->IsViewGroup()) {
            ClearPictureList(child);
        }
        delete child;
        child = childNext;
    }
    printf("GalleryAbilitySlice::ClearPictureList() | end \n");
}

void GalleryAbilitySlice::InitTitle()
{
    printf("GalleryAbilitySlice::InitTitle | start \n");
    backIcon_ = new UIImageView();
    backIcon_->SetPosition(BACK_ICON_POSITION_X, BACK_ICON_POSITION_Y);
    backIcon_->SetSrc(backIconAbsolutePath);
    backIcon_->SetTouchable(true);

    backArea_ = new UIViewGroup();
    backArea_->SetPosition(0, 0, LABEL_POSITION_X, LABEL_HEIGHT);
    backArea_->SetStyle(STYLE_BACKGROUND_OPA, 0);
    backArea_->SetTouchable(true);

    auto onClick = [this] (UIView& view, const Event& event) -> bool {
        printf("############  Next AS enter   #############\n");
        TerminateAbility();
        printf("############  Next AS exit   #############\n");
        return true;
    };
    backIconListener_ = new EventListener(onClick, nullptr);
    backIcon_->SetOnClickListener(backIconListener_);
    backArea_->SetOnClickListener(backIconListener_);

    titleLabel_ = new UILabel();
    titleLabel_->SetPosition(LABEL_POSITION_X, LABEL_POSITION_Y, LABEL_WIDTH, LABEL_HEIGHT);
    titleLabel_->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT, UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    titleLabel_->SetFont(FONT_NAME, GALLERY_FONT_SIZE);
    titleLabel_->SetText("照片");

    deleteLabel_ = new UILabel();
    deleteLabel_->SetPosition(ROOT_VIEW_WIDTH - DELETE_LABEL_WIDTH, LABEL_POSITION_Y,
                              DELETE_LABEL_WIDTH, LABEL_HEIGHT);
    deleteLabel_->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT,
                           UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    deleteLabel_->SetFont(FONT_NAME, GALLERY_DELETE_FONT_SIZE);
    deleteLabel_->SetText("全部删除");
    deleteLabel_->SetTouchable(true);
    auto deleteClick = [this] (UIView& view, const Event& event) -> bool {
        printf("############  DeleteAllData click enter #############\n");
        DeleteAllData();
        printf("############  DeleteAllData click exit  #############\n");
        return true;
    };
    deleteClickListener_ = new EventListener(deleteClick, nullptr);
    deleteLabel_->SetOnClickListener(deleteClickListener_);

    backArea_->Add(backIcon_);
    rootView_->Add(backArea_);
    rootView_->Add(titleLabel_);
    rootView_->Add(deleteLabel_);
}

void GalleryAbilitySlice::InitPictureList()
{
    printf("GalleryAbilitySlice::InitPictureList | start \n");
    picContainer_ = new UIScrollView();
    picContainer_->SetPosition(0, LABEL_POSITION_Y + LABEL_HEIGHT);
    picContainer_->Resize(ROOT_VIEW_WIDTH, (THUMBNAIL_RESOLUTION_Y + THUMBNAIL_SPACE) * THUMBNAIL_COLUMN);
    picContainer_->SetStyle(STYLE_BACKGROUND_OPA, 0);
    rootView_->Add(picContainer_);

    picList_ = new UIViewGroup();
    picList_->SetPosition(0, 0, ROOT_VIEW_WIDTH, ROOT_VIEW_HEIGHT);
    picList_->SetStyle(STYLE_BACKGROUND_OPA, 0);

    int16_t numInLine = (ROOT_VIEW_WIDTH + THUMBNAIL_SPACE) / (THUMBNAIL_RESOLUTION_X + THUMBNAIL_SPACE);
    int16_t offset = ((ROOT_VIEW_WIDTH + THUMBNAIL_SPACE) % (THUMBNAIL_RESOLUTION_X + THUMBNAIL_SPACE)) / 2; // 2: half
    AddAllPictures(Point { offset, 0 }, numInLine);

    int16_t totalHeight = (pictureCount_ / numInLine) * (THUMBNAIL_RESOLUTION_Y + THUMBNAIL_SPACE);
    if ((pictureCount_ % numInLine) != 0) {
        totalHeight += THUMBNAIL_RESOLUTION_Y + THUMBNAIL_SPACE;
    }
    picList_->Resize(ROOT_VIEW_WIDTH, totalHeight);
    printf("------------ totalHeight : %d ------------", totalHeight);
    picContainer_->Add(picList_);
}

void GalleryAbilitySlice::AddAllPictures(const Point& pos, int16_t numInLine)
{
    printf("GalleryAbilitySlice::AddAllPictures | start | %d\n", numInLine);
    Point imagePos = pos;
    DIR* drip = opendir(THUMBNAIL_DIRECTORY);
    if (drip == nullptr) {
        return;
    }
    struct dirent* info = nullptr;
    while ((info = readdir(drip)) != nullptr  && pictureCount_ < MAX_PICTURE_COUNT) {
        uint16_t imageNameLen = static_cast<uint16_t>(strlen(info->d_name));
        if (imageNameLen > MAX_PATH_LENGTH || (strcmp(info->d_name, ".") == 0) || (strcmp(info->d_name, "..") == 0)) {
            printf("GalleryAbilitySlice::AddAllPictures | imageNameLen > MAX_PATH_LENGTH | %d\n", imageNameLen);
            continue;
        }
        char* imageName = new char[imageNameLen + 1]();
        memcpy_s(imageName, imageNameLen + 1, info->d_name, imageNameLen + 1);
        pictureName_[pictureCount_] = imageName;
        pictureCount_++;

        uint16_t pathLen = static_cast<uint16_t>(strlen(THUMBNAIL_DIRECTORY)) + imageNameLen + 1;
        if (pathLen > MAX_PATH_LENGTH) {
            printf("GalleryAbilitySlice::AddAllPictures | pathLen > MAX_PATH_LENGTH | %d\n", pathLen);
            continue;
        }
        char* imagePath = new char[pathLen + 1]();
        if (sprintf_s(imagePath, pathLen + 1, "%s/%s", THUMBNAIL_DIRECTORY, info->d_name) < 0) {
            printf("GalleryAbilitySlice::AddAllPictures | sprintf_s error\n");
            delete[] imagePath;
            continue;
        }

        picList_->Add(CreateImageItem(imagePos, imageName, imagePath));
        delete[] imagePath;

        if ((pictureCount_ % numInLine) == 0) {
            imagePos.x = pos.x;
            imagePos.y += THUMBNAIL_RESOLUTION_Y + THUMBNAIL_SPACE;
        } else {
            imagePos.x += THUMBNAIL_RESOLUTION_X + THUMBNAIL_SPACE;
        }
    }
    delete info;
    closedir(drip);
}

UIView* GalleryAbilitySlice::CreateImageItem(const Point& pos, const char* imageName, const char* imagePath)
{
    UIImageView* imageView = new UIImageView();
    imageView->SetAutoEnable(false);
    imageView->Resize(THUMBNAIL_RESOLUTION_X, THUMBNAIL_RESOLUTION_Y);
    imageView->SetSrc(imagePath);
    pictureOnClickListener_[pictureOnClickListenerCount_] = GetImageClickListener(imageName);
    imageView->SetOnClickListener(pictureOnClickListener_[pictureOnClickListenerCount_++]);
    imageView->SetTouchable(true);

    if (strncmp(imageName, PHOTO_PREFIX, strlen(PHOTO_PREFIX)) == 0) {
        imageView->SetPosition(pos.x, pos.y);
        return imageView;
    }
    imageView->SetPosition(0, 0);

    UIViewGroup* imageItem = new UIViewGroup();
    imageItem->SetStyle(STYLE_BACKGROUND_OPA, 0);
    imageItem->SetPosition(pos.x, pos.y, THUMBNAIL_RESOLUTION_X, THUMBNAIL_RESOLUTION_Y);
    imageItem->SetTouchable(true);
    imageItem->SetOnClickListener(imageView->GetOnClickListener());

    UIImageView* videoTag = new UIImageView();
    videoTag->SetPosition(VIDEO_TAG_POSITION_X, VIDEO_TAG_POSITION_Y);
    videoTag->SetSrc(videoTagIconAbsolutePath);
    videoTag->SetTouchable(true);
    videoTag->SetOnClickListener(imageView->GetOnClickListener());

    imageItem->Add(imageView);
    imageItem->Add(videoTag);

    return imageItem;
}

EventListener* GalleryAbilitySlice::GetImageClickListener(const char* path)
{
    auto onClick = [this, path] (UIView& view, const Event& event) -> bool {
        printf("############  Next AS enter   #############\n");
        Want wantData = { nullptr };
        printf("------- imagePath: %s \n", path);
        bool ret = SetWantData(&wantData, path, strlen(path) + 1);
        if (!ret) {
            printf("############  SetWantData error   #############\n");
            return ret;
        }
        AbilitySlice* nextSlice = nullptr;
        if (strncmp(path, PHOTO_PREFIX, strlen(PHOTO_PREFIX)) == 0) {
            printf("--------- enter PictureAbilitySlice \n");
            nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("PictureAbilitySlice");
        } else {
            printf("--------- enter PlayerAbilitySlice \n");
            nextSlice = AbilityLoader::GetInstance().GetAbilitySliceByName("PlayerAbilitySlice");
        }
        if (nextSlice == nullptr) {
            printf("undefined nextSlice\n");
        } else {
            Present(*nextSlice, wantData);
        }
        printf("############  Next AS exit   #############\n");
        return true;
    };
    return new EventListener(onClick, nullptr);
}

void GalleryAbilitySlice::DeleteAllData()
{
    picContainer_->Invalidate();
    rootView_->Remove(picContainer_);
    ClearThumb();

    DeleteAllFilesInDir(THUMBNAIL_DIRECTORY);
    DeleteAllFilesInDir(PHOTO_DIRECTORY);
    DeleteAllFilesInDir(VIDEO_SOURCE_DIRECTORY);

    InitPictureList();
}

void GalleryAbilitySlice::DeleteAllFilesInDir(const char* path)
{
    DIR* drip = opendir(path);
    if (drip == nullptr) {
        return;
    }
    struct dirent* info = nullptr;
    while ((info = readdir(drip)) != nullptr) {
        uint16_t fileNameLen = static_cast<uint16_t>(strlen(info->d_name));
        uint16_t pathLen = static_cast<uint16_t>(strlen(path)) + fileNameLen + 1;
        if (pathLen > MAX_PATH_LENGTH) {
            printf("GalleryAbilitySlice::AddAllPictures | pathLen > MAX_PATH_LENGTH | %d\n", pathLen);
            continue;
        }
        char* filePath = new char[pathLen + 1]();
        if (sprintf_s(filePath, pathLen + 1, "%s/%s", path, info->d_name) < 0) {
            printf("GalleryAbilitySlice::AddAllPictures | sprintf_s error\n");
            delete[] filePath;
            continue;
        }
        if (unlink(filePath) != 0) {
            printf("unlink file error | %s\n", filePath);
        }
        delete[] filePath;
    }
    delete info;
    closedir(drip);
    printf("GalleryAbilitySlice::DeleteAllFilesInDir() | success | %s\n", path);
}

void GalleryAbilitySlice::OnStart(const Want &want)
{
    AbilitySlice::OnStart(want);

    rootView_ = RootView::GetWindowRootView();
    rootView_->SetPosition(ROOT_VIEW_POSITION_X, ROOT_VIEW_POSITION_Y);
    rootView_->Resize(ROOT_VIEW_WIDTH, ROOT_VIEW_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);

    const char* pathHeader = GetSrcPath();
    if (sprintf_s(backIconAbsolutePath, MAX_PATH_LENGTH, "%s%s", pathHeader, BACK_ICON_PATH) < 0) {
        printf("GalleryAbilitySlice::OnStart | backIconAbsolutePath error");
        return;
    }
    if (sprintf_s(videoTagIconAbsolutePath, MAX_PATH_LENGTH, "%s%s", pathHeader, VIDEO_TAG_ICON_PATH) < 0) {
        printf("GalleryAbilitySlice::OnStart | videoTagIconAbsolutePath error");
        return;
    }

    InitTitle();
    InitPictureList();
    SetUIContent(rootView_);
}

void GalleryAbilitySlice::OnInactive()
{
    printf("GalleryAbilitySlice::OnInactive\n");
    AbilitySlice::OnInactive();
}

void GalleryAbilitySlice::OnActive(const Want &want)
{
    printf("GalleryAbilitySlice::OnActive\n");
    AbilitySlice::OnActive(want);
}

void GalleryAbilitySlice::OnBackground()
{
    printf("GalleryAbilitySlice::OnBackground\n");
    AbilitySlice::OnBackground();
}

void GalleryAbilitySlice::OnStop()
{
    printf("GalleryAbilitySlice::OnStop\n");
    AbilitySlice::OnStop();
    Clear();
}
}