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

#include <ctype.h>
#include <string.h>
#include "devmgr_pnp_service.h"
#include "hcs_parser.h"
#include "hcs_dm_parser.h"
#include "hcs_tree_if.h"
#include "hdf_attribute_manager.h"
#include "hdf_device_info_full.h"
#include "hdf_host_info.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG attribute_manager

#define ATTR_HOST_NAME "hostName"
#define ATTR_DEV_POLICY "policy"
#define ATTR_DEV_PRIORITY "priority"
#define ATTR_DEV_PRELOAD "preload"
#define ATTR_DEV_PERMISSION "permission"
#define ATTR_DEV_MODULENAME "moduleName"
#define ATTR_DEV_SVCNAME "serviceName"
#define ATTR_DEV_MATCHATTR "deviceMatchAttr"
#define MANAGER_NODE_MATCH_ATTR "hdf_manager"

static bool HdfHostListCompare(struct HdfSListNode *listEntryFirst, struct HdfSListNode *listEntrySecond)
{
    if (listEntryFirst == NULL || listEntrySecond == NULL) {
        return false;
    }
    struct HdfHostInfo *attrFirst = (struct HdfHostInfo *)listEntryFirst;
    struct HdfHostInfo *attrSecond = (struct HdfHostInfo *)listEntrySecond;
    return attrFirst->priority <= attrSecond->priority;
}

static const struct DeviceResourceNode *GetHdfManagerNode(const struct DeviceResourceNode *node)
{
    return HcsGetNodeByMatchAttr(node, MANAGER_NODE_MATCH_ATTR);
}

static bool GetHostInfo(const struct DeviceResourceNode *hostNode, struct HdfHostInfo *hostInfo)
{
    uint16_t readNum = 0;
    if ((HcsGetString(hostNode, ATTR_HOST_NAME, &hostInfo->hostName, NULL) != HDF_SUCCESS) ||
        (strcmp(hostInfo->hostName, "") == 0)) {
        HDF_LOGW("%{public}s get host name failed", __func__);
        return false;
    }
    if ((HcsGetUint16(hostNode, ATTR_DEV_PRIORITY, &readNum, 0) != HDF_SUCCESS) ||
        (readNum > MAX_PRIORITY_NUM)) {
        HDF_LOGW("%{public}s get host priority failed, priority is: %{public}d", __func__, readNum);
        return false;
    }
    hostInfo->priority = readNum;
    return true;
}

bool HdfAttributeManagerGetHostList(struct HdfSList *hostList)
{
    const struct DeviceResourceNode *hdfManagerNode = NULL;
    const struct DeviceResourceNode *hostNode = NULL;
    uint16_t hostId = 0;
    if (hostList == NULL) {
        return false;
    }

    hdfManagerNode = GetHdfManagerNode(HdfGetRootNode());
    if (hdfManagerNode == NULL) {
        HDF_LOGE("%{public}s get hdf manager node is null", __func__);
        return false;
    }

    hostNode = hdfManagerNode->child;
    while (hostNode != NULL) {
        struct HdfHostInfo *hostInfo = HdfHostInfoNewInstance();
        if (hostInfo == NULL) {
            HdfSListFlush(hostList, HdfHostInfoDelete);
            HDF_LOGE("%{public}s new hostInfo is null", __func__);
            return false;
        }
        if (!GetHostInfo(hostNode, hostInfo)) {
            HdfHostInfoFreeInstance(hostInfo);
            hostNode = hostNode->sibling;
            continue;
        }
        hostInfo->hostId = hostId;
        if (!HdfSListAddOrder(hostList, &hostInfo->node, HdfHostListCompare)) {
            HdfHostInfoFreeInstance(hostInfo);
            hostNode = hostNode->sibling;
            continue;
        }
        hostId++;
        hostNode = hostNode->sibling;
    }
    return true;
}

static bool HdfDeviceListCompare(struct HdfSListNode *listEntryFirst, struct HdfSListNode *listEntrySecond)
{
    if (listEntryFirst == NULL || listEntrySecond == NULL) {
        return false;
    }
    struct HdfDeviceInfo *attrFirst = (struct HdfDeviceInfo *)listEntryFirst;
    struct HdfDeviceInfo *attrSecond = (struct HdfDeviceInfo *)listEntrySecond;
    return attrFirst->priority <= attrSecond->priority;
}

static const struct DeviceResourceNode *GetHostNode(const char *inHostName)
{
    const struct DeviceResourceNode *hdfManagerNode = NULL;
    const struct DeviceResourceNode *hostNode = NULL;
    const char *hostName = NULL;

    hdfManagerNode = GetHdfManagerNode(HdfGetRootNode());
    if (hdfManagerNode == NULL) {
        return NULL;
    }
    hostNode = hdfManagerNode->child;
    while (hostNode != NULL) {
        if (HcsGetString(hostNode, ATTR_HOST_NAME, &hostName, NULL) != HDF_SUCCESS) {
            hostNode = hostNode->sibling;
            continue;
        }
        if (strcmp(hostName, inHostName) == 0) {
            return hostNode;
        }
        hostNode = hostNode->sibling;
    }
    return NULL;
}

static bool CheckDeviceInfo(struct HdfDeviceInfo *deviceNodeInfo)
{
    if (deviceNodeInfo->policy > SERVICE_POLICY_PRIVATE) {
        HDF_LOGE("%{public}s policy is invalid", __func__);
        return false;
    }

    if (deviceNodeInfo->priority > MAX_PRIORITY_NUM) {
        HDF_LOGE("%{public}s priority is invalid", __func__);
        return false;
    }

    if (deviceNodeInfo->preload > DEVICE_PRELOAD_DISABLE) {
        HDF_LOGE("%{public}s preload is invalid", __func__);
        return false;
    }
    return true;
}

static bool GetDeviceNodeInfo(const struct DeviceResourceNode *deviceNode, struct HdfDeviceInfo *deviceNodeInfo)
{
    uint16_t readNum = 0;
    const char *readString = NULL;
    if (HcsGetUint16(deviceNode, ATTR_DEV_POLICY, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s get policy failed", __func__);
        return false;
    }
    deviceNodeInfo->policy = readNum;

    if (HcsGetUint16(deviceNode, ATTR_DEV_PRIORITY, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s get priority failed", __func__);
        return false;
    }
    deviceNodeInfo->priority = readNum;

    if (HcsGetUint16(deviceNode, ATTR_DEV_PRELOAD, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s get preload failed", __func__);
        return false;
    }
    deviceNodeInfo->preload = readNum;

    if (HcsGetString(deviceNode, ATTR_DEV_MODULENAME, &readString, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s get module name failed", __func__);
        return false;
    }
    deviceNodeInfo->moduleName = strdup(readString);
    if (deviceNodeInfo->moduleName == NULL) {
        return false;
    }

    if (HcsGetString(deviceNode, ATTR_DEV_SVCNAME, &readString, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s get service name failed", __func__);
        return false;
    }
    deviceNodeInfo->svcName = strdup(readString);
    if (deviceNodeInfo->svcName == NULL) {
        return false;
    }

    if (HcsGetString(deviceNode, ATTR_DEV_MATCHATTR, &readString, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s get service name failed", __func__);
        return false;
    }
    deviceNodeInfo->deviceMatchAttr = readString;
    return CheckDeviceInfo(deviceNodeInfo);
}

struct HdfSList *HdfAttributeManagerGetDeviceList(uint16_t hostId, const char *hostName)
{
    uint16_t deviceIdx = 0;
    if (hostName == NULL) {
        return NULL;
    }
    if (strcmp(hostName, PNP_HOST_NAME) == 0) {
        return DevmgrServiceGetPnpDeviceInfo();
    }

    const struct DeviceResourceNode *hostNode = GetHostNode(hostName);
    if (hostNode == NULL) {
        return NULL;
    }

    struct HdfSList *deviceList = (struct HdfSList *)OsalMemCalloc(sizeof(struct HdfSList));
    if (deviceList == NULL) {
        return NULL;
    }
    const struct DeviceResourceNode *device = hostNode->child;
    while (device != NULL) {
        const struct DeviceResourceNode *deviceNode = device->child;
        while (deviceNode != NULL) {
            struct HdfDeviceInfoFull *deviceNodeInfo = HdfDeviceInfoFullNewInstance();
            if (deviceNodeInfo == NULL) {
                HdfSListFlush(deviceList, HdfDeviceInfoDelete);
                OsalMemFree(deviceList);
                return NULL;
            }
            deviceNodeInfo->super.hostId = hostId;
            if (!GetDeviceNodeInfo(deviceNode, &deviceNodeInfo->super)) {
                HdfDeviceInfoFullFreeInstance(deviceNodeInfo);
                HDF_LOGE("%{public}s get device failed", __func__);
                deviceNodeInfo = NULL;
                deviceNode = deviceNode->sibling;
                continue;
            }
            if (!HdfSListAddOrder(deviceList, &deviceNodeInfo->super.node, HdfDeviceListCompare)) {
                HDF_LOGE("%{public}s add device %{public}s failed", __func__, deviceNodeInfo->super.svcName);
                HdfDeviceInfoFullFreeInstance(deviceNodeInfo);
                deviceNodeInfo = NULL;
                deviceNode = deviceNode->sibling;
                continue;
            }
            deviceNodeInfo->super.deviceId = deviceIdx;
            deviceNode = deviceNode->sibling;
        }
        device = device->sibling;
        deviceIdx++;
    }
    if (HdfSListCount(deviceList) == 0) {
        OsalMemFree(deviceList);
        return NULL;
    }
    return deviceList;
}

