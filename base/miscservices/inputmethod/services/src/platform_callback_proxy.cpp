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

#include <string>
#include "iremote_proxy.h"
#include "iremote_object.h"
#include "global.h"
#include "i_platform_callback.h"

/*! \class PlatformCallbackProxy
  \brief The proxy of IPlatformCallback
*/
namespace OHOS {
namespace MiscServices {
    class PlatformCallbackProxy : public IRemoteProxy<IPlatformCallback> {
    public:
        PlatformCallbackProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IPlatformCallback>(impl)
        {
        }

        ~PlatformCallbackProxy()
        {
        }

        void notifyEvent(int eventId, int userId, const std::vector<std::u16string>& eventContent)
        {
            (void)eventId;
            (void)userId;
            (void)eventContent;
        }
    };
}
}
