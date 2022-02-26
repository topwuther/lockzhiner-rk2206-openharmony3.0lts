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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_LONG_TEXT_CONTENT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_LONG_TEXT_CONTENT_H

#include <memory>
#include <string>
#include "notification_basic_content.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationLongTextContent : public NotificationBasicContent {
public:
    /**
     * Default constructor used to create a NotificationLongTextContent instance.
     */
    NotificationLongTextContent() = default;

    /**
     * A constructor used to create a NotificationLongTextContent instance with the input parameter longText passed.
     * @param longText Indicates the long text to be included. The value contains a maximum of 1024 characters.
     */
    explicit NotificationLongTextContent(const std::string &longText);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationLongTextContent() = default;

    /**
     * Sets the title to be displayed when this long text notification is expanded. After this title is set,
     * the title set by setTitle(string) will be displayed only when this notification is in the collapsed state.
     * @param exTitle Indicates the title to be displayed when this notification is expanded.
     */
    void SetExpandedTitle(const std::string &exTitle);

    /**
     * Obtains the title that will be displayed for this long text notification when it is expanded.
     * @return the title to be displayed when this notification is expanded.
     */
    std::string GetExpandedTitle() const;

    /**
     * Sets the brief text to be included in a long text notification.
     * The brief text is a summary of a long text notification and is displayed in the first line of the notification.
     * Similar to setAdditionalText(string), the font of the brief text is also smaller than the notification text.
     * The positions where the brief text and additional text will display may conflict.
     * If both texts are set, only the additional text will be displayed.
     * @param briefText Indicates the brief text to be included.
     */
    void SetBriefText(const std::string &briefText);

    /**
     * Obtains the brief text of a long text notification specified by calling the setBriefText(string) method.
     * @return the brief text of the long text notification.
     */
    std::string GetBriefText() const;

    /**
     * Sets the long text to be included in a long text notification.
     * @param longText Indicates the long text to be included. The value contains a maximum of 1024 characters.
     */
    void SetLongText(const std::string &longText);

    /**
     * Obtains a notification's long text, which is set by calling the setLongText(string) method.
     * @return the long text.
     */
    std::string GetLongText() const;

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
     * @return the NotificationLongTextContent
     */
    static NotificationLongTextContent *Unmarshalling(Parcel &parcel);

protected:
    /**
     * Read a NotificationLongTextContent object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel) override;

private:
    /**
     * the maximum lenth of longtext is 1024 characters.
     */
    static const std::size_t MAX_LONGTEXT_LENGTH;

private:
    std::string longText_ {};
    std::string expandedTitle_ {};
    std::string briefText_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_LONG_TEXT_CONTENT_H