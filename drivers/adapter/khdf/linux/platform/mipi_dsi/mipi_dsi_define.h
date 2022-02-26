/*
 * mipi_dsi_define.h
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

#ifndef MIPI_DSI_DEFINE_H
#define MIPI_DSI_DEFINE_H

#define CMD_MAX_NUM    4
#define LANE_MAX_NUM   4
#define MIPI_TX_DISABLE_LANE_ID (-1)
#define MIPI_TX_SET_DATA_SIZE 800
#define MIPI_TX_GET_DATA_SIZE 160

typedef enum {
    OUTPUT_MODE_CSI            = 0x0, /* csi mode */
    OUTPUT_MODE_DSI_VIDEO      = 0x1, /* dsi video mode */
    OUTPUT_MODE_DSI_CMD        = 0x2, /* dsi command mode */

    OUTPUT_MODE_BUTT
} OutPutModeTag;

typedef enum {
    BURST_MODE                      = 0x0,
    NON_BURST_MODE_SYNC_PULSES      = 0x1,
    NON_BURST_MODE_SYNC_EVENTS      = 0x2,

    VIDEO_DATA_MODE_BUTT
} VideoModeTag;

typedef enum {
    OUT_FORMAT_RGB_16_BIT          = 0x0,
    OUT_FORMAT_RGB_18_BIT          = 0x1,
    OUT_FORMAT_RGB_24_BIT          = 0x2,
    OUT_FORMAT_YUV420_8_BIT_NORMAL = 0x3,
    OUT_FORMAT_YUV420_8_BIT_LEGACY = 0x4,
    OUT_FORMAT_YUV422_8_BIT        = 0x5,

    OUT_FORMAT_BUTT
} OutputFormatTag;

typedef struct {
    unsigned short  vidPktSize;
    unsigned short  vidHsaPixels;
    unsigned short  vidHbpPixels;
    unsigned short  vidHlinePixels;
    unsigned short  vidVsaLines;
    unsigned short  vidVbpLines;
    unsigned short  vidVfpLines;
    unsigned short  vidActiveLines;
    unsigned short  edpiCmdSize;
} SyncInfoTag;

typedef struct {
    unsigned int    devno;                /* device number */
    short           laneId[LANE_MAX_NUM]; /* lane_id: -1 - disable */
    OutPutModeTag   outputMode;           /* output mode: CSI/DSI_VIDEO/DSI_CMD */
    VideoModeTag    videoMode;
    OutputFormatTag outputFormat;
    SyncInfoTag     syncInfo;
    unsigned int    phyDataRate;          /* mbps */
    unsigned int    pixelClk;             /* KHz */
} ComboDevCfgTag;

typedef struct {
    unsigned int    devno;                /* device number */
    unsigned short  dataType;
    unsigned short  cmdSize;
    unsigned char   *cmd;
} CmdInfoTag;

typedef struct {
    unsigned int    devno;                 /* device number */
    unsigned short  dataType;              /* DSI data type */
    unsigned short  dataParam;             /* data param,low 8 bit:1st param.high 8 bit:2nt param, set 0 if not use */
    unsigned short  getDataSize;           /* read data size */
    unsigned char   *getData;              /* read data memory address, should  malloc by user */
} GetCmdInfoTag;

#endif /* MIPI_DSI_DEFINE_H */
