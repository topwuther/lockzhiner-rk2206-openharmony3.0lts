/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "uart_if.h"
#include "uart_test.h"

#define HDF_LOG_TAG uart_test_c
struct UartTestFunc {
    enum UartTestCmd type;
    int32_t (*Func)(struct UartTest *test);
};

#define BITS_PER_WORD 10
#define MAX_SPEED_HZ 10000000
static int32_t UartWriteTest(struct UartTest *test)
{
    if (UartWrite(test->handle, test->wbuf, test->len) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartReadTest(struct UartTest *test)
{
    if (UartSetTransMode(test->handle, UART_MODE_RD_NONBLOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    } 
    if (UartRead(test->handle, test->rbuf, test->len) != 0) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

#define BAUD_921600 921600
static int32_t UartSetBaudTest(struct UartTest *test)
{
    if (UartSetBaud(test->handle, BAUD_921600) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartGetBaudTest(struct UartTest *test)
{
    uint32_t baud;

    if (UartGetBaud(test->handle, &baud) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: baud %u", __func__, baud);
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartSetAttributeTest(struct UartTest *test)
{
    struct UartAttribute attribute;

    attribute.dataBits = UART_ATTR_DATABIT_7;
    attribute.parity = UART_ATTR_PARITY_NONE;
    attribute.stopBits = UART_ATTR_STOPBIT_1;
    attribute.rts = UART_ATTR_RTS_DIS;
    attribute.cts = UART_ATTR_CTS_DIS;
    attribute.fifoRxEn = UART_ATTR_RX_FIFO_EN;
    attribute.fifoTxEn = UART_ATTR_TX_FIFO_EN;
    if (UartSetAttribute(test->handle, &attribute) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartGetAttributeTest(struct UartTest *test)
{
    struct UartAttribute attribute;

    if (UartGetAttribute(test->handle, &attribute) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("dataBits %u", attribute.dataBits);
    HDF_LOGE("parity %u", attribute.parity);
    HDF_LOGE("stopBits %u", attribute.stopBits);
    HDF_LOGE("rts %u", attribute.rts);
    HDF_LOGE("cts %u", attribute.cts);
    HDF_LOGE("fifoRxEn %u", attribute.fifoRxEn);
    HDF_LOGE("fifoTxEn %u", attribute.fifoTxEn);
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartSetTransModeTest(struct UartTest *test)
{
    if (UartSetTransMode(test->handle, UART_MODE_RD_NONBLOCK) != HDF_SUCCESS) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartReliabilityTest(struct UartTest *test)
{
    uint32_t baud;
    struct UartAttribute attribute = {0};

    (void)UartSetTransMode(test->handle, UART_MODE_RD_NONBLOCK);
    (void)UartSetTransMode(test->handle, -1);
    (void)UartWrite(test->handle, test->wbuf, test->len);
    (void)UartWrite(test->handle, NULL, -1);
    (void)UartRead(test->handle, test->rbuf, test->len);
    (void)UartRead(test->handle, NULL, -1);
    (void)UartSetBaud(test->handle, BAUD_921600);
    (void)UartSetBaud(test->handle, -1);
    (void)UartGetBaud(test->handle, &baud);
    (void)UartGetBaud(test->handle, NULL);
    (void)UartSetAttribute(test->handle, &attribute);
    (void)UartSetAttribute(test->handle, NULL);
    (void)UartGetAttribute(test->handle, &attribute);
    (void)UartGetAttribute(test->handle, NULL);
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartTestAll(struct UartTest *test)
{
    int32_t total = 0;
    int32_t error = 0;

    if (UartSetTransModeTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartWriteTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartReadTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartSetBaudTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartGetBaudTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartSetAttributeTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartGetAttributeTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (UartReliabilityTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    HDF_LOGE("%s: Uart Test Total %d Error %d", __func__, total, error);
    return HDF_SUCCESS;
}

struct UartTestFunc g_uartTestFunc[] = {
    { UAER_WRITE_TEST, UartWriteTest },
    { UART_READ_TEST, UartReadTest },
    { UART_SET_BAUD_TEST, UartSetBaudTest },
    { UART_GET_BAUD_TEST, UartGetBaudTest },
    { UART_SET_ATTRIBUTE_TEST, UartSetAttributeTest },
    { UART_GET_ATTRIBUTE_TEST, UartGetAttributeTest },
    { UART_SET_TRANSMODE_TEST, UartSetTransModeTest },
    { UART_RELIABILITY_TEST, UartReliabilityTest },
    { UART_PERFORMANCE_TEST, NULL },
    { UART_TEST_ALL, UartTestAll },
};

static int32_t UartTestEntry(struct UartTest *test, int32_t cmd)
{
    int32_t i;
    int32_t ret = HDF_ERR_NOT_SUPPORT;

    if (test == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    test->handle = UartOpen(test->port);
    if (test->handle == NULL) {
        HDF_LOGE("%s: spi test get handle fail", __func__);
        return HDF_FAILURE;
    }
    for (i = 0; i < sizeof(g_uartTestFunc) / sizeof(g_uartTestFunc[0]); i++) {
        if (cmd == g_uartTestFunc[i].type && g_uartTestFunc[i].Func != NULL) {
            ret = g_uartTestFunc[i].Func(test);
            HDF_LOGE("%s: cmd %d ret %d", __func__, cmd, ret);
            break;
        }
    }
    UartClose(test->handle);
    return ret;
}

static int32_t UartTestBind(struct HdfDeviceObject *device)
{
    static struct UartTest test;

    if (device != NULL) {
        device->service = &test.service;
    } else {
        HDF_LOGE("%s: device is NULL", __func__);
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

static int32_t UartTestInitFromHcs(struct UartTest *test, const struct DeviceResourceNode *node)
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
    ret = face->GetUint32(node, "port", &test->port, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read port fail", __func__);
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

static int32_t UartTestInit(struct HdfDeviceObject *device)
{
    struct UartTest *test = NULL;

    if (device == NULL || device->service == NULL || device->property == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    test = (struct UartTest *)device->service;
    UartTestInitFromHcs(test, device->property);
    HDF_LOGE("%s: success", __func__);
    test->TestEntry = UartTestEntry;
    return HDF_SUCCESS;
}

static void UartTestRelease(struct HdfDeviceObject *device)
{
    struct UartTest *test = NULL;

    if (device == NULL) {
        return;
    }
    test = (struct UartTest *)device->service;
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

struct HdfDriverEntry g_uartTestEntry = {
    .moduleVersion = 1,
    .Bind = UartTestBind,
    .Init = UartTestInit,
    .Release = UartTestRelease,
    .moduleName = "PLATFORM_UART_TEST",
};
HDF_INIT(g_uartTestEntry);
