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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_PUBLISH_INFO_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_PUBLISH_INFO_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace EventFwk {
class CommonEventPublishInfo : public Parcelable {
public:
    /**
     * Default constructor used to create an empty CommonEventPublishInfo instance.
     */
    CommonEventPublishInfo();

    /**
     * A constructor used to create a CommonEventPublishInfo instance by copying
     * parameters from an existing one.
     *
     */
    explicit CommonEventPublishInfo(const CommonEventPublishInfo &commonEventPublishInfo);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~CommonEventPublishInfo();

    /**
     * Set whether the type of a common event is sticky or not.
     *
     * @param sticky the type of a common event is sticky or not
     */
    void SetSticky(bool sticky);

    /**
     * Obtain whether it is a sticky common event, which can be set
     * by SetSticky(bool).
     *
     * @return the common event is sticky or not.
     */
    bool IsSticky() const;

    /**
     * Set permissions for subscribers.
     *
     * @param subscriberPermissions the permissions for subscribers
     */
    void SetSubscriberPermissions(const std::vector<std::string> &subscriberPermissions);

    /**
     *
     * Obtain subscriber permissions to a common event, which can be set by
     * setSubscriberPermissions(const std::vector<std::string>&).
     *
     * @return the permissions for subscribers
     */
    const std::vector<std::string> &GetSubscriberPermissions() const;

    /**
     * Set whether the type of a common event is ordered or not.
     *
     * @param ordered the type of a common event is ordered or not
     */
    void SetOrdered(bool ordered);

    /**
     *
     * Obtain whether it is an ordered common event, which can be set by setOrdered(boolean).
     * set by SetOrdered(bool).
     *
     * @return the type of a common event is ordered or not
     */
    bool IsOrdered() const;

    /**
     * Marshal a CommonEventData object into a Parcel.
     *
     * @param parcel the common event data into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * UnMarshal a Parcel object into a CommonEventData.
     *
     * @return the common event data
     */
    static CommonEventPublishInfo *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read a CommonEventData object from a Parcel.
     *
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    bool sticky_;
    bool ordered_;
    std::vector<std::string> subscriberPermissions_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_PUBLISH_INFO_H