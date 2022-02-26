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

#include "ans_manager_proxy.h"
#include "ans_const_define.h"
#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "message_option.h"
#include "message_parcel.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {

AnsManagerProxy::AnsManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAnsManager>(impl)
{}

AnsManagerProxy::~AnsManagerProxy()
{}

ErrCode AnsManagerProxy::Publish(const std::string &label, const sptr<NotificationRequest> &notification)
{
    if (notification == nullptr) {
        ANS_LOGW("[Publish] fail: notification is null ptr.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[Publish] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(label)) {
        ANS_LOGW("[Publish] fail: write label failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[Publish] fail: write notification parcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(PUBLISH_NOTIFICATION, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[Publish] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[Publish] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::PublishToDevice(const sptr<NotificationRequest> &notification, const std::string &deviceId)
{
    if (notification == nullptr) {
        ANS_LOGW("[PublishToDevice] fail: notification is null ptr.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[PublishToDevice] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[PublishToDevice] fail: write notification parcelable failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(deviceId)) {
        ANS_LOGW("[PublishToDevice] fail: write deviceId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(PUBLISH_NOTIFICATION_TO_DEVICE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[PublishToDevice] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[PublishToDevice] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::Cancel(int notificationId, const std::string &label)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[Cancel] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(notificationId)) {
        ANS_LOGW("[Cancel] fail: write notificationId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(label)) {
        ANS_LOGW("[Cancel] fail: write label failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(CANCEL_NOTIFICATION, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[Cancel] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[Cancel] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::CancelAll()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[CancelAll] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(CANCEL_ALL_NOTIFICATIONS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[CancelAll] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[CancelAll] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::AddSlotByType(NotificationConstant::SlotType slotType)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[AddSlotByType] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(slotType)) {
        ANS_LOGW("[AddSlotByType] fail:: write slotIds failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(ADD_SLOT_BY_TYPE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[AddSlotByType] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[AddSlotByType] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::AddSlots(const std::vector<sptr<NotificationSlot>> &slots)
{
    if (slots.empty()) {
        ANS_LOGW("[AddSlots] fail: slots is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    uint32_t slotsSize = slots.size();
    if (slotsSize > MAX_SLOT_NUM) {
        ANS_LOGW("[AddSlots] fail: slotsSize over max size.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[AddSlots] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!WriteParcelableVector(slots, data)) {
        ANS_LOGW("[AddSlots] fail: write slots failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(ADD_SLOTS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[AddSlots] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[AddSlots] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::RemoveSlotByType(const NotificationConstant::SlotType &slotType)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[RemoveSlotByType] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(slotType)) {
        ANS_LOGW("[RemoveSlotByType] fail:: write slotIds failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(REMOVE_SLOT_BY_TYPE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[RemoveSlotByType] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[RemoveSlotByType] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::RemoveAllSlots()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[RemoveAllSlots] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(REMOVE_ALL_SLOTS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[RemoveAllSlots] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[RemoveAllSlots] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::AddSlotGroups(std::vector<sptr<NotificationSlotGroup>> groups)
{
    if (groups.empty()) {
        ANS_LOGW("[AddSlotGroups] fail: groups is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    uint32_t groupsSize = groups.size();
    if (groupsSize > MAX_SLOT_GROUP_NUM) {
        ANS_LOGW("[AddSlotGroups] fail: groupsSize over max size.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[AddSlotGroups] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!WriteParcelableVector(groups, data)) {
        ANS_LOGW("[AddSlotGroups] fail: write groups failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(ADD_SLOT_GROUPS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[AddSlotGroups] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[AddSlotGroups] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSlotByType(const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSlotByType] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(slotType)) {
        ANS_LOGW("[GetSlotByType] fail:: write slotId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SLOT_BY_TYPE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetSlotByType] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetSlotByType] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    slot = reply.ReadParcelable<NotificationSlot>();
    if (slot == nullptr) {
        ANS_LOGW("[GetSlotByType] fail: read slot failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSlots(std::vector<sptr<NotificationSlot>> &slots)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSlots] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SLOTS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetSlots] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!ReadParcelableVector(slots, reply, result)) {
        ANS_LOGW("[GetSlots] fail: read slots failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group)
{
    if (groupId.empty()) {
        ANS_LOGW("[GetSlotGroup] fail: groupId is null.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSlotGroup] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(groupId)) {
        ANS_LOGW("[GetSlotGroup] fail:: write groupId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SLOT_GROUP, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetSlotGroup] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetSlotGroup] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    group = reply.ReadParcelable<NotificationSlotGroup>();
    if (group == nullptr) {
        ANS_LOGW("[GetSlotGroup] fail: read group failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSlotGroups] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SLOT_GROUPS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetSlotGroups] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!ReadParcelableVector(groups, reply, result)) {
        ANS_LOGW("[GetSlotGroups] fail: read groups failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSlotNumAsBundle(const sptr<NotificationBundleOption> &bundleOption, int &num)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[GetSlotNumAsBundle] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSlotNumAsBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteStrongParcelable(bundleOption)) {
        ANS_LOGW("[GetSlotNumAsBundle] fail:: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SLOT_NUM_AS_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadInt32(num)) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::RemoveSlotGroups(const std::vector<std::string> &groupIds)
{
    if (groupIds.empty()) {
        ANS_LOGW("[RemoveSlotGroups] fail: groupIds is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[RemoveSlotGroups] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteStringVector(groupIds)) {
        ANS_LOGW("[RemoveSlotGroups] fail:: write groupIds failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(REMOVE_SLOT_GROUPS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[RemoveSlotGroups] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[RemoveSlotGroups] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetActiveNotifications(std::vector<sptr<NotificationRequest>> &notifications)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetActiveNotifications] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_ACTIVE_NOTIFICATIONS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetActiveNotifications] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!ReadParcelableVector(notifications, reply, result)) {
        ANS_LOGW("[GetActiveNotifications] fail: read notifications failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetActiveNotificationNums(int &num)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetActiveNotificationNums] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_ACTIVE_NOTIFICATION_NUMS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetActiveNotificationNums] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetActiveNotificationNums] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadInt32(num)) {
        ANS_LOGW("[GetActiveNotificationNums] fail: read notification num failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetAllActiveNotifications(std::vector<sptr<Notification>> &notifications)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetAllActiveNotifications] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_ALL_ACTIVE_NOTIFICATIONS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetAllActiveNotifications] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!ReadParcelableVector(notifications, reply, result)) {
        ANS_LOGW("[GetAllActiveNotifications] fail: read notifications failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSpecialActiveNotifications(
    const std::vector<std::string> &key, std::vector<sptr<Notification>> &notifications)
{
    if (key.empty()) {
        ANS_LOGW("[GetSpecialActiveNotifications] fail: key is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSpecialActiveNotifications] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteStringVector(key)) {
        ANS_LOGW("[GetSpecialActiveNotifications] fail:: write key failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SPECIAL_ACTIVE_NOTIFICATIONS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetSpecialActiveNotifications] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!ReadParcelableVector(notifications, reply, result)) {
        ANS_LOGW("[GetSpecialActiveNotifications] fail: read notifications failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetNotificationAgent(const std::string &agent)
{
    if (agent.empty()) {
        ANS_LOGW("[SetNotificationAgent] fail: agent is null.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetNotificationAgent] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(agent)) {
        ANS_LOGW("[SetNotificationAgent] fail:: write agent failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_NOTIFICATION_AGENT, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetNotificationAgent] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetNotificationAgent] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetNotificationAgent(std::string &agent)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetNotificationAgent] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_NOTIFICATION_AGENT, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetNotificationAgent] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetNotificationAgent] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadString(agent)) {
        ANS_LOGW("[GetNotificationAgent] fail: read agent failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::CanPublishAsBundle(const std::string &representativeBundle, bool &canPublish)
{
    if (representativeBundle.empty()) {
        ANS_LOGW("[CanPublishAsBundle] fail: representativeBundle is null.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[CanPublishAsBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(representativeBundle)) {
        ANS_LOGW("[CanPublishAsBundle] fail: write representativeBundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(CAN_PUBLISH_AS_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[CanPublishAsBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[CanPublishAsBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(canPublish)) {
        ANS_LOGW("[CanPublishAsBundle] fail: read canPublish failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::PublishAsBundle(
    const sptr<NotificationRequest> notification, const std::string &representativeBundle)
{
    if (notification == nullptr) {
        ANS_LOGW("[PublishAsBundle] fail: notification is null ptr.");
        return ERR_ANS_INVALID_PARAM;
    }

    if (representativeBundle.empty()) {
        ANS_LOGW("[PublishAsBundle] fail: representativeBundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[PublishAsBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[PublishAsBundle] fail: write notification failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(representativeBundle)) {
        ANS_LOGW("[PublishAsBundle] fail: write representativeBundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(PUBLISH_AS_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[PublishAsBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[PublishAsBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetNotificationBadgeNum(int num)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetNotificationBadgeNum] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(num)) {
        ANS_LOGW("[SetNotificationBadgeNum] fail: write num failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_NOTIFICATION_BADGE_NUM, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetNotificationBadgeNum] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetNotificationBadgeNum] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetBundleImportance(int &importance)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetBundleImportance] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_BUNDLE_IMPORTANCE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetBundleImportance] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetBundleImportance] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadInt32(importance)) {
        ANS_LOGW("[GetBundleImportance] fail: read importance failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetDisturbMode(NotificationConstant::DisturbMode mode)
{
    if ((mode < NotificationConstant::DisturbMode::ALLOW_ALARMS) ||
        (mode > NotificationConstant::DisturbMode::ALLOW_UNKNOWN)) {
        ANS_LOGW("[SetDisturbMode] fail: input mode is not in DisturbMode.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetDisturbMode] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(mode)) {
        ANS_LOGW("[SetDisturbMode] fail: write mode failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_DISTURB_MODE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetDisturbMode] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetDisturbMode] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetDisturbMode(NotificationConstant::DisturbMode &mode)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetDisturbMode] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_DISTURB_MODE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetDisturbMode] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetDisturbMode] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    mode = static_cast<NotificationConstant::DisturbMode>(reply.ReadInt32());

    return result;
}

ErrCode AnsManagerProxy::HasNotificationPolicyAccessPermission(bool &granted)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[HasNotificationPolicyAccessPermission] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(IS_NOTIFICATION_POLICY_ACCESS_GRANTED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[HasNotificationPolicyAccessPermission] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[HasNotificationPolicyAccessPermission] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(granted)) {
        ANS_LOGW("[HasNotificationPolicyAccessPermission] fail: read granted failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetPrivateNotificationsAllowed(bool allow)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetPrivateNotificationsAllowed] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(allow)) {
        ANS_LOGW("[SetPrivateNotificationsAllowed] fail: write allow failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_PRIVATIVE_NOTIFICATIONS_ALLOWED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetPrivateNotificationsAllowed] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetPrivateNotificationsAllowed] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetPrivateNotificationsAllowed(bool &allow)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetPrivateNotificationsAllowed] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_PRIVATIVE_NOTIFICATIONS_ALLOWED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetPrivateNotificationsAllowed] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetPrivateNotificationsAllowed] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(allow)) {
        ANS_LOGW("[GetPrivateNotificationsAllowed] fail: read allow failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::RemoveNotification(
    const sptr<NotificationBundleOption> &bundleOption, int notificationId, const std::string &label)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[RemoveNotification] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[RemoveNotification] fail:, write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteStrongParcelable(bundleOption)) {
        ANS_LOGW("[RemoveNotification] fail:: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(notificationId)) {
        ANS_LOGW("[RemoveNotification] fail: write notificationId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(label)) {
        ANS_LOGW("[RemoveNotification] fail: write label failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(REMOVE_NOTIFICATION, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[RemoveNotification] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[RemoveNotification] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::RemoveAllNotifications(const sptr<NotificationBundleOption> &bundleOption)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[RemoveAllNotifications] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[RemoveAllNotifications] fail:, write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteStrongParcelable(bundleOption)) {
        ANS_LOGW("[RemoveAllNotifications] fail:: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(REMOVE_ALL_NOTIFICATIONS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[RemoveNotification] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[RemoveNotification] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::Delete(const std::string &key)
{
    if (key.empty()) {
        ANS_LOGW("[Delete] fail: key is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[Delete] fail:, write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(key)) {
        ANS_LOGW("[Delete] fail:: write key failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(DELETE_NOTIFICATION, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[Delete] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[Delete] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::DeleteByBundle(const sptr<NotificationBundleOption> &bundleOption)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[DeleteByBundle] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[DeleteByBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteStrongParcelable(bundleOption)) {
        ANS_LOGW("[DeleteByBundle] fail: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(DELETE_NOTIFICATION_BY_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[DeleteByBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[DeleteByBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::DeleteAll()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[DeleteAll] fail:, write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(DELETE_ALL_NOTIFICATIONS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[DeleteAll] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[DeleteAll] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetSlotsByBundle(
    const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[GetSlotsByBundle] fail: bundleOption is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetSlotsByBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[GetSlotsByBundle] fail:: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SLOTS_BY_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetSlotsByBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!ReadParcelableVector(slots, reply, result)) {
        ANS_LOGW("[GetSlotsByBundle] fail: read slots failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::UpdateSlots(
    const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[UpdateSlots] fail: bundleOption is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    if (slots.empty()) {
        ANS_LOGW("[UpdateSlots] fail: slots is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    uint32_t slotSize = slots.size();
    if (slotSize > MAX_SLOT_NUM) {
        ANS_LOGW("[UpdateSlots] fail: slotSize over max size.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[UpdateSlots] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[UpdateSlots] fail:: write bundleoption failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!WriteParcelableVector(slots, data)) {
        ANS_LOGW("[UpdateSlots] fail: write slots failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(UPDATE_SLOTS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[UpdateSlots] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[UpdateSlots] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::UpdateSlotGroups(
    const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[UpdateSlotGroups] fail: bundleOption is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    if (groups.empty()) {
        ANS_LOGW("[UpdateSlotGroups] fail: groups is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    uint32_t groupSize = groups.size();
    if (groupSize > MAX_SLOT_GROUP_NUM) {
        ANS_LOGW("[UpdateSlotGroups] fail: groupSize over max size.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[UpdateSlotGroups] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[UpdateSlotGroups] fail:: write bundleOption failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!WriteParcelableVector(groups, data)) {
        ANS_LOGW("[UpdateSlotGroups] fail: write groups failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(UPDATE_SLOT_GROUPS, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[UpdateSlotGroups] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[UpdateSlotGroups] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetNotificationsEnabledForBundle(const std::string &deviceId, bool enabled)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetNotificationsEnabledForBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(deviceId)) {
        ANS_LOGW("[SetNotificationsEnabledForBundle] fail: write deviceId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(enabled)) {
        ANS_LOGW("[SetNotificationsEnabledForBundle] fail: write enabled failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_NOTIFICATION_ENABLED_FOR_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetNotificationsEnabledForBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetNotificationsEnabledForBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetNotificationsEnabledForAllBundles] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(deviceId)) {
        ANS_LOGW("[SetNotificationsEnabledForAllBundles] fail: write deviceId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(enabled)) {
        ANS_LOGW("[SetNotificationsEnabledForAllBundles] fail: write enabled failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_NOTIFICATION_ENABLED_FOR_ALL_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetNotificationsEnabledForAllBundles] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetNotificationsEnabledForAllBundles] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetNotificationsEnabledForSpecialBundle(
    const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: bundleOption is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(deviceId)) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: write deviceId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: write bundleOption failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(enabled)) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: write enabled failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_NOTIFICATION_ENABLED_FOR_SPECIAL_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetNotificationsEnabledForSpecialBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::SetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool enabled)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[SetShowBadgeEnabledForBundle] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[SetShowBadgeEnabledForBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[SetShowBadgeEnabledForBundle] fail:: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(enabled)) {
        ANS_LOGW("[SetShowBadgeEnabledForBundle] fail:: write enabled failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SET_SHOW_BADGE_ENABLED_FOR_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[SetShowBadgeEnabledForBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[SetShowBadgeEnabledForBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool &enabled)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail:: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SHOW_BADGE_ENABLED_FOR_BUNDLE, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(enabled)) {
        ANS_LOGW("[GetShowBadgeEnabledForBundle] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetShowBadgeEnabled(bool &enabled)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetShowBadgeEnabled] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_SHOW_BADGE_ENABLED, option, data, reply);

    if (result != ERR_OK) {
        ANS_LOGW("[GetShowBadgeEnabled] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetShowBadgeEnabled] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(enabled)) {
        ANS_LOGW("[GetShowBadgeEnabled] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::Subscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info)
{
    if (subscriber == nullptr) {
        ANS_LOGW("[Subscribe] fail: subscriber is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[Subscribe] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool ret = data.WriteRemoteObject(subscriber->AsObject());
    if (!ret) {
        ANS_LOGW("[Subscribe] fail: write subscriber failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(info != nullptr)) {
        ANS_LOGW("[Subscribe] fail: write isSubcribeInfo failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (info != nullptr) {
        if (!data.WriteParcelable(info)) {
            ANS_LOGW("[Subscribe] fail: write subcribeInfo failed");
            return ERR_ANS_PARCELABLE_FAILED;
        }
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SUBSCRIBE_NOTIFICATION, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[Subscribe] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[Subscribe] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::Unsubscribe(
    const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info)
{
    if (subscriber == nullptr) {
        ANS_LOGW("[Unsubscribe] fail: subscriber is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[Unsubscribe] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool ret = data.WriteRemoteObject(subscriber->AsObject());
    if (!ret) {
        ANS_LOGW("[Unsubscribe] fail: write subscriber failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(info != nullptr)) {
        ANS_LOGW("[Unsubscribe] fail: write isSubcribeInfo failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (info != nullptr) {
        if (!data.WriteParcelable(info)) {
            ANS_LOGW("[Unsubscribe] fail: write subcribeInfo failed");
            return ERR_ANS_PARCELABLE_FAILED;
        }
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(UNSUBSCRIBE_NOTIFICATION, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[Unsubscribe] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[Unsubscribe] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::AreNotificationsSuspended(bool &suspended)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[AreNotificationsSuspended] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(ARE_NOTIFICATION_SUSPENDED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[AreNotificationsSuspended] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[AreNotificationsSuspended] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(suspended)) {
        ANS_LOGW("[AreNotificationsSuspended] fail: read suspended failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[GetCurrentAppSorting] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(GET_CURRENT_APP_SORTING, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[GetCurrentAppSorting] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[GetCurrentAppSorting] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sortingMap = reply.ReadParcelable<NotificationSortingMap>();
    if (sortingMap == nullptr) {
        ANS_LOGW("[GetCurrentAppSorting] fail: read sortingMap failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::IsAllowedNotify(bool &allowed)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[IsAllowedNotify] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(IS_ALLOWED_NOTIFY, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[IsAllowedNotify] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[IsAllowedNotify] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(allowed)) {
        ANS_LOGW("[IsAllowedNotify] fail: read allowed failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::IsSpecialBundleAllowedNotify(const sptr<NotificationBundleOption> &bundleOption, bool &allowed)
{
    if (bundleOption == nullptr) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: bundle is empty.");
        return ERR_ANS_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(bundleOption)) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: write bundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(IS_SPECIAL_BUNDLE_ALLOWED_NOTIFY, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadBool(allowed)) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: read allowed failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsManagerProxy::GetDescriptor())) {
        ANS_LOGW("[ShellDump] fail: write interface token failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!data.WriteString(dumpOption)) {
        ANS_LOGW("[ShellDump] fail: write option failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(SHELL_DUMP, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[ShellDump] fail: transact ErrCode=%{public}d", result);
        return ERR_ANS_TRANSACT_FAILED;
    }

    if (!reply.ReadInt32(result)) {
        ANS_LOGW("[ShellDump] fail: read result failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.ReadStringVector(&dumpInfo)) {
        ANS_LOGW("[ShellDump] fail: read dumpInfo failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return result;
}

ErrCode AnsManagerProxy::InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        ANS_LOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            ANS_LOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            ANS_LOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_ANS_TRANSACT_FAILED;
        }
    }
}

template<typename T>
bool AnsManagerProxy::WriteParcelableVector(const std::vector<sptr<T>> &parcelableVector, MessageParcel &data)
{
    if (!data.WriteInt32(parcelableVector.size())) {
        ANS_LOGW("write ParcelableVector size failed");
        return false;
    }

    for (auto &parcelable : parcelableVector) {
        if (!data.WriteStrongParcelable(parcelable)) {
            ANS_LOGW("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}

template<typename T>
bool AnsManagerProxy::ReadParcelableVector(std::vector<sptr<T>> &parcelableInfos, MessageParcel &reply, ErrCode &result)
{
    if (!reply.ReadInt32(result)) {
        ANS_LOGW("read result failed.");
        return false;
    }

    int32_t infoSize = 0;
    if (!reply.ReadInt32(infoSize)) {
        ANS_LOGW("read Parcelable size failed.");
        return false;
    }

    parcelableInfos.clear();
    for (int32_t index = 0; index < infoSize; index++) {
        sptr<T> info = reply.ReadStrongParcelable<T>();
        if (info == nullptr) {
            ANS_LOGW("read Parcelable infos failed.");
            return false;
        }
        parcelableInfos.emplace_back(info);
    }

    return true;
}

}  // namespace Notification
}  // namespace OHOS
