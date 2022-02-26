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

#ifndef OHOS_DMSLITE_SESSION_H
#define OHOS_DMSLITE_SESSION_H

#include <stdint.h>

#include "dmsfwk_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int32_t CreateDMSSessionServer();
int32_t CloseDMSSessionServer();
int32_t SendDmsMessage(const char *data, int32_t len, const char *deviceId, IDmsListener *callback);
int32_t OpenDMSSession();
void CloseDMSSession();
void InvokeCallback(const void *data, int32_t result);
void HandleSessionClosed(int32_t sessionId);
int32_t HandleSessionOpened(int32_t sessionId);
void HandleBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen);
bool IsDmsBusy();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // OHOS_DMSLITE_SESSION_H
