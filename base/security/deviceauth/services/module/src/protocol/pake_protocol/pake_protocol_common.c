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

#include "pake_protocol_common.h"
#include "alg_loader.h"
#include "hc_log.h"
#include "hc_types.h"
#include "module_common.h"
#include "pake_defs.h"
#include "pake_protocol_dl.h"
#include "pake_protocol_ec.h"
#include "protocol_common.h"

#define PAKE_SESSION_KEY_LEN 16

void DestroyPakeBaseParams(PakeBaseParams *params)
{
    if (params == NULL) {
        return;
    }

    FreeAndCleanKey(&params->psk);
    FreeAndCleanKey(&params->base);
    FreeAndCleanKey(&params->eskSelf);
    FreeAndCleanKey(&params->sharedSecret);
    FreeAndCleanKey(&params->sessionKey);
    FreeAndCleanKey(&params->hmacKey);

    HcFree(params->salt.val);
    params->salt.val = NULL;

    HcFree(params->challengeSelf.val);
    params->challengeSelf.val = NULL;

    HcFree(params->challengePeer.val);
    params->challengePeer.val = NULL;

    HcFree(params->epkSelf.val);
    params->epkSelf.val = NULL;

    HcFree(params->epkPeer.val);
    params->epkPeer.val = NULL;

    HcFree(params->idSelf.val);
    params->idSelf.val = NULL;

    HcFree(params->idPeer.val);
    params->idPeer.val = NULL;

    HcFree(params->kcfData.val);
    params->kcfData.val = NULL;

    HcFree(params->kcfDataPeer.val);
    params->kcfDataPeer.val = NULL;
}

static int32_t AllocDefaultParams(PakeBaseParams *params)
{
    params->salt.length = PAKE_SALT_LEN;
    params->salt.val = (uint8_t *)HcMalloc(params->salt.length, 0);
    if (params->salt.val == NULL) {
        return HC_ERROR;
    }

    params->challengeSelf.length = PAKE_CHALLENGE_LEN;
    params->challengeSelf.val = (uint8_t *)HcMalloc(params->challengeSelf.length, 0);
    if (params->challengeSelf.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }

    params->challengePeer.length = PAKE_CHALLENGE_LEN;
    params->challengePeer.val = (uint8_t *)HcMalloc(params->challengePeer.length, 0);
    if (params->challengePeer.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }

    params->sessionKey.length = PAKE_SESSION_KEY_LEN;
    params->sessionKey.val = (uint8_t *)HcMalloc(params->sessionKey.length, 0);
    if (params->sessionKey.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }

    params->hmacKey.length = PAKE_HMAC_KEY_LEN;
    params->hmacKey.val = (uint8_t *)HcMalloc(params->hmacKey.length, 0);
    if (params->hmacKey.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }

    params->kcfData.length = HMAC_LEN;
    params->kcfData.val = (uint8_t *)HcMalloc(params->kcfData.length, 0);
    if (params->kcfData.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }

    params->kcfDataPeer.length = HMAC_LEN;
    params->kcfDataPeer.val = (uint8_t *)HcMalloc(params->kcfDataPeer.length, 0);
    if (params->kcfDataPeer.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    return HC_SUCCESS;
}

static void FillDefaultValue(PakeBaseParams *params)
{
    params->psk.val = NULL;
    params->psk.length = 0;
    params->eskSelf.val = NULL;
    params->eskSelf.length = 0;
    params->epkSelf.val = NULL;
    params->epkSelf.length = 0;
    params->epkPeer.val = NULL;
    params->epkPeer.length = 0;
    params->base.val = NULL;
    params->base.length = 0;
    params->sharedSecret.val = NULL;
    params->sharedSecret.length = 0;
    params->idSelf.val = NULL;
    params->idSelf.length = 0;
    params->idPeer.val = NULL;
    params->idPeer.length = 0;
    params->is256ModSupported = false; /* default 384 */
    params->largePrimeNumHex = NULL;
    params->innerKeyLen = 0;
    params->supportedPakeAlg = UNSUPPORTED_ALG;
    params->curveType = CURVE_NONE;
    params->isClient = true;
}

int32_t InitPakeBaseParams(PakeBaseParams *params)
{
    if (params == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }

    int32_t res = AllocDefaultParams(params);
    if (res != HC_SUCCESS) {
        goto err;
    }

    FillDefaultValue(params);

    params->loader = GetLoaderInstance();
    if (params->loader == NULL) {
        res = HC_ERROR;
        goto err;
    }

    return HC_SUCCESS;
err:
    DestroyPakeBaseParams(params);
    return res;
}

static int32_t GeneratePakeParams(PakeBaseParams *params)
{
    int32_t res;
    uint8_t secretVal[PAKE_SECRET_LEN] = { 0 };
    Uint8Buff secret = { secretVal, PAKE_SECRET_LEN };
    if (!params->isClient) {
        res = params->loader->generateRandom(&(params->salt));
        if (res != HC_SUCCESS) {
            LOGE("Generate salt failed, res: %d.", res);
            goto err;
        }
    }

    res = params->loader->generateRandom(&(params->challengeSelf));
    if (res != HC_SUCCESS) {
        LOGE("Generate challengeSelf failed, res: %d.", res);
        goto err;
    }

    Uint8Buff keyInfo = { (uint8_t *)HICHAIN_SPEKE_BASE_INFO, strlen(HICHAIN_SPEKE_BASE_INFO) };
    res = params->loader->computeHkdf(&(params->psk), &(params->salt), &keyInfo, &secret, false);
    if (res != HC_SUCCESS) {
        LOGE("Derive secret from psk failed, res: %d.", res);
        goto err;
    }

    if ((uint32_t)params->supportedPakeAlg & EC_SPEKE) {
        res = GenerateEcPakeParams(params, &secret);
    } else if ((uint32_t)params->supportedPakeAlg & DL_SPEKE) {
        res = GenerateDlPakeParams(params, &secret);
    } else {
        res = HC_ERR_INVALID_ALG;
    }
    if (res != HC_SUCCESS) {
        LOGE("GeneratePakeParams failed, PakeAlg: 0x%x, res: %d.", params->supportedPakeAlg, res);
    }
err:
    (void)memset_s(secret.val, secret.length, 0, secret.length);
    FreeAndCleanKey(&params->psk);
    return res;
}

static int32_t DeriveKeyFromSharedSecret(PakeBaseParams *params, const Uint8Buff *sharedSecret)
{
    int32_t res;
    Uint8Buff unionKey = { NULL, 0 };
    Uint8Buff keyInfo = { (uint8_t *)HICHAIN_SPEKE_SESSIONKEY_INFO, strlen(HICHAIN_SPEKE_SESSIONKEY_INFO) };
    unionKey.length = params->sessionKey.length + params->hmacKey.length;
    unionKey.val = (uint8_t *)HcMalloc(unionKey.length, 0);
    if (unionKey.val == NULL) {
        LOGE("Malloc unionKey val failed.");
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }

    res = params->loader->computeHkdf(sharedSecret, &(params->salt), &keyInfo, &unionKey, false);
    if (res != HC_SUCCESS) {
        LOGE("computeHkdf for unionKey failed.");
        goto err;
    }
    if (memcpy_s(params->sessionKey.val, params->sessionKey.length, unionKey.val, params->sessionKey.length) != 0) {
        LOGE("memcpy sessionKey failed");
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    if (memcpy_s(params->hmacKey.val, params->hmacKey.length,
        unionKey.val + params->sessionKey.length, params->hmacKey.length) != 0) {
        LOGE("memcpy hmacKey failed");
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
err:
    FreeAndCleanKey(&unionKey);
    return res;
}

static int32_t GenerateSessionKey(PakeBaseParams *params)
{
    int32_t res = InitSingleParam(&params->sharedSecret, params->innerKeyLen);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for sharedSecret failed, res: %d.", res);
        goto err;
    }

    if ((uint32_t)params->supportedPakeAlg & EC_SPEKE) {
        res = GenerateEcSharedSecret(params, &params->sharedSecret);
    } else if ((uint32_t)params->supportedPakeAlg & DL_SPEKE) {
        res = GenerateDlSharedSecret(params, &params->sharedSecret);
    } else {
        res = HC_ERR_INVALID_ALG;
    }
    if (res != HC_SUCCESS) {
        LOGE("GenerateSharedSecret failed, pakeAlg: %x, res: %d.", params->supportedPakeAlg, res);
        goto err;
    }

    res = DeriveKeyFromSharedSecret(params, &params->sharedSecret);
    if (res != HC_SUCCESS) {
        LOGE("DeriveKeyFromSharedSecret failed.");
        goto err;
    }
    return res;
err:
    FreeAndCleanKey(&params->sharedSecret);
    FreeAndCleanKey(&params->sessionKey);
    FreeAndCleanKey(&params->hmacKey);
    return res;
}

static int32_t GenerateProof(PakeBaseParams *params)
{
    uint8_t challengeVal[PAKE_CHALLENGE_LEN + PAKE_CHALLENGE_LEN] = { 0 };
    Uint8Buff challenge = { challengeVal, PAKE_CHALLENGE_LEN + PAKE_CHALLENGE_LEN };
    if (memcpy_s(challenge.val, challenge.length, params->challengeSelf.val, params->challengeSelf.length) != EOK) {
        LOGE("Memcpy challengeSelf failed.");
        return HC_ERR_MEMORY_COPY;
    }
    if (memcpy_s(challenge.val + params->challengeSelf.length, challenge.length - params->challengeSelf.length,
        params->challengePeer.val, params->challengePeer.length) != EOK) {
        LOGE("Memcpy challengePeer failed.");
        return HC_ERR_MEMORY_COPY;
    }

    int32_t res = params->loader->computeHmac(&(params->hmacKey), &challenge, &(params->kcfData), false);
    if (res != HC_SUCCESS) {
        LOGE("compute kcfData failed");
        return res;
    }

    return res;
}

static int32_t VerifyProof(const PakeBaseParams *params)
{
    uint8_t challengeVal[PAKE_CHALLENGE_LEN + PAKE_CHALLENGE_LEN] = { 0 };
    Uint8Buff challenge = { challengeVal, PAKE_CHALLENGE_LEN + PAKE_CHALLENGE_LEN };
    if (memcpy_s(challenge.val, challenge.length, params->challengePeer.val, params->challengePeer.length) != EOK) {
        LOGE("Memcpy challengePeer failed.");
        return HC_ERR_MEMORY_COPY;
    }
    if (memcpy_s(challenge.val + params->challengePeer.length, challenge.length - params->challengePeer.length,
        params->challengeSelf.val, params->challengeSelf.length) != EOK) {
        LOGE("Memcpy challengeSelf failed.");
        return HC_ERR_MEMORY_COPY;
    }

    uint8_t verifyProofVal[HMAC_LEN] = { 0 };
    Uint8Buff verifyProof = { verifyProofVal, HMAC_LEN };
    int32_t res = params->loader->computeHmac(&(params->hmacKey), &challenge, &verifyProof, false);
    if (res != HC_SUCCESS) {
        LOGE("compute kcfData failed");
        return res;
    }

    res = memcmp(verifyProof.val, params->kcfDataPeer.val, verifyProof.length);
    if (res != HC_SUCCESS) {
        LOGE("compare failed");
        return res;
    }

    return res;
}

int32_t ClientRequestPakeProtocol(const PakeBaseParams *params)
{
    (void)params;
    return HC_SUCCESS;
}

int32_t ClientConfirmPakeProtocol(PakeBaseParams *params)
{
    int32_t res = GeneratePakeParams(params);
    if (res != HC_SUCCESS) {
        LOGE("GeneratePakeParams failed, res:%d", res);
        return res;
    }

    res = GenerateSessionKey(params);
    if (res != HC_SUCCESS) {
        LOGE("GenerateSessionKey failed, res:%d", res);
        return res;
    }

    res = GenerateProof(params);
    if (res != HC_SUCCESS) {
        LOGE("GenerateProof failed, res:%d", res);
        return res;
    }
    return res;
}

int32_t ClientVerifyConfirmPakeProtocol(const PakeBaseParams *params)
{
    int32_t res = VerifyProof(params);
    if (res != HC_SUCCESS) {
        LOGE("VerifyProof failed, res:%d", res);
        return res;
    }

    return res;
}

int32_t ServerResponsePakeProtocol(PakeBaseParams *params)
{
    int32_t res = GeneratePakeParams(params);
    if (res != HC_SUCCESS) {
        LOGE("GeneratePakeParams failed, res:%d", res);
        return res;
    }

    return res;
}

int32_t ServerConfirmPakeProtocol(PakeBaseParams *params)
{
    int32_t res = GenerateSessionKey(params);
    if (res != HC_SUCCESS) {
        LOGE("GenerateSessionKey failed, res:%d", res);
        return res;
    }

    res = VerifyProof(params);
    if (res != HC_SUCCESS) {
        LOGE("VerifyProof failed, res:%d", res);
        return res;
    }

    res = GenerateProof(params);
    if (res != HC_SUCCESS) {
        LOGE("GenerateProof failed, res:%d", res);
        return res;
    }

    return res;
}
