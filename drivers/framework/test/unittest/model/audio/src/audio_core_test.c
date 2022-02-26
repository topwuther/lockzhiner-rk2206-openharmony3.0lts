/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_core_test.h"
#include "audio_core.h"

#define HDF_LOG_TAG audio_core_test

int32_t AudioSocTestRegisterDai(void)
{
    int32_t ret;
    struct HdfDeviceObject *device = NULL;
    struct DaiData *data = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSocRegisterDai(device, data);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioSocRegisterDai fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioSocTestRegisterPlatform(void)
{
    int32_t ret;
    struct HdfDeviceObject *device = NULL;
    struct PlatformData *data = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSocRegisterPlatform(device, data);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioSocRegisterPlatform fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestRegisterCodec(void)
{
    int32_t ret;
    struct HdfDeviceObject *device = NULL;
    struct CodecData *codecData = NULL;
    struct DaiData *daiData = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioRegisterCodec(device, codecData, daiData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioRegisterCodec fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestBindDaiLink(void)
{
    int32_t ret;
    struct AudioCard *audioCard = NULL;
    struct AudioConfigData *configData = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioBindDaiLink(audioCard, configData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioBindDaiLink fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestSocDeviceRegister(void)
{
    int32_t ret;
    void *data = NULL;
    enum AudioDeviceType deviceType;
    struct HdfDeviceObject *device = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSocDeviceRegister(device, data, deviceType);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioSocDeviceRegister fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestSocRegisterDsp(void)
{
    int32_t ret;
    struct HdfDeviceObject *device = NULL;
    struct DaiData *data = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSocRegisterDsp(device, data);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioSocRegisterDsp fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestRegisterAccessory(void)
{
    int32_t ret;
    struct HdfDeviceObject *device = NULL;
    struct AccessoryData *data = NULL;
    struct DaiData *daiData = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioRegisterAccessory(device, data, daiData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioRegisterAccessory fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestUpdateRegBits(void)
{
    int32_t codecRet;
    int32_t accessoryRet;
    int32_t value;
    void *device = NULL;
    struct AudioMixerControl *mixerControl = NULL;
    HDF_LOGI("%s: enter", __func__);

    codecRet = AudioUpdateRegBits(AUDIO_CODEC_DEVICE, device, mixerControl, value);
    accessoryRet = AudioUpdateRegBits(AUDIO_ACCESSORY_DEVICE, device, mixerControl, value);
    if (codecRet != HDF_SUCCESS || accessoryRet != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioUpdateRegBits fail codecRet = %d, accessoryRet = %d", __func__, codecRet, accessoryRet);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAiaoUpdateRegBits(void)
{
    uint32_t reg;
    uint32_t mask;
    uint32_t shift;
    int32_t value;
    int32_t ret;
    struct CodecDevice *codec = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioAiaoUpdateRegBits(codec, reg, mask, shift, value);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioAiaoUpdateRegBits fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestKcontrolGetCodec(void)
{
    struct CodecDevice *codecDevice = NULL;
    const struct AudioKcontrol *kcontrol = NULL;
    HDF_LOGI("%s: enter", __func__);

    codecDevice = AudioKcontrolGetCodec(kcontrol);
    if (codecDevice == NULL) {
        HDF_LOGE("%s: AudioKcontrolGetCodec fail!", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAddControls(void)
{
    struct AudioCard *audioCard = NULL;
    const struct AudioKcontrol *controls = NULL;
    int32_t controlMaxNum = 0;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioAddControls(audioCard, controls, controlMaxNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioAddControls fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAddControl(void)
{
    struct AudioCard *audioCard = NULL;
    const struct AudioKcontrol *control = NULL;
    struct AudioKcontrol *audioKcontrol = NULL;
    HDF_LOGI("%s: enter", __func__);

    audioKcontrol = AudioAddControl(audioCard, control);
    if (audioKcontrol == NULL) {
        HDF_LOGE("%s: AudioAddControl fail!", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestDeviceReadReg(void)
{
    int32_t codecRet;
    int32_t acccessoryRet;
    uint32_t reg = 0;
    struct CodecDevice *codec = NULL;
    struct AccessoryDevice *accessory = NULL;
    HDF_LOGI("%s: enter", __func__);

    codecRet = AudioCodecDeviceReadReg(codec, reg);
    acccessoryRet = AudioAccessoryDeviceReadReg(accessory, reg);
    if (codecRet != HDF_SUCCESS || acccessoryRet != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioDeviceReadReg fail codecRet = %d, acccessoryRet = %d", __func__, codecRet, acccessoryRet);
    }
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAiaoDeviceReadReg(void)
{
    int32_t ret;
    uint32_t reg = 0;
    struct CodecDevice *codec = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioAiaoDeviceReadReg(codec, reg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioAiaoDeviceReadReg fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestInfoCtrlSw(void)
{
    int32_t ret;
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemInfo *elemInfo = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioInfoCtrlSw(kcontrol, elemInfo);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioInfoCtrlSw fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestGetCtrlSw(void)
{
    int32_t ret;
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioGetCtrlSw(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioGetCtrlSw fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestPutCtrlSw(void)
{
    int32_t ret;
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioPutCtrlSw(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AudioPutCtrlSw fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AiaoTestGetCtrlSw(void)
{
    int32_t ret;
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AiaoGetCtrlSw(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AiaoGetCtrlSw fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AiaoTestPutCtrlSw(void)
{
    int32_t ret;
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AiaoPutCtrlSw(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: AiaoPutCtrlSw fail ret = %d", __func__, ret);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}
