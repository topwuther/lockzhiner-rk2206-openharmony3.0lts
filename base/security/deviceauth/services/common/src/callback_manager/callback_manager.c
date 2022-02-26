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

#include "callback_manager.h"
#include <stddef.h>
#include "hc_log.h"

bool ProcessTransmitCallback(int64_t requestId, const uint8_t *data, uint32_t dataLen,
    const DeviceAuthCallback *callback)
{
    LOGI("[OnTransmit]: We're going to notify the service! [RequestId]: %" PRId64 ", [DataLen]: %d",
        requestId, dataLen);
    if ((callback != NULL) && (callback->onTransmit != NULL)) {
        if (callback->onTransmit(requestId, data, dataLen)) {
            LOGI("[OnTransmit]: We notify the service successfully! [RequestId]: %" PRId64 ", [DataLen]: %d",
                requestId, dataLen);
            return true;
        }
        LOGE("[OnTransmit]: An error occurs when the service channel sends data! "
            "[RequestId]: %" PRId64 ", [DataLen]: %d", requestId, dataLen);
        return false;
    }
    LOGE("[OnTransmit]: Currently, the service callback is NULL! [RequestId]: %" PRId64 ", [DataLen]: %d",
        requestId, dataLen);
    return false;
}

void ProcessSessionKeyCallback(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen,
    const DeviceAuthCallback *callback)
{
    const char *isSessionKeyExist = (sessionKeyLen > 0) ? "true" : "false";
    LOGI("[OnSessionKeyReturned]: We're going to notify the service! [RequestId]: %" PRId64 ", [IsSessionKeyExist]: %s",
        requestId, isSessionKeyExist);
    if ((callback != NULL) && (callback->onSessionKeyReturned != NULL)) {
        (void)callback->onSessionKeyReturned(requestId, sessionKey, sessionKeyLen);
        LOGI("[OnSessionKeyReturned]: We notify the service successfully! "
            "[RequestId]: %" PRId64 ", [IsSessionKeyExist]: %s", requestId, isSessionKeyExist);
        return;
    }
    LOGE("[OnSessionKeyReturned]: Currently, the service callback is NULL! "
        "[RequestId]: %" PRId64 ", [IsSessionKeyExist]: %s", requestId, isSessionKeyExist);
}

void ProcessFinishCallback(int64_t requestId, int operationCode, const char *returnData,
    const DeviceAuthCallback *callback)
{
    LOGI("[OnFinish]: We're going to notify the service! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        requestId, operationCode);
    if ((callback != NULL) && (callback->onFinish != NULL)) {
        callback->onFinish(requestId, operationCode, returnData);
        LOGI("[End]: [OnFinish]: We notify the service successfully! [RequestId]: %" PRId64 ", [OperationCode]: %d",
            requestId, operationCode);
        return;
    }
    LOGE("[End]: [OnFinish]: Currently, the service callback is NULL! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        requestId, operationCode);
}

void ProcessErrorCallback(int64_t requestId, int operationCode, int errorCode, const char *errorReturn,
    const DeviceAuthCallback *callback)
{
    LOGI("[OnError]: We're going to notify the service! [RequestId]: %" PRId64 ", [OperationCode]: %d, [errorCode]: %d",
        requestId, operationCode, errorCode);
    if ((callback != NULL) && (callback->onError != NULL)) {
        callback->onError(requestId, operationCode, errorCode, errorReturn);
        LOGI("[End]: [OnError]: We notify the service successfully! "
            "[RequestId]: %" PRId64 ", [OperationCode]: %d, [errorCode]: %d", requestId, operationCode, errorCode);
        return;
    }
    LOGE("[End]: [OnError]: Currently, the service callback is NULL! "
        "[RequestId]: %" PRId64 ", [OperationCode]: %d, [errorCode]: %d", requestId, operationCode, errorCode);
}

char *ProcessRequestCallback(int64_t requestId, int operationCode, const char *reqParams,
    const DeviceAuthCallback *callback)
{
    LOGI("[OnRequest]: We're going to notify the service! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        requestId, operationCode);
    if ((callback != NULL) && (callback->onRequest != NULL)) {
        char *returnData = callback->onRequest(requestId, operationCode, reqParams);
        LOGI("[OnRequest]: We notify the service successfully! [RequestId]: %" PRId64 ", [OperationCode]: %d",
            requestId, operationCode);
        return returnData;
    }
    LOGE("[OnRequest]: Currently, the service callback is NULL! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        requestId, operationCode);
    return NULL;
}