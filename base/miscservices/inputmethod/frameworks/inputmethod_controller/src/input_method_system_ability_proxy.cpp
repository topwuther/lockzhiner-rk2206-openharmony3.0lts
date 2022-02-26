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

#include "input_method_system_ability_proxy.h"

namespace OHOS {
namespace MiscServices {
    using namespace ErrorCode;
    InputMethodSystemAbilityProxy::InputMethodSystemAbilityProxy(const sptr<IRemoteObject> &object)
        : IRemoteProxy<IInputMethodSystemAbility>(object)
    {
    }

    void InputMethodSystemAbilityProxy::prepareInput(MessageParcel& data)
    {
        MessageParcel reply;
        MessageOption option;

        auto ret = Remote()->SendRequest(PREPARE_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::prepareInput SendRequest failed");
            return;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::prepareInput reply failed");
            return;
        }
    }

    void InputMethodSystemAbilityProxy::releaseInput(MessageParcel& data)
    {
        MessageParcel reply;
        MessageOption option;

        auto ret = Remote()->SendRequest(RELEASE_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::releaseInput SendRequest failed");
            return;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::releaseInput reply failed");
            return;
        }
    }

    void InputMethodSystemAbilityProxy::startInput(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityProxy::startInput");
        MessageParcel reply;
        MessageOption option;

        auto ret = Remote()->SendRequest(START_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::startInput SendRequest failed");
            return;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::startInput reply failed");
            return;
        }
    }

    void InputMethodSystemAbilityProxy::stopInput(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityProxy::stopInput");
        MessageParcel reply;
        MessageOption option;

        auto ret = Remote()->SendRequest(STOP_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::stopInput SendRequest failed");
            return;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::stopInput reply failed");
            return;
        }
    }

    int32_t InputMethodSystemAbilityProxy::setInputMethodCore(sptr<IInputMethodCore> &core)
    {
        IMSA_HILOGI("InputMethodSystemAbilityProxy::setInputMethodCore");

        if (core == nullptr) {
            IMSA_HILOGI("InputMethodSystemAbilityProxy::setInputMethodCore inputDataChannel is nullptr");
        }
        auto remote = Remote();
        if (remote == nullptr) {
            return -1;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(GetDescriptor())
            && data.WriteRemoteObject(core->AsObject()))) {
            return -1;
        }
        MessageParcel reply;
        MessageOption option {
            MessageOption::TF_SYNC
        };

        int32_t status = Remote()->SendRequest(SET_INPUT_METHOD_CORE, data, reply, option);

        return status;
    }

    int32_t InputMethodSystemAbilityProxy::Prepare(int32_t displayId, sptr<InputClientStub> &client,
                                                   sptr<InputDataChannelStub> &channel, InputAttribute &attribute)
    {
        MessageParcel data, reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        if (!(data.WriteInt32(displayId)
            && data.WriteRemoteObject(client->AsObject())
            && data.WriteRemoteObject(channel->AsObject())
            && data.WriteParcelable(&attribute))) {
                    return ERROR_EX_PARCELABLE;
                }

        auto ret = Remote()->SendRequest(PREPARE_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::Release(sptr<InputClientStub> &client)
    {
        IMSA_HILOGI("InputMethodSystemAbilityProxy::Release");
        MessageParcel data, reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        if (!data.WriteRemoteObject(client->AsObject().GetRefPtr())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(RELEASE_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::Start(sptr<InputClientStub> &client)
    {
        MessageParcel data, reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        if (!data.WriteRemoteObject(client->AsObject().GetRefPtr())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(START_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::Stop(sptr<InputClientStub> &client)
    {
        MessageParcel data, reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        if (!data.WriteRemoteObject(client->AsObject().GetRefPtr())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(STOP_INPUT, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::getDisplayMode(int32_t retMode)
    {
        MessageParcel data, reply;
        MessageOption option;

        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(GET_DISPLAY_MODE, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        if (!reply.ReadInt32(retMode)) {
            return ERROR_STATUS_BAD_VALUE;
        }
        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::getKeyboardWindowHeight(int32_t retHeight)
    {
        MessageParcel data, reply;
        MessageOption option;

        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(GET_KEYBOARD_WINDOW_HEIGHT, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        if (!reply.ReadInt32(retHeight)) {
            return ERROR_STATUS_BAD_VALUE;
        }
        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::getCurrentKeyboardType(KeyboardType *retType)
    {
        if (retType == nullptr) {
            return ERROR_NULL_POINTER;
        }

        MessageParcel data, reply;
        MessageOption option;

        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(GET_CURRENT_KEYBOARD_TYPE, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        KeyboardType *keyType = reply.ReadParcelable<KeyboardType>();
        if (keyType == nullptr) {
            return ERROR_NULL_POINTER;
        }
        *retType = *keyType;
        delete keyType;
        keyType = nullptr;
        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::listInputMethodEnabled(std::vector<InputMethodProperty*> *properties)
    {
        if (properties == nullptr) {
            return ERROR_NULL_POINTER;
        }

        MessageParcel data, reply;
        MessageOption option;

        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(LIST_INPUT_METHOD_ENABLED, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        auto size = reply.ReadInt32();
        while (size > 0) {
            InputMethodProperty *imp = reply.ReadParcelable<InputMethodProperty>();
            properties->push_back(imp);
            size--;
        }

        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::listInputMethod(std::vector<InputMethodProperty*> *properties)
    {
        if (properties == nullptr) {
            return ERROR_NULL_POINTER;
        }

        MessageParcel data, reply;
        MessageOption option;

        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(LIST_INPUT_METHOD, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        auto size = reply.ReadInt32();

        while (size > 0) {
            InputMethodProperty *imp = reply.ReadParcelable<InputMethodProperty>();
            properties->push_back(imp);
            size--;
        }

        return NO_ERROR;
    }

    int32_t InputMethodSystemAbilityProxy::listKeyboardType(const std::u16string& imeId,
        std::vector<KeyboardType*> *types)
    {
        if (types == nullptr) {
            return ERROR_NULL_POINTER;
        }

        MessageParcel data, reply;
        MessageOption option;

        if (!(data.WriteInterfaceToken(GetDescriptor()) && data.WriteString16(imeId))) {
            return ERROR_EX_PARCELABLE;
        }

        auto ret = Remote()->SendRequest(LIST_KEYBOARD_TYPE, data, reply, option);
        if (ret != NO_ERROR) {
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            return ret;
        }

        auto size = reply.ReadInt32();
        while (size > 0) {
            KeyboardType *kt = reply.ReadParcelable<KeyboardType>();
            types->push_back(kt);
            size--;
        }
        return NO_ERROR;
    }
}
}