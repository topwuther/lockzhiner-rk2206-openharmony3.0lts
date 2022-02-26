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

#ifndef OHOS_DISTRIBUTED_ABILITY_CONNECTION_WRAPPER_STUB_H
#define OHOS_DISTRIBUTED_ABILITY_CONNECTION_WRAPPER_STUB_H

#include "ability_connect_callback_interface.h"

#include "nocopyable.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class AbilityConnectionWrapperStub : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    explicit AbilityConnectionWrapperStub(sptr<IRemoteObject> connection);
    virtual ~AbilityConnectionWrapperStub() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject,
        int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int32_t resultCode) override;

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override;

private:
    DISALLOW_COPY_AND_MOVE(AbilityConnectionWrapperStub);
    sptr<IRemoteObject> distributedConnection_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_CONNECTION_WRAPPER_STUB_H