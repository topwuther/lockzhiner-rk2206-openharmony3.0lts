/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hks_sa.h"

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_ipc_service.h"
#include "hks_client_service.h"

namespace OHOS {
namespace Security {
namespace Hks {
REGISTER_SYSTEM_ABILITY_BY_ID(HksService, SA_ID_KEYSTORE_SERVICE, true);

std::mutex HksService::instanceLock;
sptr<HksService> HksService::instance;
const uint32_t UID_ROOT = 0;
const uint32_t UID_SYSTEM = 1000;
const uint32_t MAX_MALLOC_LEN = 1 * 1024 * 1024; // 1 MB

using HksIpcHandlerFuncProc = void (*)(const struct HksBlob *msg, const uint8_t *context);

enum HksMessage {
    HKS_MSG_BASE = 0x3a400,

    HKS_MSG_GEN_KEY = HKS_MSG_BASE,
    HKS_MSG_IMPORT_KEY,
    HKS_MSG_EXPORT_PUBLIC_KEY,
    HKS_MSG_DELETE_KEY,
    HKS_MSG_GET_KEY_PARAMSET,
    HKS_MSG_KEY_EXIST,
    HKS_MSG_GENERATE_RANDOM,
    HKS_MSG_SIGN,
    HKS_MSG_SIGN_INIT,
    HKS_MSG_SIGN_UPDATE,
    HKS_MSG_SIGN_FINAL,
    HKS_MSG_VERIFY,
    HKS_MSG_VERIFY_INIT,
    HKS_MSG_VERIFY_UPDATE,
    HKS_MSG_VERIFY_FINAL,
    HKS_MSG_ENCRYPT,
    HKS_MSG_ENCRYPT_INIT,
    HKS_MSG_ENCRYPT_UPDATE,
    HKS_MSG_ENCRYPT_FINAL,
    HKS_MSG_DECRYPT,
    HKS_MSG_DECRYPT_INIT,
    HKS_MSG_DECRYPT_UPDATE,
    HKS_MSG_DECRYPT_FINAL,
    HKS_MSG_AGREE_KEY,
    HKS_MSG_DERIVE_KEY,
    HKS_MSG_MAC,
    HKS_MSG_MAC_INIT,
    HKS_MSG_MAC_UPDATE,
    HKS_MSG_MAC_FINAL,
    HKS_MSG_GET_KEY_INFO_LIST,
    HKS_MSG_ATTEST_KEY,
    HKS_MSG_GET_CERTIFICATE_CHAIN,
    HKS_MSG_WRAP_KEY,
    HKS_MSG_UNWRAP_KEY,
    HKS_MSG_SIGN_WITH_DEVICE_KEY,
    HKS_MSG_PROVISION,
    HKS_MSG_PROVISION_VERIFY,
    HKS_MSG_EXPORT_TRUST_CERTS,

    /* new cmd type must be added before HKS_MSG_MAX */
    HKS_MSG_MAX,
};

struct HksIpcEntryPoint {
    enum HksMessage msgId;
    HksIpcHandlerFuncProc handler;
};

static struct HksIpcEntryPoint g_hksIpcMessageHandler[] = {
    { HKS_MSG_GEN_KEY, HksIpcServiceGenerateKey },
    { HKS_MSG_IMPORT_KEY, HksIpcServiceImportKey },
    { HKS_MSG_EXPORT_PUBLIC_KEY, HksIpcServiceExportPublicKey },
    { HKS_MSG_DELETE_KEY, HksIpcServiceDeleteKey },
    { HKS_MSG_GET_KEY_PARAMSET, HksIpcServiceGetKeyParamSet },
    { HKS_MSG_KEY_EXIST, HksIpcServiceKeyExist },
    { HKS_MSG_GENERATE_RANDOM, HksIpcServiceGenerateRandom },
    { HKS_MSG_SIGN, HksIpcServiceSign },
    { HKS_MSG_SIGN_INIT, HksIpcServiceSignInit },
    { HKS_MSG_SIGN_UPDATE, HksIpcServiceSignUpdate },
    { HKS_MSG_SIGN_FINAL, HksIpcServiceSignFinal },
    { HKS_MSG_VERIFY, HksIpcServiceVerify },
    { HKS_MSG_VERIFY_INIT, HksIpcServiceVerifyInit },
    { HKS_MSG_VERIFY_UPDATE, HksIpcServiceVerifyUpdate },
    { HKS_MSG_VERIFY_FINAL, HksIpcServiceVerifyFinal },
    { HKS_MSG_ENCRYPT, HksIpcServiceEncrypt },
    { HKS_MSG_ENCRYPT_INIT, HksIpcServiceEncryptInit },
    { HKS_MSG_ENCRYPT_UPDATE, HksIpcServiceEncryptUpdate },
    { HKS_MSG_ENCRYPT_FINAL, HksIpcServiceEncryptFinal },
    { HKS_MSG_DECRYPT, HksIpcServiceDecrypt },
    { HKS_MSG_DECRYPT_INIT, HksIpcServiceDecryptInit },
    { HKS_MSG_DECRYPT_UPDATE, HksIpcServiceDecryptUpdate },
    { HKS_MSG_DECRYPT_FINAL, HksIpcServiceDecryptFinal },
    { HKS_MSG_AGREE_KEY, HksIpcServiceAgreeKey },
    { HKS_MSG_DERIVE_KEY, HksIpcServiceDeriveKey },
    { HKS_MSG_MAC, HksIpcServiceMac },
    { HKS_MSG_MAC_INIT, HksIpcServiceMacInit },
    { HKS_MSG_MAC_UPDATE, HksIpcServiceMacUpdate },
    { HKS_MSG_MAC_FINAL, HksIpcServiceMacFinal },
    { HKS_MSG_GET_KEY_INFO_LIST, HksIpcServiceGetKeyInfoList },
    { HKS_MSG_ATTEST_KEY, HksIpcServiceAttestKey },
    { HKS_MSG_GET_CERTIFICATE_CHAIN, HksIpcServiceGetCertificateChain },
    { HKS_MSG_WRAP_KEY, HksIpcServiceWrapKey },
    { HKS_MSG_UNWRAP_KEY, HksIpcServiceUnwrapKey },
    { HKS_MSG_SIGN_WITH_DEVICE_KEY, HksIpcServiceSignWithDeviceKey },
    { HKS_MSG_PROVISION, HksIpcServiceProvision },
    { HKS_MSG_PROVISION_VERIFY, HksIpcServiceProvisionVerify },
    { HKS_MSG_EXPORT_TRUST_CERTS, HksIpcServiceExportTrustCerts },
};

HksService::HksService(int saId, bool runOnCreate = true)
    : SystemAbility(saId, runOnCreate), registerToService_(false), runningState_(STATE_NOT_START)
{
    HKS_LOG_D("HksService");
}

HksService::~HksService()
{
    HKS_LOG_D("~HksService");
}

sptr<HksService> HksService::GetInstance()
{
    std::lock_guard<std::mutex> autoLock(instanceLock);
    if (instance == nullptr) {
        instance = new (std::nothrow) HksService(SA_ID_KEYSTORE_SERVICE, true);
    }

    return instance;
}

bool HksService::Init()
{
    HKS_LOG_I("HksService::Init Ready to init");

    if (!registerToService_) {
        if (!Publish(HksService::GetInstance())) {
            HKS_LOG_E("HksService::Init Publish Failed");
            return false;
        }
        HKS_LOG_I("HksService::Init Publish service success");
        registerToService_ = true;
    }

    int32_t ret = HksServiceInitialize();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Init hks service failed!");
        return false;
    }

    HKS_LOG_I("HksService::Init success.");
    return true;
}

bool HksService::CanRequest() const
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    return (callingUid == UID_ROOT) || (callingUid == UID_SYSTEM);
}

int HksService::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    // this is the temporary version which comments the descriptor check
    std::u16string descriptor = HksService::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();

    if (!CanRequest()) {
        return HW_PERMISSION_DENIED;
    }
    HKS_LOG_I("OnRemoteRequest code:%d", code);

    struct HksBlob srcData = { 0, NULL };
    srcData.size = (uint32_t)data.ReadUint32();
    if (srcData.size >= MAX_MALLOC_LEN) {
        HKS_LOG_E("HksBlob size is too large!");
        return HW_SYSTEM_ERROR;
    }

    srcData.data = (uint8_t *)HksMalloc(srcData.size);
    if (srcData.data == nullptr) {
        HKS_LOG_E("Malloc HksBlob failed.");
        return HW_SYSTEM_ERROR;
    }

    const uint8_t *pdata = data.ReadBuffer((size_t)srcData.size);
    if (pdata == nullptr) {
        HKS_FREE_BLOB(srcData);
        return HKS_ERROR_BAD_STATE;
    }
    if (memcpy_s(srcData.data, srcData.size, pdata, srcData.size) != EOK) {
        HKS_LOG_E("copy remote data failed!");
        HKS_FREE_BLOB(srcData);
        return HKS_ERROR_BAD_STATE;
    }

    uint32_t size = sizeof(g_hksIpcMessageHandler) / sizeof(g_hksIpcMessageHandler[0]);
    for (uint32_t i = 0; i < size; ++i) {
        if (code == g_hksIpcMessageHandler[i].msgId) {
            g_hksIpcMessageHandler[i].handler(&srcData, (const uint8_t *)&reply);
        }
    }

    HKS_FREE_BLOB(srcData);
    return NO_ERROR;
}

void HksService::OnStart()
{
    HKS_LOG_I("HksService OnStart");

    if (runningState_ == STATE_RUNNING) {
        HKS_LOG_I("HksService has already Started");
        return;
    }

    if (!Init()) {
        HKS_LOG_E("Failed to init HksService");
        return;
    }

    runningState_ = STATE_RUNNING;
    HKS_LOG_I("HksService start success.");
}

void HksService::OnStop()
{
    HKS_LOG_I("HksService Service OnStop");
    runningState_ = STATE_NOT_START;
    registerToService_ = false;
}
} // UniversalKeystore
} // Security
} // OHOS
