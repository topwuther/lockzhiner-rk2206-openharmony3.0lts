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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_ACTION_BUTTON_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_ACTION_BUTTON_H

#include <memory>
#include <string>
#include <vector>
#include "notification_constant.h"
#include "notification_user_input.h"
#include "want_agent.h"
#include "pac_map.h"
#include "pixel_map.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationActionButton : public Parcelable {
public:
    /**
     * A static function used to create a NotificationActionButton instance with the input parameters passed.
     * @param icon Indicates the icon to represent this NotificationActionButton.
     * @param title Indicates the title of this NotificationActionButton.
     * @param wantAgent Indicates the wantAgent to be triggered when this NotificationActionButton is triggered.
     * @param extras Indicates the PacMap object containing the additional data.
     * @param semanticActionButton Indicates the semantic action to add.
     * @param autoCreatedReplies Indicates whether to allow the platform to automatically generate possible replies.
     * @param mimeTypeOnlyInputs Indicates the NotificationUserInput object to add which allows only values of
     * particular MIME types.
     * @param userInputs Indicates the NotificationUserInput object to add.
     * @param isContextual Indicates whether this NotificationActionButton is a contextual action, that is, whether this
     * NotificationActionButton is dependent on the notification message body.
     * @return the shared_ptr object owns the created NotificationActionButton object otherwise return empty object if
     * isContextual is true but icon or wantAgent is empty.
     */
    static std::shared_ptr<NotificationActionButton> Create(const std::shared_ptr<Media::PixelMap> &icon,
        const std::string &title, const std::shared_ptr<WantAgent::WantAgent> &wantAgent,
        const std::shared_ptr<AppExecFwk::PacMap> &extras = {},
        NotificationConstant::SemanticActionButton semanticActionButton =
            NotificationConstant::SemanticActionButton::NONE_ACTION_BUTTON,
        bool autoCreatedReplies = true,
        const std::vector<std::shared_ptr<NotificationUserInput>> &mimeTypeOnlyInputs = {},
        const std::vector<std::shared_ptr<NotificationUserInput>> &userInputs = {}, bool isContextual = false);

    /**
     * A static function used to create a NotificationActionButton instance by copying parameters from an existing
     * NotificationActionButton object.
     * @param actionButton Indicates the existing NotificationActionButton object.
     * @return the shared_ptr object owns the created NotificationActionButton object otherwise return empty object.
     */
    static std::shared_ptr<NotificationActionButton> Create(
        const std::shared_ptr<NotificationActionButton> &actionButton);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationActionButton() = default;

    /**
     * Obtains the icon of this NotificationActionButton.
     * @return the icon of this NotificationActionButton.
     */
    const std::shared_ptr<Media::PixelMap> GetIcon() const;

    /**
     * Obtains the title of this NotificationActionButton.
     * @return the title of this NotificationActionButton.
     */
    std::string GetTitle() const;

    /**
     * Obtains the WantAgent of this NotificationActionButton.
     * @return the WantAgent of this NotificationActionButton.
     */
    const std::shared_ptr<WantAgent::WantAgent> GetWantAgent() const;

    /**
     * Adds additional data to this NotificationActionButton.
     * @param pacMap Indicates the PacMap object containing the additional data.
     */
    void AddAdditionalData(AppExecFwk::PacMap &pacMap);

    /**
     * Obtains the additional data included in this NotificationActionButton.
     * @return the additional data included in this NotificationActionButton.
     */
    const std::shared_ptr<AppExecFwk::PacMap> GetAdditionalData() const;

    /**
     * Sets a semantic action for this NotificationActionButton.
     * @param semanticActionButton Indicates the semantic action to add.
     * For available values, see NotificationConstant::SemanticActionButton.
     */
    void SetSemanticActionButton(NotificationConstant::SemanticActionButton semanticActionButton);

    /**
     * Obtains the semantic action of this NotificationActionButton.
     * @return the semantic action of this NotificationActionButton,
     * as enumerated in NotificationConstant::SemanticActionButton.
     */
    NotificationConstant::SemanticActionButton GetSemanticActionButton() const;

    /**
     * Adds a NotificationUserInput object that only allows values of particular MIME types.
     * @param userInput Indicates the NotificationUserInput object to add.
     */
    void AddMimeTypeOnlyUserInput(const std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * Obtains the NotificationUserInput objects that only allow values of particular MIME types
     * when this NotificationActionButton is sent.
     * @return the list of NotificationUserInput objects allowing only values of particular MIME types.
     */
    std::vector<std::shared_ptr<NotificationUserInput>> GetMimeTypeOnlyUserInputs() const;

    /**
     * Adds a NotificationUserInput object used to collect user input.
     * @param userInput Indicates the NotificationUserInput object to add.
     */
    void AddNotificationUserInput(const std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * Obtains the NotificationUserInput objects to be collected from the user when this NotificationActionButton
     * is sent.
     * @return the list of NotificationUserInput objects.
     */
    std::vector<std::shared_ptr<NotificationUserInput>> GetUserInputs() const;

    /**
     * Sets whether to allow the platform to automatically generate possible replies and add them to
     * NotificationUserInput::getOptions().
     * @param autoCreatedReplies Specifies whether to allow the platform to automatically generate possible replies.
     * The value true means to allow generated replies; and the value false means not.
     */
    void SetAutoCreatedReplies(bool autoCreatedReplies);

    /**
     * Checks whether the platform can automatically generate possible replies for this NotificationActionButton.
     * @return true if the platform can generate replies; returns false otherwise.
     */
    bool IsAutoCreatedReplies() const;

    /**
     * Sets whether this NotificationActionButton is a contextual action, that is, whether this
     * NotificationActionButton is dependent on the notification message body. For example, a contextual
     * NotificationActionButton provides an address in the notification for users to open a map application.
     * @param isContextual Specifies whether this NotificationActionButton is a contextual action.
     * The value true indicates a contextual action, and the value false indicates not.
     */
    void SetContextDependent(bool isContextual);

    /**
     * Checks whether this NotificationActionButton is a contextual action, that is, whether this
     * NotificationActionButton is dependent on the notification message body. For example, a contextual
     * NotificationActionButton provides an address in the notification for users to open a map application.
     * @return true if this NotificationActionButton is a contextual action; returns false otherwise.
     */
    bool IsContextDependent() const;

    /**
     * Returns a string representation of the object.
     * @return a string representation of the object.
     */
    std::string Dump();

    /**
     * Marshal a object into a Parcel.
     * @param parcel the object into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal object from a Parcel.
     * @return the NotificationActionButton
     */
    static NotificationActionButton *Unmarshalling(Parcel &parcel);

private:
    /**
     * Default constructor used to create a NotificationActionButton instance.
     */
    NotificationActionButton() = default;

    /**
     * A constructor used to create a NotificationActionButton instance with the input parameters passed.
     * @param icon Indicates the icon to represent this NotificationActionButton.
     * @param title Indicates the title of this NotificationActionButton.
     * @param wantAgent Indicates the WantAgent to be triggered when this NotificationActionButton is triggered.
     * @param extras Indicates the PacMap object containing the additional data.
     * @param semanticActionButton Indicates the semantic action to add.
     * @param autoCreatedReplies Indicates whether to allow the platform to automatically generate possible replies.
     * @param mimeTypeOnlyInputs Indicates the NotificationUserInput object to add which allows only values of
     * particular MIME types.
     * @param userInputs Indicates the NotificationUserInput object to add.
     * @param isContextual Indicates whether this NotificationActionButton is a contextual action.
     */
    NotificationActionButton(const std::shared_ptr<Media::PixelMap> &icon, const std::string &title,
        const std::shared_ptr<WantAgent::WantAgent> &wantAgent, const std::shared_ptr<AppExecFwk::PacMap> &extras,
        NotificationConstant::SemanticActionButton semanticActionButton, bool autoCreatedReplies,
        const std::vector<std::shared_ptr<NotificationUserInput>> &mimeTypeOnlyInputs,
        const std::vector<std::shared_ptr<NotificationUserInput>> &userInputs, bool isContextual);

    /**
     * Read a NotificationActionButton object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    std::shared_ptr<Media::PixelMap> icon_ {};
    std::string title_ {};
    std::shared_ptr<WantAgent::WantAgent> wantAgent_ {};
    std::shared_ptr<AppExecFwk::PacMap> extras_ {};
    NotificationConstant::SemanticActionButton semanticActionButton_ {
        NotificationConstant::SemanticActionButton::NONE_ACTION_BUTTON
    };
    bool autoCreatedReplies_ {true};
    std::vector<std::shared_ptr<NotificationUserInput>> mimeTypeOnlyUserInputs_ {};
    std::vector<std::shared_ptr<NotificationUserInput>> userInputs_ {};
    bool isContextual_ {false};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_ACTION_BUTTON_H