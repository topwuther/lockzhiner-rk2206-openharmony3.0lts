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

#include "securec.h"
#include "constants.h"
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "device_manager_notify.h"

#include "ipc_def.h"
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
ON_IPC_SET_REQUEST(REGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcRegisterListenerReq> pReq = std::static_pointer_cast<IpcRegisterListenerReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    sptr<IRemoteObject> listener = pReq->GetListener();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(listener)) {
        DMLOG(DM_LOG_ERROR, "write listener failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::string pkgName = pBaseReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write papam failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(GET_TRUST_DEVICE_LIST, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcGetTrustdeviceReq> pReq = std::static_pointer_cast<IpcGetTrustdeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkg failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteString(extra)) {
        DMLOG(DM_LOG_ERROR, "write extra failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(GET_TRUST_DEVICE_LIST, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetTrustdeviceRsp> pRsp = std::static_pointer_cast<IpcGetTrustdeviceRsp>(pBaseRsp);
    int32_t deviceNum = reply.ReadInt32();
    int32_t deviceTotalSize = deviceNum * (int32_t)sizeof(DmDeviceInfo);
    if (deviceTotalSize > 0) {
        std::vector<DmDeviceInfo> deviceInfoVec;
        DmDeviceInfo *pDmDeviceinfo = (DmDeviceInfo *)reply.ReadRawData(deviceTotalSize);
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
    pRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(START_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcStartDiscoveryReq> pReq = std::static_pointer_cast<IpcStartDiscoveryReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    const DmSubscribeInfo dmSubscribeInfo = pReq->GetSubscribeInfo();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteRawData(&dmSubscribeInfo, sizeof(DmSubscribeInfo))) {
        DMLOG(DM_LOG_ERROR, "write subscribe info failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(START_DEVICE_DISCOVER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(STOP_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcStopDiscoveryReq> pReq = std::static_pointer_cast<IpcStopDiscoveryReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt16(subscribeId)) {
        DMLOG(DM_LOG_ERROR, "write subscribeId failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(STOP_DEVICE_DISCOVER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(AUTHENTICATE_DEVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcAuthenticateDeviceReq> pReq = std::static_pointer_cast<IpcAuthenticateDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra   = pReq->GetExtra();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    DmAppImageInfo imageInfo = pReq->GetAppImageInfo();

    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteString(extra)) {
        DMLOG(DM_LOG_ERROR, "write extra failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteRawData(&deviceInfo, sizeof(DmDeviceInfo))) {
        DMLOG(DM_LOG_ERROR, "write deviceInfo failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    int32_t appIconLen = imageInfo.GetAppIconLen();
    int32_t appThumbnailLen = imageInfo.GetAppThumbnailLen();
    if (!data.WriteInt32(appIconLen)) {
        DMLOG(DM_LOG_ERROR, "write imageinfo appicon len failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(appThumbnailLen)) {
        DMLOG(DM_LOG_ERROR, "write imageinfo appThumbnailLen failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (appIconLen > 0 && !data.WriteRawData(imageInfo.GetAppIcon(), appIconLen)) {
        DMLOG(DM_LOG_ERROR, "write imageinfo appIcon failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (appThumbnailLen > 0 && !data.WriteRawData(imageInfo.GetAppThumbnail(), appThumbnailLen)) {
        DMLOG(DM_LOG_ERROR, "write imageinfo appThumbnail failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(AUTHENTICATE_DEVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(CHECK_AUTHENTICATION, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcCheckAuthenticateReq> pReq = std::static_pointer_cast<IpcCheckAuthenticateReq>(pBaseReq);
    std::string authPara = pReq->GetAuthPara();
    if (!data.WriteString(authPara)) {
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(CHECK_AUTHENTICATION, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_DEVICE_STATE_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmDeviceState deviceState = static_cast<DmDeviceState>(data.ReadInt32());
    DmDeviceInfo dmDeviceInfo;
    size_t deviceSize = sizeof(DmDeviceInfo);
    void *deviceInfo = (void *)data.ReadRawData(deviceSize);
    if (deviceInfo != nullptr && memcpy_s(&dmDeviceInfo, deviceSize, deviceInfo, deviceSize) != 0) {
        reply.WriteInt32(DEVICEMANAGER_COPY_FAILED);
        return DEVICEMANAGER_OK;
    }
    switch (deviceState) {
        case DEVICE_STATE_ONLINE:
            DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, dmDeviceInfo);
            break;
        case DEVICE_STATE_OFFLINE:
            DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, dmDeviceInfo);
            break;
        case DEVICE_INFO_CHANGED:
            DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, dmDeviceInfo);
            break;
        default:
            DMLOG(DM_LOG_ERROR, "unknown device state:%d", deviceState);
            break;
    }
    reply.WriteInt32(DEVICEMANAGER_OK);
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_DEVICE_FOUND, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    uint16_t subscribeId = data.ReadInt16();
    DmDeviceInfo dmDeviceInfo;
    size_t deviceSize = sizeof(DmDeviceInfo);
    void *deviceInfo = (void *)data.ReadRawData(deviceSize);
    if (deviceInfo != nullptr && memcpy_s(&dmDeviceInfo, deviceSize, deviceInfo, deviceSize) != 0) {
        reply.WriteInt32(DEVICEMANAGER_COPY_FAILED);
        return DEVICEMANAGER_OK;
    }
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, dmDeviceInfo);
    reply.WriteInt32(DEVICEMANAGER_OK);
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_DISCOVER_FINISH, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    uint16_t subscribeId = data.ReadInt16();
    int32_t failedReason = data.ReadInt32();

    if (failedReason == DEVICEMANAGER_OK) {
        DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, subscribeId);
    } else {
        DeviceManagerNotify::GetInstance().OnDiscoverFailed(pkgName, subscribeId, failedReason);
    }
    reply.WriteInt32(DEVICEMANAGER_OK);
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_AUTH_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t pinToken = data.ReadInt32();
    int32_t status = data.ReadInt32();
    int32_t reason = data.ReadInt32();

    DeviceManagerNotify::GetInstance().OnAuthResult(pkgName, deviceId, pinToken, status, reason);
    reply.WriteInt32(DEVICEMANAGER_OK);
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_CHECK_AUTH_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t resultCode = data.ReadInt32();
    int32_t flag = data.ReadInt32();

    DeviceManagerNotify::GetInstance().OnCheckAuthResult(pkgName, deviceId, resultCode, flag);
    reply.WriteInt32(DEVICEMANAGER_OK);
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_GET_AUTHENTCATION_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string packagename = pReq->GetPkgName();
    if (!data.WriteString(packagename)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_GET_AUTHENTCATION_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetAuthParamRsp> pRsp = std::static_pointer_cast<IpcGetAuthParamRsp>(pBaseRsp);
    DmAuthParam authParam;
    authParam.direction = reply.ReadInt32();
    authParam.authType = reply.ReadInt32();
    if (authParam.direction == AUTH_SESSION_SIDE_CLIENT) {
        authParam.pinToken = reply.ReadInt32();
        pRsp->SetAuthParam(authParam);
        return DEVICEMANAGER_OK;
    }

    authParam.packageName = reply.ReadString();
    authParam.appName = reply.ReadString();
    authParam.appDescription = reply.ReadString();
    authParam.business = reply.ReadInt32();
    authParam.pincode = reply.ReadInt32();

    int32_t appIconLen = reply.ReadInt32();
    uint8_t *appIconBuffer = nullptr;
    int32_t appThumbnailLen = reply.ReadInt32();
    uint8_t *appThumbBuffer = nullptr;
    if (appIconLen > 0) {
        appIconBuffer = (uint8_t *)reply.ReadRawData(appIconLen);
    }
    if (appThumbnailLen > 0) {
        appThumbBuffer = (uint8_t *)reply.ReadRawData(appThumbnailLen);
    }
    authParam.imageinfo.Reset(appIconBuffer, appIconLen, appThumbBuffer, appThumbnailLen);
    pRsp->SetAuthParam(authParam);
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_USER_AUTHORIZATION_OPERATION, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcGetOperationReq> pReq = std::static_pointer_cast<IpcGetOperationReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t action = pReq->GetOperation();

    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(action)) {
        DMLOG(DM_LOG_ERROR, "write extra failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }

    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_USER_AUTHORIZATION_OPERATION, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_DEVICEMANAGER_FA_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    DMLOG(DM_LOG_INFO, "OnFaCallBack");
    std::string packagename = data.ReadString();
    std::string paramJson = data.ReadString();
    DMLOG(DM_LOG_INFO, "OnFaCallBack Packagename  is %s", packagename.c_str());
    DMLOG(DM_LOG_INFO, "OnFaCallBack Json is %s", paramJson.c_str());
    DeviceManagerNotify::GetInstance().OnFaCall(packagename, paramJson);

    if (!reply.WriteInt32(DEVICEMANAGER_OK)) {
        DMLOG(DM_LOG_ERROR, "write return failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
