/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_parse.h"

#define HDF_LOG_TAG audio_parse

int32_t AudioFillConfigData(const struct HdfDeviceObject *device, struct AudioConfigData *configData)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    ADM_LOG_DEBUG("Entry.");

    if (device == NULL || configData == NULL) {
        ADM_LOG_ERR("Input para check error: device=%p, configData=%p.", device, configData);
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        ADM_LOG_ERR("drs node is NULL.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        ADM_LOG_ERR("AudioFillConfigData: invalid drs ops fail!");
        return HDF_FAILURE;
    }

    int32_t serviceRet = drsOps->GetString(node, "serviceName", &(configData->cardServiceName), 0);
    int32_t codecRet = drsOps->GetString(node, "codecName", &(configData->codecName), 0);
    int32_t platformRet = drsOps->GetString(node, "platformName", &(configData->platformName), 0);
    int32_t cpuRet = drsOps->GetString(node, "cpuDaiName", &(configData->cpuDaiName), 0);
    int32_t codeDaiRet = drsOps->GetString(node, "codecDaiName", &(configData->codecDaiName), 0);
    int32_t dspRet = drsOps->GetString(node, "dspName", &(configData->dspName), 0);
    int32_t dspDaiRet = drsOps->GetString(node, "dspDaiName", &(configData->dspDaiName), 0);
    int32_t accessoryRet = drsOps->GetString(node, "accessoryName", &(configData->accessoryName), 0);
    int32_t accessoryDaiRet = drsOps->GetString(node, "accessoryDaiName", &(configData->accessoryDaiName), 0);
    if (serviceRet || codecRet || platformRet || cpuRet || codeDaiRet ||
        dspRet || dspDaiRet || accessoryRet || accessoryDaiRet) {
        ADM_LOG_ERR("Read audioDeviceName fail: serviceRet=%d, codecRet=%d, platformRet=%d, cpuRet=%d, codeDaiRet=%d,"
            "dspRet=%d, dspDaiRet=%d, accessoryRet=%d, accessoryDaiRet=%d",
            serviceRet, codecRet, platformRet, cpuRet, codeDaiRet, dspRet,
            dspDaiRet, accessoryRet, accessoryDaiRet);
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("Success! codecName = %s, platformName = %s, cpuDaiName = %s, codecDaiName = %s, "
        "dspName = %s, dspDaiName = %s, accessoryName = %s, accessoryDaiName = %s.",
        configData->codecName, configData->platformName, configData->cpuDaiName,
        configData->codecDaiName, configData->dspName, configData->dspDaiName,
        configData->accessoryName, configData->accessoryDaiName);

    return HDF_SUCCESS;
}
