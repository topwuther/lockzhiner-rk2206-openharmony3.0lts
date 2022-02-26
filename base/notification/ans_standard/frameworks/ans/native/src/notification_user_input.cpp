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

#include "notification_user_input.h"

#include <new>
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
void NotificationUserInput::SetInputsSource(AAFwk::Want &want, NotificationConstant::InputsSource source)
{
    want.SetParam(NotificationConstant::EXTRA_INPUTS_SOURCE, static_cast<int32_t>(source));
}

NotificationConstant::InputsSource NotificationUserInput::GetInputsSource(const AAFwk::Want &want)
{
    auto inputsSource = want.GetIntParam(NotificationConstant::EXTRA_INPUTS_SOURCE,
        static_cast<int32_t>(NotificationConstant::InputsSource::FREE_FORM_INPUT));
    return static_cast<NotificationConstant::InputsSource>(inputsSource);
}

void NotificationUserInput::AddInputsToWant(const std::vector<std::shared_ptr<NotificationUserInput>> &userInputs,
    AAFwk::Want &want, const AppExecFwk::PacMap &pacMap)
{}

std::shared_ptr<AppExecFwk::PacMap> NotificationUserInput::GetInputsFromWant(const AAFwk::Want &want)
{
    return {};
}

void NotificationUserInput::AddMimeInputToWant(const NotificationUserInput &userInput, AAFwk::Want &want,
    const std::map<std::string, std::shared_ptr<Uri>> &results)
{}

std::map<std::string, std::shared_ptr<Uri>> NotificationUserInput::GetMimeInputsFromWant(
    const AAFwk::Want &want, const std::string &inputKey)
{
    return {};
}

std::shared_ptr<NotificationUserInput> NotificationUserInput::Create(const std::string &inputKey)
{
    if (inputKey.empty()) {
        ANS_LOGE("The param of inputKey is empty");
        return {};
    }

    auto pUserInput = new (std::nothrow) NotificationUserInput(inputKey);
    if (pUserInput == nullptr) {
        ANS_LOGE("create NotificationUserInput object failed");
        return {};
    }

    return std::shared_ptr<NotificationUserInput>(pUserInput);
}

std::shared_ptr<NotificationUserInput> NotificationUserInput::Create(const std::string &inputKey,
    const std::string &tag, const std::vector<std::string> &options, bool permitFreeFormInput,
    const std::set<std::string> &permitMimeTypes, const std::shared_ptr<AppExecFwk::PacMap> &pacMap,
    NotificationConstant::InputEditType editType)
{
    if (inputKey.empty()) {
        ANS_LOGE("The param of inputKey is empty");
        return {};
    }

    if (!permitFreeFormInput) {
        if (editType == NotificationConstant::InputEditType::EDIT_ENABLED) {
            ANS_LOGE("Setting editType to enable requires permitFreeFormInput to be set to true");
            return {};
        }

        if (options.empty() && permitMimeTypes.empty()) {
            ANS_LOGE("options and permitMimeTypes cannot be empty at the same time");
            return {};
        }
    }

    auto realPacMap = pacMap;
    if (!realPacMap) {
        realPacMap = std::make_shared<AppExecFwk::PacMap>();
    }

    auto pUserInput = new (std::nothrow)
        NotificationUserInput(inputKey, tag, options, permitFreeFormInput, permitMimeTypes, realPacMap, editType);
    if (pUserInput == nullptr) {
        ANS_LOGE("create NotificationUserInput object failed");
        return {};
    }

    return std::shared_ptr<NotificationUserInput>(pUserInput);
}

NotificationUserInput::NotificationUserInput(const std::string &inputKey)
    : inputKey_(inputKey), pacMap_(std::make_shared<AppExecFwk::PacMap>())
{}

NotificationUserInput::NotificationUserInput(const std::string &inputKey, const std::string &tag,
    const std::vector<std::string> &options, bool permitFreeFormInput, const std::set<std::string> &permitMimeTypes,
    const std::shared_ptr<AppExecFwk::PacMap> &pacMap, NotificationConstant::InputEditType editType)
    : inputKey_(inputKey),
      tag_(tag),
      options_(options),
      permitFreeFormInput_(permitFreeFormInput),
      permitMimeTypes_(permitMimeTypes),
      pacMap_(pacMap),
      editType_(editType)
{}

std::string NotificationUserInput::GetInputKey() const
{
    return inputKey_;
}

void NotificationUserInput::AddAdditionalData(AppExecFwk::PacMap &pacMap)
{
    pacMap_->PutAll(pacMap);
}

const std::shared_ptr<AppExecFwk::PacMap> NotificationUserInput::GetAdditionalData() const
{
    return pacMap_;
}

void NotificationUserInput::SetEditType(NotificationConstant::InputEditType inputEditType)
{
    editType_ = inputEditType;
}

NotificationConstant::InputEditType NotificationUserInput::GetEditType() const
{
    return editType_;
}

void NotificationUserInput::SetOptions(const std::vector<std::string> &options)
{
    options_ = options;
}

std::vector<std::string> NotificationUserInput::GetOptions() const
{
    return options_;
}

void NotificationUserInput::SetPermitMimeTypes(const std::string &mimeType, bool doPermit)
{
    if (mimeType.empty()) {
        ANS_LOGE("The mimeType is invalid.");
        return;
    }

    if (doPermit) {
        permitMimeTypes_.emplace(mimeType);
        return;
    }

    permitMimeTypes_.erase(mimeType);
}

std::set<std::string> NotificationUserInput::GetPermitMimeTypes() const
{
    return permitMimeTypes_;
}

bool NotificationUserInput::IsMimeTypeOnly() const
{
    return !permitFreeFormInput_ && options_.empty() && !permitMimeTypes_.empty();
}

void NotificationUserInput::SetTag(const std::string tag)
{
    tag_ = tag;
}

std::string NotificationUserInput::GetTag() const
{
    return tag_;
}

void NotificationUserInput::SetPermitFreeFormInput(bool permitFreeFormInput)
{
    permitFreeFormInput_ = permitFreeFormInput;
}

bool NotificationUserInput::IsPermitFreeFormInput() const
{
    return permitFreeFormInput_;
}

std::string NotificationUserInput::Dump()
{
    std::string options;
    for (std::string option : options_) {
        options += option + ", ";
    }
    options.pop_back();
    options.pop_back();

    std::string permitMimeTypes;
    for (auto permitMimeType : permitMimeTypes_) {
        permitMimeTypes += permitMimeType + ", ";
    }
    permitMimeTypes.pop_back();
    permitMimeTypes.pop_back();

    return "NotificationUserInput[ inputKey = " + inputKey_ + " tag = " + tag_ +
           "options = [ " + options + " ]" +
           " permitFreeFormInput = " + (permitFreeFormInput_ ? "true" : "false") +
           " permitMimeTypes = [ " + permitMimeTypes + " ]" +
           " editType = " + std::to_string(static_cast<int32_t>(editType_)) + " ]";
}

bool NotificationUserInput::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(inputKey_)) {
        ANS_LOGE("Failed to write inputKey");
        return false;
    }

    if (!parcel.WriteString(tag_)) {
        ANS_LOGE("Failed to write tag");
        return false;
    }

    if (!parcel.WriteBool(permitFreeFormInput_)) {
        ANS_LOGE("Failed to write flag permitFreeFormInput");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(editType_))) {
        ANS_LOGE("Failed to write editType");
        return false;
    }

    auto valid = pacMap_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        ANS_LOGE("Failed to write the flag which indicate whether pacMap is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(pacMap_.get())) {
            ANS_LOGE("Failed to write pacMap");
            return false;
        }
    }

    if (!parcel.WriteStringVector(options_)) {
        ANS_LOGE("Failed to write options");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(permitMimeTypes_.size()))) {
        ANS_LOGE("Failed to write the size of permitMimeTypes");
        return false;
    }

    for (auto it = permitMimeTypes_.begin(); it != permitMimeTypes_.end(); ++it) {
        if (!parcel.WriteString(*it)) {
            ANS_LOGE("Failed to write permitMimeTypes");
            return false;
        }
    }

    return true;
}

NotificationUserInput *NotificationUserInput::Unmarshalling(Parcel &parcel)
{
    auto pUserInput = new NotificationUserInput();
    if ((nullptr != pUserInput) && !pUserInput->ReadFromParcel(parcel)) {
        delete pUserInput;
        pUserInput = nullptr;
    }

    return pUserInput;
}

bool NotificationUserInput::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(inputKey_)) {
        ANS_LOGE("Failed to read inputKey");
        return false;
    }

    if (!parcel.ReadString(tag_)) {
        ANS_LOGE("Failed to read tag");
        return false;
    }

    permitFreeFormInput_ = parcel.ReadBool();

    editType_ = static_cast<NotificationConstant::InputEditType>(parcel.ReadInt32());

    auto valid = parcel.ReadBool();
    if (valid) {
        pacMap_ = std::shared_ptr<AppExecFwk::PacMap>(parcel.ReadParcelable<AppExecFwk::PacMap>());
        if (!pacMap_) {
            ANS_LOGE("Failed to read pacMap");
            return false;
        }
    }

    if (!parcel.ReadStringVector(&options_)) {
        ANS_LOGE("Failed to read options");
        return false;
    }

    auto ssize = parcel.ReadInt32();
    for (auto it = 0; it < ssize; ++it) {
        std::string member {};
        if (!parcel.ReadString(member)) {
            ANS_LOGE("Failed to read permitMimeTypes");
            return false;
        }

        permitMimeTypes_.emplace(member);
    }

    return true;
}
}  // namespace Notification
}  // namespace OHOS
