/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_attribute_manager.h"
#include "devhost_service_clnt.h"
#include "devmgr_service.h"
#include "hcs_blob_if.h"
#include "hcs_parser.h"
#include "hcs_tree_if.h"
#include "hdf_base.h"
#include "hdf_device_info.h"
#include "hdf_host_info.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"
#ifdef LOSCFG_DRIVERS_HDF_USB_PNP_NOTIFY
#include "usb_pnp_manager.h"
#endif

#define ATTR_HOST_NAME "hostName"
#define ATTR_DEV_POLICY "policy"
#define ATTR_DEV_PRIORITY "priority"
#define ATTR_DEV_PRELOAD "preload"
#define ATTR_DEV_PERMISSION "permission"
#define ATTR_DEV_MODULENAME "moduleName"
#define ATTR_DEV_SVCNAME "serviceName"
#define ATTR_DEV_MATCHATTR "deviceMatchAttr"
#define MANAGER_NODE_MATCH_ATTR "hdf_manager"

static struct DeviceResourceNode *g_hcsTreeRoot = NULL;

void HdfGetBuildInConfigData(const unsigned char **data, unsigned int *size);

static bool CreateHcsToTree(void)
{
    uint32_t length;
    const unsigned char *hcsBlob = NULL;
    HdfGetBuildInConfigData(&hcsBlob, &length);
    if (!HcsCheckBlobFormat((const char *)hcsBlob, length)) {
        return false;
    }
    if (!HcsDecompile((const char *)hcsBlob, HBC_HEADER_LENGTH, &g_hcsTreeRoot)) {
        return false;
    }
    return true;
}

const struct DeviceResourceNode *HcsGetRootNode(void)
{
    if ((g_hcsTreeRoot == NULL) && !CreateHcsToTree()) {
        HDF_LOGE("%s: failed", __func__);
        return NULL;
    }
    return g_hcsTreeRoot;
}

const struct DeviceResourceNode *HdfGetRootNode(void)
{
    return HcsGetRootNode();
}

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
        HDF_LOGW("%s: get host name failed", __func__);
        return false;
    }
    if ((HcsGetUint16(hostNode, ATTR_DEV_PRIORITY, &readNum, 0) != HDF_SUCCESS) ||
        (readNum > MAX_PRIORITY_NUM)) {
        HDF_LOGW("%s: get host priority failed, priority is: %u", __func__, readNum);
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

    hdfManagerNode = GetHdfManagerNode(HcsGetRootNode());
    if (hdfManagerNode == NULL) {
        HDF_LOGE("%s: get hdf manager node is null", __func__);
        return false;
    }

    hostNode = hdfManagerNode->child;
    while (hostNode != NULL) {
        struct HdfHostInfo *hostInfo = HdfHostInfoNewInstance();
        if (hostInfo == NULL) {
            HdfSListFlush(hostList, HdfHostInfoDelete);
            HDF_LOGE("%s: new hostInfo is null", __func__);
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
    if (inHostName == NULL) {
        return NULL;
    }
    hdfManagerNode = GetHdfManagerNode(HcsGetRootNode());
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

static bool CheckDeviceInfo(const struct HdfDeviceInfo *deviceNodeInfo)
{
    if (deviceNodeInfo->policy > SERVICE_POLICY_PRIVATE) {
        HDF_LOGE("%s: policy %u is invalid", __func__, deviceNodeInfo->policy);
        return false;
    }

    if (deviceNodeInfo->priority > MAX_PRIORITY_NUM) {
        HDF_LOGE("%s: priority %u is invalid", __func__, deviceNodeInfo->priority);
        return false;
    }

    if (deviceNodeInfo->preload > DEVICE_PRELOAD_DISABLE) {
        HDF_LOGE("%s: preload %u is invalid", __func__, deviceNodeInfo->preload);
        return false;
    }

    return (strcmp(deviceNodeInfo->moduleName, "") != 0);
}

static bool GetDeviceNodeInfo(const struct DeviceResourceNode *deviceNode, struct HdfDeviceInfo *deviceNodeInfo)
{
    uint16_t readNum = 0;
    const char *readString = NULL;
    if (HcsGetUint16(deviceNode, ATTR_DEV_POLICY, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get policy", __func__);
        return false;
    }
    deviceNodeInfo->policy = readNum;

    if (HcsGetUint16(deviceNode, ATTR_DEV_PRIORITY, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get priority", __func__);
        return false;
    }
    deviceNodeInfo->priority = readNum;

    if (HcsGetUint16(deviceNode, ATTR_DEV_PRELOAD, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get preload", __func__);
        return false;
    }
    deviceNodeInfo->preload = readNum;

    if (HcsGetUint16(deviceNode, ATTR_DEV_PERMISSION, &readNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get permission", __func__);
        return false;
    }
    deviceNodeInfo->permission = readNum;

    if (HcsGetString(deviceNode, ATTR_DEV_MODULENAME, &readString, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get module name", __func__);
        return false;
    }
    deviceNodeInfo->moduleName = readString;

    if (HcsGetString(deviceNode, ATTR_DEV_SVCNAME, &readString, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get service name", __func__);
        return false;
    }
    deviceNodeInfo->svcName = readString;

    if (HcsGetString(deviceNode, ATTR_DEV_MATCHATTR, &readString, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to get matchattr name", __func__);
        return false;
    }
    deviceNodeInfo->deviceMatchAttr = readString;
    return CheckDeviceInfo(deviceNodeInfo);
}

struct HdfSList *HdfAttributeManagerGetDeviceList(uint16_t hostId, const char *hostName)
{
    uint16_t deviceIdx = 0;
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
            struct HdfDeviceInfo *deviceNodeInfo = HdfDeviceInfoNewInstance();
            if (deviceNodeInfo == NULL) {
                HdfSListFlush(deviceList, HdfDeviceInfoDelete);
                OsalMemFree(deviceList);
                return NULL;
            }
            deviceNodeInfo->hostId = hostId;
            if (!GetDeviceNodeInfo(deviceNode, deviceNodeInfo)) {
                HdfDeviceInfoFreeInstance(deviceNodeInfo);
                HDF_LOGE("%s: failed to get device", __func__);
                deviceNodeInfo = NULL;
                deviceNode = deviceNode->sibling;
                continue;
            }
            if (!HdfSListAddOrder(deviceList, &deviceNodeInfo->node, HdfDeviceListCompare)) {
                HDF_LOGE("%s: failed to add device %s", __func__, deviceNodeInfo->svcName);
                HdfDeviceInfoFreeInstance(deviceNodeInfo);
                deviceNodeInfo = NULL;
                deviceNode = deviceNode->sibling;
                continue;
            }
            deviceNodeInfo->deviceId = deviceIdx;
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

bool HdfDeviceListAdd(const char *moduleName, const char *serviceName, const void *privateData)
{
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    if (devMgrSvc == NULL || moduleName == NULL || serviceName == NULL) {
        return false;
    }

    struct HdfDeviceInfo *deviceNodeInfo = HdfDeviceInfoNewInstance();
    if (deviceNodeInfo == NULL) {
        return false;
    }
    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (deviceInfo->moduleName == NULL) {
                continue;
            }
            if (strcmp(deviceInfo->moduleName, moduleName) == 0) {
                deviceInfo->isDynamic = true;
                deviceNodeInfo->hostId = deviceInfo->hostId;
                deviceNodeInfo->deviceId = hostClnt->devCount;
                deviceNodeInfo->policy = deviceInfo->policy;
                deviceNodeInfo->priority = deviceInfo->priority;
                deviceNodeInfo->preload = DEVICE_PRELOAD_DISABLE;
                deviceNodeInfo->permission = deviceInfo->permission;
                deviceNodeInfo->deviceMatchAttr = deviceInfo->deviceMatchAttr;
                deviceNodeInfo->moduleName = deviceInfo->moduleName;
                char *svcName = OsalMemCalloc(strlen(serviceName) + 1);
                if (svcName == NULL) {
                    break;
                }
                if (strcpy_s(svcName, strlen(serviceName) + 1, serviceName) != EOK) {
                    HDF_LOGE("%s: failed to copy string", __func__);
                    OsalMemFree(svcName);
                    break;
                }
                deviceNodeInfo->svcName = svcName;
#ifdef LOSCFG_DRIVERS_HDF_USB_PNP_NOTIFY
                if (!UsbPnpManagerAddPrivateData(deviceNodeInfo, privateData)) {
                    break;
                }
#endif
                HdfSListAdd(hostClnt->deviceInfos, &deviceNodeInfo->node);
                hostClnt->devCount++;
                return true;
            }
        }
    }
    HdfDeviceInfoFreeInstance(deviceNodeInfo);
    return false;
}

void HdfDeviceListDel(const char *moduleName, const char *serviceName)
{
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    if (devMgrSvc == NULL || moduleName == NULL || serviceName == NULL) {
        return;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if ((strcmp(deviceInfo->moduleName, moduleName) == 0) &&
                (strcmp(deviceInfo->svcName, serviceName) == 0)) {
                HdfSListRemove(hostClnt->deviceInfos, &deviceInfo->node);
                HdfDeviceInfoFreeInstance(deviceInfo);
                hostClnt->devCount--;
                return;
            }
        }
    }
}

