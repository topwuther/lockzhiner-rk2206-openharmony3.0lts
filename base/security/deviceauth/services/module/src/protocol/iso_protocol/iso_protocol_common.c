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

#include "iso_protocol_common.h"
#include "hc_log.h"
#include "hc_types.h"
#include "protocol_common.h"
#include "securec.h"

static int IsoCalSelfToken(const IsoBaseParams *params, Uint8Buff *outHmac)
{
    int res;
    int length = params->randSelf.length + params->randPeer.length + params->authIdSelf.length +
        params->authIdPeer.length;
    uint8_t *messagePeer = (uint8_t *)HcMalloc(length, 0);
    if (messagePeer == NULL) {
        return HC_ERROR;
    }
    int usedLen = 0;
    if (memcpy_s(messagePeer, length, params->randPeer.val, params->randPeer.length) != EOK) {
        LOGE("memcpy randPeer failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    usedLen += params->randPeer.length;
    if (memcpy_s(messagePeer + usedLen, length - usedLen, params->randSelf.val, params->randSelf.length) != EOK) {
        LOGE("memcpy randSelf failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    usedLen += params->randSelf.length;
    if (memcpy_s(messagePeer + usedLen, length - usedLen, params->authIdSelf.val, params->authIdSelf.length) != EOK) {
        LOGE("memcpy authIdSelf failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    usedLen += params->authIdSelf.length;
    if (memcpy_s(messagePeer + usedLen, length - usedLen, params->authIdPeer.val, params->authIdPeer.length) != EOK) {
        LOGE("memcpy authIdPeer failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    Uint8Buff messageBuf = { messagePeer, length };
    Uint8Buff pskBuf = { (uint8_t *)params->psk, sizeof(params->psk) };
    res = params->loader->computeHmac(&pskBuf, &messageBuf, outHmac, false);
    if (res != 0) {
        LOGE("computeHmac failed.");
        goto err;
    }
err:
    HcFree(messagePeer);
    return res;
}

static int IsoCalPeerToken(const IsoBaseParams *params, Uint8Buff *selfToken)
{
    int res;
    int length = params->randSelf.length + params->randPeer.length + params->authIdPeer.length +
        params->authIdSelf.length;
    uint8_t *messageSelf = (uint8_t *)HcMalloc(length, 0);
    if (messageSelf == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    int usedLen = 0;
    if (memcpy_s(messageSelf, length, params->randSelf.val, params->randSelf.length) != EOK) {
        LOGE("memcpy randSelf failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    usedLen += params->randSelf.length;
    if (memcpy_s(messageSelf + usedLen, length - usedLen, params->randPeer.val, params->randPeer.length) != EOK) {
        LOGE("memcpy randPeer failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    usedLen += params->randPeer.length;
    if (memcpy_s(messageSelf + usedLen, length - usedLen, params->authIdPeer.val, params->authIdPeer.length) != EOK) {
        LOGE("memcpy authIdPeer failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    usedLen += params->authIdPeer.length;
    if (memcpy_s(messageSelf + usedLen, length - usedLen, params->authIdSelf.val, params->authIdSelf.length) != EOK) {
        LOGE("memcpy authIdSelf failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    Uint8Buff messageBufSelf = { messageSelf, length };
    Uint8Buff pskBuf = { (uint8_t *)params->psk, sizeof(params->psk) };
    res = params->loader->computeHmac(&pskBuf, &messageBufSelf, selfToken, false);
    if (res != HC_SUCCESS) {
        LOGE("computeHmac failed.");
        goto err;
    }
err:
    HcFree(messageSelf);
    return res;
}

int IsoClientGenRandom(IsoBaseParams *params)
{
    if (params == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    return params->loader->generateRandom(&params->randSelf);
}

int IsoClientCheckAndGenToken(const IsoBaseParams *params, const Uint8Buff *peerToken, Uint8Buff *selfToken)
{
    if (params == NULL || peerToken == NULL || selfToken == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }

    uint8_t hmacPeer[SHA256_LEN] = { 0 };
    Uint8Buff outHmac = { hmacPeer, sizeof(hmacPeer) };
    int res = IsoCalSelfToken(params, &outHmac);
    if (res != 0) {
        return res;
    }
    if (memcmp(peerToken->val, outHmac.val, outHmac.length) != 0) {
        LOGE("Compare hmac token failed.");
        return HC_ERR_PROOF_NOT_MATCH;
    }
    return IsoCalPeerToken(params, selfToken);
}

int IsoClientGenSessionKey(IsoBaseParams *params, int returnResult, const uint8_t *hmac, uint32_t hmacLen)
{
    if (params == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }

    Uint8Buff pskBuf = { params->psk, sizeof(params->psk) };
    Uint8Buff hmacMessage = { (uint8_t *)&returnResult, sizeof(int) };
    uint8_t hmacSelf[SHA256_LEN] = { 0 };
    Uint8Buff outHmacBuf = { hmacSelf, sizeof(hmacSelf) };
    int res = params->loader->computeHmac(&pskBuf, &hmacMessage, &outHmacBuf, false);
    if (res != 0) {
        return res;
    }
    if (memcmp(outHmacBuf.val, hmac, hmacLen) != 0) {
        LOGE("Compare hmac result failed.");
        return HC_ERR_PROOF_NOT_MATCH;
    }

    int hkdfSaltLen = params->randPeer.length + params->randSelf.length;
    uint8_t *hkdfSalt = (uint8_t *)HcMalloc(hkdfSaltLen, 0);
    if (hkdfSalt == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(hkdfSalt, hkdfSaltLen, params->randSelf.val, params->randSelf.length) != EOK) {
        LOGE("memcpy randSelf failed.");
        HcFree(hkdfSalt);
        return HC_ERR_MEMORY_COPY;
    }
    if (memcpy_s(hkdfSalt + params->randSelf.length, hkdfSaltLen - params->randSelf.length,
        params->randPeer.val, params->randPeer.length) != EOK) {
        LOGE("memcpy randPeer failed.");
        HcFree(hkdfSalt);
        return HC_ERR_MEMORY_COPY;
    }
    Uint8Buff hkdfSaltBuf = { hkdfSalt, hkdfSaltLen };
    Uint8Buff keyInfoBuf = { (uint8_t *)GENERATE_SESSION_KEY_STR, (uint32_t)strlen(GENERATE_SESSION_KEY_STR) };
    params->sessionKey.val = (uint8_t *)HcMalloc(params->sessionKey.length, 0);
    if (params->sessionKey.val == NULL) {
        HcFree(hkdfSalt);
        return HC_ERR_ALLOC_MEMORY;
    }
    res = params->loader->computeHkdf(&pskBuf, &hkdfSaltBuf, &keyInfoBuf, &params->sessionKey, false);
    if (res != 0) {
        LOGE("compute hkdf failed, res:%d", res);
        FreeAndCleanKey(&params->sessionKey);
    }
    HcFree(hkdfSalt);
    return res;
}

int IsoServerGenRandomAndToken(IsoBaseParams *params, Uint8Buff *selfTokenBuf)
{
    int res = params->loader->generateRandom(&params->randSelf);
    if (res != 0) {
        return res;
    }
    return IsoCalPeerToken(params, selfTokenBuf);
}

int IsoServerGenSessionKeyAndCalToken(IsoBaseParams *params, const Uint8Buff *tokenFromPeer, Uint8Buff *tokenToPeer)
{
    uint8_t hmacPeer[SHA256_LEN] = {0};
    Uint8Buff outHmac = { hmacPeer, sizeof(hmacPeer) };
    int res = IsoCalSelfToken(params, &outHmac);
    if (res != 0) {
        return res;
    }
    if (memcmp(tokenFromPeer->val, outHmac.val, outHmac.length) != 0) {
        LOGE("Compare hmac token failed.");
        return HC_ERR_PROOF_NOT_MATCH;
    }

    int hkdfSaltLen = params->randPeer.length + params->randSelf.length;
    uint8_t *hkdfSalt = (uint8_t *)HcMalloc(hkdfSaltLen, 0);
    if (hkdfSalt == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(hkdfSalt, hkdfSaltLen, params->randPeer.val, params->randPeer.length) != EOK) {
        LOGE("memcpy randPeer failed.");
        HcFree(hkdfSalt);
        return HC_ERR_MEMORY_COPY;
    }
    if (memcpy_s(hkdfSalt + params->randPeer.length, hkdfSaltLen - params->randPeer.length,
        params->randSelf.val, params->randSelf.length) != EOK) {
        LOGE("memcpy randSelf failed.");
        HcFree(hkdfSalt);
        return HC_ERR_MEMORY_COPY;
    }
    Uint8Buff hkdfSaltBuf = { hkdfSalt, hkdfSaltLen };
    Uint8Buff keyInfoBuf = { (uint8_t *)GENERATE_SESSION_KEY_STR, (uint32_t)strlen(GENERATE_SESSION_KEY_STR) };
    params->sessionKey.val = (uint8_t *)HcMalloc(params->sessionKey.length, 0);
    if (params->sessionKey.val == NULL) {
        HcFree(hkdfSalt);
        return HC_ERR_ALLOC_MEMORY;
    }
    Uint8Buff pskBuf = { params->psk, sizeof(params->psk) };
    res = params->loader->computeHkdf(&pskBuf, &hkdfSaltBuf, &keyInfoBuf, &(params->sessionKey), false);
    HcFree(hkdfSalt);
    if (res != 0) {
        FreeAndCleanKey(&params->sessionKey);
        return res;
    }

    int returnCode = 0;
    Uint8Buff messageBuf = { (uint8_t *)&returnCode, sizeof(int) };
    res = params->loader->computeHmac(&pskBuf, &messageBuf, tokenToPeer, false);
    if (res != 0) {
        FreeAndCleanKey(&params->sessionKey);
    }
    return res;
}