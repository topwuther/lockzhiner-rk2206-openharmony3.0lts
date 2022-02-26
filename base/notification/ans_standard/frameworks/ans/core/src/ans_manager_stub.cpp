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

#include "ans_manager_stub.h"
#include "ans_const_define.h"
#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "message_option.h"
#include "message_parcel.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {

const std::map<uint32_t, std::function<ErrCode(AnsManagerStub *, MessageParcel &, MessageParcel &)>>
    AnsManagerStub::interfaces_ = {
        {AnsManagerStub::PUBLISH_NOTIFICATION,
            std::bind(
                &AnsManagerStub::HandlePublish, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::PUBLISH_NOTIFICATION_TO_DEVICE,
            std::bind(&AnsManagerStub::HandlePublishToDevice, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::CANCEL_NOTIFICATION,
            std::bind(
                &AnsManagerStub::HandleCancel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::CANCEL_ALL_NOTIFICATIONS,
            std::bind(
                &AnsManagerStub::HandleCancelAll, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::ADD_SLOT_BY_TYPE,
            std::bind(&AnsManagerStub::HandleAddSlotByType, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::ADD_SLOTS,
            std::bind(
                &AnsManagerStub::HandleAddSlots, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::REMOVE_SLOT_BY_TYPE,
            std::bind(&AnsManagerStub::HandleRemoveSlotByType, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::REMOVE_ALL_SLOTS,
            std::bind(&AnsManagerStub::HandleRemoveAllSlots, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::ADD_SLOT_GROUPS,
            std::bind(&AnsManagerStub::HandleAddSlotGroups, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SLOT_BY_TYPE,
            std::bind(&AnsManagerStub::HandleGetSlotByType, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SLOTS,
            std::bind(
                &AnsManagerStub::HandleGetSlots, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::GET_SLOT_GROUP,
            std::bind(&AnsManagerStub::HandleGetSlotGroup, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SLOT_GROUPS,
            std::bind(&AnsManagerStub::HandleGetSlotGroups, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SLOT_NUM_AS_BUNDLE,
            std::bind(&AnsManagerStub::HandleGetSlotNumAsBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::REMOVE_SLOT_GROUPS,
            std::bind(&AnsManagerStub::HandleRemoveSlotGroups, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_ACTIVE_NOTIFICATIONS,
            std::bind(&AnsManagerStub::HandleGetActiveNotifications, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_ACTIVE_NOTIFICATION_NUMS,
            std::bind(&AnsManagerStub::HandleGetActiveNotificationNums, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_ALL_ACTIVE_NOTIFICATIONS,
            std::bind(&AnsManagerStub::HandleGetAllActiveNotifications, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SPECIAL_ACTIVE_NOTIFICATIONS,
            std::bind(&AnsManagerStub::HandleGetSpecialActiveNotifications, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::SET_NOTIFICATION_AGENT,
            std::bind(&AnsManagerStub::HandleSetNotificationAgent, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_NOTIFICATION_AGENT,
            std::bind(&AnsManagerStub::HandleGetNotificationAgent, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::CAN_PUBLISH_AS_BUNDLE,
            std::bind(&AnsManagerStub::HandleCanPublishAsBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::PUBLISH_AS_BUNDLE,
            std::bind(&AnsManagerStub::HandlePublishAsBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::SET_NOTIFICATION_BADGE_NUM,
            std::bind(&AnsManagerStub::HandleSetNotificationBadgeNum, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_BUNDLE_IMPORTANCE,
            std::bind(&AnsManagerStub::HandleGetBundleImportance, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::SET_DISTURB_MODE,
            std::bind(&AnsManagerStub::HandleSetDisturbMode, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_DISTURB_MODE,
            std::bind(&AnsManagerStub::HandleGetDisturbMode, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::IS_NOTIFICATION_POLICY_ACCESS_GRANTED,
            std::bind(&AnsManagerStub::HandleIsNotificationPolicyAccessGranted, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::SET_PRIVATIVE_NOTIFICATIONS_ALLOWED,
            std::bind(&AnsManagerStub::HandleSetPrivateNotificationsAllowed, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::GET_PRIVATIVE_NOTIFICATIONS_ALLOWED,
            std::bind(&AnsManagerStub::HandleGetPrivateNotificationsAllowed, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::REMOVE_NOTIFICATION,
            std::bind(&AnsManagerStub::HandleRemoveNotification, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::REMOVE_ALL_NOTIFICATIONS,
            std::bind(&AnsManagerStub::HandleRemoveAllNotifications, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::DELETE_NOTIFICATION,
            std::bind(
                &AnsManagerStub::HandleDelete, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::DELETE_NOTIFICATION_BY_BUNDLE,
            std::bind(&AnsManagerStub::HandleDeleteByBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::DELETE_ALL_NOTIFICATIONS,
            std::bind(
                &AnsManagerStub::HandleDeleteAll, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::GET_SLOTS_BY_BUNDLE,
            std::bind(&AnsManagerStub::HandleGetSlotsByBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::UPDATE_SLOTS,
            std::bind(&AnsManagerStub::HandleUpdateSlots, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::UPDATE_SLOT_GROUPS,
            std::bind(&AnsManagerStub::HandleUpdateSlotGroups, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::SET_NOTIFICATION_ENABLED_FOR_BUNDLE,
            std::bind(&AnsManagerStub::HandleSetNotificationsEnabledForBundle, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::SET_NOTIFICATION_ENABLED_FOR_ALL_BUNDLE,
            std::bind(&AnsManagerStub::HandleSetNotificationsEnabledForAllBundles, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::SET_NOTIFICATION_ENABLED_FOR_SPECIAL_BUNDLE,
            std::bind(&AnsManagerStub::HandleSetNotificationsEnabledForSpecialBundle, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::SET_SHOW_BADGE_ENABLED_FOR_BUNDLE,
            std::bind(&AnsManagerStub::HandleSetShowBadgeEnabledForBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SHOW_BADGE_ENABLED_FOR_BUNDLE,
            std::bind(&AnsManagerStub::HandleGetShowBadgeEnabledForBundle, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_SHOW_BADGE_ENABLED,
            std::bind(&AnsManagerStub::HandleGetShowBadgeEnabled, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::SUBSCRIBE_NOTIFICATION,
            std::bind(
                &AnsManagerStub::HandleSubscribe, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {AnsManagerStub::UNSUBSCRIBE_NOTIFICATION,
            std::bind(&AnsManagerStub::HandleUnsubscribe, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::ARE_NOTIFICATION_SUSPENDED,
            std::bind(&AnsManagerStub::HandleAreNotificationsSuspended, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::GET_CURRENT_APP_SORTING,
            std::bind(&AnsManagerStub::HandleGetCurrentAppSorting, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::IS_ALLOWED_NOTIFY,
            std::bind(&AnsManagerStub::HandleIsAllowedNotify, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::IS_SPECIAL_BUNDLE_ALLOWED_NOTIFY,
            std::bind(&AnsManagerStub::HandleIsSpecialBundleAllowedNotify, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {AnsManagerStub::SHELL_DUMP,
            std::bind(
                &AnsManagerStub::HandleShellDump, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
};

AnsManagerStub::AnsManagerStub()
{}

AnsManagerStub::~AnsManagerStub()
{}

int32_t AnsManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &flags)
{
    std::u16string descriptor = AnsManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ANS_LOGW("[OnRemoteRequest] fail: invalid interface token!");
        return OBJECT_NULL;
    }

    auto it = interfaces_.find(code);
    if (it == interfaces_.end()) {
        ANS_LOGW("[OnRemoteRequest] fail: unknown code!");
        return IRemoteStub<IAnsManager>::OnRemoteRequest(code, data, reply, flags);
    }

    auto fun = it->second;
    if (fun == nullptr) {
        ANS_LOGW("[OnRemoteRequest] fail: not find function!");
        return IRemoteStub<IAnsManager>::OnRemoteRequest(code, data, reply, flags);
    }

    ErrCode result = fun(this, data, reply);
    if (SUCCEEDED(result)) {
        return NO_ERROR;
    }

    ANS_LOGW("[OnRemoteRequest] fail: Failed to call interface %{public}u, err:%{public}d", code, result);
    return result;
}

ErrCode AnsManagerStub::HandlePublish(MessageParcel &data, MessageParcel &reply)
{
    std::string label;
    if (!data.ReadString(label)) {
        ANS_LOGW("[HandlePublish] fail: read label failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationRequest> notification = data.ReadParcelable<NotificationRequest>();
    if (!notification) {
        ANS_LOGW("[HandlePublish] fail: notification ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = Publish(label, notification);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandlePublish] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandlePublishToDevice(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationRequest> notification = data.ReadParcelable<NotificationRequest>();
    if (!notification) {
        ANS_LOGW("[HandlePublishToDevice] fail: notification ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::string deviceId;
    if (!data.ReadString(deviceId)) {
        ANS_LOGW("[HandlePublishToDevice] fail: read deviceId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = PublishToDevice(notification, deviceId);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandlePublishToDevice] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleCancel(MessageParcel &data, MessageParcel &reply)
{
    int notificationId = 0;
    if (!data.ReadInt32(notificationId)) {
        ANS_LOGW("[HandleCancel] fail: read notificationId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::string label;
    if (!data.ReadString(label)) {
        ANS_LOGW("[HandleCancel] fail: read label failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = Cancel(notificationId, label);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleCancel] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleCancelAll(MessageParcel &data, MessageParcel &reply)
{
    ErrCode result = CancelAll();

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleCancelAll] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleAddSlotByType(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::SlotType slotType = static_cast<NotificationConstant::SlotType>(data.ReadInt32());

    ErrCode result = AddSlotByType(slotType);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleAddSlotByType] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleAddSlots(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<NotificationSlot>> slots;
    if (!ReadParcelableVector(slots, data)) {
        ANS_LOGW("[HandleAddSlots] fail: read slotsSize failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = AddSlots(slots);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleAddSlots] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleRemoveSlotByType(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::SlotType slotType = static_cast<NotificationConstant::SlotType>(data.ReadInt32());

    ErrCode result = RemoveSlotByType(slotType);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleRemoveSlotByType] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleRemoveAllSlots(MessageParcel &data, MessageParcel &reply)
{
    ErrCode result = RemoveAllSlots();

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleRemoveAllSlots] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleAddSlotGroups(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<NotificationSlotGroup>> groups;
    if (!ReadParcelableVector(groups, data)) {
        ANS_LOGW("[HandleAddSlotGroups] fail: read slotsSize failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = AddSlotGroups(groups);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleAddSlotGroups] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSlots(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<NotificationSlot>> slots;
    ErrCode result = GetSlots(slots);

    if (!WriteParcelableVector(slots, reply, result)) {
        ANS_LOGW("[HandleGetSlots] fail: write slots failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSlotByType(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::SlotType slotType = static_cast<NotificationConstant::SlotType>(data.ReadInt32());

    sptr<NotificationSlot> slot;
    ErrCode result = GetSlotByType(slotType, slot);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetSlotByType] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteParcelable(slot)) {
        ANS_LOGW("[HandleGetSlotByType] fail: write slot failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSlotGroup(MessageParcel &data, MessageParcel &reply)
{
    std::string groupId;
    if (!data.ReadString(groupId)) {
        ANS_LOGW("[HandleGetSlotGroup] fail: read groupId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationSlotGroup> group;
    ErrCode result = GetSlotGroup(groupId, group);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetSlotGroup] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteParcelable(group)) {
        ANS_LOGW("[HandleGetSlotGroup] fail: write group failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSlotGroups(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<NotificationSlotGroup>> groups;
    ErrCode result = GetSlotGroups(groups);

    if (!WriteParcelableVector(groups, reply, result)) {
        ANS_LOGW("[HandleGetSlotGroups] fail: write groups failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSlotNumAsBundle(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadStrongParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleGetSlotNumAsBundle] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    int num = 0;
    ErrCode result = GetSlotNumAsBundle(bundleOption, num);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetSlotNumAsBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteInt32(num)) {
        ANS_LOGW("[HandleGetSlotNumAsBundle] fail: write enabled failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleRemoveSlotGroups(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> groupIds;
    if (!data.ReadStringVector(&groupIds)) {
        ANS_LOGW("[HandleRemoveSlotGroups] fail: read groupIds failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = RemoveSlotGroups(groupIds);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleRemoveSlotGroups] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetActiveNotifications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<NotificationRequest>> notifications;
    ErrCode result = GetActiveNotifications(notifications);

    if (!WriteParcelableVector(notifications, reply, result)) {
        ANS_LOGW("[HandleGetActiveNotifications] fail: write notifications failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetActiveNotificationNums(MessageParcel &data, MessageParcel &reply)
{
    int num = 0;
    ErrCode result = GetActiveNotificationNums(num);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetActiveNotificationNums] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteInt32(num)) {
        ANS_LOGW("[HandleGetActiveNotificationNums] fail: write num failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetAllActiveNotifications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<Notification>> notifications;
    ErrCode result = GetAllActiveNotifications(notifications);

    if (!WriteParcelableVector(notifications, reply, result)) {
        ANS_LOGW("[HandleGetAllActiveNotifications] fail: write notifications failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSpecialActiveNotifications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> key;
    if (!data.ReadStringVector(&key)) {
        ANS_LOGW("[HandleGetSpecialActiveNotifications] fail: read key failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::vector<sptr<Notification>> notifications;
    ErrCode result = GetSpecialActiveNotifications(key, notifications);

    if (!WriteParcelableVector(notifications, reply, result)) {
        ANS_LOGW("[HandleGetSpecialActiveNotifications] fail: write notifications failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetNotificationAgent(MessageParcel &data, MessageParcel &reply)
{
    std::string agent;
    if (!data.ReadString(agent)) {
        ANS_LOGW("[HandleSetNotificationAgent] fail: read agent failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetNotificationAgent(agent);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetNotificationAgent] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetNotificationAgent(MessageParcel &data, MessageParcel &reply)
{
    std::string agent;
    ErrCode result = GetNotificationAgent(agent);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetNotificationAgent] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteString(agent)) {
        ANS_LOGW("[HandleGetNotificationAgent] fail: write agent failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return ERR_OK;
}

ErrCode AnsManagerStub::HandleCanPublishAsBundle(MessageParcel &data, MessageParcel &reply)
{
    std::string representativeBundle;
    if (!data.ReadString(representativeBundle)) {
        ANS_LOGW("[HandleCanPublishAsBundle] fail: read representativeBundle failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool canPublish = false;
    ErrCode result = CanPublishAsBundle(representativeBundle, canPublish);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleCanPublishAsBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(canPublish)) {
        ANS_LOGW("[HandleCanPublishAsBundle] fail: write canPublish failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    return ERR_OK;
}

ErrCode AnsManagerStub::HandlePublishAsBundle(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationRequest> notification = data.ReadParcelable<NotificationRequest>();
    if (!notification) {
        ANS_LOGW("[HandlePublishAsBundle] fail: read notification failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::string representativeBundle;
    if (!data.ReadString(representativeBundle)) {
        ANS_LOGW("[HandlePublishAsBundle] fail: read representativeBundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = PublishAsBundle(notification, representativeBundle);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandlePublishAsBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetNotificationBadgeNum(MessageParcel &data, MessageParcel &reply)
{
    int num = 0;
    if (!data.ReadInt32(num)) {
        ANS_LOGW("[HandleSetNotificationBadgeNum] fail: read notification failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetNotificationBadgeNum(num);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetNotificationBadgeNum] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetBundleImportance(MessageParcel &data, MessageParcel &reply)
{
    int importance = 0;
    ErrCode result = GetBundleImportance(importance);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetBundleImportance] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteInt32(importance)) {
        ANS_LOGW("[HandleGetBundleImportance] fail: write importance failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetDisturbMode(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::DisturbMode mode = static_cast<NotificationConstant::DisturbMode>(data.ReadInt32());
    ErrCode result = SetDisturbMode(mode);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetDisturbMode] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetDisturbMode(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::DisturbMode mode;
    ErrCode result = GetDisturbMode(mode);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetDisturbMode] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteInt32(mode)) {
        ANS_LOGW("[HandleGetDisturbMode] fail: write mode failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleIsNotificationPolicyAccessGranted(MessageParcel &data, MessageParcel &reply)
{
    bool granted = false;
    ErrCode result = HasNotificationPolicyAccessPermission(granted);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleIsNotificationPolicyAccessGranted] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(granted)) {
        ANS_LOGW("[HandleIsNotificationPolicyAccessGranted] fail: write granted failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetPrivateNotificationsAllowed(MessageParcel &data, MessageParcel &reply)
{
    bool allow = false;
    if (!reply.ReadBool(allow)) {
        ANS_LOGW("[HandleSetPrivateNotificationsAllowed] fail: read allow failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetPrivateNotificationsAllowed(allow);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetPrivateNotificationsAllowed] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetPrivateNotificationsAllowed(MessageParcel &data, MessageParcel &reply)
{
    bool allow = false;
    ErrCode result = GetPrivateNotificationsAllowed(allow);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetPrivateNotificationsAllowed] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleRemoveNotification(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadStrongParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleRemoveNotification] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    int notificationId = 0;
    if (!data.ReadInt32(notificationId)) {
        ANS_LOGW("[HandleRemoveNotification] fail: read notificationId failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::string label;
    if (!data.ReadString(label)) {
        ANS_LOGW("[HandleRemoveNotification] fail: read label failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = RemoveNotification(bundleOption, notificationId, label);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleRemoveNotification] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleRemoveAllNotifications(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadStrongParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleRemoveAllNotifications] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = RemoveAllNotifications(bundleOption);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleRemoveAllNotifications] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleDelete(MessageParcel &data, MessageParcel &reply)
{
    std::string key;
    if (!data.ReadString(key)) {
        ANS_LOGW("[HandleDelete] fail: read key failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = Delete(key);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleDelete] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleDeleteByBundle(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadStrongParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleDeleteByBundle] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = DeleteByBundle(bundleOption);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleDeleteByBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleDeleteAll(MessageParcel &data, MessageParcel &reply)
{
    ErrCode result = DeleteAll();
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleDeleteAll] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetSlotsByBundle(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleGetSlotsByBundle] fail: read bundleOption failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::vector<sptr<NotificationSlot>> slots;
    ErrCode result = GetSlotsByBundle(bundleOption, slots);
    if (!WriteParcelableVector(slots, reply, result)) {
        ANS_LOGW("[HandleGetSlotsByBundle] fail: write slots failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleUpdateSlots(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleUpdateSlots] fail: read bundleOption failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::vector<sptr<NotificationSlot>> slots;
    if (!ReadParcelableVector(slots, data)) {
        ANS_LOGW("[HandleUpdateSlots] fail: read slots failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = UpdateSlots(bundleOption, slots);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleUpdateSlots] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleUpdateSlotGroups(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleUpdateSlotGroups] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::vector<sptr<NotificationSlotGroup>> groups;
    if (!ReadParcelableVector(groups, data)) {
        ANS_LOGW("[HandleUpdateSlotGroups] fail: read groups failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = UpdateSlotGroups(bundleOption, groups);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleUpdateSlotGroups] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetNotificationsEnabledForBundle(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId;
    if (!data.ReadString(deviceId)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForBundle] fail: read deviceId failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool enabled = false;
    if (!data.ReadBool(enabled)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForBundle] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetNotificationsEnabledForBundle(deviceId, enabled);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetNotificationsEnabledForAllBundles(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId;
    if (!data.ReadString(deviceId)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForAllBundles] fail: read deviceId failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool enabled = false;
    if (!data.ReadBool(enabled)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForAllBundles] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetNotificationsEnabledForAllBundles(deviceId, enabled);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForAllBundles] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetNotificationsEnabledForSpecialBundle(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId;
    if (!data.ReadString(deviceId)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForSpecialBundle] fail: read deviceId failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleSetNotificationsEnabledForSpecialBundle] fail: read bundleOption failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool enabled = false;
    if (!data.ReadBool(enabled)) {
        ANS_LOGW("[HandleSetNotificationsEnabledForSpecialBundle] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetNotificationsEnabledForSpecialBundle(deviceId, bundleOption, enabled);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW(
            "[HandleSetNotificationsEnabledForSpecialBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSetShowBadgeEnabledForBundle(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleSetShowBadgeEnabledForBundle] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool enabled = false;
    if (!data.ReadBool(enabled)) {
        ANS_LOGW("[HandleSetShowBadgeEnabledForBundle] fail: read enabled failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    ErrCode result = SetShowBadgeEnabledForBundle(bundleOption, enabled);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSetShowBadgeEnabledForBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetShowBadgeEnabledForBundle(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[HandleGetShowBadgeEnabledForBundle] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool enabled = false;
    ErrCode result = GetShowBadgeEnabledForBundle(bundleOption, enabled);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetShowBadgeEnabledForBundle] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(enabled)) {
        ANS_LOGW("[HandleGetShowBadgeEnabledForBundle] fail: write enabled failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetShowBadgeEnabled(MessageParcel &data, MessageParcel &reply)
{
    bool enabled = false;
    ErrCode result = GetShowBadgeEnabled(enabled);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetShowBadgeEnabled] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(enabled)) {
        ANS_LOGW("[HandleGetShowBadgeEnabled] fail: write enabled failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleSubscribe(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        ANS_LOGW("[HandleSubscribe] fail: read subscriber failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool subcribeInfo = false;
    if (!data.ReadBool(subcribeInfo)) {
        ANS_LOGW("[HandleSubscribe] fail: read isSubcribeInfo failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationSubscribeInfo> info = nullptr;
    if (subcribeInfo) {
        info = data.ReadParcelable<NotificationSubscribeInfo>();
        if (info == nullptr) {
            ANS_LOGW("[HandleSubscribe] fail: read info failed");
            return ERR_ANS_PARCELABLE_FAILED;
        }
    }

    ErrCode result = Subscribe(iface_cast<IAnsSubscriber>(subscriber), info);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleSubscribe] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleUnsubscribe(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        ANS_LOGW("[HandleUnsubscribe] fail: read subscriber failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool subcribeInfo = false;
    if (!data.ReadBool(subcribeInfo)) {
        ANS_LOGW("[HandleUnsubscribe] fail: read isSubcribeInfo failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationSubscribeInfo> info = nullptr;
    if (subcribeInfo) {
        info = data.ReadParcelable<NotificationSubscribeInfo>();
        if (info == nullptr) {
            ANS_LOGW("[HandleUnsubscribe] fail: read info failed");
            return ERR_ANS_PARCELABLE_FAILED;
        }
    }

    ErrCode result = Unsubscribe(iface_cast<IAnsSubscriber>(subscriber), info);
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleUnsubscribe] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleAreNotificationsSuspended(MessageParcel &data, MessageParcel &reply)
{
    bool suspended = false;
    ErrCode result = AreNotificationsSuspended(suspended);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleAreNotificationsSuspended] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(suspended)) {
        ANS_LOGW("[HandleAreNotificationsSuspended] fail: write suspended failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleGetCurrentAppSorting(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationSortingMap> sortingMap;
    ErrCode result = GetCurrentAppSorting(sortingMap);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetCurrentAppSorting] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteParcelable(sortingMap)) {
        ANS_LOGW("[HandleGetCurrentAppSorting] fail: write sortingMap failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleIsAllowedNotify(MessageParcel &data, MessageParcel &reply)
{
    bool allowed = false;
    ErrCode result = IsAllowedNotify(allowed);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleIsAllowedNotify] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(allowed)) {
        ANS_LOGW("[HandleIsAllowedNotify] fail: write allowed failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleIsSpecialBundleAllowedNotify(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationBundleOption> bundleOption = data.ReadParcelable<NotificationBundleOption>();
    if (bundleOption == nullptr) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: read bundle failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool allowed = false;
    ErrCode result = IsSpecialBundleAllowedNotify(bundleOption, allowed);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteBool(allowed)) {
        ANS_LOGW("[IsSpecialBundleAllowedNotify] fail: write allowed failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode AnsManagerStub::HandleShellDump(MessageParcel &data, MessageParcel &reply)
{
    std::string dumpOption;
    if (!data.ReadString(dumpOption)) {
        ANS_LOGW("[HandleShellDump] fail: read dumpOption failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    std::vector<std::string> notificationsInfo;
    ErrCode result = ShellDump(dumpOption, notificationsInfo);

    if (!reply.WriteInt32(result)) {
        ANS_LOGW("[HandleGetRecentNotificationsInfo] fail: write result failed, ErrCode=%{public}d", result);
        return ERR_ANS_PARCELABLE_FAILED;
    }

    if (!reply.WriteStringVector(notificationsInfo)) {
        ANS_LOGW("[HandleGetRecentNotificationsInfo] fail: write notificationsInfo failed.");
        return ERR_ANS_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

template<typename T>
bool AnsManagerStub::WriteParcelableVector(
    const std::vector<sptr<T>> &parcelableVector, MessageParcel &reply, ErrCode &result)
{
    if (!reply.WriteInt32(result)) {
        ANS_LOGW("write result failed, ErrCode=%{public}d", result);
        return false;
    }

    if (!reply.WriteInt32(parcelableVector.size())) {
        ANS_LOGW("write ParcelableVector size failed");
        return false;
    }

    for (auto &parcelable : parcelableVector) {
        if (!reply.WriteStrongParcelable(parcelable)) {
            ANS_LOGW("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}

template<typename T>
bool AnsManagerStub::ReadParcelableVector(std::vector<sptr<T>> &parcelableInfos, MessageParcel &data)
{
    int32_t infoSize = 0;
    if (!data.ReadInt32(infoSize)) {
        ANS_LOGW("read Parcelable size failed.");
        return false;
    }

    parcelableInfos.clear();
    for (int32_t index = 0; index < infoSize; index++) {
        sptr<T> info = data.ReadStrongParcelable<T>();
        if (info == nullptr) {
            ANS_LOGW("read Parcelable infos failed.");
            return false;
        }
        parcelableInfos.emplace_back(info);
    }

    return true;
}

ErrCode AnsManagerStub::Publish(const std::string &label, const sptr<NotificationRequest> &notification)
{
    ANS_LOGW("AnsManagerStub::Publish called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::PublishToDevice(const sptr<NotificationRequest> &notification, const std::string &deviceId)
{
    ANS_LOGW("AnsManagerStub::PublishToDevice called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::Cancel(int notificationId, const std::string &label)
{
    ANS_LOGW("AnsManagerStub::Cancel called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::CancelAll()
{
    ANS_LOGW("AnsManagerStub::CancelAll called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::AddSlotByType(NotificationConstant::SlotType slotType)
{
    ANS_LOGW("AnsManagerStub::AddSlotByType called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::AddSlots(const std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGW("AnsManagerStub::AddSlots called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::RemoveSlotByType(const NotificationConstant::SlotType &slotType)
{
    ANS_LOGW("AnsManagerStub::RemoveSlotByType called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::RemoveAllSlots()
{
    ANS_LOGW("AnsManagerStub::RemoveAllSlots called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::AddSlotGroups(std::vector<sptr<NotificationSlotGroup>> groups)
{
    ANS_LOGW("AnsManagerStub::AddSlotGroups called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSlotByType(const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot)
{
    ANS_LOGW("AnsManagerStub::GetSlotByType called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSlots(std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGW("AnsManagerStub::GetSlots called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group)
{
    ANS_LOGW("AnsManagerStub::GetSlotGroup called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups)
{
    ANS_LOGW("AnsManagerStub::GetSlotGroups called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSlotNumAsBundle(const sptr<NotificationBundleOption> &bundleOption, int &num)
{
    ANS_LOGW("AnsManagerStub::GetSlotNumAsBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::RemoveSlotGroups(const std::vector<std::string> &groupIds)
{
    ANS_LOGW("AnsManagerStub::RemoveSlotGroups called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetActiveNotifications(std::vector<sptr<NotificationRequest>> &notifications)
{
    ANS_LOGW("AnsManagerStub::GetActiveNotifications called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetActiveNotificationNums(int &num)
{
    ANS_LOGW("AnsManagerStub::GetActiveNotificationNums called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetAllActiveNotifications(std::vector<sptr<Notification>> &notifications)
{
    ANS_LOGW("AnsManagerStub::GetAllActiveNotifications called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSpecialActiveNotifications(
    const std::vector<std::string> &key, std::vector<sptr<Notification>> &notifications)
{
    ANS_LOGW("AnsManagerStub::GetSpecialActiveNotifications called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetNotificationAgent(const std::string &agent)
{
    ANS_LOGW("AnsManagerStub::SetNotificationAgent called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetNotificationAgent(std::string &agent)
{
    ANS_LOGW("AnsManagerStub::GetNotificationAgent called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::CanPublishAsBundle(const std::string &representativeBundle, bool &canPublish)
{
    ANS_LOGW("AnsManagerStub::CanPublishAsBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::PublishAsBundle(
    const sptr<NotificationRequest> notification, const std::string &representativeBundle)
{
    ANS_LOGW("AnsManagerStub::PublishAsBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetNotificationBadgeNum(int num)
{
    ANS_LOGW("AnsManagerStub::SetNotificationBadgeNum called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetBundleImportance(int &importance)
{
    ANS_LOGW("AnsManagerStub::GetBundleImportance called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetDisturbMode(NotificationConstant::DisturbMode mode)
{
    ANS_LOGW("AnsManagerStub::SetDisturbMode called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetDisturbMode(NotificationConstant::DisturbMode &mode)
{
    ANS_LOGW("AnsManagerStub::GetDisturbMode called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::HasNotificationPolicyAccessPermission(bool &granted)
{
    ANS_LOGW("AnsManagerStub::HasNotificationPolicyAccessPermission called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetPrivateNotificationsAllowed(bool allow)
{
    ANS_LOGW("AnsManagerStub::SetPrivateNotificationsAllowed called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetPrivateNotificationsAllowed(bool &allow)
{
    ANS_LOGW("AnsManagerStub::GetPrivateNotificationsAllowed called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::RemoveNotification(
    const sptr<NotificationBundleOption> &bundleOption, int notificationId, const std::string &label)
{
    ANS_LOGW("AnsManagerStub::RemoveNotification called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::RemoveAllNotifications(const sptr<NotificationBundleOption> &bundleOption)
{
    ANS_LOGW("AnsManagerStub::RemoveAllNotifications called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::Delete(const std::string &key)
{
    ANS_LOGW("AnsManagerStub::Delete called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::DeleteByBundle(const sptr<NotificationBundleOption> &bundleOption)
{
    ANS_LOGW("AnsManagerStub::DeleteByBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::DeleteAll()
{
    ANS_LOGW("AnsManagerStub::DeleteAll called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetSlotsByBundle(
    const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGW("AnsManagerStub::GetSlotsByBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::UpdateSlots(
    const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGW("AnsManagerStub::UpdateSlots called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::UpdateSlotGroups(
    const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups)
{
    ANS_LOGW("AnsManagerStub::UpdateSlotGroups called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetNotificationsEnabledForBundle(const std::string &bundle, bool enabled)
{
    ANS_LOGW("AnsManagerStub::SetNotificationsEnabledForBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled)
{
    ANS_LOGW("AnsManagerStub::SetNotificationsEnabledForAllBundles called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetNotificationsEnabledForSpecialBundle(
    const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled)
{
    ANS_LOGW("AnsManagerStub::SetNotificationsEnabledForSpecialBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::SetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool enabled)
{
    ANS_LOGW("AnsManagerStub::SetShowBadgeEnabledForBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool &enabled)
{
    ANS_LOGW("AnsManagerStub::GetShowBadgeEnabledForBundle called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetShowBadgeEnabled(bool &enabled)
{
    ANS_LOGW("AnsManagerStub::GetShowBadgeEnabled called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::Subscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info)
{
    ANS_LOGW("AnsManagerStub::Subscribe called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::Unsubscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info)
{
    ANS_LOGW("AnsManagerStub::Unsubscribe called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::AreNotificationsSuspended(bool &suspended)
{
    ANS_LOGW("AnsManagerStub::AreNotificationsSuspended called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap)
{
    ANS_LOGW("AnsManagerStub::GetCurrentAppSorting called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::IsAllowedNotify(bool &allowed)
{
    ANS_LOGW("AnsManagerStub::IsAllowedNotify called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::IsSpecialBundleAllowedNotify(const sptr<NotificationBundleOption> &bundleOption, bool &allowed)
{
    ANS_LOGW("AnsManagerStub::IsSpecialBundleAllowedNotify called!");
    return ERR_INVALID_OPERATION;
}

ErrCode AnsManagerStub::ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo)
{
    ANS_LOGW("AnsManagerStub::ShellDump called!");
    return ERR_INVALID_OPERATION;
}

}  // namespace Notification
}  // namespace OHOS
