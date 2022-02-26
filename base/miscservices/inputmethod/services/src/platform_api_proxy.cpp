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

#include "i_platform_api.h"
#include "i_input_method_core.h"
#include "iremote_proxy.h"
#include "message_parcel.h"
#include "peer_holder.h"
#include "input_method_setting.h"
#include "peer_holder.h"
#include "utils.h"
#include <string>
#include <cstdint>

/*! \class PlatformApiProxy
  \brief The proxy implementation of IPlatformApi
*/
namespace OHOS {
namespace MiscServices {
    class PlatformApiProxy : public IRemoteProxy<IPlatformApi> {
    public:
        PlatformApiProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IPlatformApi>(impl)
        {
        }

        ~PlatformApiProxy()
        {
        }

        int32_t registerCallback(const sptr<IPlatformCallback>& cb)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteRemoteObject(cb->AsObject());
            int32_t status = Remote()->SendRequest(REGISTER_CALLBACK, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                return code;
            }
            return ErrorCode::NO_ERROR;
        }

        std::u16string getInterfaceDescriptor()
        {
            return Utils::to_utf16("20210814");
        }

        sptr<IInputMethodCore> bindInputMethodService(const std::u16string& packageName,
            const std::u16string& intention, int userId)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteString16(packageName);
            data.WriteString16(intention);
            data.WriteInt32(userId);
            int32_t status = Remote()->SendRequest(BIND_IMS, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                LOG_DEBUG("status = %s\n", ErrorCode::ToString(status));
                return nullptr;
            }
            int code = reply.ReadException();
            if (code != 0) {// code = 0, means no exception.
                LOG_DEBUG("exception code : %d\n", code);
                return nullptr;
            }
            sptr < IInputMethodCore > ims = nullptr;
            reply.ReadRemoteObject();
            return ims;
        }

        int32_t unbindInputMethodService(int userId, const std::u16string& packageName)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            data.WriteString16(packageName);
            int32_t status = Remote()->SendRequest(UNBIND_IMS, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                return code;
            }
            return ErrorCode::NO_ERROR;
        }

        sptr<IRemoteObject> createWindowToken(int userId, int displayId, const std::u16string& packageName)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            data.WriteInt32(displayId);
            data.WriteString16(packageName);
            int32_t status = Remote()->SendRequest(CREATE_WINDOW_TOKEN, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return nullptr;
            }
            int code = reply.ReadException();
            if (code != 0) { // code = 0, means no exception.
                IMSA_HILOGE("Exception code = %d\n", code);
                return nullptr;
            }
            sptr < IRemoteObject > token = reply.ReadRemoteObject();
            return token;
        }

        int32_t destroyWindowToken(int userId, const std::u16string& packageName)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            data.WriteString16(packageName);
            int32_t status = Remote()->SendRequest(DESTROY_WINDOW_TOKEN, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                // code = 0, means no exception.
                return code;
            }
            return ErrorCode::NO_ERROR;
        }

        int32_t listInputMethod(int userId, std::vector<InputMethodProperty*> *inputMethodProperties)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            int32_t status = Remote()->SendRequest(LIST_INPUT_METHOD, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                // code = 0, means no exception.
                return code;
            }
            int size = reply.ReadInt32();
            for (int i = 0; i < size; i++) {
                InputMethodProperty *property = new InputMethodProperty();
                property = reply.ReadParcelable<InputMethodProperty>();
                inputMethodProperties->push_back(property);
            }
            return ErrorCode::NO_ERROR;
        }


        virtual int32_t getInputMethodProperty(int userId, const std::u16string& packageName,
            InputMethodProperty *inputMethodProperty)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            data.WriteString16(packageName);
            int32_t status = Remote()->SendRequest(GET_INPUT_METHOD_PROPERTY, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                // code = 0, means no exception.
                return code;
            }
            inputMethodProperty = reply.ReadParcelable<InputMethodProperty>();
            return status;
        }

        int32_t getInputMethodSetting(int userId, InputMethodSetting *inputMethodSetting)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            int32_t status = Remote()->SendRequest(GET_INPUT_METHOD_SETTING, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                // code = 0, means no exception.
                return code;
            }
            inputMethodSetting = reply.ReadParcelable<InputMethodSetting>();
            return status;
        }

        int32_t setInputMethodSetting(int userId, const InputMethodSetting& inputMethodSetting)
        {
            MessageParcel data, reply;
            MessageOption option;
            data.WriteInterfaceToken(GetDescriptor());
            data.WriteInt32(userId);
            data.WriteParcelable(&inputMethodSetting);
            int32_t status = Remote()->SendRequest(SET_INPUT_METHOD_SETTING, data, reply, option);
            if (status != ErrorCode::NO_ERROR) {
                return status;
            }
            int code = reply.ReadException();
            if (code != 0) {
                // code = 0, means no exception.
                return code;
            }
            return ErrorCode::NO_ERROR;
        }
    };
}
}

