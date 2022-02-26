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

#ifndef SYSTEM_ABILITY_CONNECTION_HANDLER_H_
#define SYSTEM_ABILITY_CONNECTION_HANDLER_H_

#include "iremote_object.h"

namespace OHOS {
class ConnectionHandler {
public:
    virtual ~ConnectionHandler() = default;
    virtual void OnConnectedSystemAbility(const sptr<IRemoteObject>& connectionCallback) = 0;
    virtual void OnDisConnectedSystemAbility(const std::u16string& name) = 0;
};
} // namespace OHOS

#endif // SYSTEM_ABILITY_CONNECTION_HANDLER_H_