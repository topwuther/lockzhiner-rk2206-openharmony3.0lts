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

#include "hiview_util.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "cmsis_os.h"

extern void HAL_NVIC_SystemReset(void);
extern void __disable_irq(void);
extern void __enable_irq(void);

#define HIVIEW_WAIT_FOREVER           0xFFFFFFFF
#define HIVIEW_MS_PER_SECOND          1000
#define HIVIEW_NS_PER_MILISECOND      1000000
#define BUFFER_SIZE                   128

void *HIVIEW_MemAlloc(uint8 modId, uint32 size)
{
    (void)modId;
    return malloc(size);
}

void HIVIEW_MemFree(uint8 modId, void *pMem)
{
    (void)modId;
    free(pMem);
}

uint64 HIVIEW_GetCurrentTime()
{
    struct timespec current = {0};
    int ret = clock_gettime(CLOCK_REALTIME, &current);
    if (ret == 0) {
        return (uint64)current.tv_sec * HIVIEW_MS_PER_SECOND + current.tv_nsec / HIVIEW_NS_PER_MILISECOND;
    } else {
        return 0;
    }
}

int32 HIVIEW_RtcGetCurrentTime(uint64 *val, HIVIEW_RtcTime *time)
{
    (void)val;
    (void)time;
    return OHOS_SUCCESS;
}

HiviewMutexId_t HIVIEW_MutexInit()
{
    return (HiviewMutexId_t)osMutexNew(NULL);
}

void HIVIEW_MutexLock(HiviewMutexId_t mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexAcquire((osMutexId_t)mutex, HIVIEW_WAIT_FOREVER);
}

void HIVIEW_MutexLockOrWait(HiviewMutexId_t mutex, uint32 timeout)
{
    if (mutex == NULL) {
        return;
    }
    osMutexAcquire((osMutexId_t)mutex, timeout);
}

void HIVIEW_MutexUnlock(HiviewMutexId_t mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexRelease((osMutexId_t)mutex);
}

uint32 HIVIEW_GetTaskId()
{
    return (uint32)osThreadGetId();
}

void HIVIEW_UartPrint(const char *str)
{
    printf("%s", str);
}

void HIVIEW_Sleep(uint32 ms)
{
    osDelay(ms / HIVIEW_MS_PER_SECOND);
}

int32 HIVIEW_FileOpen(const char *path)
{
    return open(path, O_RDWR | O_CREAT, 0);
}

int32 HIVIEW_FileClose(int32 handle)
{
    return close(handle);
}

int32 HIVIEW_FileRead(int32 handle, uint8 *buf, uint32 len)
{
    return read(handle, (char *)buf, len);
}

int32 HIVIEW_FileWrite(int32 handle, const uint8 *buf, uint32 len)
{
    return write(handle, (const char *)buf, len);
}

int32 HIVIEW_FileSeek(int32 handle, int32 offset, int32 whence)
{
    return lseek(handle, (off_t)offset, whence);
}

int32 HIVIEW_FileSize(int32 handle)
{
    return lseek(handle, 0, SEEK_END);
}

int32 HIVIEW_FileUnlink(const char *path)
{
    return unlink(path);
}

int32 HIVIEW_FileCopy(const char *src, const char *dest)
{
    if (src == NULL || dest == NULL) {
        HIVIEW_UartPrint("HIVIEW_FileCopy input param is NULL");
        return -1;
    }
    int32 fdSrc = open(src, O_RDONLY, 0);
    if (fdSrc < 0) {
        HIVIEW_UartPrint("HIVIEW_FileCopy open src file fail");
        return fdSrc;
    }
    int32 fdDest = open(dest, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fdDest < 0) {
        HIVIEW_UartPrint("HIVIEW_FileCopy open dest file fail");
        HIVIEW_FileClose(fdSrc);
        return fdDest;
    }
    boolean copyFailed = TRUE;
    uint8 *dataBuf = (uint8 *)HIVIEW_MemAlloc(MEM_POOL_HIVIEW_ID, BUFFER_SIZE);
    if (dataBuf == NULL) {
        HIVIEW_UartPrint("HIVIEW_FileCopy malloc erro");
        goto MALLOC_ERROR;
    }
    int32 nLen = HIVIEW_FileRead(fdSrc, dataBuf, BUFFER_SIZE);
    while (nLen > 0) {
        if (HIVIEW_FileWrite(fdDest, dataBuf, nLen) != nLen) {
            goto EXIT;
        }
        nLen = HIVIEW_FileRead(fdSrc, dataBuf, BUFFER_SIZE);
    }
    copyFailed = (nLen < 0);

EXIT:
    free(dataBuf);
MALLOC_ERROR:
    HIVIEW_FileClose(fdSrc);
    HIVIEW_FileClose(fdDest);
    if (copyFailed) {
        HIVIEW_UartPrint("HIVIEW_FileCopy copy failed");
        HIVIEW_FileUnlink(dest);
        return -1;
    }

    return 0;
}

int32 HIVIEW_FileMove(const char *src, const char *dest)
{
    int32 ret = HIVIEW_FileCopy(src, dest);
    if (HIVIEW_FileUnlink(src) != 0 || ret != 0) {
        return -1;
    }
    return 0;
}

void HIVIEW_WatchDogSystemReset()
{
    /* reset MCU Core */
    HAL_NVIC_SystemReset();
}

uint8 HIVIEW_WdgResetFlag()
{
    /* Depend:HAL_WdgGetResetFlag */
    return 1;
}

uint32 Change32Endian(uint32 num)
{
    unsigned char *buffer = (unsigned char *)&num;
    uint32 newEndian = (buffer[3] & 0xFF);
    newEndian |= ((buffer[2] << 8) & 0xFF00);
    newEndian |= ((buffer[1] << 16) & 0xFF0000);
    newEndian |= ((buffer[0] << 24) & 0xFF000000);
    return newEndian;
}

uint16 Change16Endian(uint16 num)
{
    unsigned char* buffer = (unsigned char*)&num;
    uint16 newEndian = (buffer[1] & 0xFF);
    newEndian |= ((buffer[0] << 8) & 0xFF00);
    return newEndian;
}