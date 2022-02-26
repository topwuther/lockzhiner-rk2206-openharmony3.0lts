/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_core.h"

#define HDF_LOG_TAG audio_core

AUDIO_LIST_HEAD(daiController);
AUDIO_LIST_HEAD(platformController);
AUDIO_LIST_HEAD(codecController);
AUDIO_LIST_HEAD(dspController);
AUDIO_LIST_HEAD(accessoryController);

int32_t AudioSocRegisterPlatform(struct HdfDeviceObject *device, struct PlatformData *data)
{
    struct PlatformDevice *platform = NULL;

    if ((device == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p.", device, data);
        return HDF_ERR_INVALID_OBJECT;
    }

    platform = (struct PlatformDevice *)OsalMemCalloc(sizeof(*platform));
    if (platform == NULL) {
        ADM_LOG_ERR("Malloc platform device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    platform->devPlatformName = data->drvPlatformName;
    platform->devData = data;
    platform->device = device;
    DListInsertHead(&platform->list, &platformController);
    ADM_LOG_INFO("Register [%s] success.", platform->devPlatformName);

    return HDF_SUCCESS;
}

int32_t AudioSocRegisterDai(struct HdfDeviceObject *device, struct DaiData *data)
{
    struct DaiDevice *dai = NULL;

    if ((device == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p.", device, data);
        return HDF_ERR_INVALID_OBJECT;
    }

    dai = (struct DaiDevice *)OsalMemCalloc(sizeof(*dai));
    if (dai == NULL) {
        ADM_LOG_ERR("Malloc dai device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    dai->devDaiName = data->drvDaiName;
    dai->devData = data;
    dai->device = device;
    DListInsertHead(&dai->list, &daiController);
    ADM_LOG_INFO("Register [%s] success.", dai->devDaiName);

    return HDF_SUCCESS;
}

int32_t AudioRegisterAccessory(struct HdfDeviceObject *device, struct AccessoryData *data, struct DaiData *daiData)
{
    struct AccessoryDevice *accessory = NULL;
    int32_t ret;

    if (device == NULL || data == NULL || daiData == NULL) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p, daiData=%p.", device, data, daiData);
        return HDF_ERR_INVALID_OBJECT;
    }

    accessory = (struct AccessoryDevice *)OsalMemCalloc(sizeof(*accessory));
    if (accessory == NULL) {
        ADM_LOG_ERR("Malloc accessory device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    OsalMutexInit(&accessory->mutex);
    accessory->devAccessoryName = data->drvAccessoryName;
    accessory->devData = data;
    accessory->device = device;

    ret = AudioSocDeviceRegister(device, (void *)daiData, AUDIO_DAI_DEVICE);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(accessory);
        ADM_LOG_ERR("Register accessory device fail ret=%d", ret);
        return HDF_ERR_IO;
    }
    DListInsertHead(&accessory->list, &accessoryController);
    ADM_LOG_INFO("Register [%s] success.", accessory->devAccessoryName);

    return HDF_SUCCESS;
}

int32_t AudioRegisterCodec(struct HdfDeviceObject *device, struct CodecData *codecData, struct DaiData *daiData)
{
    struct CodecDevice *codec = NULL;
    int32_t ret;

    if ((device == NULL) || (codecData == NULL) || (daiData == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, codecData=%p, daiData=%p.",
            device, codecData, daiData);
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = (struct CodecDevice *)OsalMemCalloc(sizeof(*codec));
    if (codec == NULL) {
        ADM_LOG_ERR("Malloc codec device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    OsalMutexInit(&codec->mutex);
    codec->devCodecName = codecData->drvCodecName;
    codec->devData = codecData;
    codec->device = device;

    ret = AudioSocDeviceRegister(device, (void *)daiData, AUDIO_DAI_DEVICE);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(codec);
        ADM_LOG_ERR("Register dai device fail ret=%d", ret);
        return HDF_ERR_IO;
    }
    DListInsertHead(&codec->list, &codecController);
    ADM_LOG_INFO("Register [%s] success.", codec->devCodecName);

    return HDF_SUCCESS;
}

int32_t AudioRegisterDsp(struct HdfDeviceObject *device, struct DspData *dspData, struct DaiData *DaiData)
{
    struct DspDevice *dspDev = NULL;
    int32_t ret;

    if ((device == NULL) || (dspData == NULL) || (DaiData == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, dspData=%p, daiData=%p.",
            device, dspData, DaiData);
        return HDF_ERR_INVALID_OBJECT;
    }

    dspDev = (struct DspDevice *)OsalMemCalloc(sizeof(*dspDev));
    if (dspDev == NULL) {
        ADM_LOG_ERR("Malloc codec device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    dspDev->devDspName = dspData->drvDspName;
    dspDev->devData = dspData;
    dspDev->device = device;

    ret = AudioSocDeviceRegister(device, (void *)DaiData, AUDIO_DAI_DEVICE);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(dspDev);
        ADM_LOG_ERR("Register dai device fail ret=%d", ret);
        return HDF_ERR_IO;
    }
    DListInsertHead(&dspDev->list, &dspController);
    ADM_LOG_INFO("Register [%s] success.", dspDev->devDspName);

    return HDF_SUCCESS;
}

int32_t AudioSocDeviceRegister(struct HdfDeviceObject *device, void *data, enum AudioDeviceType deviceType)
{
    struct PlatformData *platformData = NULL;
    struct DaiData *daiData = NULL;
    int32_t ret;

    if ((device == NULL) || (data == NULL) || (deviceType >= AUDIO_DEVICE_BUTT) || (deviceType < 0)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p, deviceType=%d.",
            device, data, deviceType);
        return HDF_ERR_INVALID_OBJECT;
    }

    switch (deviceType) {
        case AUDIO_DAI_DEVICE: {
            daiData = (struct DaiData *)data;
            ret = AudioSocRegisterDai(device, daiData);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("Register dai device fail ret=%d", ret);
                return HDF_FAILURE;
            }
            break;
        }
        case AUDIO_PLATFORM_DEVICE: {
            platformData = (struct PlatformData *)data;
            ret = AudioSocRegisterPlatform(device, platformData);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("Register dma device fail ret=%d", ret);
                return HDF_FAILURE;
            }
            break;
        }
        default: {
            ADM_LOG_ERR("Invalid device type.");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioSeekPlatformDevice(struct AudioRuntimeDeivces *rtd, const struct AudioConfigData *configData)
{
    struct PlatformDevice *platform = NULL;
    if (rtd == NULL || configData == NULL) {
        ADM_LOG_ERR("Input params check error: rtd=%p, configData=%p.", rtd, configData);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (configData->platformName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: configData->platformName is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    DLIST_FOR_EACH_ENTRY(platform, &platformController, struct PlatformDevice, list) {
        if (platform != NULL && platform->devPlatformName != NULL &&
            strcmp(platform->devPlatformName, configData->platformName) == 0) {
            rtd->platform = platform;
            break;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioSeekCpuDaiDevice(struct AudioRuntimeDeivces *rtd, const struct AudioConfigData *configData)
{
    struct DaiDevice *cpuDai = NULL;
    if (rtd == NULL || configData == NULL) {
        ADM_LOG_ERR("Input params check error: rtd=%p, configData=%p.", rtd, configData);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (configData->cpuDaiName == NULL) {
        ADM_LOG_ERR("Input cpuDaiName check error: configData->cpuDaiName is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (DListIsEmpty(&daiController)) {
        ADM_LOG_ERR("daiController is empty.");
        return HDF_FAILURE;
    }
    DLIST_FOR_EACH_ENTRY(cpuDai, &daiController, struct DaiDevice, list) {
        if (cpuDai != NULL && cpuDai->devDaiName != NULL &&
            strcmp(cpuDai->devDaiName, configData->cpuDaiName) == 0) {
            rtd->cpuDai = cpuDai;
            break;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioSeekCodecDevice(struct AudioRuntimeDeivces *rtd, const struct AudioConfigData *configData)
{
    struct CodecDevice *codec = NULL;
    struct DaiDevice *codecDai = NULL;
    if ((rtd == NULL) || (configData == NULL)) {
        ADM_LOG_ERR("Input params check error: rtd=%p, configData=%p.", rtd, configData);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (configData->codecName == NULL || configData->codecDaiName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: configData->codecName=%p, configData->codecDaiName=%p.",
                    configData->codecName, configData->codecDaiName);
        return HDF_ERR_INVALID_OBJECT;
    }

    DLIST_FOR_EACH_ENTRY(codec, &codecController, struct CodecDevice, list) {
        if (codec != NULL && codec->devCodecName != NULL && strcmp(codec->devCodecName, configData->codecName) == 0) {
            rtd->codec = codec;
            DLIST_FOR_EACH_ENTRY(codecDai, &daiController, struct DaiDevice, list) {
                if (codecDai != NULL && codecDai->device != NULL && codec->device == codecDai->device &&
                    codecDai->devDaiName != NULL && strcmp(codecDai->devDaiName, configData->codecDaiName) == 0) {
                    rtd->codecDai = codecDai;
                    break;
                }
            }
            break;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioSeekAccessoryDevice(struct AudioRuntimeDeivces *rtd, const struct AudioConfigData *configData)
{
    struct AccessoryDevice *accessory = NULL;
    struct DaiDevice *accessoryDai = NULL;
    if (rtd == NULL || configData == NULL) {
        ADM_LOG_ERR("Input params check error: rtd=%p, configData=%p.", rtd, configData);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (configData->accessoryName == NULL || configData->accessoryDaiName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: configData->accessoryName or"
            "configData->accessoryDaiName is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    DLIST_FOR_EACH_ENTRY(accessory, &accessoryController, struct AccessoryDevice, list) {
        if (accessory != NULL && accessory->devAccessoryName != NULL &&
                strcmp(accessory->devAccessoryName, configData->accessoryName) == 0) {
            rtd->accessory = accessory;
            DLIST_FOR_EACH_ENTRY(accessoryDai, &daiController, struct DaiDevice, list) {
                if (accessoryDai != NULL && accessoryDai->device != NULL &&
                    accessory->device == accessoryDai->device && accessoryDai->devDaiName != NULL &&
                    strcmp(accessoryDai->devDaiName, configData->accessoryDaiName) == 0) {
                    rtd->accessoryDai = accessoryDai;
                    break;
                }
            }
            break;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioSeekDspDevice(struct AudioRuntimeDeivces *rtd, const struct AudioConfigData *configData)
{
    struct DspDevice *dsp = NULL;
    struct DaiDevice *dspDai = NULL;
    if ((rtd == NULL) || (configData == NULL)) {
        ADM_LOG_ERR("Input params check error: rtd=%p, configData=%p.", rtd, configData);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (configData->dspName == NULL || configData->dspDaiName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: configData->dspName=%p, configData->dspDaiName=%p.",
                    configData->dspName, configData->dspDaiName);
        return HDF_ERR_INVALID_OBJECT;
    }

    DLIST_FOR_EACH_ENTRY(dsp, &dspController, struct DspDevice, list) {
        if (dsp != NULL && dsp->devDspName != NULL && strcmp(dsp->devDspName, configData->dspName) == 0) {
            rtd->dsp = dsp;
            DLIST_FOR_EACH_ENTRY(dspDai, &daiController, struct DaiDevice, list) {
                if (dspDai != NULL && dspDai->device != NULL && dsp->device == dspDai->device &&
                    dspDai->devDaiName != NULL && strcmp(dspDai->devDaiName, configData->dspDaiName) == 0) {
                    rtd->dspDai = dspDai;
                    break;
                }
            }
            break;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioBindDaiLink(struct AudioCard *audioCard, const struct AudioConfigData *configData)
{
    if ((audioCard == NULL) || (configData == NULL)) {
        ADM_LOG_ERR("Input params check error: audioCard=%p, configData=%p.", audioCard, configData);
        return HDF_ERR_INVALID_OBJECT;
    }

    audioCard->rtd = (struct AudioRuntimeDeivces *)OsalMemCalloc(sizeof(struct AudioRuntimeDeivces));
    if (audioCard->rtd == NULL) {
        ADM_LOG_ERR("Malloc audioCard->rtd fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    audioCard->rtd->complete = AUDIO_DAI_LINK_UNCOMPLETE;
    if (AudioSeekPlatformDevice(audioCard->rtd, configData) == HDF_SUCCESS) {
        ADM_LOG_DEBUG("PLATFORM [%s] is registered!", configData->platformName);
    }
    if (AudioSeekCpuDaiDevice(audioCard->rtd, configData) == HDF_SUCCESS) {
        ADM_LOG_DEBUG("CPU DAI [%s] is registered!", configData->cpuDaiName);
    }
    if (AudioSeekCodecDevice(audioCard->rtd, configData) == HDF_SUCCESS) {
        ADM_LOG_DEBUG("CODEC [%s] is registered!", configData->codecName);
    }
    if (AudioSeekAccessoryDevice(audioCard->rtd, configData) == HDF_SUCCESS) {
        ADM_LOG_DEBUG("CODEC [%s] is registered!", configData->accessoryName);
    }
    if (AudioSeekDspDevice(audioCard->rtd, configData) == HDF_SUCCESS) {
        ADM_LOG_DEBUG("CODEC [%s] is registered!", configData->dspName);
    }
    audioCard->rtd->complete = AUDIO_DAI_LINK_COMPLETE;
    ADM_LOG_DEBUG("All devices register complete!");

    return HDF_SUCCESS;
}

int32_t AudioUpdateCodecRegBits(struct CodecDevice *codec,
    const struct AudioMixerControl *mixerControl, uint32_t value)
{
    int32_t ret;
    uint32_t curValue;
    uint32_t mixerControlMask;
    if (codec == NULL || mixerControl == NULL) {
        ADM_LOG_ERR("Invalid accessory param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << mixerControl->shift;
    mixerControlMask = mixerControl->mask << mixerControl->shift;

    OsalMutexLock(&codec->mutex);
    ret = AudioCodecReadReg(codec, mixerControl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Read reg fail ret=%d.", ret);
        return HDF_FAILURE;
    }

    curValue = (curValue & ~mixerControlMask) | (value & mixerControlMask);
    ret = AudioCodecWriteReg(codec, mixerControl->reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Write reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&codec->mutex);

    ADM_LOG_DEBUG("Success.");
    return HDF_SUCCESS;
}

int32_t AudioUpdateAccessoryRegBits(struct AccessoryDevice *accessory,
    const struct AudioMixerControl *mixerControl, uint32_t value)
{
    int32_t ret;
    uint32_t curValue;
    uint32_t mixerControlMask;
    if (accessory == NULL || mixerControl == NULL) {
        ADM_LOG_ERR("Invalid accessory param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << mixerControl->shift;
    mixerControlMask = mixerControl->mask << mixerControl->shift;

    OsalMutexLock(&accessory->mutex);
    ret = AudioAccessoryReadReg(accessory, mixerControl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&accessory->mutex);
        ADM_LOG_ERR("Read reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    curValue = (curValue & ~mixerControlMask) | (value & mixerControlMask);
    ret = AudioAccessoryWriteReg(accessory, mixerControl->reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&accessory->mutex);
        ADM_LOG_ERR("Write reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&accessory->mutex);

    ADM_LOG_DEBUG("Success.");
    return HDF_SUCCESS;
}

int32_t AudioUpdateCodecAiaoRegBits(struct CodecDevice *codec,
    const struct AudioMixerControl *mixerControl, uint32_t value)
{
    int32_t ret;
    uint32_t curValue;
    uint32_t mixerControlMask;
    ADM_LOG_DEBUG("Entry to update AIAO codec  reg bits.");

    if (codec == NULL || mixerControl == NULL) {
        ADM_LOG_ERR("Invalid AIAO codec param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << mixerControl->shift;
    mixerControlMask = mixerControl->mask << mixerControl->shift;

    OsalMutexLock(&codec->mutex);
    ret = AudioCodecAiaoReadReg(codec, mixerControl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Read AIAO codec reg fail ret=%d.", ret);
        return HDF_FAILURE;
    }
    curValue = (curValue & ~mixerControlMask) | (value & mixerControlMask);
    ret = AudioCodecAiaoWriteReg(codec, mixerControl->reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Write AIAO codec reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&codec->mutex);

    ADM_LOG_DEBUG("Update AIAO codec reg bits successful.");
    return HDF_SUCCESS;
}

int32_t AudioUpdateAccessoryAiaoRegBits(struct AccessoryDevice *accessory,
    const struct AudioMixerControl *mixerControl, uint32_t value)
{
    int32_t ret;
    uint32_t curValue;
    uint32_t mixerControlMask;
    ADM_LOG_DEBUG("Entry to update AIAO accessory  reg bits.");

    if (accessory == NULL || mixerControl == NULL) {
        ADM_LOG_ERR("Invalid AIAO accessory param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << mixerControl->shift;
    mixerControlMask = mixerControl->mask << mixerControl->shift;

    OsalMutexLock(&accessory->mutex);
    ret = AudioAccessoryAiaoReadReg(accessory, mixerControl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&accessory->mutex);
        ADM_LOG_ERR("Read AIAO accessory reg fail ret=%d.", ret);
        return HDF_FAILURE;
    }
    curValue = (curValue & ~mixerControlMask) | (value & mixerControlMask);
    ret = AudioAccessoryAiaoWriteReg(accessory, mixerControl->reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&accessory->mutex);
        ADM_LOG_ERR("Write AIAO accessory reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&accessory->mutex);

    ADM_LOG_DEBUG("Update AIAO accessory reg bits successful.");
    return HDF_SUCCESS;
}

struct CodecDevice *AudioKcontrolGetCodec(const struct AudioKcontrol *kcontrol)
{
    struct AudioCard *audioCard = NULL;
    if (kcontrol == NULL || kcontrol->pri == NULL) {
        ADM_LOG_ERR("Input param kcontrol is NULL.");
        return NULL;
    }

    audioCard = (struct AudioCard *)((volatile uintptr_t)(kcontrol->pri));
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("Get codec or rtd fail.");
        return NULL;
    }

    return audioCard->rtd->codec;
}

struct AccessoryDevice *AudioKcontrolGetAccessory(const struct AudioKcontrol *kcontrol)
{
    struct AudioCard *audioCard = NULL;
    if (kcontrol == NULL || kcontrol->pri == NULL) {
        ADM_LOG_ERR("Input param kcontrol is NULL.");
        return NULL;
    }

    audioCard = (struct AudioCard *)((volatile uintptr_t)(kcontrol->pri));
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("Get accessory or rtd fail.");
        return NULL;
    }

    return audioCard->rtd->accessory;
}

struct AudioKcontrol *AudioAddControl(const struct AudioCard *audioCard, const struct AudioKcontrol *ctrl)
{
    struct AudioKcontrol *control = NULL;

    if ((audioCard == NULL) || (ctrl == NULL)) {
        ADM_LOG_ERR("Input params check error: audioCard=%p, ctrl=%p.", audioCard, ctrl);
        return NULL;
    }

    control = (struct AudioKcontrol *)OsalMemCalloc(sizeof(*control));
    if (control == NULL) {
        ADM_LOG_ERR("Malloc control fail!");
        return NULL;
    }
    control->name = ctrl->name;
    control->iface = ctrl->iface;
    control->Info = ctrl->Info;
    control->Get = ctrl->Get;
    control->Set = ctrl->Set;
    control->pri = (void *)audioCard;
    control->privateValue = ctrl->privateValue;

    return control;
}

int32_t AudioAddControls(struct AudioCard *audioCard, const struct AudioKcontrol *controls, int32_t controlMaxNum)
{
    struct AudioKcontrol *control = NULL;
    int32_t i;
    ADM_LOG_DEBUG("Entry.");

    if ((audioCard == NULL) || (controls == NULL) || (controlMaxNum <= 0)) {
        ADM_LOG_ERR("Input params check error: audioCard=%p, controls=%p, controlMaxNum=%d.",
            audioCard, controls, controlMaxNum);
        return HDF_FAILURE;
    }

    for (i = 0; i < controlMaxNum; i++) {
        control = AudioAddControl(audioCard, &controls[i]);
        if (control == NULL) {
            ADM_LOG_ERR("Add control fail!");
            return HDF_FAILURE;
        }
        DListInsertHead(&control->list, &audioCard->controls);
    }
    ADM_LOG_DEBUG("Success.");
    return HDF_SUCCESS;
}

int32_t AudioCodecReadReg(const struct CodecDevice *codec, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (codec == NULL || codec->devData == NULL || codec->devData->Read == NULL || val == NULL) {
        ADM_LOG_ERR("Input param codec is NULL.");
        return HDF_FAILURE;
    }
    ret = codec->devData->Read(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Codec device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioAccessoryReadReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (accessory == NULL || accessory->devData == NULL || accessory->devData->Read == NULL || val == NULL) {
        ADM_LOG_ERR("Input param accessory is NULL.");
        return HDF_FAILURE;
    }
    ret = accessory->devData->Read(accessory, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Accessory device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCodecAiaoReadReg(const struct CodecDevice *codec, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (codec == NULL || codec->devData == NULL || codec->devData->AiaoRead == NULL || val == NULL) {
        ADM_LOG_ERR("Input param codec is NULL.");
        return HDF_FAILURE;
    }
    ret = codec->devData->AiaoRead(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioAccessoryAiaoReadReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (accessory == NULL || accessory->devData == NULL || accessory->devData->AiaoRead == NULL || val == NULL) {
        ADM_LOG_ERR("Input param accessory is NULL.");
        return HDF_FAILURE;
    }
    ret = accessory->devData->AiaoRead(accessory, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCodecWriteReg(const struct CodecDevice *codec, uint32_t reg, uint32_t val)
{
    int32_t ret;
    if (codec == NULL || codec->devData == NULL || codec->devData->Write == NULL) {
        ADM_LOG_ERR("Input param codec is NULL.");
        return HDF_FAILURE;
    }
    ret = codec->devData->Write(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Codec device write fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioAccessoryWriteReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t val)
{
    int32_t ret;
    if (accessory == NULL || accessory->devData == NULL || accessory->devData->Write == NULL) {
        ADM_LOG_ERR("Input param accessory is NULL.");
        return HDF_FAILURE;
    }
    ret = accessory->devData->Write(accessory, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Accessory device write fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCodecAiaoWriteReg(struct CodecDevice *codec, uint32_t reg, uint32_t val)
{
    int32_t ret;
    if (codec == NULL || codec->devData == NULL || codec->devData->AiaoWrite == NULL) {
        ADM_LOG_ERR("Input param codec aiao is NULL.");
        return HDF_FAILURE;
    }
    ret = codec->devData->AiaoWrite(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Codec aiao device write fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioAccessoryAiaoWriteReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t val)
{
    int32_t ret;
    if (accessory == NULL || accessory->devData == NULL || accessory->devData->AiaoWrite == NULL) {
        ADM_LOG_ERR("Input param accessory aiao is NULL.");
        return HDF_FAILURE;
    }
    ret = accessory->devData->AiaoWrite(accessory, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Accessory aiao device write fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioInfoCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemInfo *elemInfo)
{
    struct AudioMixerControl *mixerCtrl = NULL;

    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemInfo == NULL) {
        ADM_LOG_ERR("Input param kcontrol is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    elemInfo->count = CHANNEL_MIN_NUM;
    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)(kcontrol->privateValue));
    /* stereo */
    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        elemInfo->count = CHANNEL_MAX_NUM;
    }
    elemInfo->type = 1; /* volume type */
    elemInfo->min = mixerCtrl->min;
    elemInfo->max = mixerCtrl->max;

    return HDF_SUCCESS;
}

static int32_t AudioGetCtrlOpsRReg(struct AudioCtrlElemValue *elemValue,
    const struct AudioMixerControl *mixerCtrl, uint32_t rcurValue)
{
    if (elemValue == NULL || mixerCtrl == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        if (mixerCtrl->reg == mixerCtrl->rreg) {
            rcurValue = (rcurValue >> mixerCtrl->rshift) & mixerCtrl->mask;
        } else {
            rcurValue = (rcurValue >> mixerCtrl->shift) & mixerCtrl->mask;
        }
        if (rcurValue > mixerCtrl->max || rcurValue < mixerCtrl->min) {
            ADM_LOG_ERR("Audio invalid rcurValue=%d", rcurValue);
            return HDF_FAILURE;
        }
        if (mixerCtrl->invert) {
            rcurValue = mixerCtrl->max - rcurValue;
        }
        elemValue->value[1] = rcurValue;
    }

    return HDF_SUCCESS;
}

static int32_t AudioGetCtrlOpsReg(struct AudioCtrlElemValue *elemValue,
    const struct AudioMixerControl *mixerCtrl, uint32_t curValue)
{
    if (elemValue == NULL || mixerCtrl == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    curValue = (curValue >> mixerCtrl->shift) & mixerCtrl->mask;
    if (curValue > mixerCtrl->max || curValue < mixerCtrl->min) {
        ADM_LOG_ERR("Audio invalid curValue=%d", curValue);
        return HDF_FAILURE;
    }
    if (mixerCtrl->invert) {
        curValue = mixerCtrl->max - curValue;
    }
    elemValue->value[0] = curValue;

    return HDF_SUCCESS;
}

int32_t AudioCodecGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    uint32_t curValue;
    uint32_t rcurValue;
    struct AudioMixerControl *mixerCtrl = NULL;
    struct CodecDevice *codec = NULL;
    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    codec = AudioKcontrolGetCodec(kcontrol);
    if (mixerCtrl == NULL || codec == NULL) {
        ADM_LOG_ERR("mixerCtrl and codec is NULL.");
        return HDF_FAILURE;
    }
    if (AudioCodecReadReg(codec, mixerCtrl->reg, &curValue) != HDF_SUCCESS ||
        AudioCodecReadReg(codec, mixerCtrl->rreg, &rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Read Reg fail.");
        return HDF_FAILURE;
    }
    if (AudioGetCtrlOpsReg(elemValue, mixerCtrl, curValue) != HDF_SUCCESS ||
        AudioGetCtrlOpsRReg(elemValue, mixerCtrl, rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio codec get kcontrol reg and rreg fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioAccessoryGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    uint32_t curValue;
    uint32_t rcurValue;
    struct AudioMixerControl *mixerCtrl = NULL;
    struct AccessoryDevice *accessory = NULL;
    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    accessory = AudioKcontrolGetAccessory(kcontrol);
    if (mixerCtrl == NULL || accessory == NULL) {
        ADM_LOG_ERR("mixerCtrl and accessory is NULL.");
        return HDF_FAILURE;
    }
    if (AudioAccessoryReadReg(accessory, mixerCtrl->reg, &curValue) != HDF_SUCCESS ||
        AudioAccessoryReadReg(accessory, mixerCtrl->rreg, &rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Read Reg fail.");
        return HDF_FAILURE;
    }
    if (AudioGetCtrlOpsReg(elemValue, mixerCtrl, curValue) != HDF_SUCCESS ||
        AudioGetCtrlOpsRReg(elemValue, mixerCtrl, rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio accessory get kcontrol reg and rreg fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioCodecAiaoGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    uint32_t curValue;
    uint32_t rcurValue;
    struct CodecDevice *codec = NULL;
    struct AudioMixerControl *mixerCtrl  = NULL;
    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("CODECAIAO input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = AudioKcontrolGetCodec(kcontrol);
    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    if (mixerCtrl == NULL || codec == NULL) {
        ADM_LOG_ERR("mixerCtrl and codec is NULL.");
        return HDF_FAILURE;
    }
    if (AudioCodecAiaoReadReg(codec, mixerCtrl->reg, &curValue) != HDF_SUCCESS ||
        AudioCodecAiaoReadReg(codec, mixerCtrl->rreg, &rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Read Reg fail.");
        return HDF_FAILURE;
    }
    if (AudioGetCtrlOpsReg(elemValue, mixerCtrl, curValue) != HDF_SUCCESS ||
        AudioGetCtrlOpsRReg(elemValue, mixerCtrl, rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio CODECAIAO get kcontrol reg and rreg fail.");
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("Get CODECAIAO ctrl switch successful.");
    return HDF_SUCCESS;
}

int32_t AudioAccessoryAiaoGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    uint32_t curValue;
    uint32_t rcurValue;
    struct AccessoryDevice *accessory = NULL;
    struct AudioMixerControl *mixerCtrl  = NULL;
    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("ACCESSORYCAIAO input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    accessory = AudioKcontrolGetAccessory(kcontrol);
    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    if (mixerCtrl == NULL || accessory == NULL) {
        ADM_LOG_ERR("mixerCtrl and accessory is NULL.");
        return HDF_FAILURE;
    }
    if (AudioAccessoryAiaoReadReg(accessory, mixerCtrl->reg, &curValue) != HDF_SUCCESS ||
        AudioAccessoryAiaoReadReg(accessory, mixerCtrl->rreg, &rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Read Reg fail.");
        return HDF_FAILURE;
    }
    if (AudioGetCtrlOpsReg(elemValue, mixerCtrl, curValue) != HDF_SUCCESS ||
        AudioGetCtrlOpsRReg(elemValue, mixerCtrl, rcurValue) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio ACCESSORYCAIAO get kcontrol reg and rreg fail.");
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("Get ACCESSORYCAIAO ctrl switch successful.");
    return HDF_SUCCESS;
}

static int32_t AudioSetCtrlOpsReg(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue,
    const struct AudioMixerControl *mixerCtrl, uint32_t *value)
{
    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL ||
        mixerCtrl == NULL || value == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    *value = elemValue->value[0];
    if (*value < mixerCtrl->min || *value > mixerCtrl->max) {
        ADM_LOG_ERR("Audio invalid value=%d", *value);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (mixerCtrl->invert) {
        *value = mixerCtrl->max - *value;
    }

    return HDF_SUCCESS;
}

static int32_t AudioSetCtrlOpsRReg(const struct AudioCtrlElemValue *elemValue,
    struct AudioMixerControl *mixerCtrl, uint32_t *rvalue, bool *updateRReg)
{
    uint32_t rshift;
    if (elemValue == NULL || mixerCtrl == NULL || rvalue == NULL || updateRReg == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        *updateRReg = true;
        *rvalue = elemValue->value[1];
        if (*rvalue < mixerCtrl->min || *rvalue > mixerCtrl->max) {
            ADM_LOG_ERR("Audio invalid fail.");
            return HDF_FAILURE;
        }
        if (mixerCtrl->invert) {
            *rvalue = mixerCtrl->max - *rvalue;
        }
        if (mixerCtrl->reg == mixerCtrl->rreg) {
            rshift = mixerCtrl->rshift;
        } else {
            rshift = mixerCtrl->shift;
        }
        mixerCtrl->shift = rshift;
    }

    return HDF_SUCCESS;
}

int32_t AudioCodecSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue)
{
    uint32_t value;
    uint32_t rvalue;
    bool updateRReg = false;
    struct CodecDevice *codec = NULL;
    struct AudioMixerControl *mixerCtrl = NULL;
    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    if (AudioSetCtrlOpsReg(kcontrol, elemValue, mixerCtrl, &value) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsReg is fail.");
        return HDF_ERR_INVALID_OBJECT;
    }
    codec = AudioKcontrolGetCodec(kcontrol);
    if (codec == NULL) {
        ADM_LOG_ERR("AudioKcontrolGetCodec is fail.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (AudioUpdateCodecRegBits(codec, mixerCtrl, value) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio codec stereo update reg bits fail.");
        return HDF_FAILURE;
    }

    if (AudioSetCtrlOpsRReg(elemValue, mixerCtrl, &rvalue, &updateRReg) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsRReg is fail.");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (updateRReg) {
        if (AudioUpdateCodecRegBits(codec, mixerCtrl, rvalue) != HDF_SUCCESS) {
            ADM_LOG_ERR("Audio codec stereo update reg bits fail.");
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t AudioAccessorySetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue)
{
    uint32_t value;
    uint32_t rvalue;
    bool updateRReg = false;
    struct AccessoryDevice *accessory = NULL;
    struct AudioMixerControl *mixerCtrl = NULL;
    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    if (AudioSetCtrlOpsReg(kcontrol, elemValue, mixerCtrl, &value) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsReg fail.");
        return HDF_FAILURE;
    }

    accessory = AudioKcontrolGetAccessory(kcontrol);
    if (AudioUpdateAccessoryRegBits(accessory, mixerCtrl, value) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio accessory stereo update reg bits fail.");
        return HDF_FAILURE;
    }

    if (AudioSetCtrlOpsRReg(elemValue, mixerCtrl, &rvalue, &updateRReg) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsRReg fail.");
        return HDF_FAILURE;
    }

    if (updateRReg) {
        if (AudioUpdateAccessoryRegBits(accessory, mixerCtrl, rvalue) != HDF_SUCCESS) {
            ADM_LOG_ERR("Audio accessory stereo update reg bits fail.");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioCodecAiaoSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue)
{
    uint32_t value;
    uint32_t rvalue;
    bool updateRReg = false;
    struct CodecDevice *codec = NULL;
    struct AudioMixerControl *mixerCtrl = NULL;
    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL) {
        ADM_LOG_ERR("Audio codec aiao input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    if (AudioSetCtrlOpsReg(kcontrol, elemValue, mixerCtrl, &value) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsReg fail.");
        return HDF_FAILURE;
    }

    codec = AudioKcontrolGetCodec(kcontrol);
    if (AudioUpdateCodecAiaoRegBits(codec, mixerCtrl, value) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio codec aiao stereo update reg bits fail.");
        return HDF_FAILURE;
    }

    if (AudioSetCtrlOpsRReg(elemValue, mixerCtrl, &rvalue, &updateRReg) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsRReg fail.");
        return HDF_FAILURE;
    }
    if (updateRReg) {
        if (AudioUpdateCodecAiaoRegBits(codec, mixerCtrl, rvalue) != HDF_SUCCESS) {
            ADM_LOG_ERR("Audio codec aiao stereo update reg bits fail.");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioAccessoryAiaoSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue)
{
    uint32_t value;
    uint32_t rvalue;
    bool updateRReg = false;
    struct AccessoryDevice *accessory = NULL;
    struct AudioMixerControl *mixerCtrl = NULL;
    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL) {
        ADM_LOG_ERR("Audio accessory aiao input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    mixerCtrl = (struct AudioMixerControl *)((volatile uintptr_t)kcontrol->privateValue);
    if (AudioSetCtrlOpsReg(kcontrol, elemValue, mixerCtrl, &value) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsReg fail.");
        return HDF_FAILURE;
    }

    accessory = AudioKcontrolGetAccessory(kcontrol);
    if (AudioUpdateAccessoryAiaoRegBits(accessory, mixerCtrl, value) != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio accessory aiao stereo update reg bits fail.");
        return HDF_FAILURE;
    }

    if (AudioSetCtrlOpsRReg(elemValue, mixerCtrl, &rvalue, &updateRReg) != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioSetCtrlOpsRReg fail.");
        return HDF_FAILURE;
    }

    if (updateRReg) {
        if (AudioUpdateAccessoryAiaoRegBits(accessory, mixerCtrl, rvalue) != HDF_SUCCESS) {
            ADM_LOG_ERR("Audio accessory aiao stereo update reg bits fail.");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}
