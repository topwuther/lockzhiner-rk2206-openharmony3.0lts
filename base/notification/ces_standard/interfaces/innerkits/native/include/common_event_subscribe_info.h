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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBE_INFO_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBE_INFO_H

#include "matching_skills.h"

namespace OHOS {
namespace EventFwk {
class CommonEventSubscribeInfo : public Parcelable {
public:
    enum ThreadMode {
        HANDLER,     // the main thread of this ability.
        POST,        // the event dispatch thread.
        ASYNC,       // an asynchronous thread.
        BACKGROUND,  // the background thread.
    };

    /**
     * A constructor used to create a CommonEventSubscribeInfo instance
     * with the matchingSkills parameters passed.
     *
     * @param matchingSkills the matching skills
     */
    explicit CommonEventSubscribeInfo(const MatchingSkills &matchingSkills);

    /**
     * A constructor used to create a CommonEventSubscribeInfo instance
     *
     */
    CommonEventSubscribeInfo();

    /**
     * A constructor used to create a CommonEventSubscribeInfo instance by
     * copying parameters from an existing one.
     *
     *  @param commonEventSubscribeInfo the commonEventSubscribeInfo
     */
    explicit CommonEventSubscribeInfo(const CommonEventSubscribeInfo &commonEventSubscribeInfo);

    /**
     * A deconstructor used to deconstruct
     *
     */
    ~CommonEventSubscribeInfo();

    /**
     * Set the subscriber priority for this CommonEventSubscribeInfo object.
     *
     * @param priority the subscriber priority
     */
    void SetPriority(const int32_t &priority);

    /**
     * Obtain the subscriber priority of this CommonEventSubscribeInfo object.
     *
     * @return the subscriber priority
     */
    int32_t GetPriority() const;

    /**
     * Set the permission that the publisher must have in order to send
     * a common event to this subscriber.
     *
     * @param permission the subscriber permission
     */
    void SetPermission(const std::string &permission);

    /**
     * Obtain the publisher permission of this CommonEventSubscribeInfo object.
     *
     * @return the subscriber permission
     */
    std::string GetPermission() const;

    /**
     * Obtain the thread mode of this CommonEventSubscribeInfo object.
     *
     * @return the thread mode
     */
    CommonEventSubscribeInfo::ThreadMode GetThreadMode() const;

    /**
     * Set the thread mode of this CommonEventSubscribeInfo object.
     *
     * @param threadMode the thread mode to be set, which is specified in ThreadMode. Currently, only the HANDLER mode
     * is supported.
     */
    void SetThreadMode(CommonEventSubscribeInfo::ThreadMode threadMode);

    /**
     * Set the device ID for this CommonEventSubscribeInfo object.
     * Your application will only receive common events sent from the specified device.
     *
     * @param deviceId Indicates the device ID. The value must be an existing device ID on the same ohos network.
     * Otherwise, it is invalid.
     */
    void SetDeviceId(const std::string &deviceId);

    /**
     * Obtain the device ID in this CommonEventSubscribeInfo object.
     *
     * @return the device ID.
     */
    std::string GetDeviceId() const;

    /**
     * Obtain the MatchingSkills object carried in this CommonEventSubscribeInfo object.
     *
     * @return the matchingSkills
     */
    const MatchingSkills &GetMatchingSkills() const;

    /**
     * Marshal a subscriber info object into a Parcel.
     *
     * @param parcel the common event data into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal a Parcel object into a subscriber info.
     *
     * @return subscriber info.
     */
    static CommonEventSubscribeInfo *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read a subscriber object from a Parcel.
     *
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    MatchingSkills matchingSkills_;
    int32_t priority_;
    std::string permission_;
    std::string deviceId_;
    CommonEventSubscribeInfo::ThreadMode threadMode_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBE_INFO_H
