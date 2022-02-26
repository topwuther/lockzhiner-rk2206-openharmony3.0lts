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

#include "input_client_proxy.h"

namespace OHOS {
namespace MiscServices {
using namespace ErrorCode;
    InputClientProxy::InputClientProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IInputClient>(object)
    {
    }

    int32_t InputClientProxy::onInputReady(int32_t retValue, const sptr<IInputMethodAgent>& agent,
                                           const InputChannel *channel)
    {
        IMSA_HILOGI("InputClientProxy::onInputReady");
        MessageParcel data, reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            return ERROR_EX_PARCELABLE;
        }

        if (!data.WriteInt32(retValue)) {
            return ERROR_EX_PARCELABLE;
        }
        if (agent == nullptr) {
            data.WriteInt32(0);
        } else {
            data.WriteInt32(1);
            data.WriteRemoteObject(agent->AsObject().GetRefPtr());
        }
    
        if (channel == nullptr) {
            data.WriteInt32(0);
        } else {
            data.WriteInt32(1);
            data.WriteParcelable(channel);
        }

        auto ret = Remote()->SendRequest(ON_INPUT_READY, data, reply, option);
        if (ret != NO_ERROR) {
            IMSA_HILOGI("InputClientProxy::onInputReady SendRequest failed");
            return ERROR_STATUS_FAILED_TRANSACTION;
        }

        return NO_ERROR;
    }

    int32_t InputClientProxy::onInputReleased(int32_t retValue)
    {
        MessageParcel data, reply;
        MessageOption option;

        data.WriteInterfaceToken(GetDescriptor());
        data.WriteInt32(retValue);
        auto status = Remote()->SendRequest(ON_INPUT_RELEASED, data, reply, option);
        return status;
    }

    int32_t InputClientProxy::setDisplayMode(int32_t mode)
    {
        MessageParcel data, reply;
        MessageOption option;

        data.WriteInterfaceToken(GetDescriptor());
        data.WriteInt32(mode);
        auto status = Remote()->SendRequest(SET_DISPLAY_MODE, data, reply, option);
        return status;
    }
}
}
