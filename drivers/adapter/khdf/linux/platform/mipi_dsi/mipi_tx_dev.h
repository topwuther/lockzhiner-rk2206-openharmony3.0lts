/*
 * mipi_tx_dev.h
 *
 * hi35xx mipi_tx driver implement.
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

#ifndef MIPI_TX_DEV_H
#define MIPI_TX_DEV_H
#include "hdf_base.h"
#include "mipi_dsi_if.h"

typedef struct {
    struct DsiCmdDesc readCmd;
    uint32_t readLen;
    uint8_t *out;
} GetDsiCmdDescTag;

#define HI_MIPI_TX_IOC_MAGIC   't'

#define HI_MIPI_TX_SET_DEV_CFG              _IOW(HI_MIPI_TX_IOC_MAGIC, 0x01, struct MipiCfg)
#define HI_MIPI_TX_SET_CMD                  _IOW(HI_MIPI_TX_IOC_MAGIC, 0x02, struct DsiCmdDesc)
#define HI_MIPI_TX_ENABLE                   _IO(HI_MIPI_TX_IOC_MAGIC, 0x03)
#define HI_MIPI_TX_GET_CMD                  _IOWR(HI_MIPI_TX_IOC_MAGIC, 0x04, GetDsiCmdDescTag)
#define HI_MIPI_TX_DISABLE                  _IO(HI_MIPI_TX_IOC_MAGIC, 0x05)

int32_t MipiDsiDevModuleInit(uint8_t id);
void MipiDsiDevModuleExit(uint8_t id);

#endif /* MIPI_TX_DEV_H */
