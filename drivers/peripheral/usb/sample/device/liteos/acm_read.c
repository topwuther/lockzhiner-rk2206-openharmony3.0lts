/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "hdf_io_service_if.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_mutex.h"
#include "osal_thread.h"
#include "osal_time.h"
#include "securec.h"
#define HDF_LOG_TAG   cdc_acm_read

enum UsbSerialCmd {
    USB_SERIAL_OPEN = 0,
    USB_SERIAL_CLOSE,
    USB_SERIAL_READ,
    USB_SERIAL_WRITE,
    USB_SERIAL_GET_BAUDRATE,
    USB_SERIAL_SET_BAUDRATE,
    USB_SERIAL_SET_PROP,
    USB_SERIAL_GET_PROP,
    USB_SERIAL_REGIST_PROP,
};

struct HdfSBuf *g_data;
struct HdfSBuf *g_reply;
struct HdfIoService *g_acmService;

#define STR_LEN  8192
#define SLEEP_READ 100
static void TestRead(FILE *fp)
{
    int ret;
    char str[STR_LEN] = {0};
    struct timeval time;
    HdfSbufFlush(g_reply);
    int status = g_acmService->dispatcher->Dispatch(&g_acmService->object, USB_SERIAL_READ, g_data, g_reply);
    if (status) {
        HDF_LOGE("%s: Dispatch USB_SERIAL_READ failed status = %d", __func__, status);
        return;
    }
    const char *tmp = HdfSbufReadString(g_reply);
    if (tmp && strlen(tmp) > 0) {
        gettimeofday(&time, NULL);
        ret = snprintf_s(str, STR_LEN, STR_LEN - 1, "[XTSCHECK] %d.%06d, recv data[%s] from host\n",
            time.tv_sec, time.tv_usec, tmp);
        if (ret < 0) {
            HDF_LOGE("%s: snprintf_s failed", __func__);
            return;
        }
        (void)fwrite(str, strlen(str), 1, fp);
        fflush(fp);
    }
    OsalMDelay(SLEEP_READ);
}

int main(int argc, char *argv[])
{
    int status;

    g_acmService = HdfIoServiceBind("usbfn_cdcacm");
    if (g_acmService == NULL || g_acmService->dispatcher == NULL || g_acmService->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: GetService err", __func__);
        return HDF_FAILURE;
    }

    g_data = HdfSBufObtainDefaultSize();
    g_reply = HdfSBufObtainDefaultSize();
    if (g_data == NULL || g_reply == NULL) {
        HDF_LOGE("%s: GetService err", __func__);
        return HDF_FAILURE;
    }

    status = g_acmService->dispatcher->Dispatch(&g_acmService->object, USB_SERIAL_OPEN, g_data, g_reply);
    if (status) {
        HDF_LOGE("%s: Dispatch USB_SERIAL_OPEN err", __func__);
        return HDF_FAILURE;
    }
    FILE *fp = fopen("/data/acm_read_xts", "a+");
    if (fp == NULL) {
        HDF_LOGE("%s: fopen err", __func__);
        return HDF_FAILURE;
    }
    while (1) {
        TestRead(fp);
    }
    (void)fclose(fp);
    status = g_acmService->dispatcher->Dispatch(&g_acmService->object, USB_SERIAL_CLOSE, g_data, g_reply);
    if (status) {
        HDF_LOGE("%s: Dispatch USB_SERIAL_CLOSE err", __func__);
        return HDF_FAILURE;
    }

    HdfSBufRecycle(g_data);
    HdfSBufRecycle(g_reply);

    return 0;
}
