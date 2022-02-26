/*
 * mipi_tx_hi35xx.h
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

#ifndef MIPI_TX_HI35XX_H
#define MIPI_TX_HI35XX_H

/****************************************************************************
 * macro definition                                                         *
 ****************************************************************************/
#define MIPI_TX_REGS_ADDR   0x11270000
#define MIPI_TX_REGS_SIZE   0x10000

#define MIPI_TX_IRQ         120

#define MIPI_TX_CRG         0x1201010C

#define MIPI_TX_REF_CLK     27

#define TLPX                60
#define TCLK_PREPARE        60
#define TCLK_ZERO           250
#define TCLK_TRAIL          80
#define TPRE_DELAY          100
#define THS_PREPARE         80
#define THS_ZERO            180
#define THS_TRAIL           110

/* phy addr */
#define PLL_SET0            0x60
#define PLL_SET1            0x64
#define PLL_SET2            0x65
#ifdef HI_FPGA
#define PLL_SET3            0x17
#endif
#define PLL_SET4            0x66
#define PLL_SET5            0x67

#define DATA0_TPRE_DELAY    0x28
#define DATA1_TPRE_DELAY    0x38
#define DATA2_TPRE_DELAY    0x48
#define DATA3_TPRE_DELAY    0x58

#define CLK_TLPX            0x10
#define CLK_TCLK_PREPARE    0x11
#define CLK_TCLK_ZERO       0x12
#define CLK_TCLK_TRAIL      0x13

#define DATA0_TLPX          0x20
#define DATA0_THS_PREPARE   0x21
#define DATA0_THS_ZERO      0x22
#define DATA0_THS_TRAIL     0x23
#define DATA1_TLPX          0x30
#define DATA1_THS_PREPARE   0x31
#define DATA1_THS_ZERO      0x32
#define DATA1_THS_TRAIL     0x33
#define DATA2_TLPX          0x40
#define DATA2_THS_PREPARE   0x41
#define DATA2_THS_ZERO      0x42
#define DATA2_THS_TRAIL     0x43
#define DATA3_TLPX          0x50
#define DATA3_THS_PREPARE   0x51
#define DATA3_THS_ZERO      0x52
#define DATA3_THS_TRAIL     0x53

#define MIPI_TX_READ_TIMEOUT_CNT 1000

#define PREPARE_COMPENSATE    10
#define ROUNDUP_VALUE     7999
#define INNER_PEROID      8000   /* 8 * 1000 ,1000 is 1us = 1000ns, 8 is division ratio */

typedef struct {
    unsigned char dataTpreDelay;
    unsigned char clkTlpx;
    unsigned char clkTclkPrepare;
    unsigned char clkTclkZero;
    unsigned char clkTclkTrail;
    unsigned char dataTlpx;
    unsigned char dataThsPrepare;
    unsigned char dataThsZero;
    unsigned char dataThsTrail;
} MipiTxPhyTimingParamTag;

typedef struct {
    unsigned int vallDet;
    unsigned int vactDet;
    unsigned int hallDet;
    unsigned int hactDet;
    unsigned int hbpDet;
    unsigned int hsaDet;
    unsigned int vsaDet;
} MipiTxDevPhyTag;

void MipiTxDrvGetDevStatus(MipiTxDevPhyTag *phyCtx);
#endif /* MIPI_TX_HI35XX_H */
