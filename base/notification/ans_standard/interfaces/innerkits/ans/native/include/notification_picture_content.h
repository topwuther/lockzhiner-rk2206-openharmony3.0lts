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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_PICTURE_CONTENT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_PICTURE_CONTENT_H

#include <memory>
#include <string>
#include "notification_basic_content.h"
#include "pixel_map.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationPictureContent : public NotificationBasicContent {
public:
    /**
     * Default constructor used to create an empty NotificationPictureContent instance.
     */
    NotificationPictureContent() = default;

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationPictureContent() = default;

    /**
     * Sets the title to be displayed when this picture-attached notification is expanded.
     * After this title is set, the title set by setTitle(string) will be displayed only
     * when this notification is in the collapsed state.
     * @param exTitle Indicates the title to be displayed when this notification is expanded.
     */
    void SetExpandedTitle(const std::string &exTitle);

    /**
     * Obtains the title that will be displayed for this picture-attached notification when it is expanded.
     * @return the title to be displayed when this notification is expanded.
     */
    std::string GetExpandedTitle() const;

    /**
     * Sets the brief text to be included in a picture-attached notification.
     * The brief text is a summary of a picture-attached notification and is displayed in the first line of the
     * notification. Similar to setAdditionalText(string), the font of the brief text is also smaller than the
     * notification text. The positions where the brief text and additional text will display may conflict. If both
     * texts are set, only the additional text will be displayed.
     * @param briefText Indicates the brief text to be included.
     */
    void SetBriefText(const std::string &briefText);

    /**
     * Obtains the brief text of a picture-attached notification specified by calling the setBriefText(string) method.
     * @return the brief text of the picture-attached notification.
     */
    std::string GetBriefText() const;

    /**
     * Sets the picture to be included in a notification.
     * @param bigPicture Indicates the PixelMap of the picture to be included.
     */
    void SetBigPicture(const std::shared_ptr<Media::PixelMap> &bigPicture);

    /**
     * Obtains the PixelMap of the picture specified by calling the setBigPicture(PixelMap) method.
     * @return the PixelMap of the picture included in the notification.
     */
    const std::shared_ptr<Media::PixelMap> GetBigPicture() const;

    /**
     * Returns a string representation of the object.
     * @return a string representation of the object.
     */
    std::string Dump() override;

    /**
     * Marshal a object into a Parcel.
     * @param parcel the object into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal object from a Parcel.
     * @return the NotificationPictureContent
     */
    static NotificationPictureContent *Unmarshalling(Parcel &parcel);

protected:
    /**
     * Read a NotificationPictureContent object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel) override;

private:
    std::string expandedTitle_ {};
    std::string briefText_ {};
    std::shared_ptr<Media::PixelMap> bigPicture_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_PICTURE_CONTENT_H