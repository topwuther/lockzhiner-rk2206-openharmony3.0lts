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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/mman.h>
#include <osal_sem.h>
#include <osal_thread.h>
#include <errno.h>
#include <sys/syscall.h>
#include <time.h>
#include "usbhost_nosdk_speed.h"
#include "osal_time.h"
#include "osal_mem.h"
#include "securec.h"

#define USB_DEV_FS_PATH "/dev/bus/usb"
#define URB_COMPLETE_PROCESS_STACK_SIZE 8196

#define TEST_LENGTH     512
#define TEST_CYCLE      30
#define TEST_TIME       0xffffffff
#define TEST_PRINT_TIME 2

static pid_t tid;
static int exitOk = false;
static int g_speedFlag = 0;
static unsigned int g_busNum = 1;
static unsigned int g_devAddr = 2;
static int fd;
static struct OsalSem sem;
static uint64_t g_send_count = 0;
static uint64_t g_recv_count = 0;
static uint64_t g_byteTotal = 0;
static struct UsbAdapterUrbs urb[TEST_CYCLE];
static struct UsbAdapterUrb *sendUrb = NULL;
static bool g_printData = false;
static unsigned int ifaceNum;
static unsigned char endNum;

static void CloseDevice()
{
    if (fd > 0) {
        close(fd);
    }
    return;
}

static int OpenDevice()
{
    char path[24];
    int ret;

    ret = snprintf_s(path, 24, sizeof(path), USB_DEV_FS_PATH "/%03u/%03u", g_busNum, g_devAddr);
    if (ret < 0) {
        printf("path error\n");
        return ret;
    }

    printf("open: %s\n", path);
    fd = open(path, O_RDWR);

    if (fd < 0) {
        printf("open device failed! errno=%2d(%s)\n", errno, strerror(errno));
    }

    return fd;
}

static int ClaimInterface(unsigned int iface)
{
    if (fd < 0 || iface < 0) {
        printf("parameter error\n");
        return -1;
    }

    int r = ioctl(fd, USBDEVFS_CLAIMINTERFACE, &iface);
    if (r < 0) {
        printf("claim failed: iface=%u, errno=%2d(%s)\n", iface, errno, strerror(errno));
        return HDF_FAILURE;
    }
    printf("claim success: iface=%u\n", iface);
    return HDF_SUCCESS;
}

static void FillUrb(struct UsbAdapterUrb *urb, int len)
{
    if (urb == NULL)
    {
        urb = OsalMemCalloc(sizeof(*urb));
        urb->userContext = (void *)(urb);
        urb->type = USB_ADAPTER_URB_TYPE_BULK;
        urb->streamId = 0;
        urb->endPoint = endNum;
    }
    if (endNum >> 7 == 0) {
        memset_s(urb->buffer, len, 'c', len);
    }
}

void SignalHandler(int signo)
{
    static uint32_t sigCnt = 0;
    struct itimerval new_value, old_value;
    double speed = 0;
    switch (signo) {
        case SIGALRM:
            sigCnt++;
            if (sigCnt * TEST_PRINT_TIME >= TEST_TIME) {
                g_speedFlag = 1;
                break;
            }
            speed = (g_byteTotal * 1.0) / (sigCnt * TEST_PRINT_TIME  * 1024 * 1024);
            printf("\nSpeed:%f MB/s\n", speed);
            new_value.it_value.tv_sec = TEST_PRINT_TIME;
            new_value.it_value.tv_usec = 0;
            new_value.it_interval.tv_sec = TEST_PRINT_TIME;
            new_value.it_interval.tv_usec = 0;
            setitimer(ITIMER_REAL, &new_value, &old_value);
            break;
        case SIGINT:
            g_speedFlag = 1;
            break;
        default:
            break;
   }
}

static int SendProcess(void *argurb)
{
    int i;
    int r;
    while (!g_speedFlag) {
        OsalSemWait(&sem, HDF_WAIT_FOREVER);
        for (i = 0; i < TEST_CYCLE; i++) {
            if (urb[i].inUse == 0) {
                urb[i].inUse = 1;
                urb[i].urb->userContext = (void*)(&urb[i]);
                break;
            }
        }

        if (i==TEST_CYCLE) {
            i=TEST_CYCLE-1;
        }
        sendUrb = urb[i].urb;
        FillUrb(sendUrb, TEST_LENGTH);
        r = ioctl(fd, USBDEVFS_SUBMITURB, sendUrb);
        if (r < 0) {
            printf("SubmitBulkRequest: ret:%d errno=%d\n", r, errno);
            urb[i].inUse = 0;
            continue;
        }
        g_send_count++;
    }
    return 0;
}

static int ReapProcess(void *argurb)
{
    int r;
    struct UsbAdapterUrb *urbrecv = NULL;
    struct itimerval new_value, old_value;
    if (signal(SIGUSR1, SignalHandler) == SIG_ERR) {
        printf("signal SIGUSR1 failed");
        return HDF_ERR_IO;
    }
    tid = syscall(SYS_gettid);

    while (!g_speedFlag) {
        r = ioctl(fd, USBDEVFS_REAPURB, &urbrecv);
        if (r < 0) {
            continue;
        }
        if (urbrecv == NULL) {
            continue;
        }
        if (urbrecv->status == 0) {
            if (g_byteTotal == 0) {
                new_value.it_value.tv_sec = TEST_PRINT_TIME;
                new_value.it_value.tv_usec = 0;
                new_value.it_interval.tv_sec = TEST_PRINT_TIME;
                new_value.it_interval.tv_usec = 0;
                setitimer(ITIMER_REAL, &new_value, &old_value);
            }
            g_recv_count++;
            g_byteTotal += urbrecv->actualLength;
        }
        unsigned char *recvBuf = (unsigned char*)urbrecv->buffer;

        if (g_printData == true) {
            for (int i = 0; i < urbrecv->actualLength; i++)
                printf("%c", recvBuf[i]);
            fflush(stdout);
        } else if (g_recv_count % 10000 == 0) {
            printf("#");
            fflush(stdout);
        }

        struct UsbAdapterUrbs * urbs = urbrecv->userContext;
        urbs->inUse = 0;
        OsalSemPost(&sem);
    }
    exitOk = true;
    return 0;
}

static int BeginProcess(unsigned char endPoint)
{
    int r;
    char *data = NULL;
    struct timeval time;
    int transNum = 0;
    int i;

    if (fd < 0 || endPoint <= 0) {
        printf("parameter error\n");
        return -1;
    }
    for (int i = 0; i < TEST_CYCLE; i++) {
        urb[i].urb = calloc(1, sizeof(struct UsbAdapterUrb));
        if (urb[i].urb == NULL) {
            return -1;
        }
        urb[i].inUse = 0;
        urb[i].urb->userContext = (void *)(&urb[i]);
        urb[i].urb->type = USB_ADAPTER_URB_TYPE_BULK;
        urb[i].urb->streamId = 0;
        urb[i].urb->endPoint = endPoint;

        data = OsalMemCalloc(TEST_LENGTH);//AllocMemTest(TEST_LENGTH)
        if (data == NULL) {
            return -1;
        }
        memset_s(data, TEST_LENGTH, 'c', TEST_LENGTH);
        data[TEST_LENGTH - 1] = '\0';
        urb[i].urb->buffer = (void *)data;
        urb[i].urb->bufferLength = TEST_LENGTH;
    }

    gettimeofday(&time, NULL);
    signal(SIGINT, SignalHandler);
    signal(SIGALRM, SignalHandler);

    printf("test NO SDK endpoint:%d\n", endPoint);
    printf("Start: sec%ld usec%ld\n", time.tv_sec, time.tv_usec);

    for (i = 0; i < TEST_CYCLE; i++) {
        urb[i].inUse = 1;
        urb[i].urbNum = transNum;
        urb[i].urb->userContext = (void*)(&urb[i]);
        sendUrb = urb[i].urb;
        r = ioctl(fd, USBDEVFS_SUBMITURB, sendUrb);
        if (r < 0) {
            printf("SubmitBulkRequest: ret:%d errno=%d\n", r, errno);
            urb[i].inUse = 0;
            continue;
        }
        g_send_count++;
    }

    while (!g_speedFlag) {
        OsalMSleep(10);
    }

    kill(tid, SIGUSR1);
    while (exitOk != true) {
        OsalMSleep(10);
    }
    for (int i = 0; i < TEST_CYCLE; i++) {
        munmap(urb[i].urb->buffer, TEST_LENGTH);
        free(urb[i].urb);
    }
    return HDF_SUCCESS;
}

static void ShowHelp(char *name)
{
    printf(">> usage:\n");
    printf(">>      %s [<busNum> <devAddr>]  <ifaceNum> <endpoint> [<printdata>]\n", name);
    printf("\n");
}

int main(int argc, char *argv[])
{
    int ret;
    if (argc == 6) {
        g_busNum = atoi(argv[1]);
        g_devAddr = atoi(argv[2]);
        ifaceNum = atoi(argv[3]);
        endNum = atoi(argv[4]);
        if (endNum >> 7 != 0)
        {
            g_printData = (strncmp(argv[5], "printdata", 1))?false:true;
        }
    } else if (argc == 5) {
        g_busNum = atoi(argv[1]);
        g_devAddr = atoi(argv[2]);
        ifaceNum = atoi(argv[3]);
        endNum = atoi(argv[4]);
    } else if (argc == 3) {
        ifaceNum = atoi(argv[1]);
        endNum = atoi(argv[2]);
    } else {
        printf("Error: parameter error!\n\n");
        ShowHelp(argv[0]);
        return -1;
    }
    OsalSemInit(&sem, 0);

    fd = OpenDevice();
    if (fd < 0) {
        ret = -1;
        goto err;
    }

    ret = ClaimInterface(ifaceNum);
    if (ret != HDF_SUCCESS) {
        goto err;
    }

    struct OsalThread urbReapProcess;
    struct OsalThread urbSendProcess;
    struct OsalThreadParam threadCfg;

    (void)memset_s(&threadCfg, sizeof(threadCfg), 0, sizeof(threadCfg));
    threadCfg.name = "urb reap process";
    threadCfg.priority = OSAL_THREAD_PRI_DEFAULT;
    threadCfg.stackSize = URB_COMPLETE_PROCESS_STACK_SIZE;

    ret = OsalThreadCreate(&urbReapProcess, (OsalThreadEntry)ReapProcess, NULL);
    if (ret != HDF_SUCCESS) {
        printf("OsalThreadCreate fail, ret=%d\n", ret);
        goto err;
    }

    ret = OsalThreadStart(&urbReapProcess, &threadCfg);
    if (ret != HDF_SUCCESS) {
        printf("OsalThreadStart fail, ret=%d\n", ret);
    }

    threadCfg.name = "urb send process";
    threadCfg.priority = OSAL_THREAD_PRI_DEFAULT;
    threadCfg.stackSize = URB_COMPLETE_PROCESS_STACK_SIZE;

    ret = OsalThreadCreate(&urbSendProcess, (OsalThreadEntry)SendProcess, NULL);
    if (ret != HDF_SUCCESS) {
        printf("OsalThreadCreate fail, ret=%d\n", ret);
        goto err;
    }

    ret = OsalThreadStart(&urbSendProcess, &threadCfg);
    if (ret != HDF_SUCCESS) {
        printf("OsalThreadStart fail, ret=%d\n", ret);
    }

    ret = BeginProcess(endNum);
    if (ret != HDF_SUCCESS) {
        goto err;
    }

err:
    if (ret != HDF_SUCCESS) {
        printf("please check whether usb drv so is existing or not,like acm, ecm, if not, remove it and test again!\n");
    }
    CloseDevice();
    return ret;
}

