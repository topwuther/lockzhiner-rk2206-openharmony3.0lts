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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SLOT_GROUP_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SLOT_GROUP_H

#include <string>

#include "notification_slot.h"

namespace OHOS {
namespace Notification {
class NotificationSlotGroup : public Parcelable {
public:

    /**
     * A constructor used to create a NotificationSlotGroup instance with the group ID and name initialized.
     *
     * @param id Indicates the ID of the NotificationSlotGroup.
     *           The ID must be unique and its length must not exceed 1000 characters (the excess part is automatically
     * truncated).
     * @param name Indicates the name of the NotificationSlotGroup.
     *             Its length must not exceed 1000 characters (the excess part is automatically truncated).
     */
    NotificationSlotGroup(const std::string &id, const std::string &name);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationSlotGroup();

    /**
     * Obtains the description of this NotificationSlotGroup object, which is set by SetDescription(string).
     *
     * @return Returns the description of this NotificationSlotGroup.
     */
    std::string GetDescription() const;

    /**
     * Obtains the ID of this NotificationSlotGroup object, which is set by NotificationSlotGroup(string, string).
     *
     * @return Returns the ID of this NotificationSlotGroup.
     */
    std::string GetId() const;

    /**
     * Obtains the name of this NotificationSlotGroup object, which is set by NotificationSlotGroup(string, string).
     *
     * @return Returns the name of this NotificationSlotGroup.
     */
    std::string GetName() const;

    /**
     * Obtains a list of notification slots bound to this NotificationSlotGroup object.
     * @note NotificationSlot::SetSlotGroup(string) is used to bind a notification slot to a notification slot group.
     * @param slots Indicates the slots which to set to the slotgroup.
     *
     */
    void SetSlots(const std::vector<NotificationSlot> &slots);

    /**
     * Sets a list of notification slots bound to this NotificationSlotGroup object.
     * @note NotificationSlot::SetSlotGroup(string) is used to bind a notification slot to a notification slot group.
     *
     * @return Returns the list of notification slots bound to this NotificationSlotGroup.
     */
    std::vector<NotificationSlot> GetSlots() const;

    /**
     * Checks whether this NotificationSlotGroup is disabled.
     *
     * @return Returns true if this NotificationSlotGroup is disabled; returns false otherwise.
     */
    bool IsDisabled() const;

    /**
     * Sets the description for this NotificationSlotGroup object.
     *
     * @param description Describes this NotificationSlotGroup object.
     *                    Its length must not exceed 1000 characters (the excess part is automatically truncated).
     */
    void SetDescription(const std::string &description);

    /**
     * Dumps a string representation of the object.
     *
     * @return A string representation of the object.
     */
    std::string Dump() const;

    /**
     * Marshals NotificationSlotGroup objects and writes them into Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshals a NotificationSlotGroup object from a Parcel.
     *
     * @param parcel Indicates the Parcel object for unmarshalling.
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static NotificationSlotGroup *Unmarshalling(Parcel &parcel);

    /**
     * If string length exceed 1000 characters, the excessive part is automatically truncated.
     *
     * @param truncatedString The sting which will be truncated
     * @return Returns the string that has been truncated.
     */
    std::string TruncateString(const std::string &inPutString);

private:

    /**
     * Default constructor used to create a NotificationSlotGroup instance.
     */
    NotificationSlotGroup();

    /**
     * Read NotificationSlotGroup object from a Parcel.
     *
     * @param parcel the parcel
     * @return read from parcel success or fail
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    std::string id_{};
    std::string name_{};
    std::string description_{};
    std::vector<NotificationSlot> slots_{};
    bool isDisabled_{false};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SLOT_GROUP_H