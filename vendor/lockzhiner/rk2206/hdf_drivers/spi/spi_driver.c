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
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal.h"
#include "osal_mem.h"
#include "osal_sem.h"
#include "osal_time.h"
#include "spi_core.h"
#include "spi_if.h"

#include "lz_hardware.h"

#define HDF_LOG_TAG             spi_driver_rk2206

#define BITS_PER_WORD_DEFAULT    8
#define BITS_PER_WORD_8BITS      8
#define BITS_PER_WORD_10BITS     10

typedef struct tag_hcs_info {
    uint32_t deviceNum;
    uint32_t busNum;
    uint32_t numCs;
    uint32_t maxSpeedHz;
    uint32_t mode;
    uint32_t transferMode;
    uint32_t bitsPerWord;
    uint32_t gpioCs;
    uint32_t gpioClk;
    uint32_t gpioMosi;
    uint32_t gpioMiso;
    uint32_t csM;
    uint32_t speed;
    uint32_t isSlave;
}hcs_info_s;

typedef struct tag_spi_info {
    struct SpiCntlr *cntlr;
    struct DListHead deviceList;
    hcs_info_s hcs_info;
}spi_info_s;

//////////////////////////////////////////////////////

static int32_t SpiCfg2LzCfg(struct SpiCfg *spiConfig, uint32_t speed, uint32_t csM, uint32_t isSlave, LzSpiConfig *pconf)
{
    if ((spiConfig == NULL) || (pconf == NULL))
    {
        HDF_LOGE("%s, %d: spiConfig or pconf is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    pconf->bitsPerWord = (spiConfig->bitsPerWord == BITS_PER_WORD_8BITS) ? (SPI_PERWORD_8BITS) : (SPI_PERWORD_16BITS);
    pconf->firstBit = (spiConfig->mode & SPI_MODE_LSBFE) ? (SPI_LSB) : (SPI_MSB);
    if (spiConfig->mode & SPI_CLK_PHASE)
    {
        if (spiConfig->mode & SPI_CLK_POLARITY)
            pconf->mode = SPI_MODE_3;
        else
            pconf->mode = SPI_MODE_0;
    }
    else
    {
        if (spiConfig->mode & SPI_CLK_POLARITY)
            pconf->mode = SPI_MODE_2;
        else
            pconf->mode = SPI_MODE_1;
    }

    pconf->speed = speed;
    pconf->csm = csM;
    pconf->isSlave = isSlave;
    
    return HDF_SUCCESS;
}

static struct SpiDev *FindDeviceByCsNum(const spi_info_s *pspi_info, uint32_t cs)
{
    struct SpiDev *dev = NULL;
    struct SpiDev *tmpDev = NULL;

    if ((pspi_info == NULL) || (pspi_info->hcs_info.numCs <= cs))
    {
        HDF_LOGE("%s, %d: pspi_info or pspi_info.hcs_info.numCs is null", __func__, __FILE__);
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(dev, tmpDev, &(pspi_info->deviceList), struct SpiDev, list) {
        if (dev->csNum == cs)
            break;
    }

    return dev;
}

static int32_t HdfSpiDriverOpen(struct SpiCntlr *cntlr)
{
    (void)cntlr;
    return HDF_SUCCESS;
}

static int32_t HdfSpiDriverClose(struct SpiCntlr *cntlr)
{
    (void)cntlr;
    return HDF_SUCCESS;
}

static int32_t HdfSpiDriverSetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg)
{
    spi_info_s *pspi_info = NULL;
    struct SpiDev *spiDev = NULL;

    if ((cntlr == NULL) || (cntlr->priv == NULL) || (cfg == NULL))
    {
        HDF_LOGE("%s, %d: cnltr or cntlr->priv or cfg is null", __func__, __FILE__);
        return HDF_ERR_INVALID_PARAM;
    }

    pspi_info = (spi_info_s *)cntlr->priv;
    spiDev = FindDeviceByCsNum(pspi_info, cntlr->curCs);
    if (spiDev == NULL)
    {
        HDF_LOGE("%s, %d: spiDev is null, curCs %u", __func__, __FILE__, cntlr->curCs);
        return HDF_FAILURE;
    }

    spiDev->cfg.mode = cfg->mode;
    spiDev->cfg.transferMode = cfg->transferMode;
    spiDev->cfg.bitsPerWord = cfg->bitsPerWord;
    if ((cfg->bitsPerWord != BITS_PER_WORD_8BITS) || (cfg->bitsPerWord != BITS_PER_WORD_10BITS))
    {
        HDF_LOGE("%s: bitsPerWord %u not support, use default bitsPerWord %u",
                 __func__, cfg->bitsPerWord, BITS_PER_WORD_DEFAULT);
        spiDev->cfg.bitsPerWord = BITS_PER_WORD_DEFAULT;
    }

    if (cfg->maxSpeedHz != 0)
    {
        spiDev->cfg.maxSpeedHz = cfg->maxSpeedHz;
    }
    
    return HDF_SUCCESS;
}

static int32_t HdfSpiDriverGetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg)
{
    spi_info_s *pspi_info = NULL;
    struct SpiDev *spiDev = NULL;

    HDF_LOGE("%s: Enter", __func__);
    if ((cntlr == NULL) || (cntlr->priv == NULL) || (cfg == NULL))
    {
        HDF_LOGE("%s, %d: invalid parameter", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    pspi_info = (spi_info_s *)cntlr->priv;
    spiDev = FindDeviceByCsNum(pspi_info, cntlr->curCs);
    if (spiDev == NULL)
    {
        HDF_LOGE("%s, %d: spiDev is null, curCs %u", __func__, __LINE__, cntlr->curCs);
        return HDF_FAILURE;
    }

    cfg->mode = spiDev->cfg.mode;
    cfg->transferMode = spiDev->cfg.transferMode;
    cfg->bitsPerWord = spiDev->cfg.bitsPerWord;
    cfg->maxSpeedHz = spiDev->cfg.maxSpeedHz;
    
    return HDF_SUCCESS;
}

static int32_t HdfSpiDriverTransfer(struct SpiCntlr *cntlr, struct SpiMsg *msg, uint32_t count)
{
    int32_t ret;
    spi_info_s *pspi_info = NULL;
    struct SpiDev *spiDev = NULL;
    LzSpiConfig lzConf = {
        .bitsPerWord = SPI_PERWORD_8BITS,
        .firstBit = SPI_MSB,
        .mode = SPI_MODE_3,
        .csm = SPI_CMS_ONE_CYCLES,
        .speed = 50000000,
        .isSlave = false
    };

    if ((cntlr == NULL) || (cntlr->priv == NULL) || (msg == NULL) || (count == 0))
    {
        HDF_LOGE("%s, %d: invalid parameter", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    pspi_info = (spi_info_s *)cntlr->priv;
    spiDev = FindDeviceByCsNum(pspi_info, cntlr->curCs);
    if (spiDev == NULL)
    {
        HDF_LOGE("%s, %d: spiDev is null, curCs %u", __func__, __LINE__, cntlr->curCs);
        return HDF_FAILURE;
    }

    SpiCfg2LzCfg(&spiDev->cfg, pspi_info->hcs_info.speed, pspi_info->hcs_info.csM, pspi_info->hcs_info.isSlave, &lzConf);
    for (uint32_t i = 0; i < count; i++)
    {
        if (msg[i].wbuf != NULL)
        {
            ret = LzSpiWrite(pspi_info->hcs_info.busNum, 0, msg[i].wbuf, msg[i].len);
            if (ret != LZ_HARDWARE_SUCCESS)
            {
                HDF_LOGE("%s, %d: LzSpiWrite error(%d)", __func__, __LINE__, ret);
                return HDF_FAILURE;
            }
        }

        if (msg[i].rbuf != NULL)
        {
            ret = LzSpiRead(pspi_info->hcs_info.busNum, 0, msg[i].rbuf, msg[i].len);
            if (ret != LZ_HARDWARE_SUCCESS)
            {
                HDF_LOGE("%s, %d: LzSpiRead error(%d)", __func__, __LINE__, ret);
                return HDF_FAILURE;
            }
        }
    }
    
    return HDF_SUCCESS;
}

struct SpiCntlrMethod g_spiMethod = {
    .Transfer = HdfSpiDriverTransfer,
    .SetCfg = HdfSpiDriverSetCfg,
    .GetCfg = HdfSpiDriverGetCfg,
    .Open = HdfSpiDriverOpen,
    .Close = HdfSpiDriverClose,
};

static int32_t HdfSpiDriverReadHcs(spi_info_s *pspi_info, const struct DeviceResourceNode *node)
{
    struct DeviceResourceIface *iface = NULL;
    hcs_info_s *phcs_info = NULL;

    if (node == NULL)
    {
        HDF_LOGE("%s, %d: node is null", __func__, __FILE__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (pspi_info == NULL)
    {
        HDF_LOGE("%s, %d: pspi_info is null", __func__, __FILE__);
        return HDF_ERR_INVALID_PARAM;
    }

    phcs_info = &pspi_info->hcs_info;

    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if ((iface == NULL) || (iface->GetUint32 == NULL) || (iface->GetUint8 == NULL))
    {
        HDF_LOGE("%s, %d: node is null", __func__, __FILE__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (iface->GetUint32(node, "deviceNum", &phcs_info->deviceNum, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs deviceNum fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "busNum", &phcs_info->busNum, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs busNum fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "numCs", &phcs_info->numCs, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs numCs fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "maxSpeedHz", &phcs_info->maxSpeedHz, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs maxSpeedHz fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "mode", &phcs_info->mode, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs mode fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "transferMode", &phcs_info->transferMode, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs transferMode fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "bitsPerWord", &phcs_info->bitsPerWord, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs bitsPerWord fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "gpioCs", &phcs_info->gpioCs, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs gpioCs fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "gpioClk", &phcs_info->gpioClk, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs gpioClk fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "gpioMosi", &phcs_info->gpioMosi, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs gpioMosi fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "csM", &phcs_info->csM, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs csM fail", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(node, "speed", &phcs_info->speed, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs speed fail", __func__);
        return HDF_FAILURE;
    }
    
    if (iface->GetUint32(node, "isSlave", &phcs_info->isSlave, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read hcs isSlave fail", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static void HdfSpiDriverDeviceDeinit(spi_info_s *pspi_info)
{
    struct SpiDev *dev = NULL;
    struct SpiDev *tmpDev = NULL;

    if (pspi_info == NULL)
    {
        HDF_LOGE("%s, %d: pspi_info is null", __func__, __FILE__);
        return;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(dev, tmpDev, &(pspi_info->deviceList), struct SpiDev, list) {
        DListRemove(&(dev->list));
        OsalMemFree(dev);
    }

    OsalMemFree(pspi_info);
    pspi_info = NULL;
}

static int32_t HdfSpiDriverDeviceInit(struct SpiCntlr *cntlr, const struct HdfDeviceObject *device)
{
    int32_t ret;
    spi_info_s *pspi_info = NULL;
    hcs_info_s *phcs_info = NULL;
    unsigned int busNum = 0;
    SpiBusIo spiBus = {
        .cs =   {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .clk =  {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .mosi = {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .miso = {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .miso = {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .id = FUNC_ID_SPI0,
        .mode = FUNC_MODE_M1,
    };
    LzSpiConfig spiConf = {
        .bitsPerWord = SPI_PERWORD_8BITS,
        .firstBit = SPI_MSB,
        .mode = SPI_MODE_3,
        .csm = SPI_CMS_ONE_CYCLES,
        .speed = 50000000,
        .isSlave = false
    };

    if (device->property == NULL)
    {
        HDF_LOGE("%s, %d: property is null", __func__, __FILE__);
        return HDF_ERR_INVALID_PARAM;
    }

    pspi_info = (spi_info_s *)OsalMemCalloc(sizeof(spi_info_s));
    if (pspi_info == NULL)
    {
        HDF_LOGE("%s, %d: OsalMemCalloc failed!", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = HdfSpiDriverReadHcs(pspi_info, device->property);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: HdfSpiDriverReadHcs failed!", __func__, __LINE__);
        goto err_read_hcs;
    }

    phcs_info = &(pspi_info->hcs_info);

    DListHeadInit(&pspi_info->deviceList);
    pspi_info->cntlr = cntlr;
    cntlr->priv = pspi_info;
    cntlr->busNum = pspi_info->hcs_info.busNum;
    cntlr->method = &g_spiMethod;

    /* 添加设备数量 */
    for (uint32_t i = 0; i < phcs_info->numCs; i++)
    {
        struct SpiDev *device_temp = (struct SpiDev *)OsalMemCalloc(sizeof(struct SpiDev));
        if (device_temp == NULL)
        {
            HDF_LOGE("%s, %d: OsalMemCalloc error", __func__, __LINE__);
            ret = HDF_FAILURE;
            goto err_mem_calloc;
        }

        device_temp->cntlr = pspi_info->cntlr;
        device_temp->csNum = i;
        device_temp->cfg.maxSpeedHz = phcs_info->maxSpeedHz;
        device_temp->cfg.mode = phcs_info->mode;
        device_temp->cfg.transferMode = phcs_info->transferMode;
        device_temp->cfg.bitsPerWord = phcs_info->bitsPerWord;
        DListHeadInit(&device_temp->list);
        DListInsertTail(&device_temp->list, &pspi_info->deviceList);
    }

    spiBus.cs.gpio = phcs_info->gpioCs;
    spiBus.clk.gpio = phcs_info->gpioClk;
    spiBus.mosi.gpio = phcs_info->gpioMosi;
    spiBus.miso.gpio = phcs_info->gpioMiso;
    switch (phcs_info->busNum)
    {
        case 0:
            spiBus.id = FUNC_ID_SPI0;
            spiBus.mode = FUNC_MODE_M1;
            break;
        case 1:
            spiBus.id = FUNC_ID_SPI1;
            spiBus.mode = FUNC_MODE_M1;
            break;
        default:
            HDF_LOGE("%s, %d: HdfSpiDriverReadHcs failed!", __func__, __LINE__);
            goto err_switch_busNum;
            break;
    }

    spiConf.bitsPerWord = (phcs_info->bitsPerWord == 8) ? (SPI_PERWORD_8BITS) : (SPI_PERWORD_16BITS);
    spiConf.firstBit = (phcs_info->mode & SPI_MODE_LSBFE) ? (SPI_LSB) : (SPI_MSB);
    if (phcs_info->mode & SPI_CLK_PHASE)
    {
        if (phcs_info->mode & SPI_CLK_POLARITY)
        {
            spiConf.mode = SPI_MODE_3;
        }
        else
        {
            spiConf.mode = SPI_MODE_0;
        }
    }
    else
    {
        if (phcs_info->mode & SPI_CLK_POLARITY)
        {
            spiConf.mode = SPI_MODE_2;
        }
        else
        {
            spiConf.mode = SPI_MODE_1;
        }
    }
    spiConf.speed = phcs_info->speed;
    spiConf.csm = phcs_info->csM;
    spiConf.isSlave = phcs_info->isSlave;

    SpiIoInit(spiBus);
    LzSpiInit(phcs_info->busNum, spiConf);

    return HDF_SUCCESS;

err_switch_busNum:
err_mem_calloc:
    HdfSpiDriverDeviceDeinit(pspi_info);
err_read_hcs:
    if (pspi_info != NULL)
    {
        OsalMemFree(pspi_info);
        pspi_info = NULL;
    }
    return ret;
}

static int32_t HdfSpiDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct SpiCntlr *cntlr = NULL;

    printf("%s, %d: %s Entry!\n", __FILE__, __LINE__, __func__);
    HDF_LOGI("%s: entry", __func__);

    if ((device == NULL) || (device->property == NULL))
    {
        HDF_LOGE("%s, %d: device or property is null", __func__, __FILE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    cntlr = SpiCntlrFromDevice(device);
    if (cntlr == NULL)
    {
        HDF_LOGE("%s, %d: cntlr is null", __func__, __FILE__);
        return HDF_FAILURE;
    }

    ret = HdfSpiDriverDeviceInit(cntlr, device);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: HdfSpiDriverDeviceInit error(%d)", __func__, __FILE__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static void HdfSpiDriverRelease(struct HdfDeviceObject *device)
{
    struct SpiCntlr *cntlr = NULL;
    
    HDF_LOGE("%s: Enter", __func__);
    if (device == NULL)
    {
        HDF_LOGE("%s, %d: device is null", __func__, __LINE__);
        return;
    }

    cntlr = SpiCntlrFromDevice(device);
    if (cntlr == NULL)
    {
        HDF_LOGE("%s, %d: cntlr is null", __func__, __LINE__);
        return;
    }

    if (cntlr->priv != NULL)
    {
        spi_info_s *pspi_info = (spi_info_s *)&cntlr->priv;
        struct SpiDev *dev = NULL;
        struct SpiDev *tmpDev = NULL;

        DLIST_FOR_EACH_ENTRY_SAFE(dev, tmpDev, &(pspi_info->deviceList), struct SpiDev, list) {
            if (dev != NULL)
            {
                DListRemove(&(dev->list));
                OsalMemFree(dev);
            }
        }

        OsalMemFree(pspi_info);
    }

    SpiCntlrDestroy(cntlr);
}

static int32_t HdfSpiDriverBind(struct HdfDeviceObject *device)
{
    struct SpiCntlr *cntlr = NULL;
    
    HDF_LOGE("%s: Enter", __func__);
    if (device == NULL)
    {
        HDF_LOGE("%s, %d: device is null", __func__, __FILE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    cntlr = SpiCntlrCreate(device);
    if (cntlr == NULL)
    {
        HDF_LOGE("%s, %d: SpiCntlrCreate error", __func__, __FILE__);
        return HDF_FAILURE;
    }
    
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_spiDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "spi_driver_rk2206",
    .Bind = HdfSpiDriverBind,
    .Init = HdfSpiDriverInit,
    .Release = HdfSpiDriverRelease,
};

HDF_INIT(g_spiDriverEntry);