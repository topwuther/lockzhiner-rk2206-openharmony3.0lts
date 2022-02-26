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
#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_MESSAGE_USER_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_MESSAGE_USER_H

#include <string>
#include "pixel_map.h"
#include "parcel.h"
#include "uri.h"

namespace OHOS {
namespace Notification {
class MessageUser final : public Parcelable {
public:
    /**
     * A constructor used to construct MessageUser
     */
    MessageUser();

    /**
     * Default deconstructor used to deconstruct.
     */
    ~MessageUser();

    /**
     * Sets the key used to uniquely identify this MessageUser.
     * @note Sets the key used to uniquely identify this MessageUser.
     *       If no key is set, the name set by SetName(string) is used to uniquely identify a MessageUser.
     * @param key Indicates the key to set.
     */
    void SetKey(const std::string &key);

    /**
     * Obtains the key of this MessageUser.
     *
     * @return Returns the key of this MessageUser.
     */
    std::string GetKey() const;

    /**
     * Sets the name of this MessageUser.
     * @name Indicates the name to set.
     */
    void SetName(const std::string &name);

    /**
     * Obtains the name of this MessageUser.
     *
     * @return Returns the name of this MessageUser.
     */
    std::string GetName() const;

    /**
     * Sets the pixel map of this MessageUser.
     * @param pixelMap Indicates the pixel map to set.
     */
    void SetPixelMap(const std::shared_ptr<Media::PixelMap> &pixelMap);

    /**
     * Obtains the pixel map of this MessageUser.
     *
     * @return Returns the pixel map of this MessageUser.
     */
    const std::shared_ptr<Media::PixelMap> GetPixelMap() const;

    /**
     * Sets the URI of this MessageUser.
     * @param uri Indicates the URI to set.
     */
    void SetUri(const Uri &uri);

    /**
     * Obtains the URI of this MessageUser.
     *
     * @return Returns the URI of this MessageUser.
     */
    Uri GetUri() const;

    /**
     * Sets whether this MessageUser is a machine.
     * @param machine Specifies whether this MessageUser is a machine.
     *                The value true indicates that it is, and the value false indicates not.
     */
    void SetMachine(bool machine);

    /**
     * Checks whether this MessageUser is a machine.
     *
     * @return Returns true if this MessageUser is a machine; returns false otherwise.
     */
    bool IsMachine() const;

    /**
     * Sets whether this MessageUser is important.
     * @note This method can be used to denote users who frequently interact with the user of this device.
     * @param userImportant Specifies whether this MessageUser is important.
     *                      The value true indicates that it is important, and the value false indicates not.
     */
    void SetUserAsImportant(bool userImportant);

    /**
     * Checks whether this MessageUser is important.
     *
     * @return Returns true if this MessageUser is important; returns false otherwise.
     */
    bool IsUserImportant() const;

    /**
     * Dumps a string representation of the object.
     *
     * @return A string representation of the object.
     */
    std::string Dump() const;

    /**
     * Marshals a MessageUser object into a Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshals a MessageUser object from a Parcel.
     *
     * @param parcel Indicates the Parcel object for unmarshalling.
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static MessageUser *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read NotificationSlot object from a Parcel.
     *
     * @param parcel the parcel
     * @return read from parcel success or fail
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    std::string key_{};
    std::string name_{};
    std::shared_ptr<Media::PixelMap> pixelMap_{nullptr};
    Uri uri_;
    bool isMachine_{false};
    bool isUserImportant_{false};

    // no object in parcel
    static constexpr int VALUE_NULL = -1;
    // object exist in parcel
    static constexpr int VALUE_OBJECT = 1;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_MESSAGE_USER_H