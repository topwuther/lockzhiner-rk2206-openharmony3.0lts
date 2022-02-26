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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_USER_INPUT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_USER_INPUT_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "notification_constant.h"
#include "pac_map.h"
#include "want.h"
#include "parcel.h"
#include "uri.h"

namespace OHOS {
namespace Notification {
class NotificationUserInput : public Parcelable {
public:
    /**
     * Sets the input source of this NotificationUserInput object.
     * @param want Indicates the Want to which the input result is to be added.
     * @param source Indicates the input source. For available values, see NotificationConstant::InputSource.
     */
    static void SetInputsSource(AAFwk::Want &want, NotificationConstant::InputsSource source);

    /**
     * Obtains the input source of this NotificationUserInput object.
     * @param want Indicates the Want containing the input result.
     * @return the input source of this NotificationUserInput object,
     * as enumerated in NotificationConstant::InputSource.
     */
    static NotificationConstant::InputsSource GetInputsSource(const AAFwk::Want &want);

    /**
     * Adds a list of NotificationUserInput objects to a Want. This method should only be called by user input
     * collection services when sending input results to an WantAgent.
     * @param userInputs Indicates the list of NotificationUserInput objects for which the input results are provided.
     * @param want Indicates the Want to which the input results are to be added.
     * @param pacMap Indicates the PacMap object holding the input results.
     * The PacMap key must match the keys contained in the objects specified in userInputs.
     */
    static void AddInputsToWant(const std::vector<std::shared_ptr<NotificationUserInput>> &userInputs,
        AAFwk::Want &want, const AppExecFwk::PacMap &pacMap);

    /**
     * Obtains the input text results from a specified Want.
     * @param want Indicates the Want object containing one or more user input results.
     * @return the PacMap object containing the input text results.
     */
    static std::shared_ptr<AppExecFwk::PacMap> GetInputsFromWant(const AAFwk::Want &want);

    /**
     * Adds the given NotificationUserInput object of a specified MIME type to an Want. This method is similar to
     * addInputsToWant(NotificationUserInput, Want, PacMap) except that the MIME type must be specified.
     * @param userInput Indicates the NotificationUserInput object for which the input results are provided.
     * @param want Indicates the Want to which the input results are to be added.
     * @param results Indicates a map containing the MIME type and its URI result.
     */
    static void AddMimeInputToWant(const NotificationUserInput &userInput, AAFwk::Want &want,
        const std::map<std::string, std::shared_ptr<Uri>> &results);

    /**
     * Obtains the input results of a particular MIME type from a specified Want. This method is similar to
     * GetInputsFromWant(Want) but uses the key of the NotificationUserInput.
     * @param want Indicates the Want object containing one or more user input results.
     * @param inputKey Indicates the key of the NotificationUserInput results to obtain.
     * @return a map containing the MIME type and its URI result.
     */
    static std::map<std::string, std::shared_ptr<Uri>> GetMimeInputsFromWant(
        const AAFwk::Want &want, const std::string &inputKey);

public:
    /**
     * A static function used to create a NotificationUserInput instance with the input parameters passed.
     * @param inputKey Indicates the PacMap key used to identify this input when the input is collected from the user.
     * @return the shared_ptr object owns the created NotificationUserInput object otherwise return empty object if
     * inputKey is empty.
     */
    static std::shared_ptr<NotificationUserInput> Create(const std::string &inputKey);

    /**
     * A static function used to create a NotificationUserInput instance with the input parameters passed.
     * @param inputKey Indicates the PacMap key used to identify this input when the input is collected from the user.
     * @param tag Indicates the tag to be displayed.
     * @param options Indicates the list of predefined options to be provided for user input.
     * @param permitFreeFormInput Specifies whether to allow arbitrary text values. The default value is true,
     * indicating that arbitrary text values are allowed. If this parameter is set to false, you must pass a non-empty
     * std::vector of options or a non-empty std::set of permitMimeTypes. Otherwise, return empty object.
     * @param permitMimeTypes Indicates the MIME type allowed.
     * @param pacMap Indicates the PacMap object containing the additional data.
     * @param editType Indicates the edit type to set. For details about available values, see
     * NotificationConstant::InputEditType.
     * @return the shared_ptr object owns the created NotificationUserInput object otherwise return empty object if
     * permitFreeFormInput is false but editType is InputEditType::EDIT_ENABLED.
     */
    static std::shared_ptr<NotificationUserInput> Create(const std::string &inputKey, const std::string &tag,
        const std::vector<std::string> &options, bool permitFreeFormInput, const std::set<std::string> &permitMimeTypes,
        const std::shared_ptr<AppExecFwk::PacMap> &pacMap, NotificationConstant::InputEditType editType);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationUserInput() = default;

    /**
     * Obtains the key of this NotificationUserInput object.
     * @return the key of this NotificationUserInput object.
     */
    std::string GetInputKey() const;

    /**
     * Adds additional data to this Builder.
     * The value of pacMap will replace the existing extras value in this Builder.
     * @param pacMap Indicates the PacMap object containing the additional data.
     */
    void AddAdditionalData(AppExecFwk::PacMap &pacMap);

    /**
     * Obtains the additional data included in this NotificationUserInput object.
     * @return the additional data in this NotificationUserInput object.
     */
    const std::shared_ptr<AppExecFwk::PacMap> GetAdditionalData() const;

    /**
     * Sets the edit type of the options provided by this NotificationUserInput object.
     * The edit type determines whether an option can be edited when the user taps the option.
     * @param inputEditType Indicates the edit type to set. For details about available values,
     * see NotificationConstant::InputEditType.
     */
    void SetEditType(NotificationConstant::InputEditType inputEditType);

    /**
     * Obtains the edit type of the options provided by this NotificationUserInput object.
     * @return the edit type of options.
     */
    NotificationConstant::InputEditType GetEditType() const;

    /**
     * Sets the options provided to users to satisfy user input needs. All options are displayed in a single line.
     * Due to limited space, some or all of the options may be hidden if the options are too long or
     * there are too many options.
     * @param options Indicates the list of predefined options to be provided for user input.
     */
    void SetOptions(const std::vector<std::string> &options);

    /**
     * Obtains the options provided to the user to satisfy user input needs.
     * @return the list of predefined options.
     */
    std::vector<std::string> GetOptions() const;

    /**
     * Sets whether users can input values of the given MIME type.
     * @param mimeType Indicates the MIME type allowed.
     * @param doPermit Specifies whether to allow the given MIME type.
     * The value true means to allow it, and the value false means not.
     */
    void SetPermitMimeTypes(const std::string &mimeType, bool doPermit);

    /**
     * Obtains the MIME types allowed by this NotificationUserInput object.
     * @return the list of allowed MIME types.
     */
    std::set<std::string> GetPermitMimeTypes() const;

    /**
     * Checks whether this NotificationUserInput object allows only values of particular MIME types.
     * @return true if this object allows only values of particular MIME types; returns false otherwise.
     */
    bool IsMimeTypeOnly() const;

    /**
     * Sets the tag to be displayed to the user when collecting this input from the user.
     * @param tag Indicates the tag to be displayed.
     */
    void SetTag(const std::string tag);

    /**
     * Obtains the tag to be displayed to the user when collecting this input from the user.
     * @return the tag to be displayed to the user.
     */
    std::string GetTag() const;

    /**
     * Sets whether users can input arbitrary text values.
     * @param permitFreeFormInput Specifies whether to allow arbitrary text values. The default value is true,
     * indicating that arbitrary text values are allowed. If this parameter is set to false, you must either call
     * SetOptions(std::vector<std::string>) to set a non-null and non-empty list or call
     * SetPermitMimeTypes(std::string, bool) to allow a MIME type.
     */
    void SetPermitFreeFormInput(bool permitFreeFormInput);

    /**
     * Checks whether users can provide arbitrary values for this NotificationUserInput object.
     * @return true if this object allows arbitrary values; returns false otherwise.
     */
    bool IsPermitFreeFormInput() const;

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
     * @return the NotificationUserInput
     */
    static NotificationUserInput *Unmarshalling(Parcel &parcel);

private:
    /**
     * Default constructor used to create a NotificationUserInput instance.
     */
    NotificationUserInput() = default;

    /**
     * A constructor used to create a NotificationUserInput instance with the input parameters passed.
     * @param inputKey Indicates the PacMap key used to identify this input when the input is collected from the user.
     */
    explicit NotificationUserInput(const std::string &inputKey);

    /**
     * A constructor used to create a NotificationUserInput instance with the input parameters passed.
     * @param inputKey Indicates the PacMap key used to identify this input when the input is collected from the user.
     * @param tag Indicates the tag to be displayed.
     * @param options Indicates the list of predefined options to be provided for user input.
     * @param permitFreeFormInput Specifies whether to allow arbitrary text values. The default value is true,
     * indicating that arbitrary text values are allowed. If this parameter is set to false, you must either call
     * setOptions(const std::vector<std::string> &) to set a non-empty list or call
     * setPermitMimeTypes(std::string, bool) to allow a MIME type.
     * @param permitMimeTypes Indicates the MIME type allowed.
     * @param pacMap Indicates the PacMap object containing the additional data.
     * @param editType Indicates the edit type to set. For details about available values, see
     * NotificationConstant::InputEditType.
     */
    NotificationUserInput(const std::string &inputKey, const std::string &tag, const std::vector<std::string> &options,
        bool permitFreeFormInput, const std::set<std::string> &permitMimeTypes,
        const std::shared_ptr<AppExecFwk::PacMap> &pacMap, NotificationConstant::InputEditType editType);

    /**
     * Read a NotificationUserInput object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    std::string inputKey_ {};
    std::string tag_ {};
    std::vector<std::string> options_ {};
    bool permitFreeFormInput_ {true};
    std::set<std::string> permitMimeTypes_ {};
    std::shared_ptr<AppExecFwk::PacMap> pacMap_ {};
    NotificationConstant::InputEditType editType_ {NotificationConstant::InputEditType::EDIT_AUTO};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_USER_INPUT_H
