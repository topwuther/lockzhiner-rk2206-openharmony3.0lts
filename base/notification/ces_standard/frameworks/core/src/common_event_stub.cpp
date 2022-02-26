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

#include "common_event_stub.h"
#include "common_event_publish_info.h"
#include "event_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace EventFwk {
CommonEventStub::CommonEventStub()
{}

CommonEventStub::~CommonEventStub()
{}

int CommonEventStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(ICommonEvent::Message::CES_PUBLISH_COMMON_EVENT): {
            std::unique_ptr<CommonEventData> event(data.ReadParcelable<CommonEventData>());
            std::unique_ptr<CommonEventPublishInfo> publishinfo(data.ReadParcelable<CommonEventPublishInfo>());
            sptr<IRemoteObject> commonEventListener = data.ReadParcelable<IRemoteObject>();
            if (!event) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventData>");
                return ERR_INVALID_VALUE;
            }
            if (!publishinfo) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventPublishInfo>");
                return ERR_INVALID_VALUE;
            }
            bool ret = PublishCommonEvent(*event, *publishinfo, commonEventListener);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply ");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_PUBLISH_COMMON_EVENT2): {
            std::unique_ptr<CommonEventData> event(data.ReadParcelable<CommonEventData>());
            std::unique_ptr<CommonEventPublishInfo> publishinfo(data.ReadParcelable<CommonEventPublishInfo>());
            sptr<IRemoteObject> commonEventListener = data.ReadParcelable<IRemoteObject>();
            int uid = data.ReadInt32();
            if (!event) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventData>");
                return ERR_INVALID_VALUE;
            }
            if (!publishinfo) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventPublishInfo>");
                return ERR_INVALID_VALUE;
            }
            bool ret = PublishCommonEvent(*event, *publishinfo, commonEventListener, uid);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply ");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_SUBSCRIBE_COMMON_EVENT): {
            std::unique_ptr<CommonEventSubscribeInfo> subscriberInfo(data.ReadParcelable<CommonEventSubscribeInfo>());
            sptr<IRemoteObject> commonEventListener = data.ReadParcelable<IRemoteObject>();
            if (!subscriberInfo) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventSubscribeInfo>");
                return ERR_INVALID_VALUE;
            }
            if (commonEventListener == nullptr) {
                EVENT_LOGE("Failed to ReadParcelable<IRemoteObject>");
                return ERR_INVALID_VALUE;
            }
            bool ret = SubscribeCommonEvent(*subscriberInfo, commonEventListener);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_UNSUBSCRIBE_COMMON_EVENT): {
            sptr<IRemoteObject> commonEventListener = data.ReadParcelable<IRemoteObject>();
            if (commonEventListener == nullptr) {
                EVENT_LOGE("Failed to ReadParcelable<IRemoteObject>");
                return ERR_INVALID_VALUE;
            }
            bool ret = UnsubscribeCommonEvent(commonEventListener);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_GET_STICKY_COMMON_EVENT): {
            std::string event = Str16ToStr8(data.ReadString16());
            CommonEventData eventData;
            bool ret = GetStickyCommonEvent(event, eventData);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply ret!");
                return ERR_INVALID_VALUE;
            }
            if (!reply.WriteParcelable(&eventData)) {
                EVENT_LOGE("Failed to write reply eventData!");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_DUMP_STATE): {
            std::vector<std::string> result;
            std::string event = Str16ToStr8(data.ReadString16());
            DumpState(event, result);
            reply.WriteInt32(result.size());
            for (auto stack : result) {
                reply.WriteString16(Str8ToStr16(stack));
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_FINISH_RECEIVER): {
            sptr<IRemoteObject> proxy = data.ReadParcelable<IRemoteObject>();
            if (proxy == nullptr) {
                EVENT_LOGE("Failed to ReadParcelable<IRemoteObject>");
                return ERR_INVALID_VALUE;
            }
            int receiverCode = data.ReadInt32();
            std::string receiverData = Str16ToStr8(data.ReadString16());
            bool abortEvent = data.ReadBool();
            bool ret = FinishReceiver(proxy, receiverCode, receiverData, abortEvent);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_FREEZE): {
            int uid = data.ReadInt32();
            bool ret = Freeze(uid);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        case static_cast<uint32_t>(ICommonEvent::Message::CES_UNFREEZE): {
            int uid = data.ReadInt32();
            bool ret = Unfreeze(uid);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }

        default:
            EVENT_LOGW("unknown, code = %{public}u, flags= %{public}u", code, option.GetFlags());
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return NO_ERROR;
}

bool CommonEventStub::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
    const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
    const sptr<IRemoteObject> &commonEventListener, const uid_t &uid)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::SubscribeCommonEvent(
    const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::Freeze(const uid_t &uid)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::Unfreeze(const uid_t &uid)
{
    EVENT_LOGD("called");

    return true;
}
}  // namespace EventFwk
}  // namespace OHOS