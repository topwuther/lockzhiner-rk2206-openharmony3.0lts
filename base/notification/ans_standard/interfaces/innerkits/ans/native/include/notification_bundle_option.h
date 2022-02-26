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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_BUNDLE_OPTION_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_BUNDLE_OPTION_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationBundleOption : public Parcelable {
public:
    /**
     * A default constructor used to create a NotificationBundleOption instance.
     */
    NotificationBundleOption() = default;

    /**
     * A constructor used to create a NotificationBundleOption instance based on the creator bundle name and uid.
     * @param bundleName Indicates the creator bundle name.
     * @param uid Indicates the creator uid.
     */
    NotificationBundleOption(const std::string &bundleName, const int32_t uid);

    /**
     * Default deconstructor used to deconstruct.
     */
    virtual ~NotificationBundleOption();

    /**
     * Sets the creator bundle name.
     * @param bundleName Indicates the creator bundle name.
     */
    void SetBundleName(const std::string &bundleName);

    /**
     * Obtains the creator bundle name.
     * @return the creator bundle name.
     */
    std::string GetBundleName() const;

    /**
     * Sets the creator uid.
     * @param uid Indicates the creator uid.
     */
    void SetUid(const int32_t uid);

    /**
     * Obtains the creator uid.
     * @return the creator uid.
     */
    int32_t GetUid() const;

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
     * @return the NotificationBundleOption
     */
    static NotificationBundleOption *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read data from a Parcel.
     * @param parcel the parcel
     * @return true if read success; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    std::string bundleName_{};
    int32_t uid_{};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_CONTENT_H
