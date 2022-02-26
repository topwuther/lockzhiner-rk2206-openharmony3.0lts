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

#include "group_auth_manager.h"
#include "database_manager.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "session_manager.h"

int32_t GetAuthState(int64_t authReqId, const char *groupId, const char *peerUdid, uint8_t *out, uint32_t *outLen)
{
    (void)authReqId;
    (void)groupId;
    (void)peerUdid;
    (void)out;
    (void)outLen;
    return HC_ERROR;
}

void InformDeviceDisconnection(const char *udid)
{
    (void)udid;
    return;
}

bool IsTrustedDevice(const char *udid)
{
    if (udid == NULL) {
        LOGE("InValid params in IsTrustedDevice");
        return false;
    }
    return IsTrustedDeviceExist(udid);
}

int32_t QueryTrustedDeviceNum(void)
{
    return GetTrustedDevNumber();
}

void DoAuthDevice(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL, can't start auth device!");
        return;
    }
    AuthDeviceTask *realTask = (AuthDeviceTask *)task;
    bool isClient = true;
    if (GetBoolFromJson(realTask->authParams, FIELD_IS_CLIENT, &isClient)) {
        LOGE("Failed to get role of client or server when start auth!");
        return;
    }
    int32_t result;
    if (isClient) {
        result = CreateSession(realTask->authReqId, TYPE_CLIENT_AUTH_SESSION, realTask->authParams, realTask->callback);
    } else {
        result = CreateSession(realTask->authReqId, TYPE_SERVER_AUTH_SESSION, realTask->authParams, realTask->callback);
    }
    if (result != HC_SUCCESS) {
        LOGE("Failed to create session for authDevice!");
    }
}

void DoProcessData(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL, can't process auth data!");
        return;
    }
    AuthDeviceTask *realTask = (AuthDeviceTask *)task;
    if (IsRequestExist(realTask->authReqId)) {
        int ret = ProcessSession(realTask->authReqId, AUTH_TYPE, realTask->authParams);
        if (ret != HC_SUCCESS) {
            DestroySession(realTask->authReqId);
        }
        return;
    }
    int32_t result = CreateSession(realTask->authReqId, TYPE_SERVER_AUTH_SESSION, realTask->authParams,
        realTask->callback);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create session for process auth data!");
    }
}
