/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "rtc_core.h"
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG rtc_core

int RtcSetHostMethod(struct RtcHost *host, struct RtcMethod *method)
{
    if (host == NULL || method == NULL) {
        HDF_LOGE("RtcSetHostMethod: Invalid parameter");
        return HDF_ERR_INVALID_PARAM;
    }
    host->method = method;
    return HDF_SUCCESS;
}

struct RtcHost *RtcHostCreate(struct HdfDeviceObject *device)
{
    struct RtcHost *host = NULL;

    if (device == NULL) {
        HDF_LOGE("RtcHostCreate: device NULL!");
        return NULL;
    }

    host = (struct RtcHost *)OsalMemCalloc(sizeof(*host));
    if (host == NULL) {
        HDF_LOGE("RtcHostCreate: malloc host fail!");
        return NULL;
    }
    host->device = device;
    return host;
}

void RtcHostDestroy(struct RtcHost *host)
{
    if (host != NULL) {
        host->device = NULL;
        host->method = NULL;
        host->data = NULL;
        OsalMemFree(host);
    }
}
