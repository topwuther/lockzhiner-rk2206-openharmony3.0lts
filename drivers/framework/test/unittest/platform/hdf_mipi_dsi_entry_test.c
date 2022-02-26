/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_mipi_dsi_entry_test.h"
#include "hdf_log.h"
#include "mipi_dsi_if.h"
#include "mipi_dsi_test.h"

#define HDF_LOG_TAG hdf_mipi_dsi_entry_test

int32_t HdfMipiDsiEntry(HdfTestMsg *msg)
{
    struct MipiDsiTest *test = NULL;

    if (msg == NULL) {
        return HDF_FAILURE;
    }

    test = MipiDsiTestServiceGet();
    if (test == NULL) {
        HDF_LOGE("%s: get service fail!", __func__);
        return HDF_FAILURE;
    }

    msg->result = test->doTest(test, msg->subCmd);

    return HDF_SUCCESS;
}
