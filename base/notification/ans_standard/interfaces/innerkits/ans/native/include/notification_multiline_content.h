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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_MULTILINE_CONTENT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_MULTILINE_CONTENT_H

#include <string>
#include <vector>
#include "notification_basic_content.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationMultiLineContent : public NotificationBasicContent {
public:
    /**
     * Default constructor used to create a NotificationMultiLineContent instance.
     */
    NotificationMultiLineContent() = default;

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationMultiLineContent() = default;

    /**
     * Sets the title to be displayed when this multi-line notification is expanded.
     * After this title is set, the title set by setTitle(string) will be displayed only
     * when this notification is in the collapsed state.
     * @param exTitle Indicates the title to be displayed when this notification is expanded.
     */
    void SetExpandedTitle(const std::string &exTitle);

    /**
     * Obtains the title that will be displayed for this multi-line notification when it is expanded.
     * @return the title to be displayed when this notification is expanded.
     */
    std::string GetExpandedTitle() const;

    /**
     * Sets the brief text to be included in a multi-line notification.
     * The brief text is a summary of this multi-line notification and is displayed in the first line of
     * the notification. Similar to setAdditionalText(string), the font of the brief text is also
     * smaller than the notification text set by calling setText(string).
     * The positions where the brief text and additional text will display may conflict.
     * If both texts are set, only the additional text will be displayed.
     * @param briefText Indicates the brief text to be included.
     */
    void SetBriefText(const std::string &briefText);

    /**
     * Obtains the brief text that has been set by calling setBriefText(string) for this multi-line notification.
     * @return the brief text of this notification.
     */
    std::string GetBriefText() const;

    /**
     * Adds a single line of text to this notification.
     * You can call this method up to seven times to add seven lines to a notification.
     * @param oneLine Indicates the single line of text to be included.
     */
    void AddSingleLine(const std::string &oneLine);

    /**
     * Obtains the list of lines included in this multi-line notification.
     * @return the list of lines included in this notification.
     */
    std::vector<std::string> GetAllLines() const;

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
     * @return the NotificationMultiLineContent
     */
    static NotificationMultiLineContent *Unmarshalling(Parcel &parcel);

protected:
    /**
     * Read a NotificationMultiLineContent object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel) override;

private:
    /**
     * the maximum size of vector is 7.
     */
    static const std::vector<std::string>::size_type MAX_LINES;

private:
    std::string expandedTitle_ {};
    std::string briefText_ {};
    std::vector<std::string> allLines_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_MULTILINE_CONTENT_H