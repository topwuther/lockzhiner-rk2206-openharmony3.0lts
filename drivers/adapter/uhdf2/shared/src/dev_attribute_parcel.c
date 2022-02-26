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

#include "dev_attribute_parcel.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG dev_attribute_parcel

#define ATTRIBUTE_PRIVATE_DATA_LENGTH_NULL      0
#define ATTRIBUTE_PRIVATE_DATA_LENGTH_NORMAL    1

bool DeviceAttributeReadPrivateData(struct HdfDeviceInfoFull *attribute, const void *privateData)
{
    if (privateData != NULL) {
        uint32_t length = ((struct HdfPrivateInfo *)(privateData))->length;
        attribute->super.private = (const void *)OsalMemCalloc(length);
        if (attribute->super.private != NULL) {
            memcpy_s((void *)(attribute->super.private), length, privateData, length);
            if (attribute->super.private == NULL) {
                HDF_LOGE("%s: memcpy_s private error", __func__);
                return false;
            }
        } else {
            HDF_LOGE("%s: OsalMemCalloc private error", __func__);
            return false;
        }
    }

    return true;
}

bool DeviceAttributeFullWrite(const struct HdfDeviceInfoFull *attribute, struct HdfSBuf *sbuf)
{
    uint32_t length;

    if (attribute == NULL || sbuf == NULL) {
        return false;
    }

    uint8_t ret = 1;
    ret &= HdfSbufWriteInt32(sbuf, attribute->super.deviceId);
    ret &= HdfSbufWriteInt32(sbuf, attribute->super.hostId);
    ret &= HdfSbufWriteInt32(sbuf, attribute->super.policy);
    ret &= HdfSbufWriteString(sbuf, attribute->super.svcName);
    ret &= HdfSbufWriteString(sbuf, attribute->super.moduleName);

    if (attribute->super.deviceMatchAttr != NULL) {
        length = ATTRIBUTE_PRIVATE_DATA_LENGTH_NORMAL;
        ret &= HdfSbufWriteUint32(sbuf, length);
        ret &= HdfSbufWriteString(sbuf, attribute->super.deviceMatchAttr);
    } else {
        length = ATTRIBUTE_PRIVATE_DATA_LENGTH_NULL;
        ret &= HdfSbufWriteUint32(sbuf, length);
    }

    if (attribute->super.private != NULL) {
        length = ATTRIBUTE_PRIVATE_DATA_LENGTH_NORMAL;
        ret &= HdfSbufWriteUint32(sbuf, length);
        length = ((struct HdfPrivateInfo *)(attribute->super.private))->length;
        ret &= HdfSbufWriteBuffer(sbuf, attribute->super.private, length);
    } else {
        length = ATTRIBUTE_PRIVATE_DATA_LENGTH_NULL;
        ret &= HdfSbufWriteUint32(sbuf, length);
    }

    if (ret == 0) {
        HDF_LOGE("Device attribute write parcel failed");
        return false;
    }
    return true;
}

struct HdfDeviceInfoFull *DeviceAttributeFullRead(struct HdfSBuf *sbuf)
{
    if (sbuf == NULL) {
        return NULL;
    }

    struct HdfDeviceInfoFull *attribute = HdfDeviceInfoFullNewInstance();
    if (attribute == NULL) {
        HDF_LOGE("OsalMemCalloc failed, attribute is null");
        return NULL;
    }
    HdfSbufReadUint16(sbuf, &attribute->super.deviceId);
    HdfSbufReadUint16(sbuf, &attribute->super.hostId);
    HdfSbufReadUint16(sbuf, &attribute->super.policy);
    do {
        const char *svcName = HdfSbufReadString(sbuf);
        if (svcName == NULL) {
            HDF_LOGE("Read from parcel failed, svcName is null");
            break;
        }
        attribute->super.svcName = strdup(svcName);
        if (attribute->super.svcName == NULL) {
            HDF_LOGE("Read from parcel failed, strdup svcName fail");
            break;
        }
        const char *moduleName = HdfSbufReadString(sbuf);
        if (moduleName == NULL) {
            HDF_LOGE("Read from parcel failed, driverPath is null");
            break;
        }
        attribute->super.moduleName = strdup(moduleName);
        if (attribute->super.moduleName == NULL) {
            HDF_LOGE("Read from parcel failed, strdup moduleName fail");
            break;
        }

        uint32_t length;
        if (!HdfSbufReadUint32(sbuf, &length)) {
            HDF_LOGE("Device attribute readDeviceMatchAttr length failed");
            break;
        }
        if (length == ATTRIBUTE_PRIVATE_DATA_LENGTH_NORMAL) {
            const char *deviceMatchAttr = HdfSbufReadString(sbuf);
            if (deviceMatchAttr == NULL) {
                HDF_LOGE("%s: Read from parcel failed, deviceMatchAttr is null", __func__);
                break;
            }
            attribute->super.deviceMatchAttr = strdup(deviceMatchAttr);
        }

        if (!HdfSbufReadUint32(sbuf, &length)) {
            HDF_LOGE("Device attribute readPrivate length failed");
            break;
        }
        if (length == ATTRIBUTE_PRIVATE_DATA_LENGTH_NORMAL) {
            uint32_t privateLength;
            void *privateData = NULL;
            if (!HdfSbufReadBuffer(sbuf, (const void **)(&privateData), &privateLength)) {
                HDF_LOGW("%s: HdfSbufReadBuffer privateData error!", __func__);
                privateData = NULL;
            }
            if (!DeviceAttributeReadPrivateData(attribute, privateData)) {
                HDF_LOGE("%s: Read from parcel failed, private is null", __func__);
                break;
            }
        }

        return attribute;
    } while (0);

    HdfDeviceInfoFullFreeInstance(attribute);
    return NULL;
}
