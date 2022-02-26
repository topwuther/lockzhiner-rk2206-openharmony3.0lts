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

#include "system_ability_manager_proxy.h"

#include <string>
#include <unistd.h>

#include "datetime_ex.h"
#include "errors.h"
#include "ipc_types.h"
#include "parcel.h"
#include "string_ex.h"

#include "sam_log.h"
#include "system_ability_info.h"

using namespace std;
namespace OHOS {
namespace {
const int32_t RETRY_TIME_OUT_NUMBER = 10;
const int32_t SLEEP_INTERVAL_TIME = 100;
const int32_t SLEEP_ONE_MILLI_SECOND_TIME = 1000;
const std::u16string SAMANAGER_INTERFACE_TOKEN = u"ohos.samgr.accessToken";
constexpr int32_t MAX_SYSCAP_NAME_LEN = 64;
}
sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId)
{
    return GetSystemAbilityWrapper(systemAbilityId);
}

bool SystemAbilityManagerProxy::GetSystemAbilityInfoList(int32_t systemAbilityId,
    const std::u16string& capability, std::list<std::shared_ptr<SystemAbilityInfo>>& saInfoList)
{
    HILOGD("GetSystemAbilityInfoList called, systemAbilityId is %{public}d", systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGE("GetSystemAbilityInfoList systemAbilityId is invalid");
        return false;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr!");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("GetSystemAbilityInfoList write InterfaceToken failed!");
        return false;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("GetSystemAbilityInfoList write systemAbilityId failed!");
        return false;
    }
    ret = data.WriteString16(capability);
    if (!ret) {
        HILOGW("GetSystemAbilityInfoList write capability failed!");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(GET_SYSTEM_ABILITYINFOLIST_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        return false;
    }
    ret = reply.ReadBool();
    if (!ret) {
        HILOGW("GetSystemAbilityInfoList read vector failed!");
        return false;
    }
    int32_t replySize = 0;
    ret = reply.ReadInt32(replySize);
    if (!ret) {
        HILOGW("GetSystemAbilityInfoList read replySize failed!");
        return false;
    }
    for (int32_t i = 0; i < replySize; i++) {
        std::shared_ptr<SystemAbilityInfo> spSAInfo(reply.ReadParcelable<SystemAbilityInfo>());
        if (spSAInfo != nullptr) {
            saInfoList.emplace_back(spSAInfo);
        }
    }
    return true;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    return GetSystemAbilityWrapper(systemAbilityId, deviceId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbilityWrapper(int32_t systemAbilityId, const string& deviceId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("GetSystemAbilityWrapper systemAbilityId invalid:%{public}d!", systemAbilityId);
        return nullptr;
    }

    bool isExist = false;
    int32_t timeout = RETRY_TIME_OUT_NUMBER;
    HILOGD("GetSystemAbilityWrapper:Waiting for sa %{public}d, ", systemAbilityId);
    do {
        sptr<IRemoteObject> svc;
        if (deviceId.empty()) {
            svc = CheckSystemAbility(systemAbilityId, isExist);
            if (!isExist) {
                HILOGE("%{public}s:sa %{public}d is not exist", __func__, systemAbilityId);
                return nullptr;
            }
        } else {
            svc = CheckSystemAbility(systemAbilityId, deviceId);
        }

        if (svc != nullptr) {
            HILOGD("GetSystemAbilityWrapper found sa %{public}d", systemAbilityId);
            return svc;
        }
        usleep(SLEEP_ONE_MILLI_SECOND_TIME * SLEEP_INTERVAL_TIME);
    } while (timeout--);
    HILOGE("GetSystemAbilityWrapper sa %{public}d didn't start. Returning nullptr", systemAbilityId);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetSystemAbilityWrapper remote is nullptr !");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("CheckSystemAbility SendRequest error:%{public}d!", err);
        return nullptr;
    }
    return reply.ReadRemoteObject();
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("systemAbilityId:%{public}d invalid!", systemAbilityId);
        return nullptr;
    }
    MessageParcel data;
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckSystemAbility Write systemAbilityId failed!");
        return nullptr;
    }
    return CheckSystemAbilityWrapper(CHECK_SYSTEM_ABILITY_TRANSACTION, data);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckLocalAbilityManager(const u16string& name)
{
    HILOGI("%{public}s called", __func__);
    if (name.empty()) {
        HILOGI("name is nullptr.");
        return nullptr;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteString16(name);
    if (!ret) {
        HILOGW("CheckLocalAbilityManager Write name failed!");
        return nullptr;
    }
    return CheckSystemAbilityWrapper(CHECK_LOCAL_ABILITY_TRANSACTION, data);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (!CheckInputSysAbilityId(systemAbilityId) || deviceId.empty()) {
        HILOGW("CheckSystemAbility:systemAbilityId:%{public}d or deviceId is nullptr.", systemAbilityId);
        return nullptr;
    }

    HILOGD("CheckSystemAbility: ability id is : %{public}d, deviceId is %{private}s", systemAbilityId,
        deviceId.c_str());

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSystemAbility remote is nullptr !");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGE("CheckSystemAbility parcel write name failed");
        return nullptr;
    }
    ret = data.WriteString(deviceId);
    if (!ret) {
        HILOGE("CheckSystemAbility parcel write deviceId failed");
        return nullptr;
    }

    return CheckSystemAbilityWrapper(CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION, data);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    HILOGD("%{public}s called, ability id is %{public}d, isExist is %{public}d", __func__, systemAbilityId, isExist);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbility:systemAbilityId:%{public}d invalid!", systemAbilityId);
        return nullptr;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSystemAbility remote is nullptr !");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return nullptr;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckSystemAbility Write systemAbilityId failed!");
        return nullptr;
    }

    ret = data.WriteBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbility Write isExist failed!");
        return nullptr;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("CheckSystemAbility SendRequest error:%{public}d!", err);
        return nullptr;
    }
    sptr<IRemoteObject> irsp(reply.ReadRemoteObject());

    ret = reply.ReadBool(isExist);
    if (!ret) {
        HILOGW("CheckSystemAbility Read isExist failed!");
        return nullptr;
    }

    return irsp;
}

int32_t SystemAbilityManagerProxy::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& localAbilityManagerName)
{
    HILOGI("%{public}s called, system ability name is : %{public}d ", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId) || localAbilityManagerName.empty()) {
        HILOGI("AddOnDemandSystemAbilityInfo invalid params!");
        return ERR_INVALID_VALUE;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("AddOnDemandSystemAbilityInfo remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteString16(localAbilityManagerName);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Write localAbilityManagerName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION, data, reply, option);

    HILOGI("%{public}s:add ondemand system ability %{public}d %{public}s, return %{public}d",
        __func__, systemAbilityId, err ? "fail" : "succ", err);
    if (err != ERR_NONE) {
        HILOGE("AddOnDemandSystemAbilityInfo SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("AddOnDemandSystemAbilityInfo Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::RecycleOnDemandSystemAbility()
{
    HILOGI("%{public}s called", __func__);

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("RecycleOnDemandSystemAbility remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t err = remote->SendRequest(RECYCLE_ONDEMAND_SYSTEM_ABILITY_TRANSACTION, data, reply, option);

    HILOGI("%{public}s:recycle ondemand system ability %{public}s, return %{public}d",
        __func__, err ? "fail" : "succ", err);
    if (err != ERR_NONE) {
        HILOGE("RecycleOnDemandSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HILOGW("RecycleOnDemandSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::ConnectSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityConnectionCallback>& connectionCallback)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGE("ConnectSystemAbility systemAbilityId invalid.");
        return ERR_INVALID_VALUE;
    }

    if (connectionCallback == nullptr) {
        HILOGE("ConnectSystemAbility connectionCallback nullptr.");
        return ERR_INVALID_VALUE;
    }
    HILOGI("connection ondemand system ability name is : %{public}d ", systemAbilityId);

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("ConnectSystemAbility remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("ConnectSystemAbility Write name failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteRemoteObject(connectionCallback->AsObject());
    if (!ret) {
        HILOGW("ConnectSystemAbility Write connectionCallback failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option { MessageOption::TF_ASYNC };
    int32_t err = remote->SendRequest(CONNECTION_SYSTEM_ABILITY_TRANSACTION, data, reply, option);

    HILOGI("%{public}s:connect ondemand system ability %{public}d %{public}s, err: %{public}d",
        __func__, systemAbilityId, err ? "fail" : "succ", err);

    return err;
}

int32_t SystemAbilityManagerProxy::DisConnectSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityConnectionCallback>& connectionCallback)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGE("DisConnectSystemAbility systemAbilityId is invalid.");
        return ERR_INVALID_VALUE;
    }
    if (connectionCallback == nullptr) {
        HILOGE("DisConnectSystemAbility connectionCallback nullptr.");
        return ERR_INVALID_VALUE;
    }
    HILOGI("disconnect ondemand system ability name is : %{public}d ", systemAbilityId);

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("DisConnectSystemAbility remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("DisConnectSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteRemoteObject(connectionCallback->AsObject());
    if (!ret) {
        HILOGW("DisConnectSystemAbility Write connectionCallback failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option { MessageOption::TF_ASYNC };
    int32_t err = remote->SendRequest(DISCONNECTION_SYSTEM_ABILITY_TRANSACTION, data, reply,
        option);

    HILOGI("%{public}s:disconnect ondemand system ability %{public}d %{public}s, err: %{public}d",
        __func__, systemAbilityId, err ? "fail" : "succ", err);

    return err;
}

int32_t SystemAbilityManagerProxy::AddLocalAbilityManager(const u16string& name, const sptr<IRemoteObject>& ability)
{
    HILOGI("%{public}s called, local ability name is %{public}s", __func__, Str16ToStr8(name).c_str());
    if (name.empty()) {
        HILOGI("name is invalid.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(name)) {
        HILOGW("AddLocalAbilityManager Write name failed!");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(ability)) {
        HILOGW("AddLocalAbilityManager Write ability failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return AddSystemAbilityWrapper(ADD_LOCAL_ABILITY_TRANSACTION, data);
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("RemoveSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("RemoveSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbility(int32_t systemAbilityId)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("systemAbilityId:%{public}d is invalid!", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    HILOGI("remove system ability id is : %{public}d \n", systemAbilityId);
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("RemoveSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return RemoveSystemAbilityWrapper(REMOVE_SYSTEM_ABILITY_TRANSACTION, data);
}

int32_t SystemAbilityManagerProxy::RemoveLocalAbilityManager(const u16string& name)
{
    HILOGI("%{public}s called", __func__);
    if (name.empty()) {
        HILOGI("name is null.");
        return ERR_INVALID_VALUE;
    }

    HILOGI("RemoveLocalAbilityManager name is : %{public}s \n", Str16ToStr8(name).c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteString16(name);
    if (!ret) {
        HILOGW("RemoveLocalAbilityManager Write name failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return RemoveSystemAbilityWrapper(REMOVE_LOCAL_ABILITY_TRANSACTION, data);
}

std::vector<u16string> SystemAbilityManagerProxy::ListSystemAbilities(unsigned int dumpFlags)
{
    HILOGI("%{public}s called", __func__);
    std::vector<u16string> saNames;

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return saNames;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        HILOGW("ListSystemAbilities write token failed!");
        return saNames;
    }
    bool ret = data.WriteInt32(dumpFlags);
    if (!ret) {
        HILOGW("ListSystemAbilities write dumpFlags failed!");
        return saNames;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(LIST_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGW("ListSystemAbilities transact failed!");
        return saNames;
    }
    if (reply.ReadInt32() != ERR_NONE) {
        HILOGW("ListSystemAbilities remote failed!");
        return saNames;
    }
    if (!reply.ReadString16Vector(&saNames)) {
        HILOGW("ListSystemAbilities read reply failed");
        saNames.clear();
    }
    return saNames;
}

int32_t SystemAbilityManagerProxy::SubscribeSystemAbility(int32_t systemAbilityId, const std::u16string& listenerName)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || listenerName.empty()) {
        HILOGE("SubscribeSystemAbility systemAbilityId:%{public}d or listenerName:%{public}s invalid!",
            systemAbilityId, Str16ToStr8(listenerName).c_str());
        return ERR_INVALID_VALUE;
    }

    HILOGD("SubscribeSystemAbility systemAbilityId:%{public}d: listenerName:%{public}s",
        systemAbilityId, Str16ToStr8(listenerName).c_str());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteString16(listenerName);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Write listenerName failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION, data, reply, option);

    HILOGI("%{public}s : systemAbilityId:%{public}d: listenerName:%{public}s : %{public}s return %{public}d",
        __func__, systemAbilityId, Str16ToStr8(listenerName).c_str(), err ? "fail" : "succ", err);
    if (err != ERR_NONE) {
        HILOGE("SubscribeSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("SubscribeSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

int32_t SystemAbilityManagerProxy::UnSubscribeSystemAbility(int32_t systemAbilityId, const std::u16string& listenerName)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || listenerName.empty()) {
        HILOGE("UnSubscribeSystemAbility systemAbilityId:%{public}d or listenerName:%{public}s invalid!",
            systemAbilityId, Str16ToStr8(listenerName).c_str());
        return ERR_INVALID_VALUE;
    }

    HILOGD("UnSubscribeSystemAbility systemAbilityId:%{public}d: listenerName:%{public}s",
        systemAbilityId, Str16ToStr8(listenerName).c_str());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Write systemAbilityId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    ret = data.WriteString16(listenerName);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Write listenerSaId failed!");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION, data, reply, option);

    HILOGI("%{public}s : systemAbilityId:%{public}d: listenerName:%{public}s : %{public}s return %{public}d",
        __func__, systemAbilityId, Str16ToStr8(listenerName).c_str(), err ? "fail" : "succ", err);
    if (err != ERR_NONE) {
        HILOGE("UnSubscribeSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }

    int32_t result = 0;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGW("UnSubscribeSystemAbility Read result failed!");
        return ERR_FLATTEN_OBJECT;
    }

    return result;
}

const std::u16string SystemAbilityManagerProxy::CheckOnDemandSystemAbility(int32_t systemAbilityId)
{
    HILOGI("%{public}s called", __func__);
    std::u16string localManagerName;
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckOnDemandSystemAbility remote is nullptr !");
        return localManagerName;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return localManagerName;
    }
    bool ret = data.WriteInt32(systemAbilityId);
    if (!ret) {
        HILOGW("CheckOnDemandSystemAbility Write name failed!");
        return localManagerName;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(CHECK_ONDEMAND_SYSTEM_ABILITY_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("CheckOnDemandSystemAbility SendRequest error:%{public}d!", err);
        return localManagerName;
    }

    localManagerName = reply.ReadString16();
    if (localManagerName.empty()) {
        HILOGW("CheckOnDemandSystemAbility localManagerName is empty!");
    }
    return localManagerName;
}

bool SystemAbilityManagerProxy::GetDeviceId(string& deviceId)
{
    HILOGI("%{public}s called", __func__);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return false;
    }
    int32_t err = remote->SendRequest(GET_LOCAL_DEVICE_ID_TRANSACTION, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("GetDeviceId SendRequest error:%{public}d!", err);
        return false;
    }
    bool ret = reply.ReadString(deviceId);
    if (!ret) {
        HILOGW("GetDeviceId Read deviceId failed!");
        return false;
    }
    bool result = false;
    ret = reply.ReadBool(result);
    if (!ret) {
        HILOGW("GetDeviceId Read result failed!");
        return false;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::MarshalSAExtraProp(const SAExtraProp& extraProp, MessageParcel& data) const
{
    if (!data.WriteBool(extraProp.isDistributed)) {
        HILOGW("MarshalSAExtraProp Write isDistributed failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(extraProp.dumpFlags)) {
        HILOGW("MarshalSAExtraProp Write dumpFlags failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(extraProp.capability)) {
        HILOGW("MarshalSAExtraProp Write capability failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(extraProp.permission)) {
        HILOGW("MarshalSAExtraProp Write defPermission failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    HILOGI("%{public}s called, saId is %{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("systemAbilityId:%{public}d invalid.", systemAbilityId);
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(systemAbilityId)) {
        HILOGW("AddSystemAbility Write saId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(ability)) {
        HILOGW("AddSystemAbility Write ability failed!");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = MarshalSAExtraProp(extraProp, data);
    if (ret != ERR_OK) {
        HILOGW("AddSystemAbility MarshalSAExtraProp failed!");
        return ret;
    }
    return AddSystemAbilityWrapper(ADD_SYSTEM_ABILITY_TRANSACTION, data);
}

int32_t SystemAbilityManagerProxy::AddSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("AddSystemAbility SendRequest error:%{public}d!", err);
        return err;
    }
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGE("AddSystemAbility read result error!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::RegisterSystemReadyCallback(const sptr<IRemoteObject>& systemReadyCallback)
{
    if (systemReadyCallback == nullptr) {
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(systemReadyCallback)) {
        HILOGW("RegisterSystemReadyCallback Write systemReadyCallback failed!");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(REGISTER_SYSTEM_READY_CALLBACK, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("RegisterSystemReadyCallback SendRequest error:%{public}d!", err);
        return err;
    }
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGE("RegisterSystemReadyCallback read result error!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::GetCoreSystemAbilityList(std::vector<int32_t>& coreSaList, int dumpMode)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(dumpMode)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(GET_CORE_SYSTEM_ABILITY_LIST, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("GetCoreSaList SendRequest error:%{public}d!", err);
        return err;
    }
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGE("GetCoreSaList read result error!");
        return ERR_FLATTEN_OBJECT;
    }
    if (result == ERR_NONE && (!reply.ReadInt32Vector(&coreSaList))) {
        HILOGE("GetCoreSaList ReadInt32Vector error!");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t SystemAbilityManagerProxy::AddSystemCapability(const std::string& sysCap)
{
    if (sysCap.empty() || sysCap.length() > MAX_SYSCAP_NAME_LEN) {
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(Str8ToStr16(sysCap))) {
        HILOGW("%{public}s Write sysCap failed!", __func__);
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("AddSystemCapability remote is nullptr !");
        return ERR_INVALID_OPERATION;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(ADD_SYSTEM_CAPABILITY, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("%{public}s SendRequest error:%{public}d!", __func__, err);
        return err;
    }
    int32_t result = 0;
    bool ret = reply.ReadInt32(result);
    if (!ret) {
        HILOGE("%{public}s read result error!", __func__);
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

bool SystemAbilityManagerProxy::HasSystemCapability(const std::string& sysCap)
{
    if (sysCap.empty() || sysCap.length() > MAX_SYSCAP_NAME_LEN) {
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return false;
    }
    if (!data.WriteString16(Str8ToStr16(sysCap))) {
        HILOGW("%{public}s Write sysCap failed!", __func__);
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("HasSystemCapability remote is nullptr !");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(HAS_SYSTEM_CAPABILITY, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("%{public}s SendRequest error:%{public}d!", __func__, err);
        return false;
    }
    bool result = false;
    bool ret = reply.ReadBool(result);
    if (!ret) {
        HILOGE("%{public}s read result error!", __func__);
        return false;
    }
    return result;
}

vector<string> SystemAbilityManagerProxy::GetSystemAvailableCapabilities()
{
    vector<string> sysCaps;
    MessageParcel data;
    if (!data.WriteInterfaceToken(SAMANAGER_INTERFACE_TOKEN)) {
        return sysCaps;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGI("GetSystemAvailableCapabilities remote is nullptr !");
        return sysCaps;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = remote->SendRequest(GET_AVAILABLE_SYSTEM_CAPABILITY, data, reply, option);
    if (err != ERR_NONE) {
        HILOGE("%{public}s SendRequest error:%{public}d!", __func__, err);
        return sysCaps;
    }
    vector<u16string> result;
    bool ret = reply.ReadString16Vector(&result);
    if (!ret) {
        HILOGE("%{public}s read result error!", __func__);
        return sysCaps;
    }
    for (const u16string& sysCap : result) {
        sysCaps.emplace_back(Str16ToStr8(sysCap));
    }
    return sysCaps;
}
} // namespace OHOS
