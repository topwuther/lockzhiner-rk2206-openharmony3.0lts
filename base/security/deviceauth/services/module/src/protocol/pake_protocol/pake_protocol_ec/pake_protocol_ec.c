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

#include "pake_protocol_ec.h"
#include "hc_log.h"
#include "module_common.h"

#define PAKE_PRIVATE_KEY_ANDMASK_HIGH 0xF8
#define PAKE_PRIVATE_KEY_ANDMASK_LOW  0x7F
#define PAKE_PRIVATE_KEY_ORMASK_LOW   0x40

uint32_t GetPakeEcAlg()
{
    return EC_SPEKE;
}

static int32_t GenerateEsk(PakeBaseParams *params)
{
    int32_t res = params->loader->generateRandom(&(params->eskSelf));
    if (res != HC_SUCCESS) {
        LOGE("CURVE_25519: GenerateRandom for eskSelf failed, res: %d.", res);
        FreeAndCleanKey(&params->eskSelf);
        return res;
    }
    params->eskSelf.val[PAKE_EC_KEY_LEN - 1] &= PAKE_PRIVATE_KEY_ANDMASK_HIGH;
    params->eskSelf.val[0] &= PAKE_PRIVATE_KEY_ANDMASK_LOW;
    params->eskSelf.val[0] |= PAKE_PRIVATE_KEY_ORMASK_LOW;
    return res;
}

static int32_t InitEcPakeParams(PakeBaseParams *params)
{
    params->eskSelf.length = PAKE_EC_KEY_LEN;
    params->innerKeyLen = PAKE_EC_KEY_LEN;
    int32_t res = InitSingleParam(&(params->eskSelf), params->eskSelf.length);
    if (res !=  HC_SUCCESS) {
        LOGE("InitSingleParam for eskSelf failed, res: %d.", res);
        return res;
    }
    res = InitSingleParam(&(params->epkSelf), params->innerKeyLen);
    if (res !=  HC_SUCCESS) {
        LOGE("InitSingleParam for epkSelf failed, res: %d.", res);
        return res;
    }
    res = InitSingleParam(&(params->base), params->innerKeyLen);
    if (res !=  HC_SUCCESS) {
        LOGE("InitSingleParam for base failed, res: %d.", res);
        return res;
    }
    return res;
}

int32_t GenerateEcPakeParams(PakeBaseParams *params, Uint8Buff *secret)
{
    if (params->curveType == CURVE_256) {
        LOGE("Unsupport curve type.");
        return HC_ERROR;
    }

    int32_t res = InitEcPakeParams(params);
    if (res != HC_SUCCESS) {
        LOGE("InitEcPakeParams failed, res: %d.", res);
        goto err;
    }

    res = GenerateEsk(params);
    if (res != HC_SUCCESS) {
        LOGE("GenerateEsk failed, res: %d.", res);
        goto err;
    }

    res = params->loader->hashToPoint(secret, X25519, &params->base);
    if (res != HC_SUCCESS) {
        LOGE("HashToPoint from secret to base failed, res: %d.", res);
        goto err;
    }
    KeyBuff eskSelfBuff = { params->eskSelf.val, params->eskSelf.length, false };
    KeyBuff baseBuff = { params->base.val, params->base.length, false };
    res = params->loader->agreeSharedSecret(&eskSelfBuff, &baseBuff, X25519, &params->epkSelf);
    if (res != HC_SUCCESS) {
        LOGE("AgreeSharedSecret failed, res: %d.", res);
        goto err;
    }
    return res;
err:
    FreeAndCleanKey(&params->eskSelf);
    FreeAndCleanKey(&params->base);
    return res;
}

int32_t GenerateEcSharedSecret(PakeBaseParams *params, Uint8Buff *sharedSecret)
{
    if (params->curveType == CURVE_256) {
        LOGE("Unsupport curve type.");
        return HC_ERROR;
    }

    KeyBuff eskSelfBuff = { params->eskSelf.val, params->eskSelf.length, false };
    KeyBuff epkPeerBuff = { params->epkPeer.val, params->epkPeer.length, false };
    int32_t res = params->loader->agreeSharedSecret(&eskSelfBuff, &epkPeerBuff, X25519, sharedSecret);
    if (res != HC_SUCCESS) {
        LOGE("AgreeSharedSecret failed, res: %d.", res);
        return res;
    }

    return res;
}