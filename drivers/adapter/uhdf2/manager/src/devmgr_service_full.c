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

#include "devmgr_service_full.h"
#include "devhost_service.h"
#include "devhost_service_clnt.h"
#include "device_token_clnt.h"
#include "hdf_device_token.h"
#include "hdf_driver_installer.h"
#include "hdf_log.h"
#include "hdf_map.h"
#include "hdf_message_looper.h"
#include "osal_message.h"

#define HDF_LOG_TAG devmgr_service_full
#define INVALID_PID (-1)

static Map g_hostMap = {0};
#define HOST_INIT_DIE_NUM 1
#define HOST_MAX_DIE_NUM 3

static int32_t DevmgrServiceFullHandleDeviceHostDied(struct DevHostServiceClnt *hostClnt)
{
    if (!HdfSListIsEmpty(hostClnt->deviceInfos)) {
        if (g_hostMap.nodeSize == 0) {
            MapInit(&g_hostMap);
        }
        int *hostDieValue = (int *)MapGet(&g_hostMap, hostClnt->hostName);
        if (hostDieValue == NULL) {
            int hostDieNum = HOST_INIT_DIE_NUM;
            MapSet(&g_hostMap, hostClnt->hostName, &hostDieNum, sizeof(int));
        } else {
            if (*hostDieValue > HOST_MAX_DIE_NUM) {
                *hostDieValue = 0;
                return INVALID_PID;
            }
            (*hostDieValue)++;
        }
        struct IDriverInstaller *installer = DriverInstallerGetInstance();
        if ((installer != NULL) && (installer->StartDeviceHost)) {
            hostClnt->hostPid = installer->StartDeviceHost(hostClnt->hostId, hostClnt->hostName);
            return hostClnt->hostPid;
        }
    }
    return INVALID_PID;
}

void DevmgrServiceFullOnDeviceHostDied(struct DevmgrServiceFull *inst, uint32_t hostId)
{
    (void)hostId;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevHostServiceClnt *hostClntTmp = NULL;
    if (inst == NULL) {
        return;
    }
    OsalMutexLock(&inst->super.devMgrMutex);
    DLIST_FOR_EACH_ENTRY_SAFE(hostClnt, hostClntTmp,  &inst->super.hosts, struct DevHostServiceClnt, node) {
        if (hostClnt->hostId == hostId) {
            int32_t ret = DevmgrServiceFullHandleDeviceHostDied(hostClnt);
            if (ret == INVALID_PID) {
                DListRemove(&hostClnt->node);
                DevHostServiceClntFreeInstance(hostClnt);
            }
            break;
        }
    }
    OsalMutexUnlock(&inst->super.devMgrMutex);
}

int32_t DevmgrServiceFullDispatchMessage(struct HdfMessageTask *task, struct HdfMessage *msg)
{
    (void)task;
    struct DevmgrServiceFull *fullService =
        (struct DevmgrServiceFull *)DevmgrServiceGetInstance();
    if (msg == NULL) {
        HDF_LOGE("Input msg is null");
        return HDF_ERR_INVALID_PARAM;
    }
    switch (msg->messageId) {
        case DEVMGR_MESSAGE_DEVHOST_DIED: {
            int hostId = (int)(uintptr_t)msg->data[0];
            DevmgrServiceFullOnDeviceHostDied(fullService, hostId);
            break;
        }
        default: {
            HDF_LOGE("Message is wrong, message is %{public}u", msg->messageId);
        }
    }

    return HDF_SUCCESS;
}

struct HdfMessageTask *DevmgrServiceFullGetMessageTask()
{
    struct DevmgrServiceFull *fullService =
        (struct DevmgrServiceFull *)DevmgrServiceGetInstance();
    if (fullService != NULL) {
        HDF_LOGE("Get message task failed, fullService is null");
        return &fullService->task;
    }
    return NULL;
}

void DevmgrServiceFullConstruct(struct DevmgrServiceFull *inst)
{
    static struct IHdfMessageHandler handler = {
        .Dispatch = DevmgrServiceFullDispatchMessage
    };
    if (inst != NULL) {
        HdfMessageLooperConstruct(&inst->looper);
        DevmgrServiceConstruct(&inst->super);
        HdfMessageTaskConstruct(&inst->task, &inst->looper, &handler);
    }
}

struct HdfObject *DevmgrServiceFullCreate()
{
    static struct DevmgrServiceFull *instance = NULL;
    if (instance == NULL) {
        static struct DevmgrServiceFull fullInstance;
        DevmgrServiceFullConstruct(&fullInstance);
        instance = &fullInstance;
    }
    return (struct HdfObject *)instance;
}

int DeviceManagerIsQuickLoad(void)
{
    return false;
}
