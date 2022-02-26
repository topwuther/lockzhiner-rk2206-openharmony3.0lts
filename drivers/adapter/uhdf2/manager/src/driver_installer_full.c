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

#include "driver_installer_full.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hdf_base.h"
#include "hdf_driver_installer.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define DEV_HOST_BINARY "/system/bin/hdf_devhost"
#define HDF_LOG_TAG driver_installer_full
#define MAX_CMD_LEN 256

static bool g_sigFlag = false;
static struct DriverInstaller *g_fullInstaller = NULL;

void SigChildProc(int signo)
{
    int stat;
    (void)signo;
    wait(&stat);
}

int DriverInstallerFullStartDeviceHost(uint32_t devHostId, const char* devHostName)
{
    char cmd[MAX_CMD_LEN] = {0};
    // fork process.
    if (snprintf_s(cmd, sizeof(cmd), sizeof(cmd) - 1, " %d", devHostId) < 0) {
        HDF_LOGE("Starting device host, snprintf_s failed");
        return HDF_FAILURE;
    }
    if (!g_sigFlag) {
        if (signal(SIGCHLD, SigChildProc) == SIG_ERR) {
            HDF_LOGE("Starting device host, signal failed");
            return HDF_FAILURE;
        }
        g_sigFlag = true;
    }

    pid_t fpid;
    fpid = fork();
    if (fpid < 0) {
        HDF_LOGE("Starting device host, fork failed");
        return HDF_FAILURE;
    } else if (fpid == 0) {
        char * const args[] = {DEV_HOST_BINARY, cmd, (char * const)devHostName, NULL};
        char * const envs[] = {NULL};
        if (execve(DEV_HOST_BINARY, args, envs) == -1) {
            HDF_LOGE("Start device host, execve failed");
            return HDF_FAILURE;
        }
    } else {
        HDF_LOGI("Starting device host success");
    }
    return HDF_SUCCESS;
}

static void DriverInstallerFullConstruct(struct DriverInstaller *inst)
{
    struct IDriverInstaller *pvtbl = (struct IDriverInstaller *)inst;
    pvtbl->StartDeviceHost = DriverInstallerFullStartDeviceHost;
}

struct HdfObject *DriverInstallerFullCreate(void)
{
    if (g_fullInstaller == NULL) {
        g_fullInstaller = (struct DriverInstaller *)OsalMemCalloc(sizeof(struct DriverInstaller));
        if (g_fullInstaller != NULL) {
            DriverInstallerFullConstruct(g_fullInstaller);
        }
    }

    return (struct HdfObject *)g_fullInstaller;
}

void DriverInstallerFullRelease(struct HdfObject *object)
{
    struct DriverInstaller *installer = (struct DriverInstaller *)object;
    if (g_fullInstaller == installer) {
        g_fullInstaller = NULL;
    }
    if (installer != NULL) {
        OsalMemFree(installer);
    }
}
