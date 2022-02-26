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


#ifndef LOCAL_ABILITY_MANAGER_STUB_H_
#define LOCAL_ABILITY_MANAGER_STUB_H_

#include <map>
#include "if_local_ability_manager.h"

namespace OHOS {
class LocalAbilityManagerStub : public IRemoteStub<ILocalAbilityManager> {
public:
    LocalAbilityManagerStub();
    ~LocalAbilityManagerStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

protected:
    static bool CheckInputSysAbilityId(int32_t systemAbilityId);

private:
    int32_t DebugInner(MessageParcel& data, MessageParcel& reply);
    int32_t TestInner(MessageParcel& data, MessageParcel& reply);
    int32_t DumpInner(MessageParcel& data, MessageParcel& reply);
    int32_t HandOffAbilityAfterInner(MessageParcel& data, MessageParcel& reply);
    int32_t HandOffAbilityBeginInner(MessageParcel& data, MessageParcel& reply);
    int32_t StartAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t StopAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t OnAddSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t OnRemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply);
    int32_t StartAbilityAsyncInner(MessageParcel& data, MessageParcel& reply);
    int32_t RecycleOndemandAbilityInner(MessageParcel& data, MessageParcel& reply);
    static bool CanRequest();
    static bool EnforceInterceToken(MessageParcel& data);

    using LocalAbilityManagerStubFunc =
        int32_t (LocalAbilityManagerStub::*)(MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, LocalAbilityManagerStubFunc> memberFuncMap_;
};
}
#endif
