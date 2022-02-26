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

#include "input_method_agent_proxy.h"

namespace OHOS {
namespace MiscServices {
    using namespace ErrorCode;
    InputMethodAgentProxy::InputMethodAgentProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IInputMethodAgent>(object)
    {
    }

    int32_t InputMethodAgentProxy::DispatchKey(int32_t key, int32_t status)
    {
        IMSA_HILOGI("InputMethodAgentProxy::DispatchKey key = %{public}d, status = %{public}d", key, status);
        MessageParcel data, reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            IMSA_HILOGI("InputMethodAgentProxy::DispatchKey descriptor is not match");
            return ERROR_EX_PARCELABLE;
        }

        data.WriteInt32(key);
        data.WriteInt32(status);

        auto ret = Remote()->SendRequest(DISPATCH_KEY, data, reply, option);
        return ret;
    }
}
}
