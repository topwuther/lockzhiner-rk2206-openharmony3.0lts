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
#include <random>

#include "securec.h"

#include "anonymous_string.h"
#include "auth_manager.h"
#include "constants.h"
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "dm_ability_manager.h"
#include "encrypt_utils.h"
#include "ipc_server_adapter.h"
#include "ipc_server_listener.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(IpcServerAdapter);

int32_t IpcServerAdapter::CheckParamValid(nlohmann::json &extraJson, const DmAppImageInfo &imageInfo)
{
    if (!extraJson.contains(APP_NAME_KEY) ||
        !extraJson.contains(APP_DESCRIPTION_KEY) ||
        !extraJson.contains(AUTH_TYPE)) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    std::string appName = extraJson[APP_NAME_KEY];
    std::string appDescription = extraJson[APP_DESCRIPTION_KEY];

    if (appName.empty() || appDescription.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid app image info");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    if (extraJson[AUTH_TYPE] != AUTH_TYPE_PIN) {
        DMLOG(DM_LOG_ERROR, "invalid auth type, only support pin auth");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    return DEVICEMANAGER_OK;
}

int32_t IpcServerAdapter::GenRandInt(int32_t randMin, int32_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
}

int32_t IpcServerAdapter::ModuleInit()
{
    if (SoftbusAdapter::Init() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "softbus adapter init failed");
        return DEVICEMANAGER_INIT_FAILED;
    }
    if (HichainConnector::GetInstance().Init() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "hichain connector init failed");
        return DEVICEMANAGER_INIT_FAILED;
    }
    return DEVICEMANAGER_OK;
}

int32_t IpcServerAdapter::GetTrustedDeviceList(std::string &pkgName, std::string &extra,
    DmDeviceInfo **info, int32_t *infoNum)
{
    if (info == nullptr || infoNum == nullptr) {
        return DEVICEMANAGER_NULLPTR;
    }
    DMLOG(DM_LOG_INFO, "In, pkgName: %s", pkgName.c_str());
    NodeBasicInfo *nodeInfo = nullptr;
    *info = nullptr;
    *infoNum = 0;
    int32_t ret = SoftbusAdapter::GetTrustDevices(pkgName, &nodeInfo, infoNum);
    if (ret != DEVICEMANAGER_OK || *infoNum <= 0 || nodeInfo == nullptr) {
        DMLOG(DM_LOG_ERROR, "GetTrustDevices errCode:%d, num:%d", ret, *infoNum);
        return ret;
    }
    *info = (DmDeviceInfo *)malloc(sizeof(DmDeviceInfo) * (*infoNum));
    if (*info == nullptr) {
        FreeNodeInfo(nodeInfo);
        return DEVICEMANAGER_MALLOC_ERROR;
    }
    for (int32_t i = 0; i < *infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo *deviceInfo = *info + i;
        if (memcpy_s(deviceInfo->deviceId, sizeof(deviceInfo->deviceId), nodeBasicInfo->networkId,
            std::min(sizeof(deviceInfo->deviceId), sizeof(nodeBasicInfo->networkId))) != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_ERROR, "memcpy failed");
        }
        if (memcpy_s(deviceInfo->deviceName, sizeof(deviceInfo->deviceName), nodeBasicInfo->deviceName,
            std::min(sizeof(deviceInfo->deviceName), sizeof(nodeBasicInfo->deviceName))) != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_ERROR, "memcpy failed");
        }
        deviceInfo->deviceTypeId = (DMDeviceType)nodeBasicInfo->deviceTypeId;
    }
    FreeNodeInfo(nodeInfo);
    DMLOG(DM_LOG_INFO, "success, pkgName:%s, deviceCount %d", pkgName.c_str(), *infoNum);
    return DEVICEMANAGER_OK;
}

int32_t IpcServerAdapter::StartDeviceDiscovery(std::string &pkgName, DmSubscribeInfo &dmSubscribeInfo)
{
    DMLOG(DM_LOG_INFO, "In, pkgName: %s, subscribeId %d", pkgName.c_str(),
        (int32_t)dmSubscribeInfo.subscribeId);

    DMLOG(DM_LOG_INFO, "capability: %s", dmSubscribeInfo.capability);
    SubscribeInfo subscribeInfo;

    subscribeInfo.subscribeId = dmSubscribeInfo.subscribeId;
    subscribeInfo.mode = (DiscoverMode)dmSubscribeInfo.mode;
    subscribeInfo.medium = (ExchanageMedium)dmSubscribeInfo.medium;
    subscribeInfo.freq = (ExchangeFreq)dmSubscribeInfo.freq;
    subscribeInfo.isSameAccount = dmSubscribeInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubscribeInfo.isWakeRemote;
    subscribeInfo.capability = dmSubscribeInfo.capability;
    subscribeInfo.capabilityData = nullptr;
    subscribeInfo.dataLen = 0;
    return SoftbusAdapter::StartDiscovery(pkgName, &subscribeInfo);
}

int32_t IpcServerAdapter::StopDiscovery(std::string &pkgName, uint16_t subscribeId)
{
    DMLOG(DM_LOG_INFO, "In, pkgName: %s, subscribeId %d", pkgName.c_str(), (int32_t)subscribeId);
    return SoftbusAdapter::StopDiscovery(pkgName, subscribeId);
}

int32_t IpcServerAdapter::AuthenticateDevice(std::string &pkgName, const DmDeviceInfo &deviceInfo,
    const DmAppImageInfo &imageInfo, std::string &extra)
{
    if (pkgName.empty() || extra.empty()) {
        DMLOG(DM_LOG_ERROR, "invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(extra, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice extra jsonStr error");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    int32_t ret = CheckParamValid(jsonObject, imageInfo);
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice para invalid, ret %d", ret);
        return ret;
    }
    DMLOG(DM_LOG_INFO, "AuthenticateDevice In, pkgName: %s, deviceId %s", pkgName.c_str(),
        GetAnonyString(deviceInfo.deviceId).c_str());

    AuthManager::GetInstance().AuthDeviceGroup(pkgName, deviceInfo, imageInfo, extra);
    return DEVICEMANAGER_OK;
}

int32_t IpcServerAdapter::CheckAuthentication(std::string &authPara)
{
    if (authPara.empty()) {
        DMLOG(DM_LOG_INFO, " DeviceManagerIpcAdapter::CheckAuthentication check authPara failed");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    DMLOG(DM_LOG_INFO, " DeviceManagerIpcAdapter::CheckAuthentication");
    return AuthManager::GetInstance().CheckAuthentication(authPara);
}

int32_t IpcServerAdapter::GetAuthenticationParam(std::string &pkgName, DmAuthParam &authParam)
{
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DmAbilityManager::GetInstance().StartAbilityDone();
    AuthManager::GetInstance().GetAuthenticationParam(authParam);
    return DEVICEMANAGER_OK;
}

int32_t IpcServerAdapter::SetUserOperation(std::string &pkgName, int32_t action)
{
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    AuthManager::GetInstance().OnUserOperate(action);
    return SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
