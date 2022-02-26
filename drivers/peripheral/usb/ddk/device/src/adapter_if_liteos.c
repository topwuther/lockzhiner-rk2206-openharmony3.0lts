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

#include "adapter_if.h"
#include <dirent.h>
#include <endian.h>
#include <fcntl.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "usb_handle.h"

#define HDF_LOG_TAG adapter_if
#define SLEEP_TIME 100000
#define OPEN_CNT 30

static struct RawUsbRamTestList *g_usbRamTestHead = NULL;
static bool g_usbRamTestFlag = false;

static int UsbFnAdapterOpenFn()
{
    int i;
    int ep;
    for (i = 0; i < OPEN_CNT; i++) {
        ep = handle_open("/dev/fconfig");
        if (ep > 0) {
            break;
        }
        usleep(SLEEP_TIME);
    }
    if (ep < 0) {
        HDF_LOGE("func not alloc!");
    }
    return ep;
}

static int UsbFnAdapterClosefn(int fd)
{
    if (fd < 0) {
        return HDF_ERR_INVALID_PARAM;
    }
    return handle_close(fd);
}

static int UsbFnAdapterCreateFconfigString(struct FconfigString *configString,
    const char *name)
{
    int strLen;
    int ret;

    if (configString == NULL || name == NULL) {
        HDF_LOGE("%s: configName is NULL", __func__);
        return HDF_ERR_IO;
    }
    strLen = strlen(name);
    configString->len = strLen;
    configString->s = UsbFnMemCalloc(strLen + 1);
    if (configString->s == NULL) {
        HDF_LOGE("%s: UsbFnMemCalloc failure!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    ret = memcpy_s(configString->s, (strLen + 1), name, strLen);
    if (ret) {
        HDF_LOGE("%s: memcpy_s failure!", __func__);
        UsbFnMemFree(configString->s);
        return HDF_ERR_MALLOC_FAIL;
    }
    *(configString->s + configString->len) = '\0';
    return 0;
}

static int UsbFnAdapterWriteGadget(int fd, int cmd,
    struct FconfigString *gadgetName)
{
    int ret;

    if (gadgetName == NULL) {
        HDF_LOGE("%s: udcName is NULL", __func__);
        return HDF_ERR_IO;
    }
    ret = handle_ioctl(fd, cmd, gadgetName);
    if (ret) {
        HDF_LOGE("%s: ioctl failure!", __func__);
        return HDF_ERR_IO;
    }
    return 0;
}

static int UsbFnAdapterWriteDevDesc(int fd, struct FconfigString *gadgetName,
    const struct UsbFnDeviceDesc *descriptor)
{
    int ret;
    struct FconfigDevDesc devDesc;

    if (gadgetName == NULL || descriptor == NULL) {
        HDF_LOGE("%s: udcName is NULL", __func__);
        return HDF_ERR_IO;
    }
    devDesc.gadgetName.len = gadgetName->len;
    devDesc.gadgetName.s = gadgetName->s;
    ret = memcpy_s(&devDesc.devDesc, sizeof(devDesc.devDesc), descriptor->deviceDesc,
        sizeof(devDesc.devDesc));
    if (ret) {
        HDF_LOGE("%s: memcpy_s failure!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    ret = handle_ioctl(fd, FCONFIG_CMD_WRITE_DEV_DESC, &devDesc);
    if (ret) {
        HDF_LOGE("%s: ioctl failure!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    return 0;
}

static int UsbFnAdapterWriteDevString(int fd, struct FconfigDevStrings *devStrings,
    const struct UsbFnStrings *usbFnString)
{
    struct UsbString *usbString = NULL;
    int jCount;
    int ret;

    devStrings->language = usbFnString->language;
    devStrings->strCount = 0;
    usbString = usbFnString->strings;
    while (usbString->s){
        devStrings->strCount++;
        usbString++;
    }
    devStrings->strings = UsbFnMemCalloc((devStrings->strCount + 1)* sizeof(struct FconfigUsbString));
    if (devStrings->strings == NULL) {
        HDF_LOGE("%s: UsbFnMemCalloc failure!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    devStrings->strings[devStrings->strCount].str.len = 0;
    devStrings->strings[devStrings->strCount].str.s = NULL;
    usbString = usbFnString->strings;
    for (jCount = 0; jCount < (int)devStrings->strCount; jCount++) {
        devStrings->strings[jCount].id = usbString[jCount].id;
        ret = UsbFnAdapterCreateFconfigString(&devStrings->strings[jCount].str, usbString[jCount].s);
        if (ret) {
            HDF_LOGE("%s: create string failure!", __func__);
            UsbFnMemFree(devStrings->strings[jCount].str.s);
            goto FAIL;
        }
    }
    ret=handle_ioctl(fd, FCONFIG_CMD_WRITE_STRINGS, devStrings);
    if (ret) {
        HDF_LOGE("%s: ioctl failure!", __func__);
        goto FAIL;
    }
    for (jCount = 0; jCount < (int)devStrings->strCount; jCount++) {
        UsbFnMemFree(devStrings->strings[jCount].str.s);
    }
    UsbFnMemFree(devStrings->strings);
    return 0;
FAIL:
    while ((--jCount) >= 0) {
        UsbFnMemFree(devStrings->strings[jCount].str.s);
    }
    UsbFnMemFree(devStrings->strings);
    return -1;
}

static int UsbFnAdapterWriteDevStrings(int fd, struct FconfigString *gadgetName,
    const struct UsbFnDeviceDesc *descriptor)
{
    int ret;
    int iCount;
    struct FconfigDevStrings devStrings;

    if (gadgetName == NULL || descriptor == NULL){
        HDF_LOGE("%s: udcName is NULL", __func__);
        return HDF_ERR_IO;
    }
    devStrings.gadgetName.len = gadgetName->len;
    devStrings.gadgetName.s = gadgetName->s;
    for (iCount = 0; descriptor->deviceStrings[iCount]; iCount++) {
        ret = UsbFnAdapterWriteDevString(fd, &devStrings, descriptor->deviceStrings[iCount]);
        if (ret != 0) {
            HDF_LOGE("%s: UsbFnAdapterWriteDevString fail", __func__);
            return HDF_ERR_IO;
        }
    }
    return 0;
}

static int UsbFnAdapterFillConfigDesc(struct UsbConfigDescriptor *cfgDesc,
    struct UsbFnConfiguration *UsbFnConfig)
{
    if (cfgDesc == NULL || UsbFnConfig == NULL) {
        HDF_LOGE("%s: name is NULL", __func__);
        return HDF_ERR_IO;
    }
    cfgDesc->bConfigurationValue = UsbFnConfig->configurationValue;
    cfgDesc->bmAttributes = UsbFnConfig->attributes;
    cfgDesc->bMaxPower = UsbFnConfig->maxPower;
    cfgDesc->iConfiguration = UsbFnConfig->iConfiguration;
    return 0;
}

static int UsbFnAdapterOpenPipe(const char *funcName, int epIndex)
{
    int ret;
    char epName[MAX_NAMELEN];
    int i;
    int ep;
    if (funcName == NULL || epIndex < 0) {
        return HDF_ERR_INVALID_PARAM;
    }

    ret = snprintf_s(epName, MAX_NAMELEN, MAX_NAMELEN - 1, "/dev/%s/ep%u", funcName, epIndex);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        return HDF_ERR_IO;
    }

    for (i = 0; i < OPEN_CNT; i++) {
        ep = handle_open(&epName[0]);
        if (ep > 0) {
            break;
        }
        usleep(SLEEP_TIME);
    }
    if (ep < 0) {
        HDF_LOGE("unable to open %s", epName);
        return HDF_ERR_IO;
    }
    return ep;
}

static int UsbFnAdapterClosePipe(int ep)
{
    if (ep < 0) {
        return HDF_ERR_INVALID_PARAM;
    }

    return handle_close(ep);
}

static void GetHeaderStr(struct UsbFnStrings **strings,
    struct UsbFunctionfsStringsHead *headerStr)
{
    uint32_t i, j;
    uint32_t langCount = 0;
    uint32_t strCount;
    uint32_t len = 0;
    for (i = 0; strings[i] != NULL; i++) {
        langCount++;
        for (j = 0; strings[i]->strings[j].s; j++) {
            len += strlen(strings[i]->strings[j].s) + sizeof(char);
        }
        strCount = j;
    }
    headerStr->magic = htole32(FUNCTIONFS_STRINGS_MAGIC);
    headerStr->length = htole32(sizeof(struct UsbFunctionfsStringsHead) +
        langCount * sizeof(uint16_t) + len);
    headerStr->str_count = strCount;
    headerStr->lang_count = langCount;
}

static int UsbFnWriteStrings(int ep0, struct UsbFnStrings **strings)
{
    uint8_t *str = NULL;
    uint8_t *whereDec = NULL;
    uint32_t i, j;
    int ret;
    struct UsbFunctionfsStringsHead headerStr = {0};

    GetHeaderStr(strings, &headerStr);
    str = UsbFnMemCalloc(headerStr.length);
    if (str == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    whereDec = str;
    ret = memcpy_s(whereDec, headerStr.length, &headerStr,
        sizeof(struct UsbFunctionfsStringsHead));
    if (ret != EOK) {
        goto ERR;
    }
    whereDec += sizeof(struct UsbFunctionfsStringsHead);

    for (i = 0; i < headerStr.lang_count; i++) {
        ret = memcpy_s(whereDec, headerStr.length - (whereDec - str),
            &strings[i]->language, sizeof(uint16_t));
        if (ret != EOK) {
            goto ERR;
        }
        whereDec += sizeof(uint16_t);
        for (j = 0; j < headerStr.str_count; j++) {
            if (strlen(strings[i]->strings[j].s)) {
                ret = memcpy_s(whereDec, headerStr.length - (whereDec - str),
                    strings[i]->strings[j].s, strlen(strings[i]->strings[j].s));
                whereDec += strlen(strings[i]->strings[j].s) + sizeof(char);
            } else {
                break;
            }
            if (ret != EOK) {
                goto ERR;
            }
        }
    }

    if (handle_write(ep0, str, headerStr.length) < 0) {
        goto ERR;
    }
    UsbFnMemFree(str);
    return 0;
ERR:
    UsbFnMemFree(str);
    return HDF_FAILURE;
}

void GetCountAndHead(struct UsbFunctionfsDescsHeadV2 *header, uint32_t *fsCount,
    uint32_t *hsCount, uint32_t *ssCount, const struct UsbFnFunction *func)
{
    int i;
    uint32_t lenCount = 0;
    uint32_t lenDes = 0;
    *fsCount = 0;
    *hsCount = 0;
    *ssCount = 0;

    for (i = 0; func->fsDescriptors[i] != NULL; i++) {
        (*fsCount)++;
        lenDes += func->fsDescriptors[i]->bLength;
    }
    for (i = 0; func->hsDescriptors[i] != NULL; i++) {
        (*hsCount)++;
        lenDes += func->hsDescriptors[i]->bLength;
    }
    for (i = 0; func->ssDescriptors[i] != NULL; i++) {
        (*ssCount)++;
        lenDes += func->ssDescriptors[i]->bLength;
    }

    if (*fsCount) {
        lenCount += sizeof(uint32_t);
        header->flags |= htole32(FUNCTIONFS_HAS_FS_DESC);
    }
    if (*hsCount) {
        lenCount += sizeof(uint32_t);
        header->flags |= htole32(FUNCTIONFS_HAS_HS_DESC);
    }
    if (*ssCount) {
        lenCount += sizeof(uint32_t);
        header->flags |= htole32(FUNCTIONFS_HAS_SS_DESC);
    }

    header->magic = htole32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2);
    header->length = htole32(sizeof(struct UsbFunctionfsDescsHeadV2) + lenCount + lenDes);
}

static int WriteFuncDescriptors(uint8_t **whereDec, struct UsbDescriptorHeader **headDes)
{
    int ret;
    int i;

    for (i = 0; headDes[i] != NULL; i++) {
        ret = memcpy_s(*whereDec, headDes[i]->bLength, headDes[i], headDes[i]->bLength);
        if (ret != EOK) {
            HDF_LOGE("%s: memcpy_s failure!", __func__);
            return HDF_FAILURE;
        }
        *whereDec += headDes[i]->bLength;
    }
    return 0;
}

static int CopyCount(uint8_t **whereDec, uint32_t fsCount, uint32_t hsCount, uint32_t ssCount)
{
    int ret;
    if (fsCount) {
        ret = memcpy_s(*whereDec, sizeof(uint32_t), &fsCount, sizeof(uint32_t));
        if (ret != EOK) {
            return HDF_FAILURE;
        }
        *whereDec += sizeof(uint32_t);
    }
    if (hsCount) {
        ret = memcpy_s(*whereDec, sizeof(uint32_t), &hsCount, sizeof(uint32_t));
        if (ret != EOK) {
            return HDF_FAILURE;
        }
        *whereDec += sizeof(uint32_t);
    }
    if (ssCount) {
        ret = memcpy_s(*whereDec, sizeof(uint32_t), &ssCount, sizeof(uint32_t));
        if (ret != EOK) {
            return HDF_FAILURE;
        }
        *whereDec += sizeof(uint32_t);
    }

    return 0;
}

static int UsbFnAdapterCreatPipes(int ep0, const struct UsbFnFunction *func)
{
    uint8_t *dec = NULL;
    uint8_t *whereDec = NULL;
    int ret;
    uint32_t fsCount;
    uint32_t hsCount;
    uint32_t ssCount;
    struct UsbFunctionfsDescsHeadV2 header = {0};

    GetCountAndHead(&header, &fsCount, &hsCount, &ssCount, func);

    dec = UsbFnMemCalloc(header.length);
    if (dec == NULL) {
        HDF_LOGE("%s: UsbFnMemCalloc failure!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    whereDec = dec;

    ret = memcpy_s(whereDec, header.length, &header,
        sizeof(struct UsbFunctionfsDescsHeadV2));
    if (ret != EOK) {
        return HDF_FAILURE;
    }
    whereDec += sizeof(struct UsbFunctionfsDescsHeadV2);

    ret = CopyCount(&whereDec, fsCount, hsCount, ssCount);
    if (ret != EOK) {
        return HDF_FAILURE;
    }

    ret = WriteFuncDescriptors(&whereDec, func->fsDescriptors);
    if (ret != EOK) {
        return HDF_FAILURE;
    }

    ret = WriteFuncDescriptors(&whereDec, func->hsDescriptors);
    if (ret != EOK) {
        return HDF_FAILURE;
    }

    ret = WriteFuncDescriptors(&whereDec, func->ssDescriptors);
    if (ret != EOK) {
        return HDF_FAILURE;
    }

    if (handle_write(ep0, dec, header.length) < 0) {
        HDF_LOGE("unable do write descriptors");
        UsbFnMemFree(dec);
        return HDF_ERR_IO;
    }

    UsbFnMemFree(dec);
    ret = UsbFnWriteStrings(ep0, func->strings);
    return ret;
}

static int UsbFnAdapterWriteFunctions(int fd, struct UsbFnConfiguration *UsbFnConfig,
    int cmd, struct FconfigString *gadgetName, struct FconfigString *configName)
{
    int ret;
    int fdEp0;
    int iCount;
    char tmp[MAX_PATHLEN];
    struct FconfigFuncInfo funcInfo;

    if (UsbFnConfig == NULL || gadgetName == NULL || configName == NULL) {
        HDF_LOGE("%s: UsbFnConfig is NULL", __func__);
        return HDF_ERR_IO;
    }
    funcInfo.gadgetName.len = gadgetName->len;
    funcInfo.gadgetName.s = gadgetName->s;
    funcInfo.configName.len = configName->len;
    funcInfo.configName.s = configName->s;
    for (iCount = 0; UsbFnConfig->functions[iCount] != NULL; iCount++) {
        memset_s(tmp, MAX_PATHLEN, 0, MAX_PATHLEN);
        ret = snprintf_s(tmp, MAX_PATHLEN, MAX_PATHLEN - 1,
            "generic.%s", UsbFnConfig->functions[iCount]->funcName);
        if (ret < 0) {
            return HDF_ERR_IO;
        }
        ret = UsbFnAdapterCreateFconfigString(&funcInfo.funcName, tmp);
        if (ret) {
            HDF_LOGE("%s: create config name failure!", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        ret=handle_ioctl(fd, cmd, &funcInfo);
        if (ret) {
            HDF_LOGE("%s: ioctl failure!", __func__);
            goto FAIL;
        }
        UsbFnMemFree(funcInfo.funcName.s);
        if (cmd == FCONFIG_CMD_DROP_FUNCTION) {
            continue;
        }

        fdEp0 = UsbFnAdapterOpenPipe(UsbFnConfig->functions[iCount]->funcName, 0);
        if (fd < 0) {
            goto FAIL;
        }
        ret = UsbFnAdapterCreatPipes(fdEp0, UsbFnConfig->functions[iCount]);
        if (ret) {
            goto FAIL2;
        }
        ret = UsbFnAdapterClosePipe(fdEp0);
        if (ret) {
            goto FAIL2;
        }
    }
    return 0;
FAIL2:
    UsbFnAdapterClosePipe(fdEp0);
FAIL:
    UsbFnMemFree(funcInfo.funcName.s);
    return -1;
}


static int UsbFnAdapterWriteConfigs(int fd, struct FconfigString *gadgetName,
    const struct UsbFnDeviceDesc *descriptor)
{
    struct FconfigCfgDesc configDesc;
    char tmp[MAX_PATHLEN];
    int ret;
    int iCount;
    uint8_t confVal;

    if (gadgetName == NULL || descriptor == NULL || descriptor->configs == NULL) {
        HDF_LOGE("%s: name is NULL", __func__);
        return HDF_ERR_IO;
    }
    for (iCount = 0; descriptor->configs[iCount]; iCount++) {
        configDesc.gadgetName.len = gadgetName->len;
        configDesc.gadgetName.s = gadgetName->s;
        confVal = descriptor->configs[iCount]->configurationValue;
        memset_s(tmp, MAX_PATHLEN, 0, MAX_PATHLEN);
        ret = snprintf_s(tmp, MAX_PATHLEN, MAX_PATHLEN - 1,
            "b.%d", confVal);
        if (ret < 0) {
            return HDF_ERR_IO;
        }
        ret = UsbFnAdapterCreateFconfigString(&configDesc.configName, tmp);
        if (ret) {
            HDF_LOGE("%s: create config name failure!", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        ret = UsbFnAdapterFillConfigDesc(&configDesc.cfgDesc, descriptor->configs[iCount]);
        if (ret) {
            HDF_LOGE("%s: UsbFnMemCalloc failure!", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        ret=handle_ioctl(fd, FCONFIG_CMD_ADD_CONFIG, &configDesc);
        if (ret) {
            HDF_LOGE("%s: ioctl failure!", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        ret = UsbFnAdapterWriteFunctions(fd, descriptor->configs[iCount], FCONFIG_CMD_MAKE_FUNCTION,
            gadgetName, &configDesc.configName);
        if (ret) {
            HDF_LOGE("%s: write func failure!", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        UsbFnMemFree(configDesc.configName.s);
    }
    return 0;
}

static int UsbFnAdapterWriteFcofnigUDC(int fd, int cmd, struct FconfigString *gadgetName,
    const char *udcName)
{
    int ret;
    struct FconfigUdcInfo udcInfo;

    if (gadgetName == NULL || udcName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    udcInfo.gadgetName.len = gadgetName->len;
    udcInfo.gadgetName.s = gadgetName->s;
    ret = UsbFnAdapterCreateFconfigString(&udcInfo.udcName, udcName);
    if (ret) {
        HDF_LOGE("%s: create udc_name failure!", __func__);
        return HDF_ERR_IO;
    }
    ret=handle_ioctl(fd, cmd, &udcInfo);
    if (ret) {
        HDF_LOGE("%s: ioctl failure!", __func__);
    }
    UsbFnMemFree(udcInfo.udcName.s);

    return ret;
}

static int UsbFnAdapterCreateDevice(const char *udcName,
    const char *devName, struct UsbFnDeviceDesc *descriptor)
{
    int fd;
    int ret;
    struct FconfigString gadgetName;

    fd = UsbFnAdapterOpenFn();
    if (fd < 0) {
        return HDF_ERR_IO;
    }
    ret = UsbFnAdapterCreateFconfigString(&gadgetName, devName);
    if (ret) {
        HDF_LOGE("%s: create gadget name failure!", __func__);
        goto FAIL;
    }
    ret = UsbFnAdapterWriteGadget(fd, FCONFIG_CMD_MAKE_GADGET, &gadgetName);
    if (ret) {
        dprintf("%s: UsbFnAdapterWriteGadget failed!", __func__);
        goto EXIT;
    }
    ret = UsbFnAdapterWriteDevDesc(fd, &gadgetName, descriptor);
    if (ret) {
        dprintf("%s: UsbFnAdapterWriteDevDesc failed!", __func__);
        goto EXIT;
    }
    ret = UsbFnAdapterWriteDevStrings(fd, &gadgetName, descriptor);
    if (ret) {
        dprintf("%s: UsbFnAdapterWriteDevStrings failed!", __func__);
        goto EXIT;
    }
    ret = UsbFnAdapterWriteConfigs(fd, &gadgetName, descriptor);
    if (ret) {
        dprintf("%s: UsbFnAdapterWriteConfigs failed!", __func__);
        goto EXIT;
    }
    ret = UsbFnAdapterWriteFcofnigUDC(fd, FCONFIG_CMD_ENABLE_UDC, &gadgetName, udcName);
    if (ret) {
        dprintf("%s: UsbFnAdapterWriteFcofnigUDC failed!", __func__);
        goto EXIT;
    }
    dprintf("%s: create device success!\n", __func__);
EXIT:
    UsbFnMemFree(gadgetName.s);
FAIL:
    if (UsbFnAdapterClosefn(fd) != 0) {
        dprintf("%s[%d] close fconfig failed\n", __func__, __LINE__);
    }

    return ret;
}

static int UsbFnAdapterDelConfigs(int configFd, struct FconfigString *gadgetName,
    struct UsbFnDeviceDesc *descriptor)
{
    struct FconfigCfgDesc configDesc;
    struct FconfigString configName;
    char tmp[MAX_PATHLEN];
    int ret;
    int iCount;
    uint8_t confVal;

    if (gadgetName == NULL || descriptor == NULL) {
        HDF_LOGE("%s: name is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    for (iCount = 0; descriptor->configs[iCount]; iCount++) {
        configDesc.gadgetName.len = gadgetName->len;
        configDesc.gadgetName.s = gadgetName->s;
        confVal = descriptor->configs[iCount]->configurationValue;
        ret = snprintf_s(tmp, MAX_PATHLEN, MAX_PATHLEN - 1,
            "b.%d", confVal);
        if (ret < 0) {
            HDF_LOGE("%s: snprintf_s failed", __func__);
            return HDF_ERR_IO;
        }
        ret = UsbFnAdapterCreateFconfigString(&configName, tmp);
        if (ret) {
            HDF_LOGE("%s: create config name failure!", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        configDesc.configName.len = configName.len;
        configDesc.configName.s = configName.s;
        ret = UsbFnAdapterWriteFunctions(configFd, descriptor->configs[iCount],
            FCONFIG_CMD_DROP_FUNCTION, gadgetName, &configName);
        if (ret) {
            HDF_LOGE("%s: write func failure!", __func__);
            goto FAIL;
        }
        ret = UsbFnAdapterFillConfigDesc(&configDesc.cfgDesc, descriptor->configs[iCount]);
        if (ret) {
            HDF_LOGE("%s: UsbFnMemCalloc failure!", __func__);
            goto FAIL;
        }
        ret=handle_ioctl(configFd, FCONFIG_CMD_REMOVE_CONFIG, &configDesc);
        if (ret) {
            HDF_LOGE("%s: ioctl failure!", __func__);
            goto FAIL;
        }
        UsbFnMemFree(configName.s);
    }
    return 0;
FAIL:
    UsbFnMemFree(configName.s);
    return -1;
}

static int UsbFnAdapterDelDevice(const char *devName, const char *udcName,
    struct UsbFnDeviceDesc *descriptor)
{
    int configFd;
    int ret;
    struct FconfigString gadgetName;

    if (devName == NULL || udcName == NULL || descriptor == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    configFd = UsbFnAdapterOpenFn();
    if (configFd <= 0) {
        return HDF_ERR_IO;
    }
    ret = UsbFnAdapterCreateFconfigString(&gadgetName, devName);
    if (ret) {
        HDF_LOGE("%s: create gadget_name failure!", __func__);
        return HDF_ERR_IO;
    }
    ret = UsbFnAdapterWriteFcofnigUDC(configFd, FCONFIG_CMD_DISABLE_UDC, &gadgetName, udcName);
    if (ret) {
        goto FAIL;
    }
    ret = UsbFnAdapterDelConfigs(configFd, &gadgetName, descriptor);
    if (ret) {
        goto FAIL;
    }
    ret = UsbFnAdapterWriteGadget(configFd, FCONFIG_CMD_DROP_GADGET, &gadgetName);
    if (ret) {
        goto FAIL;
    }
    ret = UsbFnAdapterClosefn(configFd);

FAIL:
    UsbFnMemFree(gadgetName.s);
    return ret;
}

static int UsbFnAdapterGetPipeInfo(int ep, struct UsbFnPipeInfo *pipeInfo)
{
    int ret;
    if (ep <= 0 || pipeInfo == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct usb_endpoint_descriptor desc;
    ret = handle_ioctl(ep, GENERIC_CMD_GET_PIPE_INFO, &desc);
    if (ret) {
        HDF_LOGE("%s: FUNCTIONFS_ENDPOINT_DESC failure!", __func__);
        return HDF_ERR_IO;
    }

    pipeInfo->type = desc.bmAttributes;
    pipeInfo->dir = USB_PIPE_DIRECTION_OUT;
    if (desc.bEndpointAddress & 0x80) {
        pipeInfo->dir = USB_PIPE_DIRECTION_IN;
    }

    pipeInfo->maxPacketSize = (desc.wMaxPacketSize[0] | (desc.wMaxPacketSize[1] << 8));
    pipeInfo->interval = desc.bInterval;

    return 0;
}

int UsbFnAdapterQueueInit(int ep)
{
    return 0;
}

static int UsbFnAdapterQueueDel(int ep)
{
    return 0;
}

static int UsbFnAdapterReleaseBuf(int ep, const struct GenericMemory *mem)
{
    return handle_ioctl(ep, GENERIC_CMD_FREE_MEM, &mem);
}

static int UsbFnAdapterPipeIo(int ep, struct IoData *ioData)
{
    int ret;
    if (ep <= 0 || ioData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (ioData->aio) {
        ret = handle_write(ep, (void*)ioData->buf, ioData->len);
    } else {
        ret = handle_ioctl(ep, GENERIC_CMD_ENDPOINT_IO, ioData);
    }
    return ret;
}

static int UsbFnAdapterCancelIo(int ep, const struct IoData *ioData)
{
    struct GenericMemory mem;

    mem.buf = ioData->buf;
    mem.size = ioData->len;

    return handle_ioctl(ep, GENERIC_CMD_CANCEL_REQUEST, &mem);
}

static int UsbFnAdapterRequestGetStatus(int ep, const struct IoData *ioData)
{
    if (ep <= 0 || ioData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    return handle_ioctl(ep, GENERIC_CMD_GET_REQ_STATUS, (void *)ioData);
}

static uint8_t *UsbFnAdapterMapAddr(int ep, uint32_t len)
{
    return handle_mmap(ep, len);
}

static int UsbFnAdapterUnmapAddr(uint8_t *mapAddr, uint32_t len)
{
    (void)mapAddr;
    (void)len;
    return 0;
}

static int Ep0Event(struct UsbFnEventAll *event, struct FconfigPollFd *pfds)
{
    int ret;
    uint8_t i;
    for (i = 0; i < event->ep0Num; i++) {
        if (pfds[i].revents & POLLIN) {
            ret = handle_read(event->ep0[i], &event->ep0Event[i].ctrlEvent, sizeof(struct UsbFnCtrlEvent));
            if (ret < 0) {
                HDF_LOGE("unable to read event from ep0");
                return ret;
            }
            event->ep0Event[i].type = USB_EP0_CTRL_EVENT;
        } else if (pfds[i].revents & POLLOUT) {
            ret = handle_ioctl(event->ep0[i], GENERIC_CMD_GET_EP0_EVENT, &event->ep0Event[i].reqEvent);
            if (ret < 0) {
                HDF_LOGE("unable to read reqEvent from ep0");
                return ret;
            }
            event->ep0Event[i].type = USB_EP0_IO_COMPLETED;
        }
    }
    return 0;
}

static int EpEvent(struct UsbFnEventAll *event, struct FconfigPollFd *pfds)
{
    uint8_t i;
    int ret;
    for (i = 0; i < event->epNum; i++) {
        if ((pfds[i + event->ep0Num].revents & POLLIN)) {
            ret = handle_read(event->epx[i], event->reqEvent[i],
                MAX_REQUEST * sizeof(struct UsbFnReqEvent));
            if (ret < 0) {
                HDF_LOGE("unable to read event from eps");
                return ret;
            }
            event->numEvent[i] =  ret / sizeof(struct UsbFnReqEvent);
        }
    }
    return 0;
}

static int UsbFnAdapterPollEvent(struct UsbFnEventAll *event, int timeout)
{
    uint8_t i;
    struct FconfigPollFd pfds[16] = {0};
    if (event == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if ((event->ep0Num + event->epNum) == 0) {
        return HDF_ERR_INVALID_PARAM;
    }
    memset_s(&pfds, sizeof(pfds), 0, sizeof(pfds));
    for (i = 0; i < event->ep0Num; i++) {
        if (event->ep0[i] <= 0) {
            HDF_LOGE("%s: ep[%d] = %d", __func__, i, event->ep0[i]);
            return HDF_ERR_INVALID_PARAM;
        }
        pfds[i].fd = event->ep0[i];
        pfds[i].events = POLLIN | POLLOUT;
    }
    for (i = 0; i < event->epNum; i++) {
        if (event->epx[i] <= 0) {
            HDF_LOGE("%s: ep[%d] = %d", __func__, i, event->epx[i]);
            return HDF_ERR_INVALID_PARAM;
        }
        pfds[i + event->ep0Num].fd = event->epx[i];
        pfds[i + event->ep0Num].events = POLLIN;
    }
    for (i = 0; i < (event->ep0Num + event->epNum); i++) {
        pfds[i].revents = handle_poll(pfds[i].fd, timeout);
        if (pfds[i].revents < 0) {
            HDF_LOGE("%s: handle_poll failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }
    if (Ep0Event(event, &pfds[0]) < 0) {
        HDF_LOGE("%s: handle_poll failed", __func__);
        return HDF_ERR_IO;
    }
    if (EpEvent(event, &pfds[0]) < 0) {
        HDF_LOGE("%s: handle_poll failed", __func__);
        return HDF_ERR_IO;
    }
    return 0;
}

static int UsbFnAdapterWriteUDC(const char *deviceName, const char *udcName, int enable)
{
    int ret;
    int configFd;
    int cmd;
    struct FconfigUdcInfo udcInfo;

    configFd = UsbFnAdapterOpenFn();
    if (configFd <= 0) {
        return HDF_ERR_IO;
    }
    ret = UsbFnAdapterCreateFconfigString(&udcInfo.gadgetName, deviceName);
    if (ret) {
        HDF_LOGE("%s: create gadget_name failure!", __func__);
        return HDF_ERR_IO;
    }
    cmd = enable ? FCONFIG_CMD_ENABLE_UDC : FCONFIG_CMD_DISABLE_UDC;
    ret = UsbFnAdapterWriteFcofnigUDC(configFd, cmd, &udcInfo.gadgetName, udcName);
    if (ret) {
       return HDF_ERR_IO;
    }
    configFd = UsbFnAdapterClosefn(configFd);
    if (ret != 0) {
        HDF_LOGE("%s: close failure!", __func__);
    }
    return 0;
}

static int UsbFnWriteProp(const char *deviceName,
    const char *propName, uint32_t propValue)
{
    struct FconfigDevdescInfo info;
    int ret;
    int configFd;
    if (deviceName == NULL || propName == NULL) {
        HDF_LOGE("%s: param invail!", __func__);
        return HDF_ERR_IO;
    }
    ret = UsbFnAdapterCreateFconfigString(&info.gadgetName, deviceName);
    if (ret) {
        HDF_LOGE("%s: create gadget name failure!", __func__);
        return HDF_ERR_IO;
    }
    info.Prop.propName = propName;
    info.Prop.propValue = (uint32_t)propValue;
    configFd = UsbFnAdapterOpenFn();
    if (configFd <= 0) {
        ret = HDF_ERR_IO;
        goto FAIL;
    }
    ret = handle_ioctl(configFd, FCONFIG_CMD_CHAGE_DEVINFO, &info);
    if (ret) {
        HDF_LOGE("%s: ioctl failure!", __func__);
        goto FAIL;
    }
    configFd = UsbFnAdapterClosefn(configFd);
    if (ret != 0) {
        HDF_LOGE("%s: close failure!", __func__);
    }
FAIL:
    UsbFnMemFree(info.gadgetName.s);

    return ret;
}

static int UsbFnWriteDesString(const char *deviceName,
    uint16_t lang, const char *stringName, const char *stringValue)
{
    struct FconfigDevDescString info;
    int ret;
    int configFd;
    if (deviceName == NULL || stringName == NULL || \
        stringValue == NULL) {
        HDF_LOGE("%s: param invail!", __func__);
        return HDF_ERR_IO;
    }
    ret = UsbFnAdapterCreateFconfigString(&info.gadgetName, deviceName);
    if (ret) {
        HDF_LOGE("%s: create gadget name failure!", __func__);
        return HDF_ERR_IO;
    }
    info.Prop.lang = lang;
    info.Prop.propName = stringName;
    info.Prop.propValue = stringValue;
    configFd = UsbFnAdapterOpenFn();
    if (configFd <= 0) {
        dprintf("%s, %d\n", __func__, __LINE__);
        ret = HDF_ERR_IO;
        goto FAIL;
    }
    ret = handle_ioctl(configFd, FCONFIG_CMD_CHAGE_DEVSTRING, &info);
    if (ret) {
        HDF_LOGE("%s: ioctl failure!", __func__);
        goto FAIL;
    }
    configFd = UsbFnAdapterClosefn(configFd);
    if (ret != 0) {
        HDF_LOGE("%s: close failure!", __func__);
    }
FAIL:
    UsbFnMemFree(info.gadgetName.s);

    return ret;
}

void *UsbFnMemAlloc(size_t size)
{
    return UsbFnMemCalloc(size);
}

void *UsbFnMemCalloc(size_t size)
{
    void *buf = NULL;
    struct RawUsbRamTestList *testEntry = NULL;
    struct RawUsbRamTestList *pos = NULL;
    uint32_t totalSize = 0;

    buf = OsalMemAlloc(size);
    if (buf != NULL) {
        (void)memset_s(buf, size, 0, size);
    }
    if (g_usbRamTestFlag) {
        if (g_usbRamTestHead == NULL) {
            g_usbRamTestHead = OsalMemAlloc(sizeof(struct RawUsbRamTestList));
            OsalMutexInit(&g_usbRamTestHead->lock);
            DListHeadInit(&g_usbRamTestHead->list);
        }
        testEntry = OsalMemAlloc(sizeof(struct RawUsbRamTestList));
        testEntry->address = (uint32_t) buf;
        testEntry->size = size;

        OsalMutexLock(&g_usbRamTestHead->lock);
        DListInsertTail(&testEntry->list, &g_usbRamTestHead->list);
        DLIST_FOR_EACH_ENTRY(pos, &g_usbRamTestHead->list, struct RawUsbRamTestList, list) {
            totalSize += pos->size;
        }
        OsalMutexUnlock(&g_usbRamTestHead->lock);

        HDF_LOGE("%{public}s add size=%{public}d totalSize=%{public}d", __func__, (uint32_t)size, totalSize);
    }
    return buf;
}

void UsbFnMemFree(void *mem)
{
    struct RawUsbRamTestList *pos = NULL;
    struct RawUsbRamTestList *tmp = NULL;
    uint32_t totalSize = 0;
    uint32_t size = 0;

    if (mem != NULL) {
        free(mem);
    }
    else {
        return;
    }

    if ((g_usbRamTestFlag == true) && (g_usbRamTestHead != NULL)) {
        OsalMutexLock(&g_usbRamTestHead->lock);
        DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &g_usbRamTestHead->list, struct RawUsbRamTestList, list) {
            if ((NULL != pos) && (pos->address == (uint32_t)mem))
            {
                size = pos->size;
                DListRemove(&pos->list);
                free(pos);
                continue;
            }
            totalSize += pos->size;
        }
        OsalMutexUnlock(&g_usbRamTestHead->lock);
        HDF_LOGE("%{public}s rm size=%{public}d totalSize=%{public}d", __func__, size, totalSize);
    }
}

int UsbFnAdpMemTestTrigger(bool enable)
{
    g_usbRamTestFlag = enable;
    return HDF_SUCCESS;
}

static struct UsbFnAdapterOps g_usbFnAdapter = {
    .createDevice = UsbFnAdapterCreateDevice,
    .delDevice = UsbFnAdapterDelDevice,

    .openPipe = UsbFnAdapterOpenPipe,
    .closePipe = UsbFnAdapterClosePipe,
    .getPipeInfo = UsbFnAdapterGetPipeInfo,

    .queueInit = UsbFnAdapterQueueInit,
    .queueDel = UsbFnAdapterQueueDel,
    .releaseBuf = UsbFnAdapterReleaseBuf,
    .pipeIo = UsbFnAdapterPipeIo,
    .cancelIo = UsbFnAdapterCancelIo,
    .getReqStatus = UsbFnAdapterRequestGetStatus,
    .mapAddr = UsbFnAdapterMapAddr,
    .unmapAddr = UsbFnAdapterUnmapAddr,
    .pollEvent = UsbFnAdapterPollEvent,
    .writeUDC = UsbFnAdapterWriteUDC,
    .writeProp = UsbFnWriteProp,
    .writeDesString = UsbFnWriteDesString,
};

struct UsbFnAdapterOps *UsbFnAdapterGetOps(void)
{
    return &g_usbFnAdapter;
}

