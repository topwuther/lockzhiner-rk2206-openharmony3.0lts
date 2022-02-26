/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "spi_if.h"
#include "spi_test.h"

#define HDF_LOG_TAG spi_test_c

#define SPI_TEST_4BITS  4
#define SPI_TEST_8BITS  8
#define SPI_TEST_16BITS 16

struct SpiTestFunc {
    enum SpiTestCmd type;
    int32_t (*Func)(struct SpiTest *test);
};

static DevHandle SpiTestGetHandle(struct SpiTest *test)
{
    struct SpiDevInfo info;

    info.busNum = test->bus;
    info.csNum = test->cs;
    return SpiOpen(&info);
}

static void SpiTestReleaseHandle(DevHandle handle)
{
    if (handle == NULL) {
        HDF_LOGE("%s: spi handle is null", __func__);
        return;
    }
    SpiClose(handle);
}

#define BITS_PER_WORD_DEFAULT    8
#define BITS_PER_WORD_8BITS      8
#define BITS_PER_WORD_10BITS     10
#define MAX_SPEED_HZ             10000000

static struct SpiCfg g_spiCfg = {
    .mode = SPI_CLK_PHASE | SPI_MODE_LOOP,
    .bitsPerWord = BITS_PER_WORD_DEFAULT,
    .maxSpeedHz = MAX_SPEED_HZ,
    .transferMode = SPI_POLLING_TRANSFER,
};

#define SPI_TEST_ONE_BYTE 1
#define SPI_TEST_TWO_BYTE 2

static int32_t SpiCmpMemByBits(uint8_t *wbuf, uint8_t *rbuf, uint32_t len, uint8_t bits)
{
    int32_t i;
    uint16_t vw;
    uint16_t vr;

    if (bits < SPI_TEST_4BITS) {
        bits = SPI_TEST_4BITS;
    } else if (bits > SPI_TEST_16BITS) {
        bits = SPI_TEST_16BITS;
    }

    for (i = 0; i < len;) {
        if (bits <= SPI_TEST_8BITS) {
            vw = *((uint8_t *)(wbuf + i)) & (~(0xFFFF << bits));
            vr = *((uint8_t *)(rbuf + i)) & (~(0xFFFF << bits));
        } else {
            vw = *((uint16_t *)(wbuf + i)) & (~(0xFFFF << bits));
            vr = *((uint16_t *)(rbuf + i)) & (~(0xFFFF << bits));
        }
        if (vw != vr) {
            HDF_LOGE("%s: compare mem fail(i=%d, vw=%u, vr=%u, bits = %u, len=%u)",
                __func__, i, vw, vr, bits, len);
            return HDF_FAILURE;
        }
        i += (bits <= SPI_TEST_8BITS) ? SPI_TEST_ONE_BYTE : SPI_TEST_TWO_BYTE;
    }
    HDF_LOGE("%s: mem size(%u) compare success", __func__, len);
    return HDF_SUCCESS;
}

static int32_t SpiDoTransferTest(struct SpiTest *test, struct SpiCfg *cfg, struct SpiMsg *msg)
{
    int32_t ret;

    ret = SpiSetCfg(test->handle, cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set config fail", __func__);
        return ret;
    }

    ret = SpiTransfer(test->handle, msg, 1);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: spi transfer err", __func__);
        return ret;
    }

    ret = SpiCmpMemByBits(msg->wbuf, msg->rbuf, msg->len, g_spiCfg.bitsPerWord);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SpiWrite(test->handle, msg->wbuf, msg->len);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: spi write err", __func__);
        return ret;
    }

    ret = SpiRead(test->handle, msg->rbuf, msg->len);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: spi read err", __func__);
        return ret;
    }

    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t SpiTransferTest(struct SpiTest *test)
{
    int32_t ret;
    struct SpiMsg msg;

    g_spiCfg.bitsPerWord = BITS_PER_WORD_8BITS;
    g_spiCfg.transferMode = SPI_POLLING_TRANSFER;

    msg.rbuf = test->rbuf;
    msg.wbuf = test->wbuf;
    msg.len = test->len;
    msg.csChange = 1; // switch off the CS after transfer
    msg.delayUs = 0;
    msg.speed = 0;    // use default speed

    ret = SpiDoTransferTest(test, &g_spiCfg, &msg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: fail, bitsPerWord = %u, ret = %d", __func__, g_spiCfg.bitsPerWord, ret);
        return ret;
    }

    g_spiCfg.bitsPerWord = BITS_PER_WORD_10BITS;
    ret = SpiDoTransferTest(test, &g_spiCfg, &msg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: fail, bitsPerWord = %u, ret = %d", __func__, g_spiCfg.bitsPerWord, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

#define DMA_TRANSFER_SINGLE_MAX   (1024 * 64 - 1)
#define DMA_TRANSFER_SIZE_TOTAL   (DMA_TRANSFER_SINGLE_MAX * 2 + 65532)
#define DMA_TRANSFER_BUF_SEED     0x5A
#define DMA_ALIGN_SIZE            64

static int32_t SpiSetDmaIntMsg(struct SpiMsg *msg, uint32_t len)
{
    uint32_t i;
    uint8_t *wbuf = NULL;
    uint8_t *rbuf = NULL;

    msg->wbuf = msg->rbuf = NULL;

    wbuf = (uint8_t *)OsalMemAllocAlign(DMA_ALIGN_SIZE, len);
    if (wbuf == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }
    rbuf = (uint8_t *)OsalMemAllocAlign(DMA_ALIGN_SIZE, len);
    if (rbuf == NULL) {
        OsalMemFree(wbuf);
        return HDF_ERR_MALLOC_FAIL;
    }

    wbuf[0] = DMA_TRANSFER_BUF_SEED;
    for (i = 1; i < len; i++) {
        wbuf[i] = wbuf[i - 1] + 1;
        rbuf[i] = 0;
    }
    msg->wbuf = wbuf;
    msg->rbuf = rbuf;
    msg->len = len;
    msg->csChange = 1;
    msg->delayUs = 0,  // switch off the CS after transfer
    msg->speed = 0;    // using default speed
    return HDF_SUCCESS;
}

static void SpiUnsetDmaIntMsg(struct SpiMsg *msg)
{
    if (msg != NULL) {
        OsalMemFree(msg->wbuf);
        OsalMemFree(msg->rbuf);
        msg->wbuf = NULL;
        msg->rbuf = NULL;
    }
}

static int32_t SpiDmaTransferTest(struct SpiTest *test)
{
    int32_t ret;
    struct SpiMsg msg;

    g_spiCfg.transferMode = SPI_DMA_TRANSFER;
    g_spiCfg.bitsPerWord = BITS_PER_WORD_8BITS;

    if (test->testDma == 0) {
        HDF_LOGI("%s: testDma not set", __func__);
        return HDF_SUCCESS;
    }

    ret = SpiSetDmaIntMsg(&msg, DMA_TRANSFER_SIZE_TOTAL);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SpiDoTransferTest(test, &g_spiCfg, &msg);
    if (ret != HDF_SUCCESS) {
        SpiUnsetDmaIntMsg(&msg);
        HDF_LOGE("%s: fail, bitsPerWord = %u, ret = %d", __func__, g_spiCfg.bitsPerWord, ret);
        return ret;
    }

    g_spiCfg.bitsPerWord = BITS_PER_WORD_10BITS;
    ret = SpiDoTransferTest(test, &g_spiCfg, &msg);
    if (ret != HDF_SUCCESS) {
        SpiUnsetDmaIntMsg(&msg);
        HDF_LOGE("%s: fail, bitsPerWord = %u, ret = %d", __func__, g_spiCfg.bitsPerWord, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t SpiIntTransferTest(struct SpiTest *test)
{
    int32_t ret;
    struct SpiMsg msg;

    g_spiCfg.transferMode = SPI_INTERRUPT_TRANSFER;
    g_spiCfg.bitsPerWord = BITS_PER_WORD_8BITS;

    ret = SpiSetDmaIntMsg(&msg, DMA_TRANSFER_SIZE_TOTAL);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SpiDoTransferTest(test, &g_spiCfg, &msg);
    if (ret != HDF_SUCCESS) {
        SpiUnsetDmaIntMsg(&msg);
        HDF_LOGE("%s: fail, bitsPerWord = %u, ret = %d", __func__, g_spiCfg.bitsPerWord, ret);
        return ret;
    }

    g_spiCfg.bitsPerWord = BITS_PER_WORD_10BITS;
    ret = SpiDoTransferTest(test, &g_spiCfg, &msg);
    if (ret != HDF_SUCCESS) {
        SpiUnsetDmaIntMsg(&msg);
        HDF_LOGE("%s: fail, bitsPerWord = %u, ret = %d", __func__, g_spiCfg.bitsPerWord, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t SpiReliabilityTest(struct SpiTest *test)
{
    struct SpiCfg cfg = {0};
    struct SpiMsg msg = {0};

    (void)SpiSetCfg(test->handle, &cfg);
    (void)SpiSetCfg(test->handle, NULL);
    (void)SpiTransfer(test->handle, &msg, 1);
    (void)SpiTransfer(test->handle, NULL, -1);
    (void)SpiWrite(test->handle, test->wbuf, test->len);
    (void)SpiWrite(test->handle, NULL, -1);
    (void)SpiRead(test->handle, test->rbuf, test->len);
    (void)SpiRead(test->handle, NULL, -1);

    (void)test;
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t SpiTestAll(struct SpiTest *test)
{
    int32_t total = 0;
    int32_t error = 0;

    if (SpiTransferTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;

    if (SpiDmaTransferTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;

    if (SpiIntTransferTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;

    if (SpiReliabilityTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;

    HDF_LOGE("%s: Spi Test Total %d Error %d", __func__, total, error);
    return HDF_SUCCESS;
}

static struct SpiTestFunc g_spiTestFunc[] = {
    {SPI_TRANSFER_TEST, SpiTransferTest},
    {SPI_DMA_TRANSFER_TEST, SpiDmaTransferTest},
    {SPI_INT_TRANSFER_TEST, SpiIntTransferTest},
    {SPI_RELIABILITY_TEST, SpiReliabilityTest},
    {SPI_PERFORMANCE_TEST, NULL},
    {SPI_TEST_ALL, SpiTestAll},
};

static int32_t SpiTestEntry(struct SpiTest *test, int32_t cmd)
{
    int32_t i;
    int32_t ret = HDF_ERR_NOT_SUPPORT;

    HDF_LOGE("%s: enter cmd %d", __func__, cmd);
    if (test == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    test->handle = SpiTestGetHandle(test);
    if (test->handle == NULL) {
        HDF_LOGE("%s: spi test get handle fail", __func__);
        return HDF_FAILURE;
    }
    for (i = 0; i < sizeof(g_spiTestFunc) / sizeof(g_spiTestFunc[0]); i++) {
        if (cmd == g_spiTestFunc[i].type && g_spiTestFunc[i].Func != NULL) {
            ret = g_spiTestFunc[i].Func(test);
            HDF_LOGE("%s: cmd %d ret %d", __func__, cmd, ret);
            break;
        }
    }
    SpiTestReleaseHandle(test->handle);
    return ret;
}

static int32_t SpiTestBind(struct HdfDeviceObject *device)
{
    static struct SpiTest test;

    if (device != NULL) {
        device->service = &test.service;
    } else {
        HDF_LOGE("%s: device is NULL", __func__);
    }
    return HDF_SUCCESS;
}

static int32_t SpiTestInitFromHcs(struct SpiTest *test, const struct DeviceResourceNode *node)
{
    int32_t ret;
    int32_t i;
    uint32_t *tmp = NULL;
    struct DeviceResourceIface *face = NULL;

    face = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (face == NULL) {
        HDF_LOGE("%s: face is null", __func__);
        return HDF_FAILURE;
    }
    if (face->GetUint32 == NULL || face->GetUint32Array == NULL) {
        HDF_LOGE("%s: GetUint32 or GetUint32Array not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = face->GetUint32(node, "bus", &test->bus, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read bus fail", __func__);
        return HDF_FAILURE;
    }
    ret = face->GetUint32(node, "cs", &test->cs, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read cs fail", __func__);
        return HDF_FAILURE;
    }
    ret = face->GetUint32(node, "len", &test->len, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read len fail", __func__);
        return HDF_FAILURE;
    }
    test->wbuf = (uint8_t *)OsalMemCalloc(test->len);
    if (test->wbuf == NULL) {
        HDF_LOGE("%s: wbuf OsalMemCalloc error\n", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    tmp = (uint32_t *)OsalMemCalloc(test->len * sizeof(uint32_t));
    if (tmp == NULL) {
        HDF_LOGE("%s: tmp OsalMemCalloc error\n", __func__);
        OsalMemFree(test->wbuf);
        return HDF_ERR_MALLOC_FAIL;
    }
    ret = face->GetUint32Array(node, "wbuf", tmp, test->len, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read wbuf fail", __func__);
        OsalMemFree(test->wbuf);
        OsalMemFree(tmp);
        return HDF_FAILURE;
    }
    for (i = 0; i < test->len; i++) {
        test->wbuf[i] = tmp[i];
    }
    OsalMemFree(tmp);
    test->rbuf = (uint8_t *)OsalMemCalloc(test->len);
    if (test->rbuf == NULL) {
        HDF_LOGE("%s: rbuf OsalMemCalloc error\n", __func__);
        OsalMemFree(test->wbuf);
        return HDF_ERR_MALLOC_FAIL;
    }
    return HDF_SUCCESS;
}

static int32_t SpiTestInit(struct HdfDeviceObject *device)
{
    struct SpiTest *test = NULL;

    if (device == NULL || device->service == NULL || device->property == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    test = (struct SpiTest *)device->service;
    SpiTestInitFromHcs(test, device->property);
    HDF_LOGE("%s: success", __func__);
    test->TestEntry = SpiTestEntry;
    return HDF_SUCCESS;
}

static void SpiTestRelease(struct HdfDeviceObject *device)
{
    struct SpiTest *test = NULL;

    if (device == NULL) {
        return;
    }
    test = (struct SpiTest *)device->service;
    if (test == NULL) {
        return;
    }
    if (test->wbuf != NULL) {
        OsalMemFree(test->wbuf);
    }
    if (test->rbuf != NULL) {
        OsalMemFree(test->rbuf);
    }
}

struct HdfDriverEntry g_spiTestEntry = {
    .moduleVersion = 1,
    .Bind = SpiTestBind,
    .Init = SpiTestInit,
    .Release = SpiTestRelease,
    .moduleName = "PLATFORM_SPI_TEST",
};
HDF_INIT(g_spiTestEntry);
