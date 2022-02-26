/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_core.h"
#include "audio_accessory_base.h"
#include "audio_dsp_if.h"
#include "gpio_if.h"
#include "osal_io.h"
#include "spi_if.h"

#define DEFAULT_SPEED 2000000
#define BITS_PER_WORD_EIGHT 8
#define DSP_CS_NUM 1
#define DSP_SPI_BUS_NUM 1

#define HDF_LOG_TAG dsp_adapter

static int32_t DspLinkDeviceInit(const struct AudioCard *card, const struct DaiDevice *device);
static int32_t DspDeviceInit(const struct DspDevice *device);
static int32_t DspDeviceReadReg(struct DspDevice *device, uint8_t *buf, uint32_t len);
static int32_t DspDeviceWriteReg(struct DspDevice *device, uint8_t *buf, uint32_t len);
static int32_t DspLinkStartup(const struct AudioCard *card, const struct DaiDevice *device);
static int32_t DspLinkHwParams(const struct AudioCard *card,
    const struct AudioPcmHwParams *param, const struct DaiDevice *device);
static int32_t DspDecodeAudioStream(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device);
static int32_t DspEncodeAudioStream(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device);
static int32_t DspEqualizerActive(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device);

struct DspData g_dspData = {
    .DspInit = DspDeviceInit,
    .Read = DspDeviceReadReg,
    .Write = DspDeviceWriteReg,
    .decode = DspDecodeAudioStream,
    .encode = DspEncodeAudioStream,
    .Equalizer = DspEqualizerActive,
};

struct AudioDaiOps g_dspLinkDeviceOps = {
    .Startup = DspLinkStartup,
    .HwParams = DspLinkHwParams,
};

struct DaiData g_dspDaiData = {
    .DaiInit = DspLinkDeviceInit,
    .ops = &g_dspLinkDeviceOps,
};

struct SpiDevInfo g_devInfo = {
    .busNum = DSP_SPI_BUS_NUM,
    .csNum = DSP_CS_NUM,
};

static int32_t DspLinkStartup(const struct AudioCard *card, const struct DaiDevice *device)
{
    (void)card;
    (void)device;
    return HDF_SUCCESS;
}

static int32_t DspCfgI2sFrequency(uint32_t rate, uint16_t *frequency)
{
    switch (rate) {
        case I2S_SAMPLE_FREQUENCY_8000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_8000;
            break;
        case I2S_SAMPLE_FREQUENCY_11025:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_11025;
            break;
        case I2S_SAMPLE_FREQUENCY_12000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_12000;
            break;
        case I2S_SAMPLE_FREQUENCY_16000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_16000;
            break;
        case I2S_SAMPLE_FREQUENCY_22050:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_22050;
            break;
        case I2S_SAMPLE_FREQUENCY_24000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_24000;
            break;
        case I2S_SAMPLE_FREQUENCY_32000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_32000;
            break;
        case I2S_SAMPLE_FREQUENCY_44100:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_44100;
            break;
        case I2S_SAMPLE_FREQUENCY_48000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_48000;
            break;
        case I2S_SAMPLE_FREQUENCY_64000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_64000;
            break;
        case I2S_SAMPLE_FREQUENCY_88200:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_88200;
            break;
        case I2S_SAMPLE_FREQUENCY_96000:
            *frequency = I2S_SAMPLE_FREQUENCY_REG_VAL_96000;
            break;
        default:
            AUDIO_DRIVER_LOG_ERR("rate: %d is not support.", rate);
            return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

static int32_t DspSetI2sBitWidth(enum AudioFormat format, uint16_t *bitWidth)
{
    switch (format) {
        case AUDIO_FORMAT_PCM_8_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        case AUDIO_FORMAT_PCM_24_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        default:
            AUDIO_DRIVER_LOG_ERR("format: %d is not support.", format);
            return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

static int DspSetI2sFrequency(uint16_t frequencyVal)
{
    return HDF_SUCCESS;
}

static int DspSetI2sFormat(uint16_t formatVal)
{
    return HDF_SUCCESS;
}

static int32_t DspLinkHwParams(const struct AudioCard *card,
    const struct AudioPcmHwParams *param, const struct DaiDevice *device)
{
    int ret;
    uint16_t frequency, bitWidth;
    (void)card;
    (void)device;
    AUDIO_DRIVER_LOG_DEBUG("entry.");
    if (param == NULL ||  param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is nullptr.");
        return HDF_ERR_INVALID_PARAM;
    }
    ret = DspCfgI2sFrequency(param->rate, &frequency);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("RateToFrequency fail.");
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = DspSetI2sBitWidth(param->format, &bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("FormatToBitWidth fail.");
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = DspSetI2sFrequency(frequency);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("SetDspI2sFs fail.");
        return HDF_FAILURE;
    }
    ret = DspSetI2sFormat(bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("SetDspI2sFormat fail.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("DspLinkHwParams: channels = %d, rate = %d, periodSize = %d, \
        periodCount = %d, format = %d, cardServiceName = %s \n",
        param->channels, param->rate, param->periodSize,
        param->periodCount, (uint32_t)param->format, param->cardServiceName);
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static int DspPowerEnable(void)
{
    return HDF_SUCCESS;
}

static int DspGpioPinInit(void)
{
    return HDF_SUCCESS;
}

static int DspI2cPinInit(void)
{
    return HDF_SUCCESS;
}

static int DspI2sInit(void)
{
    return HDF_SUCCESS;
}

static int DspI2cInit(void)
{
    return HDF_SUCCESS;
}

/* not init dsp gpio */
static int DspSpiPinInit(void)
{
    return HDF_FAILURE;
}

static int32_t DspDeviceInit(const struct DspDevice *device)
{
    DevHandle devHandle;
    struct SpiCfg devCfg = {
        .maxSpeedHz = DEFAULT_SPEED,
        .mode = SPI_CLK_POLARITY,
        .transferMode = SPI_DMA_TRANSFER,
        .bitsPerWord = BITS_PER_WORD_EIGHT,
    };

    if (DspPowerEnable() != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DspPowerEnable: return Error!");
        return HDF_FAILURE;
    }

    if (DspGpioPinInit() != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DspGpioPinInit: return Error!");
        return HDF_FAILURE;
    }

    if (DspI2cPinInit() != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DspI2cPinInit: return Error!");
        return HDF_FAILURE;
    }

    if (DspSpiPinInit() == HDF_SUCCESS) {
        devHandle = SpiOpen(&g_devInfo);
        if (devHandle == NULL) {
            AUDIO_DRIVER_LOG_ERR("DspDeviceOpen: Spi failed!");
            return HDF_FAILURE;
        }

        if (SpiSetCfg(devHandle, &devCfg) != HDF_SUCCESS) {
            SpiClose(devHandle);
            AUDIO_DRIVER_LOG_ERR("DspDeviceCfg: spi failed!");
            return HDF_FAILURE;
        }
        SpiClose(devHandle);
    } else {
        AUDIO_DRIVER_LOG_ERR("Dsp Gpio Pin: not init!");
    }

    if (DspI2cInit() != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (DspI2sInit() != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DspDeviceReadReg(struct DspDevice *device, uint8_t *buf, uint32_t len)
{
    int32_t ret;

    DevHandle devHandle = SpiOpen(&g_devInfo);
    if (devHandle == NULL) {
        AUDIO_DRIVER_LOG_ERR("DspDeviceOpen: Spi failed!");
        return HDF_FAILURE;
    }

    ret = SpiRead(devHandle, buf, len);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DspDeviceRead: spi failed!");
        SpiClose(devHandle);
        return HDF_FAILURE;
    }

    SpiClose(devHandle);

    return HDF_SUCCESS;
}

static int32_t DspDeviceWriteReg(struct DspDevice *device, uint8_t *buf, uint32_t len)
{
    int32_t ret;

    DevHandle devHandle = SpiOpen(&g_devInfo);
    if (devHandle == NULL) {
        AUDIO_DRIVER_LOG_ERR("DspDeviceOpen: Spi failed!");
        return HDF_FAILURE;
    }

    ret = SpiWrite(devHandle, buf, len);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DspDeviceRead: spi failed!");
        SpiClose(devHandle);
        return HDF_FAILURE;
    }

    SpiClose(devHandle);

    return HDF_SUCCESS;
}

static int32_t DspLinkDeviceInit(const struct AudioCard *card, const struct DaiDevice *device)
{
    if (device == NULL || device->devDaiName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is nullptr.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("dsp Link device name: %s\n", device->devDaiName);
    (void)card;
    return HDF_SUCCESS;
}

static int32_t DspDriverBind(struct HdfDeviceObject *device)
{
    struct AudioHost *audioHost = NULL;

    AUDIO_DRIVER_LOG_DEBUG("entry.");
    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    audioHost = AudioHostCreateAndBind(device);
    if (audioHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("audioHost create failed!");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_INFO("success!");
    return HDF_SUCCESS;
}

static int32_t DspGetServiceName(const struct HdfDeviceObject *device, const char **drvDspName)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("deivce property is NULL.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("from resource get drsops failed!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", drvDspName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read DspServiceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t DspGetDaiName(const struct HdfDeviceObject *device, const char **drvDaiName)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is null pointer.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("drs node is null pointer.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("drs ops fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "dspDaiName", drvDaiName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read dspDaiName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t DspDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    AUDIO_DRIVER_LOG_DEBUG("entry.\n");
    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = DspGetServiceName(device, &g_dspData.drvDspName);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get dsp service name fail.");
        return ret;
    }

    ret = DspGetDaiName(device, &g_dspDaiData.drvDaiName);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get dsp dai name fail.");
        return ret;
    }

    ret = AudioRegisterDsp(device, &g_dspData, &g_dspDaiData);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }
    AUDIO_DRIVER_LOG_INFO("success!");
    return HDF_SUCCESS;
}


static void DspDriverRelease(struct HdfDeviceObject *device)
{
    struct DspHost *dspHost = NULL;
    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL");
        return;
    }

    dspHost = (struct DspHost *)device->service;
    if (dspHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("DspHost is NULL");
        return;
    }

    OsalIoUnmap((void *)((uintptr_t)(void*)&dspHost->priv));
    OsalMemFree(dspHost);
}

static int32_t DspEqualizerActive(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device)
{
    (void)card;
    (void)buf;
    (void)device;
    AUDIO_DRIVER_LOG_DEBUG("equalizer run!!!");
    return HDF_SUCCESS;
}

static int32_t DspDecodeAudioStream(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device)
{
    (void)card;
    (void)buf;
    (void)device;
    AUDIO_DRIVER_LOG_DEBUG("decode run!!!");
    return HDF_SUCCESS;
}

static int32_t DspEncodeAudioStream(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device)
{
    (void)card;
    (void)buf;
    (void)device;
    AUDIO_DRIVER_LOG_DEBUG("encode run!!!");
    return HDF_SUCCESS;
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_dspDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "DSP",
    .Bind = DspDriverBind,
    .Init = DspDriverInit,
    .Release = DspDriverRelease,
};
HDF_INIT(g_dspDriverEntry);
