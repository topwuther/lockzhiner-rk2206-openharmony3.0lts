/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "usb_ddk_pnp_loader.h"
#include "devhost_service_clnt.h"
#include "device_resource_if.h"
#include "hcs_tree_if.h"
#include "hdf_attribute_manager.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "usb_pnp_manager.h"

#define HDF_LOG_TAG USB_DDK_PNP_LOADER

#define USB_DDK_PNP_CLASS_VENDOR_SPEC   0xFF

static struct DListHead g_usbPnpDeviceTableListHead;
static struct UsbPnpMatchIdTable **g_usbPnpMatchIdTable = NULL;

static struct HdfSBuf *UsbDdkPnpLoaderBufCreate(const char *moduleName,
    const char *serviceName, const char *deviceMatchAttr, struct UsbPnpNotifyServiceInfo serviceInfo)
{
    struct HdfSBuf *pnpData = NULL;

    pnpData = HdfSBufObtainDefaultSize();
    if (pnpData == NULL) {
        HDF_LOGE("%s: HdfSBufTypedObtain pnpData fail", __func__);
        return NULL;
    }

    if (!UsbPnpManagerWriteModuleName(pnpData, moduleName)) {
        HDF_LOGE("%s: write moduleName failed!", __func__);
        goto out;
    }

    if (!HdfSbufWriteString(pnpData, serviceName)) {
        HDF_LOGE("%s: write service name failed!", __func__);
        goto out;
    }

    if (!HdfSbufWriteString(pnpData, deviceMatchAttr)) {
        HDF_LOGE("%s: write deviceMatchAttr failed!", __func__);
        goto out;
    }

    if (!HdfSbufWriteBuffer(pnpData, (const void *)(&serviceInfo), serviceInfo.length)) {
        HDF_LOGE("%s: write privateData failed!", __func__);
        goto out;
    }

    return pnpData;

out:
    HdfSBufRecycle(pnpData);

    return NULL;
}

static bool UsbDdkPnpLoaderMatchDevice(const struct UsbPnpNotifyMatchInfoTable *dev,
    const struct UsbPnpMatchIdTable *id)
{
    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_VENDOR) &&
        (id->vendorId != dev->deviceInfo.vendorId)) {
        return false;
    }

    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_PRODUCT) &&
        (id->productId != dev->deviceInfo.productId)) {
        return false;
    }

    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_DEV_LOW) &&
        (id->bcdDeviceLow > dev->deviceInfo.bcdDeviceLow)) {
        return false;
    }

    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_DEV_HIGH) &&
        (id->bcdDeviceHigh < dev->deviceInfo.bcdDeviceHigh)) {
        return false;
    }

    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_DEV_CLASS) &&
        (id->deviceClass != dev->deviceInfo.deviceClass)) {
        return false;
    }

    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_DEV_SUBCLASS) &&
        (id->deviceSubClass != dev->deviceInfo.deviceSubClass)) {
        return false;
    }

    if ((id->matchFlag & USB_PNP_NOTIFY_MATCH_DEV_PROTOCOL) &&
        (id->deviceProtocol != dev->deviceInfo.deviceProtocol)) {
        return false;
    }

    return true;
}

static void UsbDdkPnpLoaderMatchHandle(const struct UsbPnpNotifyMatchInfoTable *dev,
    int8_t index, struct UsbPnpMatchIdTable *id, bool flag)
{
    if ((id->pnpMatchFlag == false) && (flag == true)) {
        if (!(id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_CLASS)) {
            id->interfaceClass[id->interfaceClassLength++] = dev->interfaceInfo[index].interfaceClass;
        }
        if (!(id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_SUBCLASS)) {
            id->interfaceSubClass[id->interfaceSubClassLength++] = dev->interfaceInfo[index].interfaceSubClass;
        }
        if (!(id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_PROTOCOL)) {
            id->interfaceProtocol[id->interfaceProtocolLength++] = dev->interfaceInfo[index].interfaceProtocol;
        }
        if (!(id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_NUMBER)) {
            id->interfaceNumber[id->interfaceLength++] = dev->interfaceInfo[index].interfaceNumber;
        }
    }
}

static bool UsbDdkPnpLoaderMatchFlag(const struct UsbPnpNotifyMatchInfoTable *dev,
    int8_t index, struct UsbPnpMatchIdTable *id, bool flag)
{
    int32_t i;
    bool ret = true;

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_CLASS) {
        for (i = 0; i < id->interfaceClassLength; i++) {
            if (!((id->interfaceClassMask >> i) & 0x01)) {
                break;
            }
        }
        if (i < id->interfaceClassLength) {
            ret = false;
            goto out;
        }
    }

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_SUBCLASS) {
        for (i = 0; i < id->interfaceSubClassLength; i++) {
            if (!((id->interfaceSubClassMask >> i) & 0x01)) {
                break;
            }
        }
        if (i < id->interfaceSubClassLength) {
            ret = false;
            goto out;
        }
    }

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_PROTOCOL) {
        for (i = 0; i < id->interfaceProtocolLength; i++) {
            if (!((id->interfaceProtocolMask >> i) & 0x01)) {
                break;
            }
        }
        if (i < id->interfaceProtocolLength) {
            ret = false;
            goto out;
        }
    }

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_NUMBER) {
        for (i = 0; i < id->interfaceLength; i++) {
            if (!((id->interfaceMask >> i) & 0x01)) {
                break;
            }
        }
        if (i < id->interfaceLength) {
            ret = false;
            goto out;
        }
    }

    ret = true;

out:
    UsbDdkPnpLoaderMatchHandle(dev, index, id, flag);

    return ret;
}

static bool UsbDdkPnpLoaderMatchInterface(const struct UsbPnpNotifyMatchInfoTable *dev,
    int8_t index, struct UsbPnpMatchIdTable *id)
{
    uint32_t i;
    bool maskFlag = true;

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_CLASS) {
        for (i = 0; i < id->interfaceClassLength; i++) {
            if (id->interfaceClass[i] == dev->interfaceInfo[index].interfaceClass) {
                id->interfaceClassMask |= (1 << i);
                break;
            }
        }

        if (i >= id->interfaceClassLength) {
            maskFlag = false;
        }
    }

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_SUBCLASS) {
        for (i = 0; i < id->interfaceSubClassLength; i++) {
            if (id->interfaceSubClass[i] == dev->interfaceInfo[index].interfaceSubClass) {
                id->interfaceSubClassMask |= (1 << i);
                break;
            }
        }

        if (i >= id->interfaceSubClassLength) {
            maskFlag = false;
        }
    }

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_PROTOCOL) {
        for (i = 0; i < id->interfaceProtocolLength; i++) {
            if (id->interfaceProtocol[i] == dev->interfaceInfo[index].interfaceProtocol) {
                id->interfaceProtocolMask |= (1 << i);
                break;
            }
        }

        if (i >= id->interfaceProtocolLength) {
            maskFlag = false;
        }
    }

    if (id->matchFlag & USB_PNP_NOTIFY_MATCH_INT_NUMBER) {
        for (i = 0; i < id->interfaceLength; i++) {
            if (id->interfaceNumber[i] == dev->interfaceInfo[index].interfaceNumber) {
                id->interfaceMask |= (1 << i);
                break;
            }
        }

        if (i >= id->interfaceLength) {
            maskFlag = false;
        }
    }

    return maskFlag;
}

static bool UsbDdkPnpLoaderMatchOneIdIntf(const struct UsbPnpNotifyMatchInfoTable *dev,
    int8_t index, struct UsbPnpMatchIdTable *id)
{
    bool maskFlag = true;

    if (dev->deviceInfo.deviceClass == USB_DDK_PNP_CLASS_VENDOR_SPEC &&
        !(id->matchFlag & USB_PNP_NOTIFY_MATCH_VENDOR) &&
        (id->matchFlag & (USB_PNP_NOTIFY_MATCH_INT_CLASS | USB_PNP_NOTIFY_MATCH_INT_SUBCLASS |
        USB_PNP_NOTIFY_MATCH_INT_PROTOCOL | USB_PNP_NOTIFY_MATCH_INT_NUMBER))) {
        return false;
    }

    maskFlag = UsbDdkPnpLoaderMatchInterface(dev, index, id);
    if (UsbDdkPnpLoaderMatchFlag(dev, index, id, maskFlag) != true) {
        return false;
    }

    if (id->pnpMatchFlag == false) {
        id->pnpMatchFlag = true;
    } else {
        return false;
    }

    return true;
}

static int32_t UsbDdkPnpLoaderParseIdInfClass(const struct DeviceResourceNode *node,
    const struct DeviceResourceIface *devResIface, struct UsbPnpMatchIdTable *table)
{
    table->interfaceClassMask = 0;
    table->interfaceClassLength = devResIface->GetElemNum(node, "interfaceClass");
    if (table->interfaceClassLength <= 0) {
        HDF_LOGE("%s: read interfaceClass length=%d fail!", __func__, table->interfaceClassLength);
        return HDF_FAILURE;
    }
    if (devResIface->GetUint8Array(node, "interfaceClass", table->interfaceClass, \
        table->interfaceClassLength, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read interfaceClass fail!", __func__);
        return HDF_FAILURE;
    }
    if (!(table->matchFlag & USB_PNP_NOTIFY_MATCH_INT_CLASS)) {
        table->interfaceClassLength = 0;
    }

    table->interfaceSubClassMask = 0;
    table->interfaceSubClassLength = devResIface->GetElemNum(node, "interfaceSubClass");
    if (table->interfaceSubClassLength <= 0) {
        HDF_LOGE("%s: read interfaceSubClass length=%d fail!",
            __func__, table->interfaceSubClassLength);
        return HDF_FAILURE;
    }
    if (devResIface->GetUint8Array(node, "interfaceSubClass", table->interfaceSubClass, \
        table->interfaceSubClassLength, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read interfaceSubClass fail!", __func__);
        return HDF_FAILURE;
    }
    if (!(table->matchFlag & USB_PNP_NOTIFY_MATCH_INT_SUBCLASS)) {
        table->interfaceSubClassLength = 0;
    }

    return HDF_SUCCESS;
}


static int32_t UsbDdkPnpLoaderParseIdInferface(const struct DeviceResourceNode *node,
    const struct DeviceResourceIface *devResIface, struct UsbPnpMatchIdTable *table)
{
    if (UsbDdkPnpLoaderParseIdInfClass(node, devResIface, table) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    table->interfaceProtocolMask = 0;
    table->interfaceProtocolLength = devResIface->GetElemNum(node, "interfaceProtocol");
    if (table->interfaceProtocolLength <= 0) {
        HDF_LOGE("%s: read interfaceProtocol length=%d fail!",
            __func__, table->interfaceProtocolLength);
        return HDF_FAILURE;
    }
    if (devResIface->GetUint8Array(node, "interfaceProtocol", table->interfaceProtocol, \
        table->interfaceProtocolLength, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read interfaceProtocol fail!", __func__);
        return HDF_FAILURE;
    }
    if (!(table->matchFlag & USB_PNP_NOTIFY_MATCH_INT_PROTOCOL)) {
        table->interfaceProtocolLength = 0;
    }

    table->interfaceMask = 0;
    table->interfaceLength = devResIface->GetElemNum(node, "interfaceNumber");
    if (table->interfaceLength <= 0) {
        HDF_LOGE("%s: read interfaceNumber length=%d fail!", __func__, table->interfaceLength);
        return HDF_FAILURE;
    }
    if (devResIface->GetUint8Array(node, "interfaceNumber", table->interfaceNumber, \
        table->interfaceLength, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read interfaceNumber fail!", __func__);
        return HDF_FAILURE;
    }
    if (!(table->matchFlag & USB_PNP_NOTIFY_MATCH_INT_NUMBER)) {
        table->interfaceLength = 0;
    }

    return HDF_SUCCESS;
}

static int32_t UsbDdkPnpLoaderParseIdDevice(const struct DeviceResourceNode *node,
    const struct DeviceResourceIface *devResIface, struct UsbPnpMatchIdTable *table)
{
    if (devResIface->GetUint16(node, "vendorId", &table->vendorId, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read vendorId fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint16(node, "productId", &table->productId, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read productId fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint16(node, "bcdDeviceLow", &table->bcdDeviceLow, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read bcdDeviceLow fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint16(node, "bcdDeviceHigh", &table->bcdDeviceHigh, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read bcdDeviceHigh fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint8(node, "deviceClass", &table->deviceClass, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read deviceClass fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint8(node, "deviceSubClass", &table->deviceSubClass, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read deviceSubClass fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint8(node, "deviceProtocol", &table->deviceProtocol, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read deviceProtocol fail!", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t UsbDdkPnpLoaderParseIdTable(const struct DeviceResourceNode *node,
    const struct DeviceResourceIface *devResIface, struct UsbPnpMatchIdTable *table)
{
    if (node == NULL || table == NULL || devResIface == NULL) {
        HDF_LOGE("%s: node or table or devResIface is NULL!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetString(node, "moduleName", &table->moduleName, "") != HDF_SUCCESS) {
        HDF_LOGE("%s: read moduleName fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetString(node, "serviceName", &table->serviceName, "") != HDF_SUCCESS) {
        HDF_LOGE("%s: read serviceName fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetString(node, "deviceMatchAttr", &table->deviceMatchAttr, "") != HDF_SUCCESS) {
        HDF_LOGE("%s: read deviceMatchAttr fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint8(node, "length", &table->length, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read length fail!", __func__);
        return HDF_FAILURE;
    }

    if (devResIface->GetUint16(node, "matchFlag", &table->matchFlag, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read matchFlag fail!", __func__);
        return HDF_FAILURE;
    }

    if (UsbDdkPnpLoaderParseIdDevice(node, devResIface, table) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return UsbDdkPnpLoaderParseIdInferface(node, devResIface, table);
}

static struct UsbPnpMatchIdTable **UsbDdkPnpLoaderParseTableList(
    const struct DeviceResourceNode *node, int32_t idTabCount, const struct DeviceResourceIface *devResIface)
{
    int32_t ret;
    int32_t count;
    const char *idTableName = NULL;
    struct UsbPnpMatchIdTable **idTable = NULL;
    const struct DeviceResourceNode *tableNode = NULL;

    idTable = (struct UsbPnpMatchIdTable **)OsalMemCalloc((idTabCount + 1) * sizeof(struct UsbPnpMatchIdTable *));
    if (idTable == NULL) {
        HDF_LOGE("%s: OsalMemCalloc failure!", __func__);
        return NULL;
    }
    idTable[idTabCount] = NULL;
    for (count = 0; count < idTabCount; count++) {
        idTable[count] = (struct UsbPnpMatchIdTable *)OsalMemCalloc(sizeof(struct UsbPnpMatchIdTable));
        if (idTable[count] == NULL) {
            HDF_LOGE("%s: OsalMemCalloc failure!", __func__);
            goto out;
        }
        ret = devResIface->GetStringArrayElem(node, "idTableList", count, &idTableName, NULL);
        if (ret != HDF_SUCCESS) {
            goto out;
        }
        tableNode = devResIface->GetChildNode(node, idTableName);
        if (tableNode == NULL) {
            HDF_LOGE("%s: tableNode is NULL!", __func__);
            goto out;
        }
        if (UsbDdkPnpLoaderParseIdTable(tableNode, devResIface, idTable[count]) != HDF_SUCCESS) {
            HDF_LOGE("%s: UsbDdkPnpLoaderParseIdTable failure!", __func__);
            goto out;
        }
    }

    return idTable;

out:
    while ((--count) >= 0) {
        OsalMemFree(idTable[count]);
    }
    OsalMemFree(idTable);

    return NULL;
}

static struct UsbPnpMatchIdTable **UsbDdkPnpLoaderParseTable(const struct DeviceResourceNode *node)
{
    struct DeviceResourceIface *devResIface = NULL;
    int32_t idTabCount;

    if (node == NULL) {
        HDF_LOGE("%s: node is NULL!", __func__);
        return NULL;
    }

    devResIface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (devResIface == NULL) {
        HDF_LOGE("%s: devResIface is NULL!", __func__);
        return NULL;
    }
    idTabCount = devResIface->GetElemNum(node, "idTableList");
    if (idTabCount <= 0) {
        HDF_LOGE("%s: idTableList not found!", __func__);
        return NULL;
    }

    return UsbDdkPnpLoaderParseTableList(node, idTabCount, devResIface);
}

static struct UsbPnpMatchIdTable **UsbDdkPnpLoaderParseDeviceId(const struct DeviceResourceNode *node)
{
    const char *deviceIdName = NULL;
    struct DeviceResourceIface *devResIface = NULL;
    const struct DeviceResourceNode *deviceIdNode = NULL;

    if (node == NULL) {
        HDF_LOGE("%s: node is NULL!", __func__);
        return NULL;
    }

    devResIface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (devResIface == NULL) {
        HDF_LOGE("%s: devResIface is NULL!", __func__);
        return NULL;
    }

    if (devResIface->GetString(node, "usb_pnp_device_id", &deviceIdName, NULL) != HDF_SUCCESS) {
        HDF_LOGE("%s: get usb_pnp_device_id name failure!", __func__);
        return NULL;
    }

    deviceIdNode = devResIface->GetChildNode(node, deviceIdName);
    if (deviceIdNode == NULL) {
        HDF_LOGE("%s: deviceIdNode is NULL!", __func__);
        return NULL;
    }

    return UsbDdkPnpLoaderParseTable(deviceIdNode);
}

static struct UsbPnpMatchIdTable **UsbDdkPnpLoaderPnpMatch(void)
{
    struct DeviceResourceIface *devResInstance = NULL;
    const struct DeviceResourceNode *rootNode = NULL;
    const struct DeviceResourceNode *usbPnpNode = NULL;

    devResInstance = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (devResInstance == NULL) {
        HDF_LOGE("%s: devResInstance is NULL!", __func__);
        return NULL;
    }

    rootNode = devResInstance->GetRootNode();
    if (rootNode == NULL) {
        HDF_LOGE("%s: devResNode is NULL!", __func__);
        return NULL;
    }

    usbPnpNode = devResInstance->GetNodeByMatchAttr(rootNode, "usb_pnp_match");
    if (usbPnpNode == NULL) {
        HDF_LOGE("%s: usbPnpNode is NULL!", __func__);
        return NULL;
    }

    return UsbDdkPnpLoaderParseDeviceId(usbPnpNode);
}

static int32_t UsbDdkPnpLoaderDispatchPnpDevice(
    const struct IDevmgrService *devmgrSvc, struct HdfSBuf *data, bool isReg)
{
    uint32_t infoSize = 0;
    struct UsbPnpNotifyServiceInfo *privateData = NULL;
    struct UsbPnpManagerDeviceInfo managerInfo;

    const char *moduleName = HdfSbufReadString(data);
    if (moduleName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    const char *serviceName = HdfSbufReadString(data);
    if (serviceName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    const char *deviceMatchAttr = HdfSbufReadString(data);
    if (deviceMatchAttr == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadBuffer(data, (const void **)(&privateData), &infoSize)) {
        HDF_LOGW("%s: HdfSbufReadBuffer privateData error!", __func__);
        privateData = NULL;
    }

    managerInfo.devmgrSvc = (struct IDevmgrService *)devmgrSvc;
    managerInfo.moduleName = moduleName;
    managerInfo.serviceName = serviceName;
    managerInfo.deviceMatchAttr = deviceMatchAttr;
    managerInfo.privateData = privateData;
    managerInfo.isReg = isReg;

    return UsbPnpManagerRegisterOrUnregisterDevice(managerInfo);
}

static int UsbDdkPnpLoaderDeviceListAdd(const struct UsbPnpNotifyMatchInfoTable *info,
    const struct UsbPnpMatchIdTable *idTable)
{
    int ret;
    unsigned char *ptr = NULL;
    struct UsbPnpDeviceListTable *deviceTableListTemp = NULL;

    ptr = OsalMemAlloc(sizeof(struct UsbPnpDeviceListTable));
    if (ptr == NULL) {
        ret = HDF_ERR_MALLOC_FAIL;
        HDF_LOGE("%s:%d OsalMemAlloc faile, ret=%d ", __func__, __LINE__, ret);
    } else {
        deviceTableListTemp = (struct UsbPnpDeviceListTable *)ptr;

        DListHeadInit(&deviceTableListTemp->list);
        deviceTableListTemp->moduleName = idTable->moduleName;
        deviceTableListTemp->serviceName = idTable->serviceName;
        deviceTableListTemp->deviceMatchAttr = idTable->deviceMatchAttr;
        deviceTableListTemp->status = USB_PNP_ADD_STATUS;
        deviceTableListTemp->usbDevAddr = info->usbDevAddr;
        deviceTableListTemp->devNum = info->devNum;
        deviceTableListTemp->busNum = info->busNum;
        deviceTableListTemp->interfaceLength = idTable->interfaceLength;
        memcpy_s(deviceTableListTemp->interfaceNumber, USB_PNP_INFO_MAX_INTERFACES, \
            idTable->interfaceNumber, USB_PNP_INFO_MAX_INTERFACES);

        DListInsertTail(&deviceTableListTemp->list, &g_usbPnpDeviceTableListHead);

        ret = HDF_SUCCESS;
    }

    return ret;
}

static struct UsbPnpDeviceListTable *UsbDdkPnpLoaderAddInterface(
    const struct UsbPnpNotifyMatchInfoTable *info, const struct UsbPnpMatchIdTable *idTable)
{
    struct UsbPnpDeviceListTable *deviceListTablePos = NULL;
    struct UsbPnpDeviceListTable *deviceListTableTemp = NULL;

    if (DListIsEmpty(&g_usbPnpDeviceTableListHead)) {
        HDF_LOGE("%s:%d g_usbPnpDeviceTableListHead is empty. ", __func__, __LINE__);
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(deviceListTablePos, deviceListTableTemp, &g_usbPnpDeviceTableListHead,
        struct UsbPnpDeviceListTable, list) {
        if ((strcmp(deviceListTablePos->moduleName, idTable->moduleName) == 0) && \
            (strcmp(deviceListTablePos->serviceName, idTable->serviceName) == 0) && \
            (strcmp(deviceListTablePos->deviceMatchAttr, idTable->deviceMatchAttr) == 0) && \
            (deviceListTablePos->usbDevAddr == info->usbDevAddr) && \
            (deviceListTablePos->devNum = info->devNum) && \
            (deviceListTablePos->busNum = info->busNum)) {
            return deviceListTablePos;
        }
    }

    HDF_LOGE("%s:%d usbDevAddr=0x%x, interface=%d-%d-%d to \
        be add but not exist. ",
        __func__, __LINE__, (uint32_t)info->usbDevAddr, info->devNum, info->busNum, info->numInfos);

    return NULL;
}

static int UsbDdkPnpLoaderrAddPnpDevice(const struct IDevmgrService *devmgrSvc,
    const struct UsbPnpNotifyMatchInfoTable *infoTable, const struct UsbPnpMatchIdTable *idTable, uint32_t cmdId)
{
    int ret;
    struct HdfSBuf *pnpData = NULL;
    struct UsbPnpNotifyServiceInfo serviceInfo;
    struct UsbPnpDeviceListTable *deviceListTable = NULL;

    deviceListTable = UsbDdkPnpLoaderAddInterface(infoTable, idTable);
    if ((deviceListTable != NULL) && (deviceListTable->status != USB_PNP_REMOVE_STATUS)) {
        HDF_LOGI("%s:%d %s-%s is already exist!",
            __func__, __LINE__, idTable->moduleName, idTable->serviceName);
        return HDF_SUCCESS;
    }

    serviceInfo.length = sizeof(struct UsbPnpNotifyServiceInfo) - (USB_PNP_INFO_MAX_INTERFACES
        - idTable->interfaceLength);
    serviceInfo.devNum = infoTable->devNum;
    serviceInfo.busNum = infoTable->busNum;
    serviceInfo.interfaceLength = idTable->interfaceLength;
    memcpy_s(serviceInfo.interfaceNumber, USB_PNP_INFO_MAX_INTERFACES, \
        idTable->interfaceNumber, USB_PNP_INFO_MAX_INTERFACES);
    pnpData = UsbDdkPnpLoaderBufCreate(idTable->moduleName, idTable->serviceName,
        idTable->deviceMatchAttr, serviceInfo);
    if (pnpData == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("%s: UsbDdkPnpLoaderBufCreate faile", __func__);
        goto error;
    }

    ret = UsbDdkPnpLoaderDispatchPnpDevice(devmgrSvc, pnpData, true);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d handle failed, %s-%s cmdId is %d, ret=%d",
            __func__, __LINE__, idTable->moduleName, idTable->serviceName, cmdId, ret);
    } else {
        if (cmdId == USB_PNP_NOTIFY_ADD_INTERFACE) {
            if (deviceListTable == NULL) {
                ret = HDF_ERR_INVALID_OBJECT;
                HDF_LOGE("%s:%d UsbDdkPnpLoaderAddInterface faile", __func__, __LINE__);
                goto error;
            }
            deviceListTable->status = USB_PNP_ADD_STATUS;
        } else {
            ret = UsbDdkPnpLoaderDeviceListAdd(infoTable, idTable);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("%s:%d UsbDdkPnpLoaderDeviceListAdd faile", __func__, __LINE__);
                goto error;
            }
        }
    }
error:
    HdfSBufRecycle(pnpData);
    return ret;
}

static void UsbDdkPnpLoaderAddDevice(uint32_t cmdId, uint8_t index, const struct IDevmgrService *devmgrSvc,
    const struct UsbPnpNotifyMatchInfoTable *infoTable, struct UsbPnpMatchIdTable **matchIdTable)
{
    int ret = HDF_FAILURE;
    struct UsbPnpMatchIdTable *idTable = NULL;
    int32_t tableCount;

    for (tableCount = 0, idTable = matchIdTable[0]; idTable != NULL; idTable = matchIdTable[++tableCount]) {
        if (!UsbDdkPnpLoaderMatchDevice(infoTable, idTable)) {
            continue;
        }

        if (!UsbDdkPnpLoaderMatchOneIdIntf(infoTable, index, idTable)) {
            continue;
        }

        HDF_LOGD("%s:%d matchDevice end, index=%d tableCount=%d is match \
            idTable=%p, moduleName=%s, serviceName=%s",
            __func__, __LINE__, index, tableCount, idTable, idTable->moduleName, idTable->serviceName);

        ret = UsbDdkPnpLoaderrAddPnpDevice(devmgrSvc, infoTable, idTable, cmdId);
        if (ret != HDF_SUCCESS) {
            continue;
        }
    }

    HDF_LOGD("%s:%d AddDevice end, index=%d, ret=%d", __func__, __LINE__, index, ret);
}

static int UsbDdkPnpLoaderRemoveHandle(const struct IDevmgrService *devmgrSvc,
    struct UsbPnpDeviceListTable *deviceListTablePos)
{
    struct UsbPnpNotifyServiceInfo serviceInfo;
    struct HdfSBuf *pnpData = NULL;
    int ret = HDF_SUCCESS;

    serviceInfo.length = sizeof(struct UsbPnpNotifyServiceInfo) - (USB_PNP_INFO_MAX_INTERFACES \
        - deviceListTablePos->interfaceLength);
    serviceInfo.devNum = deviceListTablePos->devNum;
    serviceInfo.busNum = deviceListTablePos->busNum;
    serviceInfo.interfaceLength = deviceListTablePos->interfaceLength;
    memcpy_s(serviceInfo.interfaceNumber, USB_PNP_INFO_MAX_INTERFACES, \
        deviceListTablePos->interfaceNumber, USB_PNP_INFO_MAX_INTERFACES);
    pnpData = UsbDdkPnpLoaderBufCreate(deviceListTablePos->moduleName, deviceListTablePos->serviceName, \
        deviceListTablePos->deviceMatchAttr, serviceInfo);
    if (pnpData == NULL) {
        HDF_LOGE("%s: UsbDdkPnpLoaderBufCreate faile", __func__);
        return HDF_FAILURE;
    }

    if (deviceListTablePos->status != USB_PNP_REMOVE_STATUS) {
        ret = UsbDdkPnpLoaderDispatchPnpDevice(devmgrSvc, pnpData, false);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s:%d UsbDdkPnpLoaderDispatchPnpDevice faile ret=%d",
                __func__, __LINE__, ret);
            goto error;
        }
        deviceListTablePos->status = USB_PNP_REMOVE_STATUS;
    }

error:
    HdfSBufRecycle(pnpData);
    return ret;
}

static int UsbDdkPnpLoaderRemoveDevice(const struct IDevmgrService *devmgrSvc,
    struct UsbPnpRemoveInfo removeInfo, uint32_t cmdId)
{
    int ret = HDF_SUCCESS;
    struct UsbPnpDeviceListTable *deviceListTablePos = NULL;
    struct UsbPnpDeviceListTable *deviceListTableTemp = NULL;
    bool findFlag = false;
    int32_t i;

    if (DListIsEmpty(&g_usbPnpDeviceTableListHead)) {
        HDF_LOGE("%s:%d g_usbPnpDeviceTableListHead is empty. ", __func__, __LINE__);
        return HDF_SUCCESS;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(deviceListTablePos, deviceListTableTemp, &g_usbPnpDeviceTableListHead,
        struct UsbPnpDeviceListTable, list) {
        if (deviceListTablePos->usbDevAddr == removeInfo.usbDevAddr) {
            if (removeInfo.removeType == USB_PNP_NOTIFY_REMOVE_INTERFACE_NUM) {
                for (i = 0; i < deviceListTablePos->interfaceLength; i++) {
                    if (deviceListTablePos->interfaceNumber[i] == removeInfo.interfaceNum) {
                        break;
                    }
                }

                if (i >= deviceListTablePos->interfaceLength) {
                    continue;
                }
            }
            findFlag = true;

            ret = UsbDdkPnpLoaderRemoveHandle(devmgrSvc, deviceListTablePos);
            if (ret != HDF_SUCCESS) {
                break;
            }

            if (cmdId != USB_PNP_NOTIFY_REMOVE_INTERFACE) {
                DListRemove(&deviceListTablePos->list);
                OsalMemFree(deviceListTablePos);
                deviceListTablePos = NULL;
            }
        }
    }

    if (findFlag == false) {
        HDF_LOGE("%s:%d removeType=%d, usbDevAddr=0x%x, to be remove but not exist.",
            __func__, __LINE__, removeInfo.removeType, (uint32_t)removeInfo.usbDevAddr);
        ret = HDF_FAILURE;
    }

    return ret;
}

static int UsbDdkPnpLoaderDevice(const struct UsbPnpNotifyMatchInfoTable *infoTable,
    const struct IDevmgrService *super, uint32_t id)
{
    int8_t i;
    int32_t tableCount;
    struct UsbPnpMatchIdTable *idTable = NULL;

    if ((infoTable == NULL) || (g_usbPnpMatchIdTable == NULL) || (g_usbPnpMatchIdTable[0] == NULL)) {
        HDF_LOGE("%s:%d infoTable or super or g_usbPnpMatchIdTable is NULL!", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (i = 0; i < infoTable->numInfos; i++) {
        UsbDdkPnpLoaderAddDevice(id, i, super, infoTable, g_usbPnpMatchIdTable);
    }

    for (tableCount = 0, idTable = g_usbPnpMatchIdTable[0]; idTable != NULL;
        idTable = g_usbPnpMatchIdTable[++tableCount]) {
        idTable->interfaceClassMask = 0;
        if (!(idTable->matchFlag & USB_PNP_NOTIFY_MATCH_INT_CLASS)) {
            idTable->interfaceClassLength = 0;
        }
        idTable->interfaceSubClassMask = 0;
        if (!(idTable->matchFlag & USB_PNP_NOTIFY_MATCH_INT_SUBCLASS)) {
            idTable->interfaceSubClassLength = 0;
        }
        idTable->interfaceProtocolMask = 0;
        if (!(idTable->matchFlag & USB_PNP_NOTIFY_MATCH_INT_PROTOCOL)) {
            idTable->interfaceProtocolLength = 0;
        }
        idTable->interfaceMask = 0;
        if (!(idTable->matchFlag & USB_PNP_NOTIFY_MATCH_INT_NUMBER)) {
            idTable->interfaceLength = 0;
        }
        idTable->pnpMatchFlag = false;
    }

    return HDF_SUCCESS;
}

static int UsbDdkPnpLoaderEventSend(const struct HdfIoService *serv, const char *eventData)
{
    int ret;
    int replyData = 0;
    struct HdfSBuf *data = NULL;

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        ret = HDF_DEV_ERR_NO_MEMORY;
        HDF_LOGE("%s: fail to obtain sbuf data", __func__);
        return ret;
    }

    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (reply == NULL) {
        ret = HDF_DEV_ERR_NO_MEMORY;
        HDF_LOGE("%s: fail to obtain sbuf reply", __func__);
        goto out;
    }

    if (!HdfSbufWriteString(data, eventData)) {
        ret = HDF_FAILURE;
        HDF_LOGE("%s: fail to write sbuf", __func__);
        goto out;
    }

    ret = serv->dispatcher->Dispatch((struct HdfObject *)&serv->object, USB_PNP_NOTIFY_REPORT_INTERFACE, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: fail to send serivice call, ret=%d", __func__, ret);
        goto out;
    }

    if (!HdfSbufReadInt32(reply, &replyData)) {
        ret = HDF_ERR_INVALID_OBJECT;
        HDF_LOGE("%s: fail to get service call reply", __func__);
        goto out;
    }

    HDF_LOGI("%s:%d get reply is 0x%x", __func__, __LINE__, replyData);

out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);

    return ret;
}

int UsbDdkPnpLoaderEventReceived(void *priv, uint32_t id, struct HdfSBuf *data)
{
    int ret;
    bool flag = false;
    uint32_t infoSize;
    struct UsbPnpNotifyMatchInfoTable *infoTable = NULL;
    struct IDevmgrService *super = (struct IDevmgrService *)priv;
    struct UsbPnpRemoveInfo removeInfo;

    flag = HdfSbufReadBuffer(data, (const void **)(&infoTable), &infoSize);
    if ((flag == false) || (infoTable == NULL)) {
        ret = HDF_ERR_INVALID_PARAM;
        HDF_LOGE("%s: fail to read infoTable in event data, flag=%d, infoTable=%p", \
            __func__, flag, infoTable);
        return ret;
    }

    HDF_LOGI("%s:%d id=%d infoSize=%d, usbDevAddr=0x%x, devNum=%d, \
        busNum=%d, infoTable=0x%x-0x%x success",
        __func__, __LINE__, id, infoSize, (uint32_t)infoTable->usbDevAddr, infoTable->devNum,
        infoTable->busNum, infoTable->deviceInfo.vendorId, infoTable->deviceInfo.productId);

    switch (id) {
        case USB_PNP_NOTIFY_ADD_INTERFACE:
        case USB_PNP_NOTIFY_ADD_DEVICE:
        case USB_PNP_NOTIFY_REPORT_INTERFACE:
#if USB_PNP_NOTIFY_TEST_MODE == true
        case USB_PNP_NOTIFY_ADD_TEST:
#endif
            ret = UsbDdkPnpLoaderDevice(infoTable, super, id);
            break;
        case USB_PNP_NOTIFY_REMOVE_INTERFACE:
        case USB_PNP_NOTIFY_REMOVE_DEVICE:
#if USB_PNP_NOTIFY_TEST_MODE == true
        case USB_PNP_NOTIFY_REMOVE_TEST:
#endif
            removeInfo.removeType = infoTable->removeType;
            removeInfo.usbDevAddr = infoTable->usbDevAddr;
            removeInfo.devNum = infoTable->devNum;
            removeInfo.busNum = infoTable->busNum;
            removeInfo.interfaceNum = infoTable->interfaceInfo[0].interfaceNumber;
            ret = UsbDdkPnpLoaderRemoveDevice(super, removeInfo, id);
            break;
        default:
            ret = HDF_ERR_INVALID_PARAM;
            break;
    }

    HDF_LOGI("%s:%d ret=%d DONE", __func__, __LINE__, ret);

    return ret;
}

int UsbDdkPnpLoaderEventHandle(void)
{
    int status;
    int32_t tableCount = 0;
    static bool firstInitFlag = true;
    const struct UsbPnpMatchIdTable *idTable = NULL;
    struct HdfIoService *usbPnpServ = HdfIoServiceBind(USB_PNP_NOTIFY_SERVICE_NAME);

    if (usbPnpServ == NULL) {
        HDF_LOGE("%s: HdfIoServiceBind faile.", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (firstInitFlag == true) {
        firstInitFlag = false;

        DListHeadInit(&g_usbPnpDeviceTableListHead);
    }

    g_usbPnpMatchIdTable = UsbDdkPnpLoaderPnpMatch();
    if ((g_usbPnpMatchIdTable == NULL) || (g_usbPnpMatchIdTable[0] == NULL)) {
        status = HDF_ERR_INVALID_PARAM;
        HDF_LOGE("%s: g_usbPnpMatchIdTable or g_usbPnpMatchIdTable[0] is NULL!", __func__);
        return status;
    }

    status = UsbDdkPnpLoaderEventSend(usbPnpServ, "USB PNP Handle Info");
    if (status != HDF_SUCCESS) {
        HDF_LOGE("UsbDdkPnpLoaderEventSend faile status=%d", status);
        goto error;
    }
    return status;
error:
    for (idTable = g_usbPnpMatchIdTable[0]; idTable != NULL; ) {
        tableCount++;
        idTable = g_usbPnpMatchIdTable[tableCount];
    }
    while ((--tableCount) >= 0) {
        OsalMemFree(g_usbPnpMatchIdTable[tableCount]);
        g_usbPnpMatchIdTable[tableCount] = NULL;
    }
    OsalMemFree(g_usbPnpMatchIdTable);
    g_usbPnpMatchIdTable = NULL;

    return status;
}
