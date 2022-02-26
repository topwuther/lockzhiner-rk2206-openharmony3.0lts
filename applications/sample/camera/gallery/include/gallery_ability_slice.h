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

#ifndef OHOS_GALLERY_ABILITY_SLICE_H
#define OHOS_GALLERY_ABILITY_SLICE_H

#include <ability_loader.h>
#include <components/ui_image_view.h>
#include <components/ui_label.h>
#include <components/ui_scroll_view.h>
#include "event_listener.h"
#include "gallery_config.h"

namespace OHOS {
class GalleryAbilitySlice : public AbilitySlice {
public:
    GalleryAbilitySlice() = default;
    ~GalleryAbilitySlice() override;

protected:
    void OnStart(const Want &want) override;
    void OnInactive() override;
    void OnActive(const Want &want) override;
    void OnBackground() override;
    void OnStop() override;

private:
    void Clear();
    void ClearThumb();
    void ClearPictureList(const UIView* view);
    void InitTitle();
    void InitPictureList();
    void AddAllPictures(const Point& pos, int16_t numInLine);
    UIView* CreateImageItem(const Point& pos, const char* imageName, const char* imagePath);
    EventListener* GetImageClickListener(const char* path);
    void DeleteAllData();
    void DeleteAllFilesInDir(const char* path);

    RootView* rootView_ { nullptr };
    uint16_t pictureCount_ { 0 };
    UIViewGroup* backArea_ { nullptr };
    UIImageView* backIcon_ { nullptr };
    EventListener* backIconListener_ { nullptr };
    EventListener* deleteClickListener_ { nullptr };
    UILabel* titleLabel_ { nullptr };
    UILabel* deleteLabel_ { nullptr };
    UIScrollView* picContainer_ { nullptr };
    UIViewGroup* picList_ { nullptr };
    char* pictureName_[MAX_PICTURE_COUNT] = { nullptr };
    char backIconAbsolutePath[MAX_PATH_LENGTH] = { 0 };
    char videoTagIconAbsolutePath[MAX_PATH_LENGTH] = { 0 };
    uint16_t pictureOnClickListenerCount_ { 0 };
    EventListener* pictureOnClickListener_[MAX_PICTURE_COUNT] = { nullptr };
};
}
#endif // OHOS_GALLERY_ABILITY_SLICE_H