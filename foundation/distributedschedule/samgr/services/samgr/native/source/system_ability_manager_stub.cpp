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

#include "system_ability_manager_stub.h"

#include "errors.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "sam_log.h"
#include "string_ex.h"
#include "system_ability_info.h"
#include "system_ability_manager.h"
#include "tools.h"

namespace OHOS {
namespace {
constexpr int32_t MULTIUSER_HAP_PER_USER_RANGE = 100000;
constexpr int32_t HID_HAP = 10000;  /* first hap user */
constexpr int32_t UID_SHELL = 2000;
const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
}
SystemAbilityManagerStub::SystemAbilityManagerStub()
{
    memberFuncMap_[GET_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::GetSystemAbilityInner;
    memberFuncMap_[CHECK_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckSystemAbilityInner;
    memberFuncMap_[ADD_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::AddSystemAbilityInner;
    memberFuncMap_[REMOVE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::RemoveSystemAbilityInner;
    memberFuncMap_[LIST_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::ListSystemAbilityInner;
    memberFuncMap_[SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::SubsSystemAbilityInner;
    memberFuncMap_[CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckRemtSystemAbilityInner;
    memberFuncMap_[ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::AddOndemandSystemAbilityInner;
    memberFuncMap_[RECYCLE_ONDEMAND_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::RecycleOndemandSystemAbilityInner;
    memberFuncMap_[CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckSystemAbilityImmeInner;
    memberFuncMap_[CONNECTION_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::ConnOndemandSystemAbilityInner;
    memberFuncMap_[DISCONNECTION_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::DisConnOndemandSystemAbilityInner;
    memberFuncMap_[CHECK_ONDEMAND_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckOndemandSystemAbilityInner;
    memberFuncMap_[CHECK_REMOTE_SYSTEM_ABILITY_FOR_JAVA_TRANSACTION] =
        &SystemAbilityManagerStub::CheckRemtSystemAbilityForJavaInner;
    memberFuncMap_[GET_SYSTEM_ABILITYINFOLIST_TRANSACTION] =
        &SystemAbilityManagerStub::GetSystemAbilityInfoListInner;
    memberFuncMap_[UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::UnSubsSystemAbilityInner;
    memberFuncMap_[GET_LOCAL_DEVICE_ID_TRANSACTION] =
        &SystemAbilityManagerStub::GetDeviceIdInner;
    memberFuncMap_[ADD_LOCAL_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::AddLocalAbilityInner;
    memberFuncMap_[CHECK_LOCAL_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::CheckLocalAbilityInner;
    memberFuncMap_[REMOVE_LOCAL_ABILITY_TRANSACTION] =
        &SystemAbilityManagerStub::RemoveLocalAbilityInner;
    memberFuncMap_[REGISTER_SYSTEM_READY_CALLBACK] =
        &SystemAbilityManagerStub::RegisterSystemReadyCallbackInner;
    memberFuncMap_[GET_CORE_SYSTEM_ABILITY_LIST] =
        &SystemAbilityManagerStub::GetCoreSystemAbilityListInner;
    memberFuncMap_[ADD_SYSTEM_CAPABILITY] =
        &SystemAbilityManagerStub::AddSystemCapabilityInner;
    memberFuncMap_[HAS_SYSTEM_CAPABILITY] =
        &SystemAbilityManagerStub::HasSystemCapabilityInner;
    memberFuncMap_[GET_AVAILABLE_SYSTEM_CAPABILITY] =
        &SystemAbilityManagerStub::GetSystemAvailableCapabilitiesInner;
}
int32_t SystemAbilityManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption &option)
{
    HILOGI("SystemAbilityManagerStub::OnReceived, code = %{public}d, flags= %{public}d",
        code, option.GetFlags());
    if (code != GET_SYSTEM_ABILITY_TRANSACTION && code != CHECK_SYSTEM_ABILITY_TRANSACTION) {
        if (!EnforceInterceToken(data)) {
            HILOGI("SystemAbilityManagerStub::OnReceived, code = %{public}d, check interfaceToken failed", code);
            return ERR_PERMISSION_DENIED;
        }
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("SystemAbilityManagerStub: default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool SystemAbilityManagerStub::EnforceInterceToken(MessageParcel& data)
{
    std::u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == SAMANAGER_INTERFACE_TOKEN;
}

int32_t SystemAbilityManagerStub::GetSystemAbilityInfoListInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("GetSystemAbilityInfoListInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = 0;
    bool ret = data.ReadInt32(systemAbilityId);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::GetSystemAbilityInfoListInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    std::u16string capability = data.ReadString16();
    std::list<std::shared_ptr<SystemAbilityInfo>> saInfoList;
    bool saRet = GetSystemAbilityInfoList(systemAbilityId, capability, saInfoList);
    if (!reply.WriteBool(saRet)) {
        HILOGW("SystemAbilityManagerStub::GetSystemAbilityListInner write reply size failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(saInfoList.size()))) {
        HILOGW("SystemAbilityManagerStub::GetSystemAbilityListInner write reply size failed!");
        return ERR_FLATTEN_OBJECT;
    }
    for (const auto& sa : saInfoList) {
        if (!reply.WriteParcelable(sa.get())) {
            HILOGW("SystemAbilityManagerStub::GetSystemAbilityListInner write vector failed!");
            return ERR_FLATTEN_OBJECT;
        }
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::CheckLocalAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("CheckLocalAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::u16string name = data.ReadString16();
    if (name.empty()) {
        HILOGW("SystemAbilityManagerStub::CheckLocalAbilityInner read name failed!");
        return ERR_NULL_OBJECT;
    }
    bool ret = reply.WriteRemoteObject(CheckLocalAbilityManager(name));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckLocalAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::AddLocalAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddLocalAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::u16string name = data.ReadString16();
    if (name.empty()) {
        HILOGW("SystemAbilityManagerStub::AddLocalAbilityInner read name failed!");
        return ERR_NULL_OBJECT;
    }

    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOGW("SystemAbilityManagerStub::AddLocalAbilityInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = AddLocalAbilityManager(name, object);
    HILOGI("SystemAbilityManagerStub::AddLocalAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::AddLocalAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::RemoveLocalAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("RemoveLocalAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::u16string name = data.ReadString16();
    if (name.empty()) {
        HILOGW("SystemAbilityManagerStub::RemoveLocalAbilityInner read name failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = RemoveLocalAbilityManager(name);
    HILOGI("SystemAbilityManagerStub::RemoveLocalAbilityInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::RemoveLocalAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::ListSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("ListSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t dumpFlag = 0;
    bool ret = data.ReadInt32(dumpFlag);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::ListSystemAbilityInner read dumpflag failed!");
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<std::u16string> saNameVector = ListSystemAbilities(dumpFlag);
    if (saNameVector.empty()) {
        HILOGI("List System Abilities list errors");
        ret = reply.WriteInt32(ERR_INVALID_VALUE);
    } else {
        HILOGI("SystemAbilityManagerStub::ListSystemAbilityInner list success");
        ret = reply.WriteInt32(ERR_NONE);
        if (ret) {
            ret = reply.WriteString16Vector(saNameVector);
        }
    }

    if (!ret) {
        HILOGW("SystemAbilityManagerStub::ListSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::SubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SubsSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    std::u16string listenerName = data.ReadString16();
    if (listenerName.empty()) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner read listenerName failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = SubscribeSystemAbility(systemAbilityId, listenerName);
    HILOGI("SystemAbilityManagerStub::SubsSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::UnSubsSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("UnSubsSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::UnSubsSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    std::u16string listenerName = data.ReadString16();
    if (listenerName.empty()) {
        HILOGW("SystemAbilityManagerStub::SubsSystemAbilityInner read listenerName failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = UnSubscribeSystemAbility(systemAbilityId, listenerName);
    HILOGI("SystemAbilityManagerStub::UnSubscribeSystemAbility result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::UnSubscribeSystemAbility write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckRemtSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    std::string deviceId;
    bool ret = data.ReadString(deviceId);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::string uuid = SystemAbilityManager::GetInstance()->TransformDeviceId(deviceId, UUID, false);
    ret = reply.WriteRemoteObject(GetSystemAbility(systemAbilityId, uuid));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::CheckRemtSystemAbilityForJavaInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityForJavaInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }

    std::u16string str16DeviceId = data.ReadString16();
    if (str16DeviceId.empty()) {
        HILOGW("SystemAbilityManagerStub::CheckRemtSystemAbilityInner read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::string deviceId = Str16ToStr8(str16DeviceId);
    std::string uuid = SystemAbilityManager::GetInstance()->TransformDeviceId(deviceId, UUID, false);
    bool result = false;
    result = reply.WriteRemoteObject(GetSystemAbility(systemAbilityId, uuid));
    if (!result) {
        HILOGE("SystemAbilityManagerStub::CheckRemtSystemAbilityForJavaInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::AddOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::AddOndemandSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    std::u16string localManagerName = data.ReadString16();
    if (localManagerName.empty()) {
        HILOGW("SystemAbilityManagerStub::AddOndemandSystemAbilityInner read localName failed!");
        return ERR_NULL_OBJECT;
    }

    int32_t result = AddOnDemandSystemAbilityInfo(systemAbilityId, localManagerName);
    HILOGI("SystemAbilityManagerStub::AddOndemandSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::AddOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::RecycleOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("RecycleOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t result = RecycleOnDemandSystemAbility();
    HILOGI("SystemAbilityManagerStub::RecycleOndemandSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::RecycleOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckSystemAbilityImmeInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    bool isExist = false;
    bool ret = data.ReadBool(isExist);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner read isExist failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteRemoteObject(CheckSystemAbility(systemAbilityId, isExist));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    ret = reply.WriteBool(isExist);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityImmeInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::ConnOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("ConnOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::ConnOndemandSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOGW("SystemAbilityManagerStub::ConnOndemandSystemAbilityInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<ISystemAbilityConnectionCallback> connectionAbility =
        iface_cast<ISystemAbilityConnectionCallback>(object);
    int32_t result = ConnectSystemAbility(systemAbilityId, connectionAbility);
    HILOGI("SystemAbilityManagerStub::ConnOndemandSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::ConnOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::DisConnOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("DisConnOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::DisConnOndemandSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOGW("SystemAbilityManagerStub::DisConnOndemandSystemAbilityInner readParcel failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<ISystemAbilityConnectionCallback> connectionAbility =
        iface_cast<ISystemAbilityConnectionCallback>(object);
    int32_t result = DisConnectSystemAbility(systemAbilityId, connectionAbility);
    HILOGI("SystemAbilityManagerStub::DisConnOndemandSystemAbilityInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::DisConnOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerStub::CheckOndemandSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("CheckOndemandSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckOndemandSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    bool ret = reply.WriteString16((CheckOnDemandSystemAbility(systemAbilityId)).c_str());
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::ConnOndemandSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::GetDeviceIdInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("GetDeviceIdInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::string deviceId = SystemAbilityManager::GetInstance()->GetLocalNodeId();
    if (deviceId.empty()) {
        HILOGW("SystemAbilityManagerStub::GetLocalDeviceIdInner get  deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    bool result = reply.WriteString(deviceId);
    if (!result) {
        HILOGW("SystemAbilityManagerStub::GetDeviceIdInner write deviceId failed.");
        return ERR_FLATTEN_OBJECT;
    }

    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::GetDeviceIdInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::UnmarshalingSaExtraProp(MessageParcel& data, SAExtraProp& extraProp)
{
    bool isDistributed = false;
    bool ret = data.ReadBool(isDistributed);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::UnmarshalingSaExtraProp read isDistributed failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t dumpFlags = 0;
    ret = data.ReadInt32(dumpFlags);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::UnmarshalingSaExtraProp read dumpFlags failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::u16string capability = data.ReadString16();
    std::u16string permission = data.ReadString16();
    extraProp.isDistributed = isDistributed;
    extraProp.dumpFlags = dumpFlags;
    extraProp.capability = capability;
    extraProp.permission = permission;
    return ERR_OK;
}

int32_t SystemAbilityManagerStub::AddSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("AddSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    SAExtraProp extraProp;
    int32_t result = UnmarshalingSaExtraProp(data, extraProp);
    if (result != ERR_OK) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner UnmarshalingSaExtraProp failed!");
        return result;
    }
    result = AddSystemAbility(systemAbilityId, object, extraProp);
    HILOGI("SystemAbilityManagerStub::AddSystemAbilityExtraInner result is %d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::AddSystemAbilityExtraInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::GetSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::GetSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    bool ret = reply.WriteRemoteObject(GetSystemAbility(systemAbilityId));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub:GetSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::CheckSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::CheckSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    bool ret = reply.WriteRemoteObject(CheckSystemAbility(systemAbilityId));
    if (!ret) {
        HILOGW("SystemAbilityManagerStub:CheckSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::RemoveSystemAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("RemoveSystemAbilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t systemAbilityId = data.ReadInt32();
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("SystemAbilityManagerStub::RemoveSystemAbilityInner read systemAbilityId failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RemoveSystemAbility(systemAbilityId);
    HILOGI("SystemAbilityManagerStub::RemoveSystemAbilityInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::RemoveSystemAbilityInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::RegisterSystemReadyCallbackInner(MessageParcel& data, MessageParcel& reply)
{
    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOGW("SystemAbilityManagerStub::RegisterSystemReadyCallback readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterSystemReadyCallback(object);
    HILOGI("SystemAbilityManagerStub::RegisterSystemReadyCallbackInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::RegisterSystemReadyCallbackInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::GetCoreSystemAbilityListInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SystemAbilityManagerStub::GetCoreSystemAbilityListInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    int32_t dumpMode = data.ReadInt32();
    std::vector<int32_t> coreSaList;
    int32_t result = GetCoreSystemAbilityList(coreSaList, dumpMode);
    HILOGI("SystemAbilityManagerStub::GetCoreSystemAbilityListInner result is %{public}d", result);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGW("SystemAbilityManagerStub::GetCoreSystemAbilityListInner write reply failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (result == ERR_NONE && (!reply.WriteInt32Vector(coreSaList))) {
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerStub::AddSystemCapabilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SystemAbilityManagerStub::AddSystemCapabilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }

    std::u16string sysCap = data.ReadString16();
    bool ret = AddSystemCapability(Str16ToStr8(sysCap));
    if (!reply.WriteInt32(ret)) {
        HILOGW("SystemAbilityManagerStub::AddSystemCapabilityInner write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::HasSystemCapabilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SystemAbilityManagerStub::HasSystemCapabilityInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }

    std::u16string sysCap = data.ReadString16();
    if (!reply.WriteBool(HasSystemCapability(Str16ToStr8(sysCap)))) {
        HILOGW("SystemAbilityManagerStub::HasSystemCapabilityInner write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::GetSystemAvailableCapabilitiesInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CanRequest()) {
        HILOGE("SystemAbilityManagerStub::GetSystemAvailableCapabilitiesInner PERMISSION DENIED!");
        return ERR_PERMISSION_DENIED;
    }
    std::vector<std::string> sysCaps = GetSystemAvailableCapabilities();
    std::vector<std::u16string> u16SysCaps;
    for (const std::string& sysCap : sysCaps) {
        u16SysCaps.emplace_back(Str8ToStr16(sysCap));
    }
    if (!reply.WriteString16Vector(u16SysCaps)) {
        HILOGW("SystemAbilityManagerStub::GetSystemAvailableCapabilitiesInner write reply failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t SystemAbilityManagerStub::GetHapIdMultiuser(int32_t uid)
{
    return uid % MULTIUSER_HAP_PER_USER_RANGE;
}

bool SystemAbilityManagerStub::CanRequest()
{
    // never allow non-system uid request
    auto callingUid = IPCSkeleton::GetCallingUid();
    auto uid = GetHapIdMultiuser(callingUid);
    return (uid < HID_HAP) && (uid != UID_SHELL);
}

bool SystemAbilityManagerStub::IsSystemApp(int32_t callingUid)
{
    auto uid = GetHapIdMultiuser(callingUid);
    return uid < HID_HAP;
}
} // namespace OHOS
