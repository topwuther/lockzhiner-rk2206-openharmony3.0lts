/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

#include <stdint.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "device_resource_if.h"
#include "osal.h"
#include "adc_core.h"
#include "adc_if.h"

#include "lz_hardware.h"

#define HDF_LOG_TAG                 adc_driver_rk2206

/* ADC采集位长 */
#define HDF_ADC_DRIVER_DATA_WIDTH   10

/* ADC引脚的范围 */
#define HDF_ADC_DRIVER_GPIO_MIN     (GPIO0_PC0)
#define HDF_ADC_DRIVER_GPIO_MAX     (GPIO0_PC7)

/* ADC的通道范围 */
#define HDF_ADC_DRIVER_CHANNEL_MIN  (0)
#define HDF_ADC_DRIVER_CHANNEL_MAX  (7)

typedef enum tag_hdf_adc_driver_select_voltage {
    HDF_ADC_DRIVER_SELECT_VOLTAGE_AVDD = 0,
    HDF_ADC_DRIVER_SELECT_VOLTAGE_INERNAL_REFERENCE_VOLTAGE,
    HDF_ADC_DRIVER_SELECT_VOLTAGE_MAX
} HDF_ADC_DRV_SELECT_VOL_E;

typedef struct tag_hcs_info {
    uint32_t deviceNum;     // 设备号
    uint32_t dataWidth;     // ADC采集位长
    uint32_t gpio;          // gpio引脚序号
    uint32_t channelId;     // 采集通道号
    uint32_t selectVoltage; // ADC参考电压信号
}hcs_info_s;

typedef struct tag_adc_info {
    struct AdcDevice device;
    hcs_info_s hcs_info;
}adc_info_s;

////////////////////////////////////////////////////////

static inline int32_t HdfAdcDriverStart(struct AdcDevice *device)
{
    if ((device == NULL) || (device->priv == NULL))
    {
         HDF_LOGE("%s, %d: device or priv is null", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    LzSaradcInit();

    return HDF_SUCCESS;
}

static inline int32_t HdfAdcDriverStop(struct AdcDevice *device)
{
    if ((device == NULL) || (device->priv == NULL))
    {
        HDF_LOGE("%s, %d: device or priv is null", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    LzSaradcDeinit();

    return HDF_SUCCESS;
}

static inline uint32_t HdfAdcDriverRead(struct AdcDevice *device, uint32_t channel, uint32_t *val)
{
    adc_info_s *padc_info = NULL;
    int ret = 0;
    unsigned int uiValue = 0;
    
    if ((device == NULL) || (device->priv == NULL))
    {
        HDF_LOGE("%s, %d: device or priv is null", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    padc_info = (adc_info_s *)device->priv;
    if (padc_info == NULL)
    {
        HDF_LOGE("%s, %d: padc_info is null", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (channel != padc_info->hcs_info.channelId)
    {
        HDF_LOGE("%s, %d: channel(%d) error!", __func__, __LINE__, channel);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = LzSaradcReadValue(channel, &uiValue);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        HDF_LOGE("%s, %d: LzSaradcReadValue error(%d)", __func__, __LINE__, ret);
        return HDF_ERR_IO;
    }

    *val = (uint32_t)(uiValue);

    return HDF_SUCCESS;
}

static struct AdcMethod g_adcMethod = {
    .read = HdfAdcDriverRead,
    .stop = HdfAdcDriverStop,
    .start = HdfAdcDriverStart,
};

static int32_t HdfAdcDriverReadHcs(adc_info_s *padc_info, struct DeviceResourceNode *node)
{
    int32_t ret;
    struct DeviceResourceIface *iface = NULL;
    hcs_info_s *phcs_info = NULL;

    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if ((iface == NULL) || (iface->GetUint32 == NULL))
    {
        HDF_LOGE("%s, %d: invalid iface", __func__, __LINE__);
        return HDF_ERR_NOT_SUPPORT;
    }

    phcs_info = &padc_info->hcs_info;

    ret = iface->GetUint32(node, "deviceNum", &phcs_info->deviceNum, 0);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: read hcs deviceNum failed", __func__, __LINE__);
        return ret;
    }

    ret = iface->GetUint32(node, "dataWidth", &phcs_info->dataWidth, 0);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: read hcs dataWidth failed", __func__, __LINE__);
        return ret;
    }

    ret = iface->GetUint32(node, "gpio", &phcs_info->gpio, 0);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: read hcs gpio failed", __func__, __LINE__);
        return ret;
    }
    if ((phcs_info->gpio < HDF_ADC_DRIVER_GPIO_MIN) || (HDF_ADC_DRIVER_GPIO_MAX < phcs_info->gpio))
    {
        HDF_LOGE("%s, %d: gpio(%d) is out of the range!", __func__, __LINE__, phcs_info->gpio);
        return HDF_ERR_NOT_SUPPORT;
    }

    ret = iface->GetUint32(node, "channelId", &phcs_info->channelId, 0);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: read hcs channelId failed", __func__, __LINE__);
        return ret;
    }
    if ((phcs_info->channelId < HDF_ADC_DRIVER_CHANNEL_MIN) || (HDF_ADC_DRIVER_CHANNEL_MAX < phcs_info->channelId))
    {
        HDF_LOGE("%s, %d: channelId(%d) is out of the range!", __func__, __LINE__, phcs_info->channelId);
        return HDF_ERR_NOT_SUPPORT;
    }

    ret = iface->GetUint32(node, "selectVoltage", &phcs_info->selectVoltage, 0);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: read hcs selectVoltage failed", __func__, __LINE__);
        return ret;
    }
    if (phcs_info->selectVoltage >= HDF_ADC_DRIVER_SELECT_VOLTAGE_MAX)
    {
        HDF_LOGE("%s, %d: selectVoltage(%d) is out of the range!", __func__, __LINE__, phcs_info->selectVoltage);
        return HDF_ERR_NOT_SUPPORT;
    }
    
    return HDF_SUCCESS;
}

static uint32_t HdfAdcDriverInitSRADC(hcs_info_s hcs_info)
{
    DevIo adc_devio = {
        .isr =   {.gpio = INVALID_GPIO},
        .rst =   {.gpio = INVALID_GPIO},
        .ctrl1 = {
            .gpio = GPIO0_PC0,
            .func = MUX_FUNC1,
            .type = PULL_NONE,
            .drv = DRIVE_KEEP,
            .dir = LZGPIO_DIR_IN,
            .val = LZGPIO_LEVEL_KEEP
        },
        .ctrl2 = {.gpio = INVALID_GPIO},
    };

    adc_devio.ctrl1.gpio = hcs_info.gpio;

    DevIoInit(adc_devio);

    return HDF_SUCCESS;
}

static uint32_t HdfAdcDriverDeinitSRADC(hcs_info_s hcs_info)
{
    return HDF_SUCCESS;
}

static int32_t HdfAdcDriverParseInit(struct HdfDeviceObject *device, struct DeviceResourceNode *node)
{
    int32_t ret;
    adc_info_s *padc_info = NULL;

    padc_info = (adc_info_s *)OsalMemCalloc(sizeof(adc_info_s));
    if (padc_info == NULL)
    {
        HDF_LOGE("%s, %d: OsalMemCalloc failed", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = HdfAdcDriverReadHcs(padc_info, node);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: HdfAdcDriverReadHcs failed(%d)", __func__, __LINE__, ret);
        goto err_read_hcs;
    }

    ret = HdfAdcDriverInitSRADC(padc_info->hcs_info);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: HdfAdcDriverInitSRADC failed(%d)", __func__, __LINE__, ret);
        goto err_init_sradc;
    }

    padc_info->device.priv = (void *)padc_info;
    padc_info->device.devNum = padc_info->hcs_info.deviceNum;
    padc_info->device.chanNum = padc_info->hcs_info.channelId;
    padc_info->device.ops = &g_adcMethod;
    
    ret = AdcDeviceAdd(&padc_info->device);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: HdfAdcDriverInitSRADC failed(%d)", __func__, __LINE__, ret);
        goto err_adc_device_add;
    }

    return HDF_SUCCESS;

err_adc_device_add:
    HdfAdcDriverDeinitSRADC(padc_info->hcs_info);
err_init_sradc:
err_read_hcs:
    if (padc_info != NULL)
    {
        OsalMemFree(padc_info);
    }
    return ret;
}

static int32_t HdfAdcDriverParseDeinit(struct HdfDeviceObject *device)
{
    int32_t ret;
    adc_info_s *padc_info = NULL;

    if (device == NULL)
    {
        HDF_LOGE("%s, %d: device is null", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    padc_info = (adc_info_s *)device->priv;
    if (padc_info == NULL)
    {
        HDF_LOGE("%s, %d: padc_info is null", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    HdfAdcDriverDeinitSRADC(padc_info->hcs_info);
    
    if (padc_info != NULL)
    {
        OsalMemFree(padc_info);
    }

    return HDF_SUCCESS;
}


////////////////////////////////////////////////////////
static int32_t HdfAdcDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct DeviceResourceNode *childNode = NULL;

    printf("%s, %d: Hdf Adc Driver Entry!\n", __FILE__, __LINE__);
    HDF_LOGI("%s: Enter", __func__);
    if ((device == NULL) || (device->property == NULL))
    {
        HDF_LOGE("%s, %d: device or property is null", __func__, __FILE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = HDF_SUCCESS;
    DEV_RES_NODE_FOR_EACH_CHILD_NODE(device->property, childNode) {
        ret = HdfAdcDriverParseInit(device, childNode);
        if (ret != HDF_SUCCESS)
        {
            HDF_LOGE("%s, %d: HdfAdcDriverParseInit error", __func__, __FILE__);
            break;
        }
    }

    return HDF_SUCCESS;
}

static void HdfAdcDriverRelease(struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *childNode = NULL;

    HDF_LOGI("%s: enter", __func__);
    if ((device == NULL) || (device->property == NULL))
    {
        HDF_LOGE("%s, %d: device or property is null", __func__, __FILE__);
        return;
    }

    DEV_RES_NODE_FOR_EACH_CHILD_NODE(device->property, childNode) {
        HdfAdcDriverParseDeinit(childNode);
    }
}

static struct HdfDriverEntry g_adcDriverEntry = {
    .moduleVersion = 1,
    .Init = HdfAdcDriverInit,
    .Release = HdfAdcDriverRelease,
    .moduleName = "adc_driver_rk2206",
};

HDF_INIT(g_adcDriverEntry);
