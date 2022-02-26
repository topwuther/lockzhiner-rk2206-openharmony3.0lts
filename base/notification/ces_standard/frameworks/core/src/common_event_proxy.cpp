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

#include "common_event_proxy.h"
#include "event_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace EventFwk {
using namespace OHOS::AppExecFwk;

CommonEventProxy::CommonEventProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<ICommonEvent>(object)
{
    EVENT_LOGD("CommonEventProxy instance created");
}

CommonEventProxy::~CommonEventProxy()
{
    EVENT_LOGD("CommonEventProxy instance destoryed");
}

bool CommonEventProxy::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishInfo,
    const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteParcelable(&event)) {
        EVENT_LOGE("Failed to write parcelable event");
        return false;
    }

    if (!data.WriteParcelable(&publishInfo)) {
        EVENT_LOGE("Failed to write parcelable publishInfo");
        return false;
    }

    if (!data.WriteParcelable(commonEventListener)) {
        EVENT_LOGE("Failed to write parcelable commonEventListener");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_PUBLISH_COMMON_EVENT, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishInfo,
    const sptr<IRemoteObject> &commonEventListener, const uid_t &uid)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteParcelable(&event)) {
        EVENT_LOGE("Failed to write parcelable event");
        return false;
    }

    if (!data.WriteParcelable(&publishInfo)) {
        EVENT_LOGE("Failed to write parcelable publishInfo");
        return false;
    }

    if (!data.WriteParcelable(commonEventListener)) {
        EVENT_LOGE("Failed to write parcelable commonEventListener");
        return false;
    }

    if (!data.WriteInt32(uid)) {
        EVENT_LOGE("Failed to write int uid");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_PUBLISH_COMMON_EVENT2, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::SubscribeCommonEvent(
    const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteParcelable(&subscriberInfo)) {
        EVENT_LOGE("Failed to write parcelable subscriberInfo");
        return false;
    }
    if (!data.WriteParcelable(commonEventListener)) {
        EVENT_LOGE("Failed to write parcelable commonEventListener");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_SUBSCRIBE_COMMON_EVENT, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteParcelable(commonEventListener)) {
        EVENT_LOGE("Failed to write parcelable commonEventListener");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_UNSUBSCRIBE_COMMON_EVENT, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteString16(Str8ToStr16(event))) {
        EVENT_LOGE("Failed to write string event");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_GET_STICKY_COMMON_EVENT, data, reply);
    if (ret) {
        ret = reply.ReadBool();
        if (ret) {
            std::unique_ptr<CommonEventData> eventDataPtr(reply.ReadParcelable<CommonEventData>());
            eventData = *eventDataPtr;
        }
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteString16(Str8ToStr16(event))) {
        EVENT_LOGE("Failed to write string event");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_DUMP_STATE, data, reply);
    if (ret) {
        int32_t stackNum = reply.ReadInt32();
        for (int i = 0; i < stackNum; i++) {
            std::string stack = Str16ToStr8(reply.ReadString16());
            state.emplace_back(stack);
        }
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int &code, const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteParcelable(proxy)) {
        EVENT_LOGE("Failed to write parcelable proxy");
        return false;
    }
    if (!data.WriteInt32(code)) {
        EVENT_LOGE("Failed to write int code");
        return false;
    }
    if (!data.WriteString16(Str8ToStr16(receiverData))) {
        EVENT_LOGE("Failed to write string receiverData");
        return false;
    }
    if (!data.WriteBool(abortEvent)) {
        EVENT_LOGE("Failed to write bool abortEvent");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_FINISH_RECEIVER, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::Freeze(const uid_t &uid)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteInt32(uid)) {
        EVENT_LOGE("Failed to write int uid");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_FREEZE, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::Unfreeze(const uid_t &uid)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;

    if (!data.WriteInt32(uid)) {
        EVENT_LOGE("Failed to write int uid");
        return false;
    }

    bool ret = SendRequest(ICommonEvent::Message::CES_UNFREEZE, data, reply);
    if (ret) {
        ret = reply.ReadBool();
    }

    EVENT_LOGD("end");
    return ret;
}

bool CommonEventProxy::SendRequest(ICommonEvent::Message code, MessageParcel &data, MessageParcel &reply)
{
    EVENT_LOGD("start");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EVENT_LOGE("Remote is NULL, %{public}d", code);
        return false;
    }

    MessageOption option(MessageOption::TF_SYNC);
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        EVENT_LOGE("Failed to SendRequest %{public}d, error code: %{public}d", code, result);
        return false;
    }

    EVENT_LOGD("end");
    return true;
}
}  // namespace EventFwk
}  // namespace OHOS