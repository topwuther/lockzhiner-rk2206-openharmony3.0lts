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

#include "i_input_method_agent.h"
#include "iremote_proxy.h"
#include "iremote_broker.h"
#include "global.h"

/*! \class BpInputMethodAgentProxy
  \brief The proxy implementation of IInputMethodAgent

  This class should be implemented by input client
*/
namespace OHOS {
namespace MiscServices {
    class InputMethodAgentProxy : public IRemoteProxy<IInputMethodAgent> {
    public:
        InputMethodAgentProxy(const sptr<IRemoteObject>& impl)
            : IRemoteProxy<IInputMethodAgent>(impl)
        {
        }

        ~InputMethodAgentProxy()
        {
        }

        int32_t dispatchKey(int key, int status)
        {
            (void)key;
            (void)status;
            return NO_ERROR;
        }
    };
}
}

