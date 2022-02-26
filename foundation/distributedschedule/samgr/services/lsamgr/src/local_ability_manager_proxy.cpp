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

#include "local_ability_manager_proxy.h"
#include <unistd.h>
#include "ipc_types.h"
#include "parcel.h"

using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace {
const std::u16string LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN = u"ohos.localabilitymanager.accessToken";
}
bool LocalAbilityManagerProxy::SADump(int32_t systemAbilityId)
{
    bool ret = TransactInner(DUMP_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "SADump transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::Debug(int32_t systemAbilityId)
{
    bool ret = TransactInner(DEBUG_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "Debug transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::Test(int32_t systemAbilityId)
{
    bool ret = TransactInner(TEST_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "Test transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::HandoffAbilityAfter(const std::u16string& begin, const std::u16string& after)
{
    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HiLog::Error(label_, "HandoffAbilityAfter Remote return null");
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HiLog::Warn(label_, "HandoffAbilityAfter interface token check failed");
        return false;
    }
    bool ret = data.WriteString16(begin);
    if (!ret) {
        HiLog::Warn(label_, "HandoffAbilityAfter write begin failed!");
        return false;
    }
    ret = data.WriteString16(after);
    if (!ret) {
        HiLog::Warn(label_, "HandoffAbilityAfter write after failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t status = iro->SendRequest(HAND_OFF_ABILITY_AFTER_TRANSACTION, data, reply, option);
    if (status != NO_ERROR) {
        HiLog::Error(label_, "HandoffAbilityAfter Transact failed, return value : %{public}d", status);
        return false;
    }
    bool result = true;
    ret = reply.ReadBool(result);
    if (!ret) {
        HiLog::Warn(label_, "read reply failed!");
        return false;
    }
    return result;
}

bool LocalAbilityManagerProxy::HandoffAbilityBegin(int32_t systemAbilityId)
{
    bool ret = TransactInner(HAND_OFF_ABILITY_BEGIN_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "HandoffAbilityBegin transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::StartAbility(int32_t systemAbilityId)
{
    bool ret = TransactInner(START_ABILITY_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "StartAbility transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::StopAbility(int32_t systemAbilityId)
{
    bool ret = TransactInner(STOP_ABILITY_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "StopAbility transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::OnAddSystemAbility(int32_t saId, const std::string& deviceId)
{
    bool ret = TransactInner(ON_ADD_SYSTEM_ABILITY_TRANSACTION, saId, deviceId);
    if (!ret) {
        HiLog::Error(label_, "OnAddSystemAbility transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::OnRemoveSystemAbility(int32_t saId, const std::string& deviceId)
{
    bool ret = TransactInner(ON_REMOVE_SYSTEM_ABILITY_TRANSACTION, saId, deviceId);
    if (!ret) {
        HiLog::Error(label_, "OnRemoveSystemAbility transact failed!");
    }
    return ret;
}

void LocalAbilityManagerProxy::StartAbilityAsyn(int32_t systemAbilityId)
{
    bool ret = TransactInner(START_ABILITY_ASYN_TRANSACTION, TRANS_ASYNC, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "StartAbilityAsyn transact failed!");
    }
    return;
}

bool LocalAbilityManagerProxy::RecycleOndemandSystemAbility(int32_t systemAbilityId)
{
    bool ret = TransactInner(RECYCLE_SYSTEM_ABILITY_TRANSACTION, 0, systemAbilityId);
    if (!ret) {
        HiLog::Error(label_, "RecycleOndemandSystemAbility transact failed!");
    }
    return ret;
}

bool LocalAbilityManagerProxy::TransactInner(uint32_t code, uint32_t flags, int32_t systemAbilityId)
{
    if (systemAbilityId <= 0) {
        HiLog::Warn(label_, "TransactInner systemAbilityId invalid.");
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HiLog::Error(label_, "TransactInner Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HiLog::Warn(label_, "TransactInner interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HiLog::Warn(label_, "TransactInner write systemAbilityId failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    if (flags == TRANS_ASYNC) {
        option.SetFlags(MessageOption::TF_ASYNC);
    }
    int32_t status = iro->SendRequest(code, data, reply, option);
    if (status != NO_ERROR) {
        HiLog::Error(label_, "TransactInner Transact failed, return value : %{public}d", status);
        return false;
    }
    bool result = true;
    ret = reply.ReadBool(result);
    if (!ret) {
        HiLog::Warn(label_, "read reply failed!");
        return false;
    }
    return (flags == TRANS_ASYNC) ? true : result;
}

bool LocalAbilityManagerProxy::TransactInner(uint32_t code, int32_t saId, const std::string& deviceId)
{
    if (saId <= 0) {
        HiLog::Info(label_, "TransactInner saId:%{public}d is invalid!", saId);
        return false;
    }

    sptr<IRemoteObject> iro = Remote();
    if (iro == nullptr) {
        HiLog::Error(label_, "TransactInner Remote return null");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN)) {
        HiLog::Warn(label_, "TransactInner interface token check failed");
        return false;
    }
    bool ret = data.WriteInt32(saId);
    if (!ret) {
        HiLog::Warn(label_, "TransactInner write saId failed!");
        return false;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HiLog::Warn(label_, "TransactInner write deviceId failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t status = iro->SendRequest(code, data, reply, option);
    if (status != NO_ERROR) {
        HiLog::Error(label_, "TransactInner Transact failed, return value : %{public}d", status);
        return false;
    }
    bool result = true;
    ret = reply.ReadBool(result);
    if (!ret) {
        HiLog::Warn(label_, "read reply failed!");
        return false;
    }
    return result;
}
}
