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

#include "ipc_cmd_register.h"
#include "ipc_def.h"

#include "securec.h"
#include "constants.h"
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "device_manager_notify.h"

#include "dm_device_info.h"
#include "dm_subscribe_info.h"

#include "ipc_register_listener_req.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_check_authenticate_req.h"
#include "ipc_get_authenticationparam_rsp.h"
#include "ipc_set_useroperation_req.h"

namespace OHOS {
namespace DistributedHardware {
ON_IPC_SET_REQUEST(REGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcRegisterListenerReq> pReq = std::static_pointer_cast<IpcRegisterListenerReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    SvcIdentity svcIdentity = pReq->GetSvcIdentity();

    IpcIoInit(&request, buffer, buffLen, 1);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushSvc(&request, &svcIdentity);
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_DEVICE_MANAGER_LISTENER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_DEVICE_MANAGER_LISTENER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(GET_TRUST_DEVICE_LIST, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcGetTrustdeviceReq> pReq = std::static_pointer_cast<IpcGetTrustdeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushString(&request, extra.c_str());
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(GET_TRUST_DEVICE_LIST, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetTrustdeviceRsp> pRsp = std::static_pointer_cast<IpcGetTrustdeviceRsp>(pBaseRsp);
    int32_t deviceNum = IpcIoPopInt32(&reply);
    uint32_t deviceTotalSize = deviceNum * (int32_t)sizeof(DmDeviceInfo);

    if (deviceTotalSize > 0) {
        std::vector<DmDeviceInfo> deviceInfoVec;
        DmDeviceInfo *pDmDeviceinfo = (DmDeviceInfo *)IpcIoPopFlatObj(&reply, &deviceTotalSize);
        if (pDmDeviceinfo == nullptr) {
            DMLOG(DM_LOG_ERROR, "GetTrustedDeviceList read node info failed!");
            pRsp->SetErrCode(DEVICEMANAGER_IPC_TRANSACTION_FAILED);
            return DEVICEMANAGER_IPC_TRANSACTION_FAILED;
        }
        for (int32_t i = 0; i < deviceNum; ++i) {
            pDmDeviceinfo = pDmDeviceinfo + i;
            deviceInfoVec.emplace_back(*pDmDeviceinfo);
        }
        pRsp->SetDeviceVec(deviceInfoVec);
    }
    pRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(START_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcStartDiscoveryReq> pReq = std::static_pointer_cast<IpcStartDiscoveryReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    const DmSubscribeInfo dmSubscribeInfo = pReq->GetSubscribeInfo();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushFlatObj(&request, &dmSubscribeInfo, sizeof(DmSubscribeInfo));
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(START_DEVICE_DISCOVER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(STOP_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcStopDiscoveryReq> pReq = std::static_pointer_cast<IpcStopDiscoveryReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushUint16(&request, subscribeId);
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(STOP_DEVICE_DISCOVER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(AUTHENTICATE_DEVICE, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcAuthenticateDeviceReq> pReq = std::static_pointer_cast<IpcAuthenticateDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra   = pReq->GetExtra();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushString(&request, extra.c_str());
    IpcIoPushFlatObj(&request, &deviceInfo, sizeof(DmDeviceInfo));
    // L1 暂时没有考虑appimage校验（8k限制）
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(AUTHENTICATE_DEVICE, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(CHECK_AUTHENTICATION, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcCheckAuthenticateReq> pReq = std::static_pointer_cast<IpcCheckAuthenticateReq>(pBaseReq);
    std::string authPara = pReq->GetAuthPara();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, authPara.c_str());
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(CHECK_AUTHENTICATION, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_DEVICE_STATE_NOTIFY, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&reply, &len);
    DmDeviceState deviceState = static_cast<DmDeviceState>(IpcIoPopInt32(&reply));
    uint32_t size;
    const DmDeviceInfo *deviceInfo = (const DmDeviceInfo*)IpcIoPopFlatObj(&reply, &size);
    if (pkgName == "" || len == 0 || deviceInfo == NULL) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOnline, get para failed");
        return;
    }
    switch (deviceState) {
        case DEVICE_STATE_ONLINE:
            DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, *deviceInfo);
            break;
        case DEVICE_STATE_OFFLINE:
            DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, *deviceInfo);
            break;
        case DEVICE_INFO_CHANGED:
            DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, *deviceInfo);
            break;
        default:
            DMLOG(DM_LOG_ERROR, "unknown device state:%d", deviceState);
            break;
    }
}

ON_IPC_CMD(SERVER_DEVICE_FOUND, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&reply, &len);
    uint16_t subscribeId = IpcIoPopUint16(&reply);
    uint32_t size;
    const DmDeviceInfo *deviceInfo = (const DmDeviceInfo*)IpcIoPopFlatObj(&reply, &size);
    if (pkgName == "" || len == 0 || deviceInfo == NULL) {
        DMLOG(DM_LOG_ERROR, "OnDeviceChanged, get para failed");
        return;
    }
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, *deviceInfo);
}

ON_IPC_CMD(SERVER_DISCOVER_FINISH, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&reply, &len);
    uint16_t subscribeId = IpcIoPopUint16(&reply);
    int32_t failedReason = IpcIoPopInt32(&reply);

    if (pkgName == "" || len == 0) {
        DMLOG(DM_LOG_ERROR, "OnDiscoverySuccess, get para failed");
        return;
    }
    if (failedReason == DEVICEMANAGER_OK) {
        DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, subscribeId);
    } else {
        DeviceManagerNotify::GetInstance().OnDiscoverFailed(pkgName, subscribeId, failedReason);
    }
}

ON_IPC_CMD(SERVER_AUTH_RESULT, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&reply, &len);
    size_t devIdLen = 0;
    std::string deviceId = (const char *)IpcIoPopString(&reply, &devIdLen);
    int32_t pinToken = IpcIoPopInt32(&reply);
    int32_t status = IpcIoPopInt32(&reply);
    int32_t reason = IpcIoPopInt32(&reply);

    if (pkgName == "" || len == 0 || deviceId == "" || devIdLen == 0) {
        DMLOG(DM_LOG_ERROR, "OnAuthResult, get para failed");
        return;
    }
    DeviceManagerNotify::GetInstance().OnAuthResult(pkgName, deviceId, pinToken, status, reason);
}

ON_IPC_CMD(SERVER_CHECK_AUTH_RESULT, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&reply, &len);
    size_t devIdLen = 0;
    std::string deviceId = (const char *)IpcIoPopString(&reply, &devIdLen);
    int32_t resultCode = IpcIoPopInt32(&reply);
    int32_t flag = IpcIoPopInt32(&reply);

    if (pkgName == "" || len == 0 || deviceId == "" || devIdLen == 0) {
        DMLOG(DM_LOG_ERROR, "OnAuthResult, get para failed");
        return;
    }
    DeviceManagerNotify::GetInstance().OnCheckAuthResult(pkgName, deviceId, resultCode, flag);
}

ON_IPC_SET_REQUEST(SERVER_GET_AUTHENTCATION_INFO, std::shared_ptr<IpcReq> pBaseReq, IpcIo& request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string packagename = pReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, packagename.c_str());
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_GET_AUTHENTCATION_INFO, IpcIo& reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetAuthParamRsp> pRsp = std::static_pointer_cast<IpcGetAuthParamRsp>(pBaseRsp);
    DmAuthParam authParam = {0};
    authParam.direction = IpcIoPopInt32(&reply);
    authParam.authType = IpcIoPopInt32(&reply);
    if (authParam.direction == AUTH_SESSION_SIDE_CLIENT) {
        authParam.pinToken = IpcIoPopInt32(&reply);
        pRsp->SetAuthParam(authParam);
        return DEVICEMANAGER_OK;
    }
    size_t PackagerNamelen = 0;
    authParam.packageName = strdup((const char *)IpcIoPopString(&reply, &PackagerNamelen));
    size_t appNameLen = 0;
    authParam.appName = strdup((const char *)IpcIoPopString(&reply, &appNameLen));
    size_t appDesLen = 0;
    authParam.appDescription = strdup((const char *)IpcIoPopString(&reply, &appDesLen));
    authParam.business = IpcIoPopInt32(&reply);
    authParam.pincode = IpcIoPopInt32(&reply);

    uint32_t appIconLen = IpcIoPopInt32(&reply);
    uint8_t *appIconBuffer = nullptr;
    uint32_t appThumbnailLen = IpcIoPopInt32(&reply);
    uint8_t *appThumbBuffer = nullptr;

    if (appIconLen > 0) {
        appIconBuffer = (uint8_t *)IpcIoPopFlatObj(&reply, &appIconLen);
    }
    if (appThumbnailLen > 0) {
        appThumbBuffer = (uint8_t *)IpcIoPopFlatObj(&reply, &appThumbnailLen);
    }

    authParam.imageinfo.Reset(appIconBuffer, appIconLen, appThumbBuffer, appThumbnailLen);
    pRsp->SetAuthParam(authParam);
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_USER_AUTHORIZATION_OPERATION, std::shared_ptr<IpcReq> pBaseReq, IpcIo& request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcGetOperationReq> pReq = std::static_pointer_cast<IpcGetOperationReq>(pBaseReq);
    std::string pkgName= pReq->GetPkgName();
    int32_t action = pReq->GetOperation();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushInt32(&request, action);
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_USER_AUTHORIZATION_OPERATION, IpcIo& reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_DEVICEMANAGER_FA_NOTIFY, IpcIo &reply)
{
    size_t len = 0;
    std::string packagename = (const char *)IpcIoPopString(&reply, &len);
    size_t jsonLen = 0;
    std::string paramJson = (const char *)IpcIoPopString(&reply, &jsonLen);
    DeviceManagerNotify::GetInstance().OnFaCall(packagename, paramJson);
}
} // namespace DistributedHardware
} // namespace OHOS
