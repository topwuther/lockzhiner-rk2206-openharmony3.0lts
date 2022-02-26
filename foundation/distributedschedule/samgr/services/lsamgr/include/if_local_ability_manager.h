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

#ifndef IF_LOCAL_ABILITY_MANAGER_H_
#define IF_LOCAL_ABILITY_MANAGER_H_

#include <string>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "iremote_proxy.h"

namespace OHOS {
class ILocalAbilityManager : public IRemoteBroker {
public:
    virtual bool Debug(int32_t saId) = 0;
    virtual bool Test(int32_t saId) = 0;
    virtual bool SADump(int32_t saId) = 0;
    virtual bool HandoffAbilityAfter(const std::u16string& begin, const std::u16string& after) = 0;
    virtual bool HandoffAbilityBegin(int32_t saId) = 0;
    virtual bool StartAbility(int32_t saId) = 0;
    virtual bool StopAbility(int32_t saId) = 0;
    virtual bool OnAddSystemAbility(int32_t saId, const std::string& deviceId = "") = 0;
    virtual bool OnRemoveSystemAbility(int32_t saId, const std::string& deviceId = "") = 0;
    virtual void StartAbilityAsyn(int32_t saId) = 0;
    virtual bool RecycleOndemandSystemAbility(int32_t saId) = 0;
public:

    enum {
        DEBUG_TRANSACTION = 1,
        TEST_TRANSACTION = 2,
        DUMP_TRANSACTION = 3,
        HAND_OFF_ABILITY_AFTER_TRANSACTION = 4,
        HAND_OFF_ABILITY_BEGIN_TRANSACTION = 5,
        START_ABILITY_TRANSACTION = 6,
        STOP_ABILITY_TRANSACTION = 7,
        ON_ADD_SYSTEM_ABILITY_TRANSACTION = 8,
        ON_REMOVE_SYSTEM_ABILITY_TRANSACTION = 9,
        START_ABILITY_ASYN_TRANSACTION = 10,
        RECYCLE_SYSTEM_ABILITY_TRANSACTION = 11,
    };
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ILocalAbilityManager");
};
}
#endif // !defined(IF_LOCAL_ABILITY_MANAGER_H_)
