/*
 * hdf_wlan_queue.c
 *
 * wlan queue of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hdf_wlan_queue.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_mutex.h"

typedef struct {
    HdfWlanQueue base;
    OSAL_DECLARE_MUTEX(lock);
    uint16_t elementCount;
    uint16_t maxElements;
    uint16_t tailIndex;
    uint16_t headIndex;
    void *elements[0];
} HdfWlanQueueImpl;

#define MAX_QUEUE_SIZE 30000

HdfWlanQueue *CreateQueue(uint16_t maxQueueSize)
{
    int32_t ret;
    HdfWlanQueueImpl *impl = NULL;
    if (maxQueueSize > MAX_QUEUE_SIZE) {
        HDF_LOGE("%s:Max queue size is %d", __func__, MAX_QUEUE_SIZE);
        return NULL;
    }
    impl = OsalMemCalloc(sizeof(HdfWlanQueueImpl) + (sizeof(void *) * maxQueueSize));
    if (impl == NULL) {
        HDF_LOGE("%s:oom", __func__);
        return NULL;
    }
    impl->maxElements = maxQueueSize;
    ret = OsalMutexInit(&impl->lock);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(impl);
        return NULL;
    }
    return (HdfWlanQueue*)impl;
}

void DestroyQueue(HdfWlanQueue *queue)
{
    int32_t ret;
    HdfWlanQueueImpl *impl = NULL;
    if (queue == NULL) {
        return;
    }
    impl = (HdfWlanQueueImpl *)queue;
    ret = OsalMutexDestroy(&impl->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: OsalSpinDestroy failed!ret=%d", __func__, ret);
    }
    OsalMemFree(impl);
}

void *PopQueue(HdfWlanQueue *queue)
{
    int32_t ret;
    HdfWlanQueueImpl *impl = NULL;
    if (queue == NULL) {
        return NULL;
    }
    impl = (HdfWlanQueueImpl *)queue;
    void *result = NULL;
    if (queue == NULL) {
        return NULL;
    }
    ret = OsalMutexLock(&impl->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:Get lock failed!ret=%d", __func__, ret);
        return NULL;
    }
    if (impl->elementCount > 0) {
        uint16_t headIndex = impl->headIndex;
        result = impl->elements[headIndex++];
        impl->headIndex = ((headIndex >= impl->maxElements) ? 0 : headIndex);
        impl->elementCount--;
    }
    ret = OsalMutexUnlock(&impl->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:Release lock failed!ret=%d", __func__, ret);
    }
    return result;
}

int32_t PushQueue(HdfWlanQueue *queue, void *context)
{
    int32_t ret;
    HdfWlanQueueImpl *impl = NULL;
    if (queue == NULL) {
        return HDF_FAILURE;
    }
    impl = (HdfWlanQueueImpl *)queue;
    ret = OsalMutexLock(&impl->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:Get lock failed!ret=%d", __func__, ret);
        return HDF_FAILURE;
    }
    do {
        if (impl->elementCount >= impl->maxElements) {
            HDF_LOGE("%s:queue full!", __func__);
            ret = HDF_FAILURE;
            break;
        }

        uint16_t tailIndex = impl->tailIndex;
        /* Saves the address of the element in the queue */
        impl->elements[tailIndex++] = context;
        impl->tailIndex = ((tailIndex >= impl->maxElements) ? 0 : tailIndex);
        impl->elementCount++;
    } while (false);
    ret = OsalMutexUnlock(&impl->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:Release lock failed!ret=%d", __func__, ret);
    }
    return ret;
}
