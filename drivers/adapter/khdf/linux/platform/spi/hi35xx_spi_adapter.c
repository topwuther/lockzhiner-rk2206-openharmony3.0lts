/*
 * spi_adapter.c
 *
 * spi driver adapter of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/spi/spi.h>
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_dlist.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "spi_core.h"
#include "spi_if.h"

#define HDF_LOG_TAG HDF_SPI_LINUX_ADAPTER
#define SPI_DEV_NEED_FIND_NEXT 0
#define SPI_DEV_CREAT_FAILURE  1
#define SPI_DEV_FIND_SUCCESS   2

static uint16_t HdfSpiModeToLinuxMode(uint16_t mode)
{
    return ((!!(mode & SPI_CLK_PHASE) ? SPI_CPHA : 0) |
            (!!(mode & SPI_CLK_POLARITY) ? SPI_CPOL : 0) |
            (!!(mode & SPI_MODE_CS_HIGH) ? SPI_CS_HIGH : 0) |
            (!!(mode & SPI_MODE_LSBFE) ? SPI_LSB_FIRST : 0) |
            (!!(mode & SPI_MODE_3WIRE) ? SPI_3WIRE : 0) |
            (!!(mode & SPI_MODE_LOOP) ? SPI_LOOP : 0) |
            (!!(mode & SPI_MODE_NOCS) ? SPI_NO_CS : 0) |
            (!!(mode & SPI_MODE_READY) ? SPI_READY : 0));
}

static uint16_t LinuxSpiModeToHdfMode(uint16_t mode)
{
    return ((!!(mode & SPI_CPHA) ? SPI_CLK_PHASE : 0) |
            (!!(mode & SPI_CPOL) ? SPI_CLK_POLARITY : 0) |
            (!!(mode & SPI_CS_HIGH) ? SPI_MODE_CS_HIGH : 0) |
            (!!(mode & SPI_LSB_FIRST) ? SPI_MODE_LSBFE : 0) |
            (!!(mode & SPI_3WIRE) ? SPI_MODE_3WIRE : 0) |
            (!!(mode & SPI_LOOP) ? SPI_MODE_LOOP : 0) |
            (!!(mode & SPI_NO_CS) ? SPI_MODE_NOCS : 0) |
            (!!(mode & SPI_READY) ? SPI_MODE_READY : 0));
}

static struct SpiDev *SpiFindDeviceByCsNum(const struct SpiCntlr *cntlr, uint32_t cs)
{
    struct SpiDev *dev = NULL;
    struct SpiDev *tmpDev = NULL;

    if (cntlr->numCs <= cs) {
        HDF_LOGE("%s: invalid cs %u", __func__, cs);
        return NULL;
    }
    DLIST_FOR_EACH_ENTRY_SAFE(dev, tmpDev, &(cntlr->list), struct SpiDev, list) {
        if (dev->csNum == cs) {
            return dev;
        }
    }
    return NULL;
}

static int32_t SpiAdatperSetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg)
{
    int32_t ret;
    struct SpiDev *dev = NULL;
    struct spi_device *spidev = NULL;

    if (cntlr == NULL || cfg == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    dev = SpiFindDeviceByCsNum(cntlr, cntlr->curCs);
    if (dev == NULL || dev->priv == NULL) {
        HDF_LOGE("%s: dev is invalid", __func__);
        return HDF_FAILURE;
    }

    spidev = (struct spi_device *)dev->priv;
    spidev->bits_per_word = cfg->bitsPerWord;
    spidev->max_speed_hz = cfg->maxSpeedHz;
    spidev->mode = HdfSpiModeToLinuxMode(cfg->mode);
    ret = spi_setup(spidev);
    if (ret != 0) {
        HDF_LOGE("%s: spi_setup fail, ret is %d", __func__, ret);
        return HDF_FAILURE;
    }

    dev->cfg = *cfg;
    return HDF_SUCCESS;
}

static int32_t SpiAdatperGetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg)
{
    struct SpiDev *dev = NULL;

    if (cntlr == NULL || cfg == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    dev = SpiFindDeviceByCsNum(cntlr, cntlr->curCs);
    if (dev == NULL || dev->priv == NULL) {
        HDF_LOGE("%s: fail, dev is invalid", __func__);
        return HDF_FAILURE;
    }

    *cfg = dev->cfg;
    return HDF_SUCCESS;
}

static int32_t SpiAdatperTransfer(struct SpiCntlr *cntlr, struct SpiMsg *msg, uint32_t count)
{
    int32_t ret;
    uint32_t i;
    struct spi_message xfer;
    struct SpiDev *dev = NULL;
    struct spi_transfer *transfer = NULL;

    if (cntlr == NULL || msg == NULL || count == 0) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    dev = SpiFindDeviceByCsNum(cntlr, cntlr->curCs);
    if (dev == NULL || dev->priv == NULL) {
        HDF_LOGE("%s fail, spidev is null\n", __func__);
        return HDF_FAILURE;
    }

    transfer = kcalloc(count, sizeof(*transfer), GFP_KERNEL);
    if (transfer == NULL) {
        HDF_LOGE("%s: transfer alloc memory failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    spi_message_init(&xfer);
    for (i = 0; i < count; i++) {
        transfer[i].tx_buf = msg[i].wbuf;
        transfer[i].rx_buf = msg[i].rbuf;
        transfer[i].len = msg[i].len;
        if (msg[i].speed != 0)
            transfer[i].speed_hz = msg[i].speed;
        if (msg[i].csChange != 0)
            transfer[i].cs_change = msg[i].csChange;
        if (msg[i].delayUs != 0)
            transfer[i].delay_usecs = msg[i].delayUs;
        spi_message_add_tail(&transfer[i], &xfer);
    }

    ret = spi_sync(dev->priv, &xfer);
    kfree(transfer);
    return ret;
}

static const char *GetSpiDevName(const struct device *dev)
{
    if (dev->init_name) {
        return dev->init_name;
    }
    return kobject_name(&dev->kobj);
}

static void SpiDevInit(struct SpiDev *dev, const struct spi_device *spidev)
{
    dev->cfg.bitsPerWord = spidev->bits_per_word;
    dev->cfg.maxSpeedHz = spidev->max_speed_hz;
    dev->cfg.mode = LinuxSpiModeToHdfMode(spidev->mode);
    dev->cfg.transferMode = SPI_INTERRUPT_TRANSFER;
    dev->priv = (struct spi_device *)spidev;
}

static struct SpiDev *SpiDevCreat(struct SpiCntlr *cntlr)
{
    struct SpiDev *dev = NULL;

    dev = (struct SpiDev *)OsalMemCalloc(sizeof(*dev));
    if (dev == NULL) {
        HDF_LOGE("%s: OsalMemCalloc dev error", __func__);
        return NULL;
    }

    DListHeadInit(&dev->list);
    DListInsertTail(&dev->list, &cntlr->list);
    return dev;
}

static int32_t SpiFindDeviceFromBus(struct device *dev, void *para)
{
    struct spi_device *spidev = NULL;
    struct SpiDev *spi = NULL;
    struct SpiCntlr *cntlr = (struct SpiCntlr *)para;

    if (dev == NULL || cntlr == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    spidev = CONTAINER_OF(dev, struct spi_device, dev);
    get_device(&spidev->dev);

    if (spidev->master == NULL) {
        put_device(&spidev->dev);
        HDF_LOGE("%s: spi_device %s -> master is NULL", __func__, GetSpiDevName(&spidev->dev));
        return HDF_ERR_INVALID_PARAM;
    }
    HDF_LOGI("%s: spi_device %s, find success", __func__, GetSpiDevName(&spidev->dev));
    HDF_LOGI("%s: spi_device bus_num %d, chip_select %d", __func__,
        spidev->master->bus_num, spidev->chip_select);

    if (spidev->master->bus_num == cntlr->busNum && spidev->chip_select == cntlr->curCs) {
        spi = SpiFindDeviceByCsNum(cntlr, cntlr->curCs);
        if (spi == NULL) {
            spi = SpiDevCreat(cntlr);
        }
        if (spi == NULL) {
            put_device(&spidev->dev);
            return SPI_DEV_CREAT_FAILURE;
        }
        SpiDevInit(spi, spidev);
        return SPI_DEV_FIND_SUCCESS;
    } else {
        put_device(&spidev->dev);
        return SPI_DEV_NEED_FIND_NEXT;
    }
}

static int32_t SpiAdatperOpen(struct SpiCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("%s: fail, cntlr is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    ret = bus_for_each_dev(&spi_bus_type, NULL, (void *)cntlr, SpiFindDeviceFromBus);
    if (ret != SPI_DEV_FIND_SUCCESS) {
        HDF_LOGE("%s: spidev find fail, ret is %d", __func__, ret);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t SpiAdatperClose(struct SpiCntlr *cntlr)
{
    struct SpiDev *dev = NULL;
    struct spi_device *spidev = NULL;

    if (cntlr == NULL) {
        HDF_LOGE("%s: fail, cntlr is NULL", __func__);
        return HDF_FAILURE;
    }

    dev = SpiFindDeviceByCsNum(cntlr, cntlr->curCs);
    if (dev == NULL) {
        HDF_LOGE("%s fail, dev is NULL", __func__);
        return HDF_FAILURE;
    }

    spidev = (struct spi_device *)dev->priv;
    if (spidev == NULL) {
        HDF_LOGE("%s fail, spidev is NULL", __func__);
        return HDF_FAILURE;
    }
    put_device(&spidev->dev);
    return HDF_SUCCESS;
}

struct SpiCntlrMethod g_method = {
    .Transfer = SpiAdatperTransfer,
    .SetCfg = SpiAdatperSetCfg,
    .GetCfg = SpiAdatperGetCfg,
    .Open = SpiAdatperOpen,
    .Close = SpiAdatperClose,
};

static int32_t SpiGetBaseCfgFromHcs(struct SpiCntlr *cntlr, const struct DeviceResourceNode *node)
{
    struct DeviceResourceIface *iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);

    if (iface == NULL || iface->GetUint32 == NULL) {
        HDF_LOGE("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(node, "busNum", &cntlr->busNum, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read busNum fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(node, "numCs", &cntlr->numCs, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read numCs fail", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int SpiCntlrInit(struct SpiCntlr *cntlr, const struct HdfDeviceObject *device)
{
    int ret;

    if (device->property == NULL) {
        HDF_LOGE("%s: property is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = SpiGetBaseCfgFromHcs(cntlr, device->property);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: SpiGetBaseCfgFromHcs error", __func__);
        return HDF_FAILURE;
    }

    cntlr->method = &g_method;
    return 0;
}

static int32_t HdfSpiDeviceBind(struct HdfDeviceObject *device)
{
    HDF_LOGI("%s: entry", __func__);
    if (device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return (SpiCntlrCreate(device) == NULL) ? HDF_FAILURE : HDF_SUCCESS;
}

static int32_t HdfSpiDeviceInit(struct HdfDeviceObject *device)
{
    int ret;
    struct SpiCntlr *cntlr = NULL;

    HDF_LOGI("%s: entry", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    cntlr = SpiCntlrFromDevice(device);
    if (cntlr == NULL) {
        HDF_LOGE("%s: cntlr is NULL", __func__);
        return HDF_FAILURE;
    }

    ret = SpiCntlrInit(cntlr, device);
    if (ret != 0) {
        HDF_LOGE("%s: SpiCntlrInit error", __func__);
        return HDF_FAILURE;
    }
    return ret;
}

static void HdfSpiDeviceRelease(struct HdfDeviceObject *device)
{
    struct SpiCntlr *cntlr = NULL;
    struct SpiDev *dev = NULL;
    struct SpiDev *tmpDev = NULL;

    HDF_LOGI("%s: entry", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is NULL", __func__);
        return;
    }
    cntlr = SpiCntlrFromDevice(device);
    if (cntlr == NULL) {
        HDF_LOGE("%s: cntlr is NULL", __func__);
        return;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(dev, tmpDev, &(cntlr->list), struct SpiDev, list) {
        DListRemove(&(dev->list));
        OsalMemFree(dev);
    }
    SpiCntlrDestroy(cntlr);
}

struct HdfDriverEntry g_hdfSpiDevice = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_SPI",
    .Bind = HdfSpiDeviceBind,
    .Init = HdfSpiDeviceInit,
    .Release = HdfSpiDeviceRelease,
};

HDF_INIT(g_hdfSpiDevice);
