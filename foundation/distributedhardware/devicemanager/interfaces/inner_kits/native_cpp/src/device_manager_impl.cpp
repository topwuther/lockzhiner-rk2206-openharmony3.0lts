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

#include "device_manager_impl.h"

#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "device_manager_notify.h"
#include "constants.h"

#include "ipc_authenticate_device_req.h"
#include "ipc_check_authenticate_req.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_rsp.h"
#include "ipc_req.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_get_authenticationparam_rsp.h"
#include "ipc_set_useroperation_req.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerImpl);

int32_t DeviceManagerImpl::InitDeviceManager(std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::InitDeviceManager start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || dmInitCallback == nullptr) {
        DMLOG(DM_LOG_ERROR, "InitDeviceManager error: Invalid parameter");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    int32_t ret = ipcClientProxy_->Init(pkgName);
    if (ret != DEVICEMANAGER_OK) {
        return ret;
    }
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    DMLOG(DM_LOG_INFO, "success");
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::UnInitDeviceManager(std::string &pkgName)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::UnInitDeviceManager start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "InitDeviceManager error: Invalid parameter");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    int32_t ret = ipcClientProxy_->UnInit(pkgName);
    if (ret != DEVICEMANAGER_OK) {
        return ret;
    }
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    DMLOG(DM_LOG_INFO, "DeviceManager::UnInitDeviceManager completed with errcode %d", ret);
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(std::string &pkgName, std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::GetTrustedDeviceList start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    std::shared_ptr<IpcGetTrustdeviceReq> req = std::make_shared<IpcGetTrustdeviceReq>();
    std::shared_ptr<IpcGetTrustdeviceRsp> rsp = std::make_shared<IpcGetTrustdeviceRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    if (ipcClientProxy_->SendRequest(GET_TRUST_DEVICE_LIST, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }
    if (rsp->GetErrCode() == DEVICEMANAGER_OK) {
        deviceList = rsp->GetDeviceVec();
    }
    DMLOG(DM_LOG_INFO, "DeviceManager::GetTrustedDeviceList completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(std::string &pkgName, std::string &extra,
    std::shared_ptr<DeviceStateCallback> callback)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::RegisterDevStateCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || callback == nullptr) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    DMLOG(DM_LOG_INFO, "DeviceManager::RegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(std::string &pkgName)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::UnRegisterDevStateCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    DMLOG(DM_LOG_INFO, "DeviceManager::UnRegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::StartDeviceDiscovery(std::string &pkgName, DmSubscribeInfo &subscribeInfo,
    std::shared_ptr<DiscoverCallback> callback)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::StartDeviceDiscovery start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || callback == nullptr) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DMLOG(DM_LOG_INFO, "DeviceManager StartDeviceDiscovery in, pkgName %s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterDiscoverCallback(pkgName, subscribeInfo.subscribeId, callback);

    std::shared_ptr<IpcStartDiscoveryReq> req = std::make_shared<IpcStartDiscoveryReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetSubscribeInfo(subscribeInfo);
    if (ipcClientProxy_->SendRequest(START_DEVICE_DISCOVER, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "DeviceManager StartDeviceDiscovery Failed with ret %d", ret);
        return ret;
    }
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(std::string &pkgName, uint16_t subscribeId)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::StopDeviceDiscovery start , pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DMLOG(DM_LOG_INFO, "StopDeviceDiscovery in, pkgName %s", pkgName.c_str());
    std::shared_ptr<IpcStopDiscoveryReq> req = std::make_shared<IpcStopDiscoveryReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetSubscribeId(subscribeId);
    if (ipcClientProxy_->SendRequest(STOP_DEVICE_DISCOVER, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "StopDeviceDiscovery Failed with ret %d", ret);
        return ret;
    }
    DeviceManagerNotify::GetInstance().UnRegisterDiscoverCallback(pkgName, subscribeId);
    DMLOG(DM_LOG_INFO, "DeviceManager::StopDeviceDiscovery completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::AuthenticateDevice(std::string &pkgName, const DmDeviceInfo &deviceInfo,
    const DmAppImageInfo &imageInfo, std::string &extra, std::shared_ptr<AuthenticateCallback> callback)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::AuthenticateDevice start , pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DMLOG(DM_LOG_INFO, "AuthenticateDevice in, pkgName %s", pkgName.c_str());
    std::shared_ptr<IpcAuthenticateDeviceReq> req = std::make_shared<IpcAuthenticateDeviceReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    req->SetDeviceInfo(deviceInfo);
    req->SetAppImageInfo(imageInfo);
    if (ipcClientProxy_->SendRequest(AUTHENTICATE_DEVICE, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice Failed with ret %d", ret);
        return ret;
    }
    std::string strDeviceId = deviceInfo.deviceId;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, strDeviceId, callback);
    DMLOG(DM_LOG_INFO, "DeviceManager::AuthenticateDevice completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::CheckAuthentication(std::string &pkgName, std::string &authPara,
    std::shared_ptr<CheckAuthCallback> callback)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::CheckAuthentication start , pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    DMLOG(DM_LOG_INFO, "CheckAuthentication in, pkgName %s", pkgName.c_str());
    std::shared_ptr<IpcCheckAuthenticateReq> req = std::make_shared<IpcCheckAuthenticateReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetAuthPara(authPara);
    if (ipcClientProxy_->SendRequest(CHECK_AUTHENTICATION, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "CheckAuthentication Failed with ret %d", ret);
        return ret;
    }
    DeviceManagerNotify::GetInstance().RegisterCheckAuthenticationCallback(pkgName, authPara, callback);
    DMLOG(DM_LOG_INFO, "DeviceManager::CheckAuthentication completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::GetAuthenticationParam(std::string &pkgName, DmAuthParam &authParam)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::GetAuthenticationParam start");
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcGetAuthParamRsp> rsp = std::make_shared<IpcGetAuthParamRsp>();
    req->SetPkgName(pkgName);
    if (ipcClientProxy_->SendRequest(SERVER_GET_AUTHENTCATION_INFO, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }
    authParam = rsp->GetAuthParam();
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::SetUserOperation(std::string &pkgName, int32_t action)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::SetUserOperation start");
    std::shared_ptr<IpcGetOperationReq> req = std::make_shared<IpcGetOperationReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();

    req->SetPkgName(pkgName);
    req->SetOperation(action);

    if (ipcClientProxy_->SendRequest(SERVER_USER_AUTHORIZATION_OPERATION, req, rsp) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_IPC_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "CheckAuthentication Failed with ret %d", ret);
        return ret;
    }
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::RegisterDeviceManagerFaCallback(std::string &packageName,
    std::shared_ptr<DeviceManagerFaCallback> callback)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::RegisterDeviceManagerFaCallback start, pkgName: %s", packageName.c_str());
    if (packageName.empty() || callback == nullptr) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(packageName, callback);
    DMLOG(DM_LOG_INFO, "DeviceManager::RegisterDevStateCallback completed, pkgName: %s", packageName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t DeviceManagerImpl::UnRegisterDeviceManagerFaCallback(std::string &pkgName)
{
    DMLOG(DM_LOG_INFO, "DeviceManager::UnRegisterDeviceManagerFaCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    DMLOG(DM_LOG_INFO, "DeviceManager::UnRegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
