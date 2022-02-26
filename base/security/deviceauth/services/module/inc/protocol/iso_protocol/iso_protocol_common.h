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

#ifndef ISO_PROTOCOL_COMMON_H
#define ISO_PROTOCOL_COMMON_H

#include "common_defs.h"
#include "alg_defs.h"

#define GENERATE_SESSION_KEY_STR "hichain_iso_session_key"

#define RAND_BYTE_LEN 16
#define PSK_LEN 32
#define ISO_SESSION_KEY_LEN 32

typedef struct IsoBaseParamsT {
    Uint8Buff randSelf;
    Uint8Buff randPeer;
    Uint8Buff authIdSelf;
    Uint8Buff authIdPeer;
    Uint8Buff sessionKey;
    uint8_t psk[PSK_LEN];
    const AlgLoader *loader;
} IsoBaseParams;

int IsoClientGenRandom(IsoBaseParams *params);
int IsoClientCheckAndGenToken(const IsoBaseParams *params, const Uint8Buff *perrToken, Uint8Buff *selfToken);
int IsoClientGenSessionKey(IsoBaseParams *params, int returnResult, const uint8_t *hmac, uint32_t hmacLen);

int IsoServerGenRandomAndToken(IsoBaseParams *params, Uint8Buff *selfTokenBuf);
int IsoServerGenSessionKeyAndCalToken(IsoBaseParams *params, const Uint8Buff *tokenFromPeer, Uint8Buff *tokenToPeer);

#endif