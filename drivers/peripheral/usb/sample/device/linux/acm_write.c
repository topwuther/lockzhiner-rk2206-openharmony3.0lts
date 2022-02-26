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

#include <hdf_log.h>
#include <hdf_remote_service.h>
#include <hdf_sbuf.h>
#include <servmgr_hdi.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "cdcacm.h"

#define HDF_LOG_TAG   cdc_acm_write

struct HdfSBuf *g_data;
struct HdfSBuf *g_reply;
struct HdfRemoteService *g_acmService;

static void TestWrite(char *buf)
{
    HdfSbufFlush(g_data);
    (void)HdfSbufWriteString(g_data, buf);
    int status = g_acmService->dispatcher->Dispatch(g_acmService, USB_SERIAL_WRITE, g_data, g_reply);
    if (status <= 0) {
        HDF_LOGE("%s: Dispatch USB_SERIAL_WRITE failed status = %d", __func__, status);
    }
}

#define STR_LEN 1024
#define NUM_INPUT 2
int main(int argc, char *argv[])
{
    char str[STR_LEN] = {0};
    int status;
    FILE *fp = NULL;
    struct timeval time;
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    if (servmgr == NULL) {
        HDF_LOGE("%s: HDIServiceManagerGet err", __func__);
        return HDF_FAILURE;
    }
    g_acmService = servmgr->GetService(servmgr, "usbfn_cdcacm");
    HDIServiceManagerRelease(servmgr);
    if (g_acmService == NULL) {
        HDF_LOGE("%s: GetService err", __func__);
        return HDF_FAILURE;
    }

    g_data = HdfSBufTypedObtain(SBUF_IPC);
    g_reply = HdfSBufTypedObtain(SBUF_IPC);
    if (g_data == NULL || g_reply == NULL) {
        HDF_LOGE("%s: GetService err", __func__);
        return HDF_FAILURE;
    }

    status = g_acmService->dispatcher->Dispatch(g_acmService, USB_SERIAL_OPEN, g_data, g_reply);
    if (status) {
        HDF_LOGE("%s: Dispatch USB_SERIAL_OPEN err", __func__);
        return HDF_FAILURE;
    }
    if (argc >= NUM_INPUT) {
        gettimeofday(&time, NULL);
        fp = fopen("/data/acm_write_xts", "a+");
        status = snprintf_s(str, STR_LEN, STR_LEN - 1, "[XTSCHECK] %d.%06d, send data[%s] to host\n",
            time.tv_sec, time.tv_usec, argv[1]);
        if (status < 0) {
            HDF_LOGE("%s: snprintf_s failed", __func__);
            return HDF_FAILURE;
        }
        (void)fwrite(str, strlen(str), 1, fp);
        (void)fclose(fp);
        TestWrite(argv[1]);
    }
    status = g_acmService->dispatcher->Dispatch(g_acmService, USB_SERIAL_CLOSE, g_data, g_reply);
    if (status) {
        HDF_LOGE("%s: Dispatch USB_SERIAL_CLOSE err", __func__);
        return HDF_FAILURE;
    }

    HdfSBufRecycle(g_data);
    HdfSBufRecycle(g_reply);

    return 0;
}
