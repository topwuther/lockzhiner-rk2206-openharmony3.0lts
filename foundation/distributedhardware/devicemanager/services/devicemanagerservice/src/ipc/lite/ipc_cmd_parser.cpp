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

#include "device_manager_errno.h"
#include "device_manager_log.h"

#include "ipc_def.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_check_auth_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_server_adapter.h"
#include "ipc_server_stub.h"

namespace OHOS {
namespace DistributedHardware {
ON_IPC_SET_REQUEST(SERVER_DEVICE_STATE_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::static_pointer_cast<IpcNotifyDeviceStateReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t deviceState = pReq->GetDeviceState();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushInt32(&request, deviceState);
    IpcIoPushFlatObj(&request, &deviceInfo, sizeof(DmDeviceInfo));
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_STATE_NOTIFY, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FOUND, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::static_pointer_cast<IpcNotifyDeviceFoundReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushUint16(&request, subscribeId);
    IpcIoPushFlatObj(&request, &deviceInfo, sizeof(DmDeviceInfo));
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FOUND, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_DISCOVER_FINISH, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::static_pointer_cast<IpcNotifyDiscoverResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    int32_t result = pReq->GetResult();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushUint16(&request, subscribeId);
    IpcIoPushInt32(&request, result);
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DISCOVER_FINISH, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyAuthResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t pinToken = pReq->GetPinToken();
    int32_t status = pReq->GetStatus();
    int32_t reason = pReq->GetReason();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushString(&request, deviceId.c_str());
    IpcIoPushInt32(&request, pinToken);
    IpcIoPushInt32(&request, status);
    IpcIoPushInt32(&request, reason);
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_AUTH_RESULT, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SET_REQUEST(SERVER_CHECK_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcNotifyCheckAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyCheckAuthResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t result = pReq->GetResult();
    int32_t flag = pReq->GetFlag();

    IpcIoInit(&request, buffer, buffLen, 0);
    IpcIoPushString(&request, pkgName.c_str());
    IpcIoPushString(&request, deviceId.c_str());
    IpcIoPushInt32(&request, result);
    IpcIoPushInt32(&request, flag);
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_CHECK_AUTH_RESULT, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}

ON_IPC_SERVER_CMD(GET_TRUST_DEVICE_LIST, IpcIo &req, IpcIo &reply)
{
    DMLOG(DM_LOG_INFO, "enter GetTrustedDeviceList.");
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&req, &len);
    std::string extra = (const char *)IpcIoPopString(&req, &len);
    DmDeviceInfo *info = nullptr;
    int32_t infoNum = 0;
    int32_t ret = IpcServerAdapter::GetInstance().GetTrustedDeviceList(pkgName, extra, &info, &infoNum);
    IpcIoPushInt32(&reply, infoNum);
    if (infoNum > 0) {
        IpcIoPushFlatObj(&reply, info, sizeof(DmDeviceInfo) * infoNum);
        free(info);
    }
    IpcIoPushInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(REGISTER_DEVICE_MANAGER_LISTENER, IpcIo &req, IpcIo &reply)
{
    int32_t errCode = RegisterDeviceManagerListener(&req, &reply);
    IpcIoPushInt32(&reply, errCode);
}

ON_IPC_SERVER_CMD(UNREGISTER_DEVICE_MANAGER_LISTENER, IpcIo &req, IpcIo &reply)
{
    int32_t errCode = UnRegisterDeviceManagerListener(&req, &reply);
    IpcIoPushInt32(&reply, errCode);
}

ON_IPC_SERVER_CMD(START_DEVICE_DISCOVER, IpcIo &req, IpcIo &reply)
{
    DMLOG(DM_LOG_INFO, "StartDeviceDiscovery service listener.");
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&req, &len);
    uint32_t size = 0;
    DmSubscribeInfo *pDmSubscribeInfo = (DmSubscribeInfo*)IpcIoPopFlatObj(&req, &size);

    int32_t ret = IpcServerAdapter::GetInstance().StartDeviceDiscovery(pkgName, *pDmSubscribeInfo);
    IpcIoPushInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(STOP_DEVICE_DISCOVER, IpcIo &req, IpcIo &reply)
{
    DMLOG(DM_LOG_INFO, "StopDeviceDiscovery service listener.");
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&req, &len);
    uint16_t subscribeId = IpcIoPopUint16(&req);
    int32_t ret = IpcServerAdapter::GetInstance().StopDiscovery(pkgName, subscribeId);
    IpcIoPushInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(AUTHENTICATE_DEVICE, IpcIo &req, IpcIo &reply)
{
    DMLOG(DM_LOG_INFO, "AuthenticateDevice service listener.");
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(&req, &len);
    size_t extraLen = 0;
    std::string extra = (const char *)IpcIoPopString(&req, &extraLen);
    uint32_t size;
    DmDeviceInfo *deviceInfo = (DmDeviceInfo*)IpcIoPopFlatObj(&req, &size);
    DmAppImageInfo imageInfo(nullptr, 0, nullptr, 0);
    int32_t ret = IpcServerAdapter::GetInstance().AuthenticateDevice(pkgName, *deviceInfo, imageInfo, extra);
    IpcIoPushInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(CHECK_AUTHENTICATION, IpcIo &req, IpcIo &reply)
{
    DMLOG(DM_LOG_INFO, "CheckAuthentication service listener.");
    size_t authParaLen = 0;
    std::string authPara = (const char *)IpcIoPopString(&req, &authParaLen);
    int32_t ret = IpcServerAdapter::GetInstance().CheckAuthentication(authPara);
    IpcIoPushInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(SERVER_GET_AUTHENTCATION_INFO, IpcIo &req, IpcIo &reply)
{
    size_t len = 0;
    std::string packName = (const char *)IpcIoPopString(&req, &len);
    DmAuthParam authParam = {0};
    DMLOG(DM_LOG_ERROR, "DeviceManagerStub:: GET_AUTHENTCATION_INFO:pkgName:%s", packName.c_str());
    IpcServerAdapter::GetInstance().GetAuthenticationParam(packName, authParam);
    if (authParam.direction == AUTH_SESSION_SIDE_CLIENT) {
        IpcIoPushInt32(&reply, authParam.direction);
        IpcIoPushInt32(&reply, authParam.authType);
        IpcIoPushInt32(&reply, authParam.pinToken);
        DMLOG(DM_LOG_DEBUG, "DeviceManagerStub::is Client so just return direction");
        return;
    }

    int32_t appIconLen = authParam.imageinfo.GetAppIconLen();
    int32_t appThumbnailLen = authParam.imageinfo.GetAppThumbnailLen();

    IpcIoPushInt32(&reply, authParam.direction);
    IpcIoPushInt32(&reply, authParam.authType);
    IpcIoPushString(&reply, authParam.packageName.c_str());
    IpcIoPushString(&reply, authParam.appName.c_str());
    IpcIoPushString(&reply, authParam.appDescription.c_str());
    IpcIoPushInt32(&reply, authParam.business);
    IpcIoPushInt32(&reply, authParam.pincode);
    IpcIoPushInt32(&reply, appIconLen);
    IpcIoPushInt32(&reply, appThumbnailLen);

    if (appIconLen > 0 && authParam.imageinfo.GetAppIcon() != nullptr) {
        IpcIoPushFlatObj(&reply, authParam.imageinfo.GetAppIcon(), appIconLen);
    }

    if (appThumbnailLen > 0 && authParam.imageinfo.GetAppThumbnail() != nullptr) {
        IpcIoPushFlatObj(&reply, authParam.imageinfo.GetAppThumbnail(), appThumbnailLen);
    }
}

ON_IPC_SERVER_CMD(SERVER_USER_AUTHORIZATION_OPERATION, IpcIo &req, IpcIo &reply)
{
    size_t len = 0;
    std::string packName = (const char *)IpcIoPopString(&req, &len);
    int32_t action = IpcIoPopInt32(&reply);
    IpcServerAdapter::GetInstance().SetUserOperation(packName, action);

    IpcIoPushInt32(&reply, action);
}

ON_IPC_SET_REQUEST(SERVER_DEVICEMANAGER_FA_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, IpcIo& request,
    uint8_t *buffer, size_t buffLen)
{
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);
    std::string packagname = pReq->GetPkgName();
    std::string paramJson = pReq->GetJsonParam();
    IpcIoPushString(&request, packagname.c_str());
    IpcIoPushString(&request, paramJson.c_str());
    return DEVICEMANAGER_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICEMANAGER_FA_NOTIFY, IpcIo& reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    pBaseRsp->SetErrCode(IpcIoPopInt32(&reply));
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
