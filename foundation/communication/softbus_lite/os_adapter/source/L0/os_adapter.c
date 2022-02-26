/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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
#include "os_adapter.h"
#include "lwip/sockets.h"

MutexId MutexInit(void)
{
    return (MutexId)osMutexNew(NULL);
}

void MutexLock(MutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexAcquire(mutex, osWaitForever);
}

void MutexUnlock(MutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexRelease(mutex);
}

void CloseSocket(int *fd)
{
    if (fd == NULL) {
        return;
    }

    if (*fd >= 0) {
        closesocket(*fd);
        *fd = -1;
    }
}

int WriteMsgQue(unsigned int queueID, const void *bufferAddr, unsigned int bufferSize)
{
    if (bufferAddr == NULL) {
        return -1;
    }
    (void)bufferSize;
    return osMessageQueuePut((osMessageQueueId_t)queueID, (VOID*)bufferAddr, 0, 0);
}

int CreateMsgQue(const char *queueName,
    unsigned short len, unsigned int *queueID,
    unsigned int flags, unsigned short maxMsgSize)
{
    osMessageQueueId_t id;

    if (queueID == NULL) {
        return -1;
    }

    (void)queueName;
    (void)flags;

    id = osMessageQueueNew(len, maxMsgSize, NULL);
    if (NULL == id) {
        return -1;
    }
    *queueID = (unsigned int)id;
    return 0;
}

int DeleteMsgQue(unsigned int queueID)
{
    return osMessageQueueDelete((osMessageQueueId_t)queueID);
}

int ReadMsgQue(unsigned int queueID,
    void *bufferAddr, unsigned int *bufferSize)
{
    if (bufferAddr == NULL || bufferSize == NULL) {
        return -1;
    }
    return osMessageQueueGet((osMessageQueueId_t)queueID, bufferAddr, NULL, osWaitForever);
}

int SoftBusCheckPermission(const char* permissionName)
{
    if (permissionName == NULL) {
        return -1;
    }
    return 0;
}

