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

#include "hichain_connector.h"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <securec.h>

#include "nlohmann/json.hpp"

#include "parameter.h"

#include "anonymous_string.h"
#include "device_manager_log.h"
#include "encrypt_utils.h"
#include "softbus_adapter.h"
#include "constants.h"
#include "ipc_server_listener_adapter.h"
#include "ipc_check_authenticate_req.h"
#include "softbus_session.h"
#include "auth_manager.h"
#include "ipc_server_stub.h"
#include "dm_ability_manager.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string DEVICE_ID = "DEVICE_ID";
const std::string WIFI_IP = "WIFI_IP";
const std::string WIFI_PORT = "WIFI_PORT";
const std::string BR_MAC = "BR_MAC";
const std::string BLE_MAC = "BLE_MAC";
const std::string ETH_IP = "ETH_IP";
const std::string ETH_PORT = "ETH_PORT";
const std::string DEVICE_MANAGER_APP = "ohos.distributedhardware.devicemanager";
const std::string DEVICE_MANAGER_GROUPNAME = "DMPeerToPeerGroup";

const int64_t MIN_REQUEST_ID = 1000000000;
const int64_t MAX_REQUEST_ID = 9999999999;
const int32_t FIELD_EXPIRE_TIME_VALUE = 7;
}

IMPLEMENT_SINGLE_INSTANCE(HichainConnector);

void from_json(const nlohmann::json& jsonObject, GroupInfo& groupInfo)
{
    if (jsonObject.find(FIELD_GROUP_NAME) != jsonObject.end()) {
        groupInfo.groupName = jsonObject.at(FIELD_GROUP_NAME).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_ID) != jsonObject.end()) {
        groupInfo.groupId = jsonObject.at(FIELD_GROUP_ID).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_OWNER) != jsonObject.end()) {
        groupInfo.groupOwner = jsonObject.at(FIELD_GROUP_OWNER).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }

    if (jsonObject.find(FIELD_GROUP_VISIBILITY) != jsonObject.end()) {
        groupInfo.groupVisibility = jsonObject.at(FIELD_GROUP_VISIBILITY).get<int32_t>();
    }
}

int HichainConnector::Init()
{
    DMLOG(DM_LOG_INFO, "HichainConnector::init, init device auth service.");
    InitDeviceAuthService();

    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        DMLOG(DM_LOG_INFO, "HichainConnector::init, failed to init group manager!");
        return -1;
    }
    deviceGroupManager_->regCallback(DEVICE_MANAGER_APP.c_str(), &deviceAuthCallback_);
    DMLOG(DM_LOG_INFO, "HichainConnector::init, init hichain adapter success.");
    return 0;
}

int64_t HichainConnector::GenRequestId()
{
    return EncryptUtils::GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
}

int32_t HichainConnector::CreateGroup(int64_t requestId, const std::string &groupName)
{
    if (deviceGroupManager_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::CreateGroup group manager is null, requestId %lld.", requestId);
        return FAIL;
    }

    DMLOG(DM_LOG_INFO, "HichainConnector::CreateGroup requestId %lld", requestId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceID = localDeviceId;

    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    jsonObj[FIELD_DEVICE_ID] = sLocalDeviceID;
    jsonObj[FIELD_GROUP_NAME] = groupName;
    jsonObj[FIELD_USER_TYPE] = 0;
    jsonObj[FIELD_GROUP_VISIBILITY] = GROUP_VISIBILITY_PUBLIC;
    jsonObj[FIELD_EXPIRE_TIME] = FIELD_EXPIRE_TIME_VALUE;
    int32_t ret =  deviceGroupManager_->createGroup(requestId, DEVICE_MANAGER_APP.c_str(), jsonObj.dump().c_str());
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "Faild to start CreateGroup task, ret: %d, requestId %lld.", ret, requestId);
        return ret;
    }

    return SUCCESS;
}

void HichainConnector::RegisterConnectorCallback(std::shared_ptr<HichainConnectorCallback> callback)
{
    hichainConnectorCallback_ = callback;
}

void HichainConnector::OnGroupCreated(int64_t requestId, const std::string &returnData)
{
    if (hichainConnectorCallback_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::OnGroupCreated hichainConnectorCallback_ not registe.");
        return;
    }

    nlohmann::json jsonObject = nlohmann::json::parse(returnData);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::OnGroupCreated returnData not json.");
        hichainConnectorCallback_->OnGroupCreated(requestId, "");
        return;
    }

    if (jsonObject.find(FIELD_GROUP_ID) == jsonObject.end()) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::OnGroupCreated failed to get groupId.");
        hichainConnectorCallback_->OnGroupCreated(requestId, "");
        return;
    }

    std::string groupId = jsonObject.at(FIELD_GROUP_ID).get<std::string>();
    DMLOG(DM_LOG_INFO, "group create success, groupId:%s.", GetAnonyString(groupId).c_str());
    hichainConnectorCallback_->OnGroupCreated(requestId, groupId);
}

void HichainConnector::OnMemberJoin(int64_t requestId, int32_t status)
{
    AbilityRole role = DmAbilityManager::GetInstance().GetAbilityRole();
    DMLOG(DM_LOG_INFO, "HichainConnector::OnMemberJoin:: role = %d", (int32_t)role);

    if (role == AbilityRole::ABILITY_ROLE_INITIATIVE) {
        AuthManager::GetInstance().NotifyHostOnCheckAuthResult(requestId, status);
        return;
    }

    if (hichainConnectorCallback_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::OnMemberJoin hichainConnectorCallback_ not registe.");
        return;
    }
    hichainConnectorCallback_->OnMemberJoin(requestId, status);
}

int32_t HichainConnector::AddMemeber(std::string deviceId, std::shared_ptr<MsgResponseAuth> msgResponseAuth)
{
    DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber");
    if (deviceGroupManager_ == nullptr) {
        DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber group manager is null.");
        return -1;
    }

    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string connectInfo = GetConnectPara(deviceId, msgResponseAuth);

    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = msgResponseAuth->GetGroupId();
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    jsonObj[FIELD_PIN_CODE] = std::to_string(msgResponseAuth->GetPinCode()).c_str();
    jsonObj[FIELD_IS_ADMIN] = false;
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;
    jsonObj[FIELD_GROUP_NAME] = msgResponseAuth->GetGroupName();
    jsonObj[FIELD_CONNECT_PARAMS] = connectInfo.c_str();
    std::string tmpStr = jsonObj.dump();

    DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber completed");
    return deviceGroupManager_->addMemberToGroup(msgResponseAuth->GetRequestId(), DEVICE_MANAGER_APP.c_str(),
        tmpStr.c_str());
}

std::string HichainConnector::GetConnectPara(std::string deviceId, std::shared_ptr<MsgResponseAuth> msgResponseAuth)
{
    ConnectionAddr *addrInfo = SoftbusAdapter::GetConnectAddr(deviceId);
    if (addrInfo == nullptr) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::GetConnectPara addrInfo error");
        return "";
    }

    DMLOG(DM_LOG_ERROR, "HichainConnector::GetConnectPara get addrInfo");
    nlohmann::json jsonPara;
    jsonPara[DEVICE_ID] = msgResponseAuth->GetDeviceId();
    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_ETH) {
        DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber addr type is ETH");
        jsonPara[ETH_IP] = addrInfo->info.ip.ip;
        jsonPara[ETH_PORT] = addrInfo->info.ip.port;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_WLAN) {
        DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber addr type is WIFI");
        jsonPara[WIFI_IP] = addrInfo->info.ip.ip;
        jsonPara[WIFI_PORT] = addrInfo->info.ip.port;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BR) {
        DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber addr type is br");
        jsonPara[BR_MAC] = addrInfo->info.br.brMac;
    } else if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BLE) {
        DMLOG(DM_LOG_INFO, "HichainConnector::AddMemeber addr type is ble");
        jsonPara[BLE_MAC] = addrInfo->info.ble.bleMac;
    } else {
        DMLOG(DM_LOG_ERROR, "HichainConnector::AddMemeber addrInfo not right");
        return "";
    }
    return jsonPara.dump();
}

void HichainConnector::GetRelatedGroups(std::string deviceId, std::vector<GroupInfo> &groupList)
{
    DMLOG(DM_LOG_INFO, "HichainConnector::GetRelatedGroups Start to get local related groups.");
    uint32_t groupNum = 0;
    char *returnGroups = nullptr;
    int32_t ret = deviceGroupManager_->getRelatedGroups(DEVICE_MANAGER_APP.c_str(), deviceId.c_str(),
        &returnGroups, &groupNum);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::GetRelatedGroups faild , ret: %d.", ret);
        return;
    }

    if (returnGroups == nullptr) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::GetRelatedGroups faild , returnGroups is nullptr");
        return;
    }

    if (groupNum == 0) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::GetRelatedGroups group failed, groupNum is 0.");
        return;
    }

    std::string relatedGroups = std::string(returnGroups);
    nlohmann::json jsonObject = nlohmann::json::parse(relatedGroups);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "returnGroups parse error");
        return;
    }

    std::vector<GroupInfo> groupInfos = jsonObject.get<std::vector<GroupInfo>>();
    if (groupInfos.size() == 0) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::GetRelatedGroups group failed, groupInfos is empty.");
        return;
    }

    groupList = groupInfos;
    return;
}

void HichainConnector::GetSyncGroupList(std::vector<GroupInfo> &groupList, std::vector<std::string> &syncGroupList)
{
    if (groupList.empty()) {
        DMLOG(DM_LOG_ERROR, "groupList is empty.");
        return;
    }

    for (auto group : groupList) {
        if (IsGroupInfoInvalid(group)) {
            continue;
        }
        syncGroupList.push_back(group.groupId);
    }
}

bool HichainConnector::IsGroupInfoInvalid(GroupInfo &group)
{
    if (group.groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP || group.groupVisibility == GROUP_VISIBILITY_PUBLIC ||
        group.groupOwner != DEVICE_MANAGER_APP) {
        return true;
    }
    return false;
}

void HichainConnector::SyncGroups(std::string deviceId, std::vector<std::string> &remoteGroupIdList)
{
    std::vector<GroupInfo> groupInfoList;
    GetRelatedGroups(deviceId, groupInfoList);
    for (auto &groupInfo : groupInfoList) {
        if (IsGroupInfoInvalid(groupInfo)) {
            continue;
        }

        auto iter = std::find(remoteGroupIdList.begin(), remoteGroupIdList.end(), groupInfo.groupId);
        if (iter == remoteGroupIdList.end()) {
            (void)DelMemberFromGroup(groupInfo.groupId, deviceId);
        }
    }
}

int32_t HichainConnector::DelMemberFromGroup(std::string groupId, std::string deviceId)
{
    int64_t requestId = GenRequestId();
    DMLOG(DM_LOG_INFO, "Start to delete memeber from group, requestId %lld, deviceId %s, groupId %s",
        requestId, GetAnonyString(deviceId).c_str(), GetAnonyString(groupId).c_str());
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_DELETE_ID] = deviceId;
    std::string deleteParams = jsonObj.dump();

    int32_t ret = deviceGroupManager_->deleteMemberFromGroup(requestId, DEVICE_MANAGER_APP.c_str(),
        deleteParams.c_str());
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::DelMemberFromGroup faild , ret: %d.", ret);
        return ret;
    }
    return 0;
}

void HichainConnector::DeleteGroup(std::string &groupId)
{
    int64_t requestId = GenRequestId();
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.dump();

    int32_t ret = deviceGroupManager_->deleteGroup(requestId, DEVICE_MANAGER_APP.c_str(), disbandParams.c_str());
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "HichainConnector::DeleteGroup faild , ret: %d.", ret);
    }
}

void HichainAuthenCallBack::onFinish(int64_t requestId, int32_t operationCode, const char *returnData)
{
    std::string data = "";
    if (returnData != nullptr) {
        data = std::string(returnData);
    }

    DMLOG(DM_LOG_INFO, "HichainAuthenCallBack::onFinish reqId:%lld, operation:%d", requestId, operationCode);
    if (operationCode == GroupOperationCode::MEMBER_JOIN) {
        DMLOG(DM_LOG_INFO, "Add Member To Group success");
        HichainConnector::GetInstance().OnMemberJoin(requestId, SUCCESS);
    }

    if (operationCode == GroupOperationCode::GROUP_CREATE) {
        DMLOG(DM_LOG_INFO, "Create group success");
        HichainConnector::GetInstance().OnGroupCreated(requestId, data);
    }

    if (operationCode == GroupOperationCode::MEMBER_DELETE) {
        DMLOG(DM_LOG_INFO, "Delete Member from group success");
    }

    if (operationCode == GroupOperationCode::GROUP_DISBAND) {
        DMLOG(DM_LOG_INFO, "Disband group success");
    }
}

void HichainAuthenCallBack::onError(int64_t requestId, int32_t operationCode, int32_t errorCode,
    const char *errorReturn)
{
    (void)errorReturn;
    DMLOG(DM_LOG_INFO, "HichainAuthenCallBack::onError reqId:%lld, operation:%d, errorCode:%d.",
        requestId, operationCode, errorCode);

    if (operationCode == GroupOperationCode::MEMBER_JOIN) {
        DMLOG(DM_LOG_ERROR, "Add Member To Group failed");
        HichainConnector::GetInstance().OnMemberJoin(requestId, FAIL);
    }

    if (operationCode == GroupOperationCode::GROUP_CREATE) {
        DMLOG(DM_LOG_ERROR, "Create group failed");
        HichainConnector::GetInstance().OnGroupCreated(requestId, "{}");
    }

    if (operationCode == GroupOperationCode::MEMBER_DELETE) {
        DMLOG(DM_LOG_ERROR, "Delete Member from group failed");
    }

    if (operationCode == GroupOperationCode::GROUP_DISBAND) {
        DMLOG(DM_LOG_ERROR, "Disband group failed");
    }
}

char *HichainAuthenCallBack::onRequest(int64_t requestId, int32_t operationCode, const char *reqParams)
{
    if (operationCode != GroupOperationCode::MEMBER_JOIN) {
        DMLOG(DM_LOG_ERROR, "HichainAuthenCallBack::onRequest operationCode %d", operationCode);
        return nullptr;
    }

    int32_t pinCode = AuthManager::GetInstance().GetPincode(requestId);
    nlohmann::json jsonObj;
    if (pinCode == FAIL) {
        jsonObj[FIELD_CONFIRMATION] = REQUEST_REJECTED;
    } else {
        jsonObj[FIELD_CONFIRMATION] = REQUEST_ACCEPTED;
    }
    jsonObj[FIELD_PIN_CODE] = std::to_string(pinCode).c_str();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;

    std::string jsonStr = jsonObj.dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}
}
}
