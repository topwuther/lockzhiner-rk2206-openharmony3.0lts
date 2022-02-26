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

#include "securec.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "hdf_io_service_if.h"
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "osal_mutex.h"
#include "signal.h"

#define TEST_WRITE              true
#define TEST_READ               false
#define SERVER_NAME_SDKAPI "usb_sdkapispeed_service"
#define SERVER_NAME_RAWAPI "usb_rawapispeed_service"
#define SERVER_NAME_NOSDK  "usb_nosdkspeed_service"

enum UsbSerialCmd {
    USB_SERIAL_OPEN = 0,
    USB_SERIAL_CLOSE,
    USB_SERIAL_SPEED,
};
struct UsbSpeedTest {
    int busNum;
    int devAddr;
    int ifaceNum;
    int writeOrRead;
    bool printData;
    int paramNum;
};
enum speedServer {
    SDKAPI_SERVER = 0,
    RAWAPI_SERVER,
    NOSDK_SERVER,
};

static struct HdfIoService *g_service = NULL;
static struct HdfSBuf *g_data = NULL;
static struct HdfSBuf *g_reply = NULL;
static struct OsalMutex g_lock;
static enum speedServer spdserver = SDKAPI_SERVER;

static sigset_t mask;
pid_t stopHandlerTid;

void speedTest(struct UsbSpeedTest test)
{
    int ret = 0;

    OsalMutexLock(&g_lock);
    HdfSbufFlush(g_data);
    bool bufok = HdfSbufWriteBuffer(g_data, (const void *)&test, sizeof(test));
    if (!bufok) {
        printf("HdfSbufWriteBuffer err");
        ret = 0;
        goto RET;
    }
    int status = g_service->dispatcher->Dispatch(&g_service->object, USB_SERIAL_SPEED, g_data, g_reply);
    if (status < 0) {
        printf("%s: Dispatch USB_SERIAL_SPEED failed status = %d\n", __func__, status);
    }
RET:
    OsalMutexUnlock(&g_lock);
}

void speedInit()
{
    int status;

    switch (spdserver) {
        case SDKAPI_SERVER:
            g_service = HdfIoServiceBind(SERVER_NAME_SDKAPI);
            break;
        case RAWAPI_SERVER:
            g_service = HdfIoServiceBind(SERVER_NAME_RAWAPI);
            break;
        case NOSDK_SERVER:
            g_service = HdfIoServiceBind(SERVER_NAME_NOSDK);
            break;
    }
    if (g_service == NULL || g_service->dispatcher == NULL || g_service->dispatcher->Dispatch == NULL) {
        printf("%s: GetService spdserver=%d err \n", __func__, spdserver);
        return;
    }

    g_data = HdfSBufObtain(2000);
    g_reply = HdfSBufObtain(2000);
    if (g_data == NULL || g_reply == NULL) {
        printf("%s: HdfSBufTypedObtain err", __func__);
        return;
    }

    status = g_service->dispatcher->Dispatch(&g_service->object, USB_SERIAL_OPEN, g_data, g_reply);
    if (status) {
        printf("%s: Dispatch USB_SERIAL_OPEN err status = %d\n", __func__, status);
        return;
    }

    if (OsalMutexInit(&g_lock) != HDF_SUCCESS) {
        printf("%s: init lock fail!", __func__);
        return;
    }
}

void speedExit()
{
    int status = g_service->dispatcher->Dispatch(&g_service->object, USB_SERIAL_CLOSE, g_data, g_reply);
    if (status) {
        printf("%s: Dispatch USB_SERIAL_CLOSE err status = %d\n", __func__, status);
    }

    if (g_service != NULL) {
        HdfIoServiceRecycle(g_service);
        g_service = NULL;
    }
    HdfSBufRecycle(g_data);
    HdfSBufRecycle(g_reply);
}

static void ShowHelp(char *name)
{
    printf(">> usage:\n");
    printf(">>      %s <-SDK>/<-RAW>/<-NOSDK> [<busNum> <devAddr>]  <ifaceNum> <w>/<r>/<endpoint> [printdata]> \n", name);
    printf("\n");
}

static void *stop_handler(void *arg)
{
    int err, signo;
    stopHandlerTid = getpid();

    for (;;) {
        err = sigwait(&mask, &signo);
        if (err != 0) {
            printf("Sigwait failed: %d\n", err);
        }

        switch (signo) {
        case SIGINT:
        case SIGQUIT:
            printf("normal exit\n");
            speedExit();
            return 0;
        default:
            printf("Unexpected signal %d\n", signo);
        }
    }
}

static enum speedServer checkServer(const char* input)
{
    char middle[10] = {0};
    enum speedServer out = SDKAPI_SERVER;
    strncpy_s(middle, sizeof(middle), input, strlen(input));
    if (!strcmp(middle, "-SDK")) {
        out = SDKAPI_SERVER;
    } else if (!strcmp(middle, "-RAW")) {
        out = RAWAPI_SERVER;
    } else if (!strcmp(middle, "-NOSDK")) {
        out = NOSDK_SERVER;
    } else {
        out = SDKAPI_SERVER;
    }
    return out;
}

int main(int argc, char *argv[]) {
    struct UsbSpeedTest test;
    int busNum = 1;
    int devAddr = 2;
    int ifaceNum = 3;
    int writeOrRead;
    bool printData = false;
    int paramNum;
    int ret = 0;
    if (argc == 7) {
        if (argv[1] != NULL) {
            spdserver = checkServer(argv[1]);
        }
        if (argv[2] != NULL) {
            busNum = atoi(argv[2]);
        }
        if (argv[3] != NULL) {
            devAddr = atoi(argv[3]);
        }
        if (argv[4] != NULL) {
            ifaceNum = atoi(argv[4]);
        }
        if (!strncmp(argv[5], "r", 1)) {
            writeOrRead = TEST_READ;
        } else if (!strncmp(argv[5], "w", 1)) {
            writeOrRead = TEST_WRITE;
        } else {
            writeOrRead = atoi(argv[5]);
        }
        if (writeOrRead == TEST_READ)
        {
            printData = (strncmp(argv[6], "printdata", 1))?false:true;
        }
    } else if (argc == 6) {
        spdserver = checkServer(argv[1]);
        busNum = atoi(argv[2]);
        devAddr = atoi(argv[3]);
        ifaceNum = atoi(argv[4]);
        if (!strncmp(argv[5], "r", 1)) {
            writeOrRead = TEST_READ;
        } else if (!strncmp(argv[5], "w", 1)) {
            writeOrRead = TEST_WRITE;
        } else {
            writeOrRead = atoi(argv[5]);
        }
    } else if (argc == 4) {
        spdserver = checkServer(argv[1]);
        ifaceNum = atoi(argv[2]);
        if (!strncmp(argv[3], "r", 1)) {
            writeOrRead = TEST_READ;
        } else if (!strncmp(argv[3], "w", 1)) {
            writeOrRead = TEST_WRITE;
        } else {
            writeOrRead = atoi(argv[3]);
        }
    } else {
        printf("Error: parameter error!\n\n");
        ShowHelp(argv[0]);
        ret = HDF_FAILURE;
        goto END;
    }
    paramNum = argc - 1;
    test.busNum = busNum;
    test.devAddr = devAddr;
    test.ifaceNum = ifaceNum;
    test.writeOrRead = writeOrRead;
    test.printData = printData;
    test.paramNum = paramNum;
    pthread_t threads;

    int err;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
        printf("SIG_BLOCK error\n");
        ret = HDF_FAILURE;
        goto END;
    }
    if (pthread_create(&threads, NULL, stop_handler, NULL) != 0) {
        printf("Could not create core thread\n");
        ret = HDF_FAILURE;
        goto END;
    }

    speedInit();
    speedTest(test);
    kill(stopHandlerTid, SIGINT);
    pthread_join(threads, NULL);
END:
    return ret;
}
