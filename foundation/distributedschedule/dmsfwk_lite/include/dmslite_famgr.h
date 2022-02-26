/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef OHOS_DMSLITE_FAMGR_H
#define OHOS_DMSLITE_FAMGR_H

#include <stdint.h>

#include "dmsfwk_interface.h"
#include "dmslite_inner_common.h"
#include "bundle_manager.h"
#include "want.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    Want *want;
    CallerInfo *callerInfo;
    IDmsListener *callback;
} RequestData;

/**
* @brief Starts ability from remote
* @param bundleName callee bundle name, e.g. ohos.distributedschedule.helloworld
* @param abilityName callee ability name, e.g. MainAbility
* @param onStartAbilityDone called when ability started done
* @return DmsLiteCommonErrorCode
*/
int32_t StartAbilityFromRemote(const char *bundleName, const char *abilityName,
    StartAbilityCallback onStartAbilityDone);

int32_t StartRemoteAbility(const Want *want, CallerInfo *callerInfo, IDmsListener *callback);
void FreeRequestData(const Want *want, CallerInfo *callerInfo);
int32_t StartRemoteAbilityInner(const Want *want, const CallerInfo *callerInfo,
    const IDmsListener *callback);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // OHOS_DMSLITE_FAMGR_H