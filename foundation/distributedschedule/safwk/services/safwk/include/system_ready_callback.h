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

#ifndef INTERFACES_INNERKITS_SYSTEM_READY_CALLBACK_H
#define INTERFACES_INNERKITS_SYSTEM_READY_CALLBACK_H

#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
class SystemReadyCallback : public IRemoteBroker {
public:
    virtual void OnSystemReadyNotify() const = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.SystemReadyCallback");
    enum {
        ON_SYSTEM_READY_EVENT = 1,
    };
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SYSTEM_READY_CALLBACK_H )

