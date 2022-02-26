/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_sem.h"
#include "osal_thread.h"
#include "plat_log.h"
#include "securec.h"

#define HDF_LOG_TAG gpio_core

#define MAX_CNT_PER_CNTLR          1024
#define GPIO_IRQ_STACK_SIZE        10000
#define GPIO_IRQ_THREAD_NAME_LEN   32

struct GpioIrqBridge {
    uint16_t local;
    GpioIrqFunc func;
    void *data;
    struct OsalThread thread;
    char name[GPIO_IRQ_THREAD_NAME_LEN];
    OsalSpinlock spin;
    struct OsalSem sem;
    struct GpioCntlr *cntlr;
    bool stop;
};

static struct DListHead g_cntlrList;
static OsalSpinlock g_listLock;
static uint32_t g_irqFlags;

static struct DListHead *GpioCntlrListGet(void)
{
    static struct DListHead *list = NULL;
    uint32_t flags;

    if (list == NULL) {
        list = &g_cntlrList;
        DListHeadInit(list);
        OsalSpinInit(&g_listLock);
    }
    while (OsalSpinLockIrqSave(&g_listLock, &flags) != HDF_SUCCESS);
    g_irqFlags = flags;
    return list;
}

static void GpioCntlrListPut(void)
{
    (void)OsalSpinUnlockIrqRestore(&g_listLock, &g_irqFlags);
}

static uint16_t GpioCntlrQueryStart(struct GpioCntlr *cntlr, struct DListHead *list)
{
    uint16_t freeStart;
    uint16_t freeCount;
    struct GpioCntlr *iterLast = NULL;
    struct GpioCntlr *iterCur = NULL;
    struct GpioCntlr *tmp = NULL;

    DLIST_FOR_EACH_ENTRY_SAFE(iterCur, tmp, list, struct GpioCntlr, list) {
        if (iterLast == NULL) {
            freeStart = 0;
            freeCount = iterCur->start;
        } else {
            freeStart = iterLast->start + iterLast->count;
            freeCount = iterCur->start - freeStart;
        }

        if (cntlr->start < freeStart) {
            HDF_LOGE("GpioCntlrQueryStart: start:%u not available(freeStart:%u, freeCount:%u)",
                cntlr->start, freeStart, freeCount);
            return GPIO_NUM_MAX;
        }

        if ((cntlr->start + cntlr->count) <= (freeStart + freeCount)) {
            return freeStart;
        }

        iterLast = iterCur;
    }
    if (iterLast == NULL) { // empty list
        return cntlr->start;
    }
    if (cntlr->start >= (iterLast->start + iterLast->count)) {
        return iterLast->start + iterLast->count;
    }
    HDF_LOGE("GpioCntlrQueryStart: start:%u not available(lastStart:%u, lastCount:%u)",
        cntlr->start, iterLast->start, iterLast->count);
    return GPIO_NUM_MAX;
}

int32_t GpioCntlrAdd(struct GpioCntlr *cntlr)
{
    uint16_t start;
    struct DListHead *list = NULL;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->device == NULL) {
        HDF_LOGE("GpioCntlrAdd: no device associated!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->ops == NULL) {
        HDF_LOGE("GpioCntlrAdd: no ops supplied!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->count >= MAX_CNT_PER_CNTLR) {
        HDF_LOGE("GpioCntlrAdd: invalid gpio count:%u", cntlr->count);
        return HDF_ERR_INVALID_PARAM;
    }

    if (cntlr->ginfos == NULL) {
        cntlr->ginfos = OsalMemCalloc(sizeof(*cntlr->ginfos) * cntlr->count);
        if (cntlr->ginfos == NULL) {
            return HDF_ERR_MALLOC_FAIL;
        }
    }

    OsalSpinInit(&cntlr->spin);

    list = GpioCntlrListGet();
    if ((start = GpioCntlrQueryStart(cntlr, list)) >= GPIO_NUM_MAX) {
        HDF_LOGE("GpioCntlrAdd: query range for start:%d fail:%d", cntlr->start, start);
        GpioCntlrListPut();
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr->start = start;
    DListHeadInit(&cntlr->list);
    DListInsertTail(&cntlr->list, list);
    GpioCntlrListPut();
    HDF_LOGI("GpioCntlrAdd: start:%u count:%u", cntlr->start, cntlr->count);

    return HDF_SUCCESS;
}

void GpioCntlrRemove(struct GpioCntlr *cntlr)
{
    if (cntlr == NULL) {
        return;
    }

    if (cntlr->device == NULL) {
        HDF_LOGE("GpioCntlrRemove: no device associated!");
        return;
    }

    cntlr->device->service = NULL;
    (void)GpioCntlrListGet();
    DListRemove(&cntlr->list);
    GpioCntlrListPut();
    if (cntlr->ginfos != NULL) {
        OsalMemFree(cntlr->ginfos);
        cntlr->ginfos = NULL;
    }
    (void)OsalSpinDestroy(&cntlr->spin);
}

struct GpioCntlr *GpioGetCntlr(uint16_t gpio)
{
    struct DListHead *list = NULL;
    struct GpioCntlr *cntlr = NULL;
    struct GpioCntlr *tmp = NULL;

    list = GpioCntlrListGet();
    DLIST_FOR_EACH_ENTRY_SAFE(cntlr, tmp, list, struct GpioCntlr, list) {
        if (gpio >= cntlr->start && gpio < (cntlr->start + cntlr->count)) {
            GpioCntlrListPut();
            return cntlr;
        }
    }
    GpioCntlrListPut();
    HDF_LOGE("GpioGetCntlr: gpio %u not in any controllers!", gpio);
    return NULL;
}

int32_t GpioCntlrWrite(struct GpioCntlr *cntlr, uint16_t local, uint16_t val)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->write == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return cntlr->ops->write(cntlr, local, val);
}

int32_t GpioCntlrRead(struct GpioCntlr *cntlr, uint16_t local, uint16_t *val)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->read == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    if (val == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    return cntlr->ops->read(cntlr, local, val);
}

int32_t GpioCntlrSetDir(struct GpioCntlr *cntlr, uint16_t local, uint16_t dir)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setDir == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return cntlr->ops->setDir(cntlr, local, dir);
}

int32_t GpioCntlrGetDir(struct GpioCntlr *cntlr, uint16_t local, uint16_t *dir)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getDir == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    if (dir == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    return cntlr->ops->getDir(cntlr, local, dir);
}

int32_t GpioCntlrToIrq(struct GpioCntlr *cntlr, uint16_t local, uint16_t *irq)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->toIrq == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return cntlr->ops->toIrq(cntlr, local, irq);
}

static int32_t GpioIrqBridgeFunc(uint16_t local, void *data)
{
    uint32_t flags;
    struct GpioIrqBridge *bridge = (struct GpioIrqBridge *)data;

    (void)local;
    (void)OsalSpinLockIrqSave(&bridge->spin, &flags);
    if (!bridge->stop) {
        (void)OsalSemPost(&bridge->sem);
    }
    (void)OsalSpinUnlockIrqRestore(&bridge->spin, &flags);
    return HDF_SUCCESS;
}

static int GpioIrqThreadWorker(void *data)
{
    int32_t ret;
    struct GpioIrqBridge *bridge = (struct GpioIrqBridge *)data;

    while (true) {
        ret = OsalSemWait(&bridge->sem, HDF_WAIT_FOREVER);
        if (ret != HDF_SUCCESS) {
            continue;
        }
        if (bridge->stop) {
            break;
        }
        PLAT_LOGV("GpioIrqThreadWorker: enter! gpio:%u-%u", bridge->cntlr->start, bridge->local);
        (void)bridge->func(bridge->local + bridge->cntlr->start, bridge->data);
    }
    /* it's the bridge struct we create before, so release it ourself */
    (void)OsalSemDestroy(&bridge->sem);
    (void)OsalSpinDestroy(&bridge->spin);
    OsalMemFree(bridge);
    HDF_LOGI("GpioIrqThreadWorker: normal exit!");
    return HDF_SUCCESS;
}

static struct GpioIrqBridge *GpioIrqBridgeCreate(struct GpioCntlr *cntlr,
    uint16_t local, GpioIrqFunc func, void *arg)
{
    int32_t ret;
    struct OsalThreadParam cfg;
    struct GpioIrqBridge *bridge = NULL;

    bridge = (struct GpioIrqBridge *)OsalMemCalloc(sizeof(*bridge));
    if (bridge == NULL) {
        return NULL;
    }

    bridge->local = local;
    bridge->func = func;
    bridge->data = arg;
    bridge->cntlr = cntlr;
    bridge->stop = false;

    if (snprintf_s(bridge->name, GPIO_IRQ_THREAD_NAME_LEN, GPIO_IRQ_THREAD_NAME_LEN - 1,
        "GPIO_IRQ_TSK_%d_%d", bridge->cntlr->start, bridge->local) < 0) {
        HDF_LOGE("GpioIrqBridgeCreate: format thread name fail!");
        goto __ERR_FORMAT_NAME;
    }

    if (OsalSpinInit(&bridge->spin) != HDF_SUCCESS) {
        HDF_LOGE("GpioIrqBridgeCreate: init spin fail!");
        goto __ERR_INIT_SPIN;
    }

    if (OsalSemInit(&bridge->sem, 0) != HDF_SUCCESS) {
        HDF_LOGE("GpioIrqBridgeCreate: init sem fail!");
        goto __ERR_INIT_SEM;
    }

    ret = OsalThreadCreate(&bridge->thread, (OsalThreadEntry)GpioIrqThreadWorker, (void *)bridge);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("GpioIrqBridgeCreate: create irq fail!");
        goto __ERR_CREATE_THREAD;
    }

    cfg.name = bridge->name;
    cfg.priority = OSAL_THREAD_PRI_HIGH;
    cfg.stackSize = GPIO_IRQ_STACK_SIZE;
    if (OsalThreadStart(&bridge->thread, &cfg) != HDF_SUCCESS) {
        HDF_LOGE("GpioIrqBridgeCreate: start irq thread fail:%d", ret);
        goto __ERR_START_THREAD;
    }
    return bridge;
__ERR_START_THREAD:
    (void)OsalThreadDestroy(&bridge->thread);
__ERR_CREATE_THREAD:
    (void)OsalSemDestroy(&bridge->sem);
__ERR_INIT_SEM:
    (void)OsalSpinDestroy(&bridge->spin);
__ERR_INIT_SPIN:
__ERR_FORMAT_NAME:
    OsalMemFree(bridge);
    return NULL;
}

static void GpioIrqBridgeDestroy(struct GpioIrqBridge *bridge)
{
    uint32_t flags;
    (void)OsalSpinLockIrqSave(&bridge->spin, &flags);
    if (!bridge->stop) {
        bridge->stop = true;
        (void)OsalSemPost(&bridge->sem);
    }
    (void)OsalSpinUnlockIrqRestore(&bridge->spin, &flags);
}

void GpioCntlrIrqCallback(struct GpioCntlr *cntlr, uint16_t local)
{
    struct GpioInfo *ginfo = NULL;

    if (cntlr != NULL && local < cntlr->count && cntlr->ginfos != NULL) {
        ginfo = &cntlr->ginfos[local];
        if (ginfo != NULL && ginfo->irqFunc != NULL) {
            (void)ginfo->irqFunc(local, ginfo->irqData);
        } else {
            HDF_LOGW("GpioCntlrIrqCallback: ginfo or irqFunc is NULL!");
        }
    } else {
        HDF_LOGW("GpioCntlrIrqCallback: invalid cntlr(ginfos) or loal num:%u!", local);
    }
}

int32_t GpioCntlrSetIrq(struct GpioCntlr *cntlr, uint16_t local, uint16_t mode, GpioIrqFunc func, void *arg)
{
    int32_t ret;
    uint32_t flags;
    GpioIrqFunc theFunc = func;
    void *theData = arg;
    struct GpioIrqBridge *bridge = NULL;
    void *oldFunc = NULL;
    void *oldData = NULL;

    if (cntlr == NULL || cntlr->ginfos == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (local >= cntlr->count) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (cntlr->ops == NULL || cntlr->ops->setIrq == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if ((mode & GPIO_IRQ_USING_THREAD) != 0) {
        bridge = GpioIrqBridgeCreate(cntlr, local, func, arg);
        if (bridge != NULL) {
            theData = bridge;
            theFunc = GpioIrqBridgeFunc;
        }
        if (bridge == NULL) {
            return HDF_FAILURE;
        }
    }

    (void)OsalSpinLockIrqSave(&cntlr->spin, &flags);
    oldFunc = cntlr->ginfos[local].irqFunc;
    oldData = cntlr->ginfos[local].irqData;
    cntlr->ginfos[local].irqFunc = theFunc;
    cntlr->ginfos[local].irqData = theData;
    ret = cntlr->ops->setIrq(cntlr, local, mode, theFunc, theData);
    if (ret == HDF_SUCCESS) {
        if (oldFunc == GpioIrqBridgeFunc) {
            GpioIrqBridgeDestroy((struct GpioIrqBridge *)oldData);
        }
    } else {
        cntlr->ginfos[local].irqFunc = oldFunc;
        cntlr->ginfos[local].irqData = oldData;
        if (bridge != NULL) {
            GpioIrqBridgeDestroy(bridge);
            bridge = NULL;
        }
    }
    (void)OsalSpinUnlockIrqRestore(&cntlr->spin, &flags);
    return ret;
}

int32_t GpioCntlrUnsetIrq(struct GpioCntlr *cntlr, uint16_t local)
{
    int32_t ret;
    uint32_t flags;
    struct GpioIrqBridge *bridge = NULL;

    if (cntlr == NULL || cntlr->ginfos == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (local >= cntlr->count) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (cntlr->ops == NULL || cntlr->ops->unsetIrq == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    (void)OsalSpinLockIrqSave(&cntlr->spin, &flags);
    ret = cntlr->ops->unsetIrq(cntlr, local);
    if (ret == HDF_SUCCESS) {
        if (cntlr->ginfos[local].irqFunc == GpioIrqBridgeFunc) {
            bridge = (struct GpioIrqBridge *)cntlr->ginfos[local].irqData;
            GpioIrqBridgeDestroy(bridge);
        }
        cntlr->ginfos[local].irqFunc = NULL;
        cntlr->ginfos[local].irqData = NULL;
    }
    (void)OsalSpinUnlockIrqRestore(&cntlr->spin, &flags);
    return ret;
}

int32_t GpioCntlrEnableIrq(struct GpioCntlr *cntlr, uint16_t local)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->enableIrq == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return cntlr->ops->enableIrq(cntlr, local);
}

int32_t GpioCntlrDisableIrq(struct GpioCntlr *cntlr, uint16_t local)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->disableIrq == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return cntlr->ops->disableIrq(cntlr, local);
}
