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

#include "notification_action_button.h"

#include <new>
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
std::shared_ptr<NotificationActionButton> NotificationActionButton::Create(const std::shared_ptr<Media::PixelMap> &icon,
    const std::string &title, const std::shared_ptr<WantAgent::WantAgent> &wantAgent,
    const std::shared_ptr<AppExecFwk::PacMap> &extras, NotificationConstant::SemanticActionButton semanticActionButton,
    bool autoCreatedReplies, const std::vector<std::shared_ptr<NotificationUserInput>> &mimeTypeOnlyInputs,
    const std::vector<std::shared_ptr<NotificationUserInput>> &userInputs, bool isContextual)
{
    if (isContextual && (!icon || !wantAgent)) {
        ANS_LOGE("icon or wantAgent can not be null when isContextual is true");
        return {};
    }

    auto realExtras = extras;
    if (!realExtras) {
        realExtras = std::make_shared<AppExecFwk::PacMap>();

        if (!realExtras) {
            ANS_LOGE("create PacMap object failed");
            return {};
        }
    }

    std::vector<std::shared_ptr<NotificationUserInput>> onlyInputs = mimeTypeOnlyInputs;
    std::vector<std::shared_ptr<NotificationUserInput>> textInputs {};
    for (auto &userInput : userInputs) {
        if (!userInput) {
            continue;
        }

        if (userInput->IsMimeTypeOnly()) {
            onlyInputs.push_back(userInput);
        } else {
            textInputs.push_back(userInput);
        }
    }

    auto pActionButton = new (std::nothrow) NotificationActionButton(icon,
        title,
        wantAgent,
        realExtras,
        semanticActionButton,
        autoCreatedReplies,
        onlyInputs,
        textInputs,
        isContextual);
    if (pActionButton == nullptr) {
        ANS_LOGE("create NotificationActionButton object failed");
        return {};
    }

    return std::shared_ptr<NotificationActionButton>(pActionButton);
}

std::shared_ptr<NotificationActionButton> NotificationActionButton::Create(
    const std::shared_ptr<NotificationActionButton> &actionButton)
{
    return NotificationActionButton::Create(actionButton->GetIcon(),
        actionButton->GetTitle(),
        actionButton->GetWantAgent(),
        actionButton->GetAdditionalData(),
        actionButton->GetSemanticActionButton(),
        actionButton->IsAutoCreatedReplies(),
        actionButton->GetMimeTypeOnlyUserInputs(),
        actionButton->GetUserInputs(),
        actionButton->IsContextDependent());
}

NotificationActionButton::NotificationActionButton(const std::shared_ptr<Media::PixelMap> &icon,
    const std::string &title, const std::shared_ptr<WantAgent::WantAgent> &wantAgent,
    const std::shared_ptr<AppExecFwk::PacMap> &extras, NotificationConstant::SemanticActionButton semanticActionButton,
    bool autoCreatedReplies, const std::vector<std::shared_ptr<NotificationUserInput>> &mimeTypeOnlyInputs,
    const std::vector<std::shared_ptr<NotificationUserInput>> &userInputs, bool isContextual)
    : icon_(icon),
      title_(title),
      wantAgent_(wantAgent),
      extras_(extras),
      semanticActionButton_(semanticActionButton),
      autoCreatedReplies_(autoCreatedReplies),
      mimeTypeOnlyUserInputs_(mimeTypeOnlyInputs),
      userInputs_(userInputs),
      isContextual_(isContextual)
{}

const std::shared_ptr<Media::PixelMap> NotificationActionButton::GetIcon() const
{
    return icon_;
}

std::string NotificationActionButton::GetTitle() const
{
    return title_;
}

const std::shared_ptr<WantAgent::WantAgent> NotificationActionButton::GetWantAgent() const
{
    return wantAgent_;
}

void NotificationActionButton::AddAdditionalData(AppExecFwk::PacMap &pacMap)
{
    extras_->PutAll(pacMap);
}

const std::shared_ptr<AppExecFwk::PacMap> NotificationActionButton::GetAdditionalData() const
{
    return extras_;
}

void NotificationActionButton::SetSemanticActionButton(NotificationConstant::SemanticActionButton semanticActionButton)
{
    semanticActionButton_ = semanticActionButton;
}

NotificationConstant::SemanticActionButton NotificationActionButton::GetSemanticActionButton() const
{
    return semanticActionButton_;
}

void NotificationActionButton::AddMimeTypeOnlyUserInput(const std::shared_ptr<NotificationUserInput> &userInput)
{
    if (!userInput) {
        ANS_LOGE("The userInput is invalid.");
        return;
    }

    if (!userInput->IsMimeTypeOnly()) {
        ANS_LOGE("The userInput is not a particular MIME types.");
        return;
    }

    mimeTypeOnlyUserInputs_.emplace_back(userInput);
}

std::vector<std::shared_ptr<NotificationUserInput>> NotificationActionButton::GetMimeTypeOnlyUserInputs() const
{
    return mimeTypeOnlyUserInputs_;
}

void NotificationActionButton::AddNotificationUserInput(const std::shared_ptr<NotificationUserInput> &userInput)
{
    if (!userInput) {
        ANS_LOGE("The userInput is invalid.");
        return;
    }

    userInputs_.emplace_back(userInput);
}

std::vector<std::shared_ptr<NotificationUserInput>> NotificationActionButton::GetUserInputs() const
{
    return userInputs_;
}

void NotificationActionButton::SetAutoCreatedReplies(bool autoCreatedReplies)
{
    autoCreatedReplies_ = autoCreatedReplies;
}

bool NotificationActionButton::IsAutoCreatedReplies() const
{
    return autoCreatedReplies_;
}

void NotificationActionButton::SetContextDependent(bool isContextual)
{
    isContextual_ = isContextual;
}

bool NotificationActionButton::IsContextDependent() const
{
    return isContextual_;
}

std::string NotificationActionButton::Dump()
{
    return "NotificationActionButton[ title = " + title_ +
           ", semanticActionButton = " + std::to_string(static_cast<int32_t>(semanticActionButton_)) +
           ", autoCreatedReplies = " + (autoCreatedReplies_ ? "true" : "false") +
           ", isContextual = " + (isContextual_ ? "true" : "false") + " ]";
}

bool NotificationActionButton::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(title_)) {
        ANS_LOGE("Failed to write title");
        return false;
    }

    if (!parcel.WriteBool(autoCreatedReplies_)) {
        ANS_LOGE("Failed to write flag autoCreatedReplies");
        return false;
    }

    if (!parcel.WriteBool(isContextual_)) {
        ANS_LOGE("Failed to write flag isContextual");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(semanticActionButton_))) {
        ANS_LOGE("Failed to write semanticActionButton");
        return false;
    }

    bool valid {false};

    valid = icon_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        ANS_LOGE("Failed to write the flag which indicate whether icon is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(icon_.get())) {
            ANS_LOGE("Failed to write icon");
            return false;
        }
    }

    valid = wantAgent_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        ANS_LOGE("Failed to write the flag which indicate whether wantAgent is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(wantAgent_.get())) {
            ANS_LOGE("Failed to write wantAgent");
            return false;
        }
    }

    valid = extras_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        ANS_LOGE("Failed to write the flag which indicate whether extras is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(extras_.get())) {
            ANS_LOGE("Failed to write extras");
            return false;
        }
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(userInputs_.size()))) {
        ANS_LOGE("Failed to write the size of userInputs");
        return false;
    }

    for (auto it = userInputs_.begin(); it != userInputs_.end(); ++it) {
        valid = (*it) ? true : false;
        if (!parcel.WriteBool(valid)) {
            ANS_LOGE("Failed to write the flag which indicate whether userInput is null");
            return false;
        }

        if (valid) {
            if (!parcel.WriteParcelable(it->get())) {
                ANS_LOGE("Failed to write userInput");
                return false;
            }
        }
    }

    return true;
}

NotificationActionButton *NotificationActionButton::Unmarshalling(Parcel &parcel)
{
    auto pButton = new NotificationActionButton();
    if ((nullptr != pButton) && !pButton->ReadFromParcel(parcel)) {
        delete pButton;
        pButton = nullptr;
    }

    return pButton;
}

bool NotificationActionButton::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(title_)) {
        ANS_LOGE("Failed to read title");
        return false;
    }

    autoCreatedReplies_ = parcel.ReadBool();

    isContextual_ = parcel.ReadBool();

    semanticActionButton_ = static_cast<NotificationConstant::SemanticActionButton>(parcel.ReadInt32());

    bool valid {false};

    valid = parcel.ReadBool();
    if (valid) {
        icon_ = std::shared_ptr<Media::PixelMap>(parcel.ReadParcelable<Media::PixelMap>());
        if (!icon_) {
            ANS_LOGE("Failed to read icon");
            return false;
        }
    }

    valid = parcel.ReadBool();
    if (valid) {
        wantAgent_ = std::shared_ptr<WantAgent::WantAgent>(parcel.ReadParcelable<WantAgent::WantAgent>());
        if (!wantAgent_) {
            ANS_LOGE("Failed to read wantAgent");
            return false;
        }
    }

    valid = parcel.ReadBool();
    if (valid) {
        extras_ = std::shared_ptr<AppExecFwk::PacMap>(parcel.ReadParcelable<AppExecFwk::PacMap>());
        if (!extras_) {
            ANS_LOGE("Failed to read extras");
            return false;
        }
    }

    auto vsize = parcel.ReadInt32();
    for (auto it = 0; it < vsize; ++it) {
        valid = parcel.ReadBool();
        NotificationUserInput *member {nullptr};
        if (valid) {
            member = parcel.ReadParcelable<NotificationUserInput>();
            if (nullptr == member) {
                ANS_LOGE("Failed to read userInput");
                return false;
            }
        }

        userInputs_.emplace_back(member);
    }

    return true;
}
}  // namespace Notification
}  // namespace OHOS
