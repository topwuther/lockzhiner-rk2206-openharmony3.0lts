/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef UART_TEST_H
#define UART_TEST_H

#include "hdf_device_desc.h"
#include "hdf_platform.h"

enum UartTestCmd {
    UAER_WRITE_TEST = 0,
    UART_READ_TEST,
    UART_SET_BAUD_TEST,
    UART_GET_BAUD_TEST,
    UART_SET_ATTRIBUTE_TEST,
    UART_GET_ATTRIBUTE_TEST,
    UART_SET_TRANSMODE_TEST,
    UART_RELIABILITY_TEST,
    UART_PERFORMANCE_TEST,
    UART_TEST_ALL,
};

struct UartTest {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    int32_t (*TestEntry)(struct UartTest *test, int32_t cmd);
    uint32_t port;
    uint32_t len;
    uint8_t *wbuf;
    uint8_t *rbuf;
    DevHandle handle;
};

static inline struct UartTest *GetUartTest(void)
{
    return (struct UartTest *)DevSvcManagerClntGetService("UART_TEST");
}

#endif /* UART_TEST_H */
