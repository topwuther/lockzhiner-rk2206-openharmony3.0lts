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

#include "local_ability_manager_stub.h"

#include "errors.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "safwk_log.h"
#include "system_ability_definition.h"

using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace {
const std::string TAG = "LocalAbilityManagerStub";

constexpr int32_t UID_ROOT = 0;
constexpr int32_t UID_SYSTEM = 1000;
const std::u16string LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN = u"ohos.localabilitymanager.accessToken";
}

LocalAbilityManagerStub::LocalAbilityManagerStub()
{
    memberFuncMap_[DEBUG_TRANSACTION] =
        &LocalAbilityManagerStub::DebugInner;
    memberFuncMap_[TEST_TRANSACTION] =
        &LocalAbilityManagerStub::TestInner;
    memberFuncMap_[DUMP_TRANSACTION] =
        &LocalAbilityManagerStub::DumpInner;
    memberFuncMap_[HAND_OFF_ABILITY_AFTER_TRANSACTION] =
        &LocalAbilityManagerStub::HandOffAbilityAfterInner;
    memberFuncMap_[HAND_OFF_ABILITY_BEGIN_TRANSACTION] =
        &LocalAbilityManagerStub::HandOffAbilityBeginInner;
    memberFuncMap_[START_ABILITY_TRANSACTION] =
        &LocalAbilityManagerStub::StartAbilityInner;
    memberFuncMap_[STOP_ABILITY_TRANSACTION] =
        &LocalAbilityManagerStub::StopAbilityInner;
    memberFuncMap_[ON_ADD_SYSTEM_ABILITY_TRANSACTION] =
        &LocalAbilityManagerStub::OnAddSystemAbilityInner;
    memberFuncMap_[ON_REMOVE_SYSTEM_ABILITY_TRANSACTION] =
        &LocalAbilityManagerStub::OnRemoveSystemAbilityInner;
    memberFuncMap_[START_ABILITY_ASYN_TRANSACTION] =
        &LocalAbilityManagerStub::StartAbilityAsyncInner;
    memberFuncMap_[RECYCLE_SYSTEM_ABILITY_TRANSACTION] =
        &LocalAbilityManagerStub::RecycleOndemandAbilityInner;
}

int32_t LocalAbilityManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HILOGI(TAG, "code:%{public}d, flags:%{public}d", code, option.GetFlags());
    if (!CanRequest()) {
        HILOGW(TAG, "permission denied!");
        return ERR_PERMISSION_DENIED;
    }
    if (!EnforceInterceToken(data)) {
        HILOGW(TAG, "check interface token failed!");
        return ERR_PERMISSION_DENIED;
    }
    auto iter = memberFuncMap_.find(code);
    if (iter != memberFuncMap_.end()) {
        auto memberFunc = iter->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW(TAG, "unknown request code!");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t LocalAbilityManagerStub::DebugInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = Debug(saId);
    HILOGI(TAG, "%{public}s to debug", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::TestInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = Test(saId);
    HILOGI(TAG, "%{public}s to test", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::DumpInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = SADump(saId);
    HILOGI(TAG, "%{public}s to dump", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::HandOffAbilityAfterInner(MessageParcel& data, MessageParcel& reply)
{
    std::u16string begin = data.ReadString16();
    if (begin.empty()) {
        HILOGW(TAG, "read begin failed!");
        return ERR_NULL_OBJECT;
    }

    std::u16string after = data.ReadString16();
    if (after.empty()) {
        HILOGW(TAG, "read after failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = HandoffAbilityAfter(begin, after);
    HILOGI(TAG, "%{public}s to handoff ability after", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::HandOffAbilityBeginInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = HandoffAbilityBegin(saId);
    HILOGI(TAG, "%{public}s to handoff ability begin", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::StartAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = StartAbility(saId);
    HILOGI(TAG, "%{public}s to start ability", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::StopAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = StopAbility(saId);
    HILOGI(TAG, "%{public}s to stop ability", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::OnAddSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }
    const std::string& deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOGW(TAG, "read deviceId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = OnAddSystemAbility(saId, deviceId);
    HILOGI(TAG, "%{public}s to notify SA add", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::OnRemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }
    const std::string& deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOGW(TAG, "read deviceId failed!");
        return ERR_NULL_OBJECT;
    }
    bool result = OnRemoveSystemAbility(saId, deviceId);
    HILOGI(TAG, "%{public}s to notify SA add", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::StartAbilityAsyncInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    StartAbilityAsyn(saId);
    return ERR_NONE;
}

int32_t LocalAbilityManagerStub::RecycleOndemandAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t saId = data.ReadInt32();
    if (!CheckInputSysAbilityId(saId)) {
        HILOGW(TAG, "read saId failed!");
        return ERR_NULL_OBJECT;
    }

    bool result = RecycleOndemandSystemAbility(saId);
    HILOGI(TAG, "%{public}s to recycle on-demand SA", result ? "success" : "failed");
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW(TAG, "write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

bool LocalAbilityManagerStub::CheckInputSysAbilityId(int32_t systemAbilityId)
{
    return (systemAbilityId >= FIRST_SYS_ABILITY_ID) && (systemAbilityId <= LAST_SYS_ABILITY_ID);
}

bool LocalAbilityManagerStub::CanRequest()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    return (callingUid == UID_ROOT) || (callingUid == UID_SYSTEM);
}

bool LocalAbilityManagerStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN;
}
}
