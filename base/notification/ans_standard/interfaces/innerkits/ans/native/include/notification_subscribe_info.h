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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATIN_SUBSCRIBER_INFO_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATIN_SUBSCRIBER_INFO_H

#include <memory>
#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace Notification {

class NotificationSubscribeInfo final : public Parcelable {
public:
    /**
     * @brief Default constructor used to create a instance.
     */
    NotificationSubscribeInfo();

    /**
     * @brief Default deconstructor used to deconstruct.
     */
    ~NotificationSubscribeInfo();

    /**
     * @brief A constructor used to create a NotificationSubscribeInfo instance by copying parameters from an existing
     * one.
     *
     * @param Indicates the NotificationSubscribeInfo object.
     */
    NotificationSubscribeInfo(const NotificationSubscribeInfo &subscribeInfo);

    /**
     * @brief Sets a single application name as the filter criterion,
     * which means to subscribe to notifications of this application.
     *
     * @param appName Indicates the application name.
     **/
    void AddAppName(const std::string appName);

    /**
     * @brief Sets multiple application names as the filter criteria,
     * which means to subscribe to notifications of these applications.
     *
     * @param appNames Indicates the set of application names.
     **/
    void AddAppNames(const std::vector<std::string> &appNames);

    /**
     * @brief Obtains the application names in the current NotificationSubscribeInfo object.
     * The application names can be set by calling AddAppNames.
     *
     * @return Returns the set of application names.
     **/
    std::vector<std::string> GetAppNames() const;

    /**
     * @brief Marshals a NotificationSubscribeInfo object into a Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals a NotificationSubscribeInfo object from a Parcel.
     *
     * @param parcel Indicates the Parcel object for unmarshalling.
     */
    static NotificationSubscribeInfo *Unmarshalling(Parcel &parcel);

    /**
     * @brief Dump subscribe info.
     *
     * @return Return subscribe info.
     */
    std::string Dump();

private:
    bool ReadFromParcel(Parcel &parcel);

private:
    std::vector<std::string> appNames_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATIN_SUBSCRIBER_INFO_H