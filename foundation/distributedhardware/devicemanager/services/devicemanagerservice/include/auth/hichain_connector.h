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

#ifndef OHOS_HICHAIN_CONNECTOR_H
#define OHOS_HICHAIN_CONNECTOR_H

#include <string>
#include <memory>
#include <cstdint>

#include "nlohmann/json.hpp"

#include "device_auth.h"

#include "single_instance.h"
#include "msg_response_auth.h"

namespace OHOS {
namespace DistributedHardware {
enum {
    HICHAIN_SUCCESS = 0,
    GROUP_CREATE_FAILED = 1,
    MEMBER_ADD_FAILED = 2,
    CREATE_CHANNEL_FAILED = 3,
};

struct GroupInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;

    GroupInfo() : groupName(""), groupId(""), groupOwner(""), groupType(0), groupVisibility(0) {}
};

void from_json(const nlohmann::json& jsonObject, GroupInfo& groupInfo);

class HichainConnectorCallback {
public:
    virtual void OnGroupCreated(int64_t requestId, const std::string &groupId) = 0;
    virtual void OnMemberJoin(int64_t requestId, int32_t status) = 0;
};

class HichainAuthenCallBack {
public:
    static bool onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    static void onSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen);
    static void onFinish(int64_t requestId, int32_t operationCode, const char *returnData);
    static void onError(int64_t requestId, int32_t operationCode, int32_t errorCode, const char *errorReturn);
    static char *onRequest(int64_t requestId, int32_t operationCode, const char *reqParams);
};

class HichainConnector {
DECLARE_SINGLE_INSTANCE(HichainConnector);
public:
    int32_t Init();
    bool OnTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    int64_t GenRequestId();
    void RegisterConnectorCallback(std::shared_ptr<HichainConnectorCallback> callback);
    int32_t CreateGroup(int64_t requestId, const std::string &groupName);
    int32_t AddMemeber(std::string deviceId, std::shared_ptr<MsgResponseAuth> msgResponseAuth);
    void GetRelatedGroups(std::string DeviceId, std::vector<GroupInfo> &groupList);
    void SyncGroups(std::string deviceId, std::vector<std::string> &remoteGroupIdList);
    int32_t DelMemberFromGroup(std::string groupId, std::string deviceId);
    void DeleteGroup(std::string &groupId);
    void OnGroupCreated(int64_t requestId, const std::string &returnData);
    void GetSyncGroupList(std::vector<GroupInfo> &groupList, std::vector<std::string> &syncGroupList);
    void OnMemberJoin(int64_t requestId, int32_t status);
private:
    std::string GetConnectPara(std::string deviceId, std::shared_ptr<MsgResponseAuth> msgResponseAuth);
    bool IsGroupInfoInvalid(GroupInfo &group);

private:
    std::atomic<int32_t> requestIdIndex_ {0};
    std::shared_ptr<HichainConnectorCallback> hichainConnectorCallback_ = nullptr;
    const DeviceGroupManager *deviceGroupManager_ = nullptr;
    DeviceAuthCallback deviceAuthCallback_ = {
        .onTransmit = nullptr,
        .onFinish = HichainAuthenCallBack::onFinish,
        .onError = HichainAuthenCallBack::onError,
        .onRequest = HichainAuthenCallBack::onRequest,
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_ADAPTER_H
