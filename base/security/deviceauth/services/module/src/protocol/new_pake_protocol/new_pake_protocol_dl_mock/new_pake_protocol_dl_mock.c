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

#include "new_pake_protocol_dl.h"
#include "common_defs.h"
#include "hc_log.h"
#include "pake_defs.h"
#include "protocol_common.h"

uint32_t GetPakeNewDlAlg()
{
    return UNSUPPORTED_ALG;
}

int32_t GenerateNewDlPakeParams(PakeBaseParams *params, const Uint8Buff *secret)
{
    (void)params;
    (void)secret;
    LOGE("NEW-PAKE-DL unsupported.");
    return HC_ERROR;
}

int32_t AgreeNewDlSharedSecret(PakeBaseParams *params, Uint8Buff *sharedSecret)
{
    (void)params;
    (void)sharedSecret;
    LOGE("NEW-PAKE-DL unsupported.");
    return HC_ERROR;
}