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

#include "constants.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_check_auth_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_server_adapter.h"
#include "ipc_server_stub.h"
#include "ipc_notify_dmfa_result_req.h"

#include "device_manager_errno.h"
#include "device_manager_log.h"

namespace OHOS {
namespace DistributedHardware {
ON_IPC_SET_REQUEST(SERVER_DEVICE_STATE_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::static_pointer_cast<IpcNotifyDeviceStateReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t deviceState = pReq->GetDeviceState();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(deviceState)) {
        DMLOG(DM_LOG_ERROR, "write state failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteRawData(&deviceInfo, sizeof(DmDeviceInfo))) {
        DMLOG(DM_LOG_ERROR, "write deviceInfo failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_STATE_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FOUND, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::static_pointer_cast<IpcNotifyDeviceFoundReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt16(subscribeId)) {
        DMLOG(DM_LOG_ERROR, "write subscribeId failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteRawData(&deviceInfo, sizeof(DmDeviceInfo))) {
        DMLOG(DM_LOG_ERROR, "write deviceInfo failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FOUND, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_DISCOVER_FINISH, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::static_pointer_cast<IpcNotifyDiscoverResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    int32_t result = pReq->GetResult();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt16(subscribeId)) {
        DMLOG(DM_LOG_ERROR, "write subscribeId failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DISCOVER_FINISH, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyAuthResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t pinToken = pReq->GetPinToken();
    int32_t status = pReq->GetStatus();
    int32_t reason = pReq->GetReason();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteString(deviceId)) {
        DMLOG(DM_LOG_ERROR, "write deviceId failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pinToken)) {
        DMLOG(DM_LOG_ERROR, "write pinToken failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(status)) {
        DMLOG(DM_LOG_ERROR, "write status failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(reason)) {
        DMLOG(DM_LOG_ERROR, "write reason failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_AUTH_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_CHECK_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    std::shared_ptr<IpcNotifyCheckAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyCheckAuthResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t result = pReq->GetResult();
    int32_t flag = pReq->GetFlag();
    if (!data.WriteString(pkgName)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteString(deviceId)) {
        DMLOG(DM_LOG_ERROR, "write deviceId failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(flag)) {
        DMLOG(DM_LOG_ERROR, "write flag failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_CHECK_AUTH_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(GET_TRUST_DEVICE_LIST, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    DMLOG(DM_LOG_INFO, "pkgName:%s, extra:%s", pkgName.c_str(), extra.c_str());
    DmDeviceInfo *info = nullptr;
    int32_t infoNum = 0;
    int32_t result = IpcServerAdapter::GetInstance().GetTrustedDeviceList(pkgName, extra, &info, &infoNum);
    reply.WriteInt32(infoNum);
    if (infoNum > 0 && info != nullptr) {
        if (!reply.WriteRawData(info, sizeof(DmDeviceInfo) * infoNum)) {
            DMLOG(DM_LOG_ERROR, "write subscribeInfo failed");
        }
        free(info);
    }
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(REGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    sptr<IRemoteObject> listener = data.ReadRemoteObject();
    int32_t result = IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, listener);
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(UNREGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(pkgName);
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(START_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmSubscribeInfo *subscribeInfo = (DmSubscribeInfo *)data.ReadRawData(sizeof(DmSubscribeInfo));
    int32_t result = DEVICEMANAGER_NULLPTR;

    if (subscribeInfo != nullptr) {
        DMLOG(DM_LOG_INFO, "pkgName:%s, subscribeId: %d", pkgName.c_str(), subscribeInfo->subscribeId);
        result = IpcServerAdapter::GetInstance().StartDeviceDiscovery(pkgName, *subscribeInfo);
    }
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(STOP_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    uint16_t subscribeId = data.ReadInt32();
    DMLOG(DM_LOG_INFO, "pkgName:%s, subscribeId: %d", pkgName.c_str(), subscribeId);
    int32_t result = IpcServerAdapter::GetInstance().StopDiscovery(pkgName, subscribeId);
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(AUTHENTICATE_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    DmDeviceInfo *deviceInfo = (DmDeviceInfo *)data.ReadRawData(sizeof(DmDeviceInfo));
    int32_t appIconLen = data.ReadInt32();
    int32_t appThumbnailLen = data.ReadInt32();
    uint8_t *appIcon = appIconLen > 0? (uint8_t *)data.ReadRawData(appIconLen) : nullptr;
    uint8_t *appThumbnail = appThumbnailLen > 0? (uint8_t *)data.ReadRawData(appThumbnailLen) : nullptr;

    DmAppImageInfo imageInfo(appIcon, appIconLen, appThumbnail, appThumbnailLen);
    int32_t result = DEVICEMANAGER_OK;

    if (deviceInfo != nullptr) {
        result = IpcServerAdapter::GetInstance().AuthenticateDevice(pkgName, *deviceInfo, imageInfo, extra);
    }
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(CHECK_AUTHENTICATION, MessageParcel &data, MessageParcel &reply)
{
    std::string authPara = data.ReadString();
    int32_t result = IpcServerAdapter::GetInstance().CheckAuthentication(authPara);
    if (!reply.WriteInt32(result)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_GET_AUTHENTCATION_INFO, MessageParcel &data, MessageParcel &reply)
{
    std::string packName = data.ReadString();
    DmAuthParam authParam;
    int32_t ret = DEVICEMANAGER_OK;
    DMLOG(DM_LOG_ERROR, "DeviceManagerStub:: GET_AUTHENTCATION_INFO:pkgName:%s", packName.c_str());
    IpcServerAdapter::GetInstance().GetAuthenticationParam(packName, authParam);
    if (authParam.direction == AUTH_SESSION_SIDE_CLIENT) {
        if (!reply.WriteInt32(authParam.direction) || !reply.WriteInt32(authParam.authType) ||
            !reply.WriteInt32(authParam.pinToken)) {
            DMLOG(DM_LOG_ERROR, "DeviceManagerStub::wirte client fail");
            ret = DEVICEMANAGER_WRITE_FAILED;
        }
        return ret;
    }

    int32_t appIconLen = authParam.imageinfo.GetAppIconLen();
    int32_t appThumbnailLen = authParam.imageinfo.GetAppThumbnailLen();
    if (!reply.WriteInt32(authParam.direction) || !reply.WriteInt32(authParam.authType) ||
        !reply.WriteString(authParam.packageName) || !reply.WriteString(authParam.appName) ||
        !reply.WriteString(authParam.appDescription) || !reply.WriteInt32(authParam.business) ||
        !reply.WriteInt32(authParam.pincode) || !reply.WriteInt32(appIconLen) ||
        !reply.WriteInt32(appThumbnailLen)) {
        DMLOG(DM_LOG_ERROR, "write reply failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }

    if (appIconLen > 0 && authParam.imageinfo.GetAppIcon() != nullptr) {
        if (!reply.WriteRawData(authParam.imageinfo.GetAppIcon(), appIconLen)) {
            DMLOG(DM_LOG_ERROR, "write appIcon failed");
            return DEVICEMANAGER_WRITE_FAILED;
        }
    }

    if (appThumbnailLen > 0 && authParam.imageinfo.GetAppThumbnail() != nullptr) {
        if (!reply.WriteRawData(authParam.imageinfo.GetAppThumbnail(), appThumbnailLen)) {
            DMLOG(DM_LOG_ERROR, "write appThumbnail failed");
            return DEVICEMANAGER_WRITE_FAILED;
        }
    }

    return DEVICEMANAGER_OK;
}

ON_IPC_CMD(SERVER_USER_AUTHORIZATION_OPERATION, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int32_t action = data.ReadInt32();
    int result = IpcServerAdapter::GetInstance().SetUserOperation(packageName, action);

    if (!reply.WriteInt32(action)) {
        DMLOG(DM_LOG_ERROR, "write result failed");
        return DEVICEMANAGER_WRITE_FAILED;
    }
    return result;
}

ON_IPC_SET_REQUEST(SERVER_DEVICEMANAGER_FA_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel& data)
{
    DMLOG(DM_LOG_INFO, "OnFaCallBack");
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);
    std::string packagname = pReq->GetPkgName();
    std::string paramJson = pReq->GetJsonParam();
    if (!data.WriteString(packagname)) {
        DMLOG(DM_LOG_ERROR, "write pkgName failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    if (!data.WriteString(paramJson)) {
        DMLOG(DM_LOG_ERROR, "write paramJson failed");
        return DEVICEMANAGER_FLATTEN_OBJECT;
    }
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICEMANAGER_FA_NOTIFY, MessageParcel& reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
