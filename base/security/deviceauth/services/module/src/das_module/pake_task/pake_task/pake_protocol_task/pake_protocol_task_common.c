/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "pake_protocol_task_common.h"
#include "hc_log.h"
#include "hc_types.h"
#include "module_common.h"
#include "pake_protocol_common.h"
#include "pake_task_common.h"

void DestroyDasPakeParams(PakeParams *params)
{
    if (params == NULL) {
        return;
    }

    DestroyPakeBaseParams(&(params->baseParams));

    if (params->returnKey.val != NULL) {
        (void)memset_s(params->returnKey.val, params->returnKey.length, 0, params->returnKey.length);
        HcFree(params->returnKey.val);
        params->returnKey.val = NULL;
    }

    HcFree(params->packageName);
    params->packageName = NULL;

    HcFree(params->serviceType);
    params->serviceType = NULL;

    HcFree(params->nonce.val);
    params->nonce.val = NULL;
}

static int32_t FillReturnKey(PakeParams *params, const CJson *in)
{
    if (params->opCode == OP_UNBIND) {
        params->returnKey.val = NULL;
        params->returnKey.length = 0;
        return HC_SUCCESS;
    }
    int32_t res = GetIntFromJson(in, FIELD_KEY_LENGTH, (int *)&(params->returnKey.length));
    if (res != HC_SUCCESS) {
        LOGD("Get key length failed, use default, res: %d", res);
        params->returnKey.length = DEFAULT_RETURN_KEY_LENGTH;
    }
    if (params->returnKey.length < MIN_OUTPUT_KEY_LEN || params->returnKey.length > MAX_OUTPUT_KEY_LEN) {
        LOGE("Output key length is invalid.");
        return HC_ERR_INVALID_LEN;
    }
    res = InitSingleParam(&params->returnKey, params->returnKey.length);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for returnKey failed, res: %d.", res);
    }
    return res;
}

int32_t InitDasPakeParams(PakeParams *params, const CJson *in)
{
    if (params == NULL || in == NULL) {
        LOGE("%s; %s;", (params == NULL) ? "params is null" : "params is not null",
            (in == NULL) ? "in is null" : "in is not null");
        return HC_ERR_INVALID_PARAMS;
    }

    int32_t res = InitPakeBaseParams(&(params->baseParams));
    if (res != HC_SUCCESS) {
        LOGE("InitPakeBaseParams failed, res: %d.", res);
        goto err;
    }

    res = FillDasPakeParams(params, in);
    if (res != HC_SUCCESS) {
        LOGE("FillDasPakeParams failed, res: %d.", res);
        goto err;
    }

    res = FillReturnKey(params, in);
    if (res != HC_SUCCESS) {
        LOGE("FillReturnKey failed, res: %d.", res);
        goto err;
    }

    return HC_SUCCESS;
err:
    DestroyDasPakeParams(params);
    return res;
}