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

#include "hks_response.h"

#include <dlfcn.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "ipc_skeleton.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_type_inner.h"

using namespace OHOS;

void HksSendResponse(const uint8_t *context, int32_t result, const struct HksBlob *response)
{
    if (context == nullptr) {
        HKS_LOG_E("SendResponse NULL Pointer");
        return;
    }

    MessageParcel *reply = (MessageParcel *)context;
    reply->WriteInt32(result);
    if (response == nullptr) {
        reply->WriteUint32(0);
    } else {
        reply->WriteUint32(response->size);
        reply->WriteBuffer(response->data, (size_t)response->size);
    }
}

int32_t HksGetProcessNameForIPC(const uint8_t *context, struct HksBlob *processName)
{
    if ((context == nullptr) || (processName == nullptr)) {
        HKS_LOG_D("Don't need get process name in hosp.");
        return HKS_SUCCESS;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    uint8_t *name = (uint8_t *)HksMalloc(sizeof(callingUid));
    if (name == NULL) {
        HKS_LOG_E("GetProcessName malloc failed.");
        return HKS_ERROR_MALLOC_FAIL;
    }

    (void)memcpy_s(name, sizeof(callingUid), &callingUid, sizeof(callingUid));
    processName->size = sizeof(callingUid);
    processName->data = name;
    return HKS_SUCCESS;
}
