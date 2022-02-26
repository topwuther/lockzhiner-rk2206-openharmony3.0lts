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

#include "hks_request.h"

#include <securec.h>
#include "iservice_registry.h"
#include "hks_param.h"
#include "hks_log.h"

using namespace OHOS;

sptr<IRemoteObject> GetHksProxy();

namespace {
constexpr int SA_ID_KEYSTORE_SERVICE = 3510;
const std::u16string SA_KEYSTORE_SERVICE_DESCRIPTOR = u"ohos.security.hks.service";
static sptr<IRemoteObject> hksProxy = GetHksProxy();
}

sptr<IRemoteObject> GetHksProxy()
{
    if (hksProxy != nullptr) {
        HKS_LOG_D("GetHksProxy hksProxy already exist.");
        return hksProxy;
    }

    auto registry = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (registry == nullptr) {
        HKS_LOG_E("GetHksProxy registry is null");
        return nullptr;
    }

    hksProxy = registry->GetSystemAbility(SA_ID_KEYSTORE_SERVICE);
    if (hksProxy == nullptr) {
        HKS_LOG_E("GetHksProxy GetSystemAbility %d is null", SA_ID_KEYSTORE_SERVICE);
        return nullptr;
    }

    return hksProxy;
}

int32_t HksSendRequest(enum HksMessage type, const struct HksBlob *inBlob,
    struct HksBlob *outBlob, const struct HksParamSet *paramSet)
{
    if (GetHksProxy() == nullptr) {
        HKS_LOG_E("HwKeystoreProxy is null.");
        return HKS_ERROR_BAD_STATE;
    }
    enum HksSendType sendType = HKS_SEND_TYPE_SYNC;
    struct HksParam *sendTypeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_ASYNCHRONIZED, &sendTypeParam);
    if (ret == HKS_SUCCESS) {
        sendType = (enum HksSendType)sendTypeParam->uint32Param;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (sendType == HKS_SEND_TYPE_SYNC) {
        option = MessageOption::TF_SYNC;
    } else {
        option = MessageOption::TF_ASYNC;
    }

    data.WriteInterfaceToken(SA_KEYSTORE_SERVICE_DESCRIPTOR);
    data.WriteUint32(inBlob->size);
    data.WriteBuffer(inBlob->data, (size_t)inBlob->size);
    int error = hksProxy->SendRequest(type, data, reply, option);
    if (error != 0) {
        return error;
    }

    ret = reply.ReadInt32();
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    size_t outLen = reply.ReadUint32();
    if (outLen == 0) {
        if (outBlob != NULL) {
            outBlob->size = 0;
        }
        return ret;
    }

    if (CheckBlob(outBlob) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    const uint8_t *outData = reply.ReadBuffer(outLen);
    if (outData == nullptr) {
        return HKS_ERROR_BAD_STATE;
    }
    if (memcpy_s(outBlob->data, outBlob->size, outData, outLen) != EOK) {
        return HKS_ERROR_BAD_STATE;
    }
    outBlob->size = outLen;
    return HKS_SUCCESS;
}
