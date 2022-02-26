/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "input_method_system_ability_stub.h"
#include "message_handler.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    /*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the parameters from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
    int32_t InputMethodSystemAbilityStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::OnRemoteRequest code = %{public}u", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }

        switch (code) {
            case PREPARE_INPUT: {
                prepareInput(data);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case RELEASE_INPUT: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                releaseInput(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case START_INPUT: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                startInput(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case STOP_INPUT: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                stopInput(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case SET_INPUT_METHOD_CORE: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                setInputMethodCoreFromHap(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case GET_DISPLAY_MODE: {
                int32_t mode = 0;
                int32_t status = getDisplayMode(mode);
                if (status == ErrorCode::NO_ERROR) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteInt32(mode);
                } else {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                }
                break;
            }
            case GET_KEYBOARD_WINDOW_HEIGHT: {
                int32_t height = 0;
                int32_t status = getKeyboardWindowHeight(height);
                if (status == ErrorCode::NO_ERROR) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteInt32(height);
                } else {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                }
                break;
            }
            case GET_CURRENT_KEYBOARD_TYPE: {
                KeyboardType type;
                int32_t status = getCurrentKeyboardType(&type);
                if (status == ErrorCode::NO_ERROR) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteParcelable(&type);
                } else if (status == ErrorCode::ERROR_NULL_POINTER) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteInt32(0);
                } else {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                }
                break;
            }
            case LIST_INPUT_METHOD_ENABLED: {
                std::vector<InputMethodProperty*> properties;
                int32_t ret = listInputMethodEnabled(&properties);
                if (ret != ErrorCode::NO_ERROR) {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE); // write exception code
                    reply.WriteInt32(-1);
                } else {
                    reply.WriteInt32(NO_ERROR);
                    int32_t size = properties.size();
                    reply.WriteInt32(size);
                    for (int32_t i = 0; i < size; i++) {
                        reply.WriteParcelable(properties[i]);
                    }
                    properties.clear();
                }
                break;
            }
            case LIST_INPUT_METHOD: {
                std::vector<InputMethodProperty*> properties;
                int32_t ret = listInputMethod(&properties);
                if (ret != ErrorCode::NO_ERROR) {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE); // write exception code
                    reply.WriteInt32(-1);
                    return ret;
                }
                reply.WriteInt32(NO_ERROR);
                int32_t size = properties.size();
                reply.WriteInt32(size);
                for (int32_t i = 0; i < size; i++) {
                    reply.WriteParcelable(properties[i]);
                }
                properties.clear();
                break;
            }
            case LIST_KEYBOARD_TYPE: {
                std::u16string imeId = data.ReadString16();
                std::vector<KeyboardType*> kbdTypes;
                int32_t ret = listKeyboardType(imeId, &kbdTypes);
                if (ret != ErrorCode::NO_ERROR) {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                    return ret;
                }
                reply.WriteInt32(NO_ERROR);
                int32_t size = kbdTypes.size();
                reply.WriteInt32(size);
                for (int32_t i = 0; i < size; i++) {
                    reply.WriteParcelable(kbdTypes[i]);
                }
                kbdTypes.clear();
                break;
            }
            default: {
                return BRemoteObject::OnRemoteRequest(code, data, reply, option);
            }
        }
        return NO_ERROR;
    }

    /*! Prepare input
    \n Send prepareInput command to work thread.
        The handling of prepareInput is in the work thread of PerUserSession.
    \see PerUserSession::OnPrepareInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::prepareInput(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::prepareInput");
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteInt32(pid);
        parcel->WriteInt32(uid);
        parcel->WriteInt32(data.ReadInt32());
        parcel->WriteRemoteObject(data.ReadRemoteObject());
        parcel->WriteRemoteObject(data.ReadRemoteObject());
        parcel->WriteParcelable(data.ReadParcelable<InputAttribute>());

        Message *msg = new Message(MSG_ID_PREPARE_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Release input
    \n Send releaseInput command to work thread.
        The handling of releaseInput is in the work thread of PerUserSession.
    \see PerUserSession::OnReleaseInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::releaseInput(MessageParcel& data)
    {
        IMSA_HILOGE("InputMethodSystemAbilityStub::releaseInput");
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_RELEASE_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);

    }

    /*! Start input
    \n Send startInput command to work thread.
        The handling of startInput is in the work thread of PerUserSession.
    \see PerUserSession::OnStartInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::startInput(MessageParcel& data)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_START_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Stop input
    \n Send stopInput command to work thread.
        The handling of stopInput is in the work thread of PerUserSession.
    \see PerUserSession::OnStopInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::stopInput(MessageParcel& data)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_STOP_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

        /*! Prepare input
    \n Send prepareInput command to work thread.
        The handling of prepareInput is in the work thread of PerUserSession.
    \see PerUserSession::OnPrepareInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::setInputMethodCoreFromHap(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::setInputMethodCoreFromHap");
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_SET_INPUT_METHOD_CORE, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Get user id from uid
    \param uid the uid from which the remote call is
    \return return user id of the remote caller
    */
    int32_t InputMethodSystemAbilityStub::getUserId(int32_t uid)
    {
        return uid/1000000;
    }
}
}