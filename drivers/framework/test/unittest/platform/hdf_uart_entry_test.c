/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_uart_entry_test.h"
#include "hdf_log.h"
#include "uart_if.h"
#include "uart_test.h"

#define HDF_LOG_TAG hdf_uart_entry_test

int32_t HdfUartUnitTestEntry(HdfTestMsg *msg)
{
    struct UartTest *test = NULL;

    if (msg == NULL) {
        return HDF_FAILURE;
    }
    test = GetUartTest();
    if (test == NULL || test->TestEntry == NULL) {
        msg->result = HDF_FAILURE;
        return HDF_FAILURE;
    }
    msg->result = test->TestEntry(test, msg->subCmd);
    return msg->result;
}
