/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HI3516_AIAO_IMPL_H
#define HI3516_AIAO_IMPL_H

#include <linux/types.h>
#include "hi3516_common.h"
/* AIO  base addres */
#define AIAO_REG_BASE            0x113b0000
#define AIO_CLK_SEL_SEPARATE     0x0
#define AIO_CLK_SEL_INSEPARATE   0x1
#define AIAO_MAX_REG_SIZE   (64 * 1024)

static const int AIP_I2S_REG_CFG0  = 0x0100;
static const int AIP_I2S_REG_CFG1  = 0x0104;
static const int AOP_I2S_REG_CFG0  = 0x0140;
static const int AOP_I2S_REG_CFG1  = 0x0144;
/* aop */
static const int OFFSET_MULTL = 8;
static const int AOP_BUFF_SADDR_REG    = 0x2080;
static const int AOP_BUFF_SIZE_REG     = 0x2084;
static const int AOP_BUFF_WPTR_REG     = 0x2088;
static const int AOP_BUFF_RPTR_REG     = 0x208C;
static const int AOP_TRANS_SIZE_REG      = 0x2094;
static const int AOP_INT_ENA_REG         = 0x20A0;
static const int AOP_INT_STATUS_REG      = 0x20A8;
static const int AOP_INT_CLR_REG         = 0x20AC;
static const int AOP_INF_ATTRI_REG       = 0x2000;
static const int AOP_CTRL_REG            = 0x2004;
static const int AIP_BUFF_SADDR_REG      = 0x1080;
static const int AIP_BUFF_SIZE_REG       = 0x1084;
static const int AIP_BUFF_WPTR_REG       = 0x1088;
static const int AIP_BUFF_RPTR_REG       = 0x108C;
static const int AIP_TRANS_SIZE_REG      = 0x1094;
static const int AIP_INT_CLR_REG         = 0x10AC;
static const int AIP_INF_ATTRI_REG       = 0x1000;
static const int AIP_CTRL_REG            = 0x1004;

#define CRG_REG_ADDR            0x12010000
#define PERI_CRG103             (CRG_REG_ADDR + 0x019c)
#define PERI_CRG105             (CRG_REG_ADDR + 0x01a4)
#define PERI_CRG106             (CRG_REG_ADDR + 0x01a8)
#define PERI_CRG107             (CRG_REG_ADDR + 0x01ac)

#define AIAO_INT_ENA_REG    0x0
#define AIAO_INT_STATUS_REG 0x4
#define AIAO_INT_RAW_REG    0x8

#define HI_AIO_MAX_NUM 3
#define HI_AI_DEV_MAX_NUM 3
#define HI_AO_DEV_MAX_NUM 3

#define AIO_BCLK_TO_FSCLK_RATIO_16  16
#define AIO_BCLK_TO_FSCLK_RATIO_32  32
#define AIO_BCLK_TO_FSCLK_RATIO_48  48
#define AIO_BCLK_TO_FSCLK_RATIO_64  64
#define AIO_BCLK_TO_FSCLK_RATIO_128 128
#define AIO_BCLK_TO_FSCLK_RATIO_256 256
#define AIO_BCLK_TO_FSCLK_RATIO_320 320
#define AIO_BCLK_TO_FSCLK_RATIO_384 384

#define AIO_MCLK_TO_BCLK_RATIO_1    1
#define AIO_MCLK_TO_BCLK_RATIO_2    2
#define AIO_MCLK_TO_BCLK_RATIO_3    3
#define AIO_MCLK_TO_BCLK_RATIO_4    4
#define AIO_MCLK_TO_BCLK_RATIO_6    6
#define AIO_MCLK_TO_BCLK_RATIO_8    8
#define AIO_MCLK_TO_BCLK_RATIO_12   12
#define AIO_MCLK_TO_BCLK_RATIO_16   16
#define AIO_MCLK_TO_BCLK_RATIO_24   24
#define AIO_MCLK_TO_BCLK_RATIO_32   32
#define AIO_MCLK_TO_BCLK_RATIO_48   48
#define AIO_MCLK_TO_BCLK_RATIO_64   64

#define AIO_CHN_CNT_1   1
#define AIO_CHN_CNT_2   2
#define AIO_CHN_CNT_4   4
#define AIO_CHN_CNT_8   8
#define AIO_CHN_CNT_16  16
#define AIO_CHN_CNT_20  20

#define AIO_CHN_ID_0    0
#define AIO_CHN_ID_1    1
#define AIO_CHN_ID_2    2

#define AIO_FIFO_BIT_WIDTH_256  256
#define AIO_FIFO_BIT_WIDTH_320  320
#define AIO_FIFO_BIT_WIDTH_384  384

#define AI_CHN_CNT_20 20

#define AIO_MCLK_48K_256FS_1188M    0x00152EF0 /* 48k * 256 */
#define AIO_MCLK_441K_256FS_1188M   0x00137653 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1188M    0x000E1F4B /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1188M    0x001A7AAC /* 48k * 320 */
#define AIO_MCLK_441K_320FS_1188M   0x00185FA0 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1188M    0x0011A71E /* 32k * 320 */

#define W_CHANNEL_ID_MAX 13
#define R_CHANNEL_ID_MAX 3
#define TX_BUFF_MULTIPLE 32

typedef enum {
    HI_FALSE = 0,
    HI_TRUE  = 1,
} HiBool;

#define AOP_SUPPORT_MULTI_SLOT  HI_FALSE

#define HI_AI_EXPAND 0x01
#define HI_AI_CUT    0x02

#define AIP_SWITCH_RX_BCLK    0x0028 /* AIAO I2S RX BCLK SWITCH Internal BCLK selection */
#define AOP_SWITCH_TX_BCLK    0x002c /* AIAO I2S TX BCLK SWITCH Internal BCLK selection */
#define AIO_SOFT_RESET_STATUS 0x0030

#define SYS_AIO_SAMPLE_CLK256 0x05
#define SYS_AIO_SAMPLE_CLK128 0x04
#define SYS_AIO_SAMPLE_CLK64  0x03
#define SYS_AIO_SAMPLE_CLK48  0x02
#define SYS_AIO_SAMPLE_CLK32  0x01
#define SYS_AIO_SAMPLE_CLK16  0x00

#define SYS_AIO_BS_CLK1 0x00
#define SYS_AIO_BS_CLK2 0x02
#define SYS_AIO_BS_CLK3 0x01
#define SYS_AIO_BS_CLK4 0x03
#define SYS_AIO_BS_CLK6 0x04
#define SYS_AIO_BS_CLK8 0x05
#define SYS_AIO_BS_CLK12 0x06
#define SYS_AIO_BS_CLK16 0x07
#define SYS_AIO_BS_CLK24 0x08
#define SYS_AIO_BS_CLK32 0x09
#define SYS_AIO_BS_CLK48 0x0a
#define SYS_AIO_BS_CLK64 0x0b

#define AIAO_CHN_CNT_20 20

typedef enum {
    TX_SD_SOURCE_SEL0 = 0,
    TX_SD_SOURCE_SEL1 = 1,
    TX_SD_SOURCE_SEL2 = 2,
} TxSdSourceSel;

typedef enum {
    RX_CH_NUM0  = 0,
    RX_CH_NUM1  = 1,
    RX_CH_NUM2  = 2,
    RX_CH_NUM3  = 3,
    RX_CH_NUM4  = 4,
} RxChNum;

typedef enum {
    HI_AUDIO_CLK_SELECT_BASE       = 0,  /* Audio base clk. */
    HI_AUDIO_CLK_SELECT_SPARE,           /* Audio spare clk. */

    HI_AUDIO_CLK_SELECT_BUTT,
} HiAudioClkSelect;

typedef struct {
    HiAudioClkSelect clkSelect;
    /* 256fs */
    unsigned int mclkDiv48k256fs;
    unsigned int mclkDiv441k256fs;
    unsigned int mclkDiv32k256fs;
    /* 320fs */
    unsigned int mclkDiv48k320fs;
    unsigned int mclkDiv441k320fs;
    unsigned int mclkDiv32k320fs;
} AiaoClkInfo;

typedef enum {
    HI_AIO_MODE_I2S_MASTER  = 0,   /* AIO I2S master mode */
    HI_AIO_MODE_I2S_SLAVE,         /* AIO I2S slave mode */
    HI_AIO_MODE_PCM_SLAVE_STD,     /* AIO PCM slave standard mode */
    HI_AIO_MODE_PCM_SLAVE_NON_STD,    /* AIO PCM slave non-standard mode */
    HI_AIO_MODE_PCM_MASTER_STD,    /* AIO PCM master standard mode */
    HI_AIO_MODE_PCM_MASTER_NON_STD,   /* AIO PCM master non-standard mode */
    HI_AIO_MODE_BUTT
} AudioWorkMode;

typedef enum {
    HI_AUDIO_SOUND_MODE_MONO   = 0, /* mono */
    HI_AUDIO_SOUND_MODE_STEREO = 1, /* stereo */
    HI_AUDIO_SOUND_MODE_BUTT
} AudioSndMode;

typedef enum {
    HI_AIO_I2STYPE_INNERCODEC = 0, /* AIO I2S connect inner audio CODEC */
    HI_AIO_I2STYPE_INNERHDMI,      /* AIO I2S connect Inner HDMI        */
    HI_AIO_I2STYPE_EXTERN,         /* AIO I2S connect extern hardware   */
} AioI2sType;

typedef struct {
    AudioSampleRate sampleRate;        /* sample rate */
    AudioBitWidth   bitWidth;          /* bit_width */
    AudioWorkMode   workMode;          /* master or slave mode */
    AudioSndMode    soundMode;         /* momo or steror */
    int32_t         expandFlag;        /* expand 8bit to 16bit, use AI_EXPAND(only valid for AI 8bit),
                                          use AI_CUT(only valid for extern codec for 24bit) */
    int32_t         frameNum;          /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    int32_t         pointNumPerFrame;  /* point num per frame (80/160/240/320/480/1024/2048)
                                             (ADPCM IMA should add 1 point, AMR only support 160) */
    int32_t         channelCnt;         /* channle number on FS, valid value:1/2/4/8 */
    int32_t         clkShare;          /* 0: AI and AO clock is separate
                                          1: AI and AO clock is inseparate, AI use AO's clock */
    AioI2sType      i2sType;           /* i2s type */
} AudioAttr;

typedef enum {
    AIO_TYPE_AI = 0,
    AIO_TYPE_AO,
    AIO_TYPE_BUTT
} AioType;

typedef struct {
    int32_t initialized; /* initialed flag */
    int32_t aiConfig;
    int32_t aoConfig;
    AudioAttr aiAttr;
    AudioAttr aoAttr;
} AioState;

// define the union u_i2s_crg_cfg0
typedef union {
    struct {
        unsigned int aiaoMclkDiv : 27;  // [26..0]
        unsigned int reserved0 : 5;      // [31..27]
    } Bits;
    unsigned int u32;
} UI2sCrgCfg0;

// define the union u_i2s_crg_cfg1
typedef union {
    struct {
        unsigned int aiaoBclkDiv : 4;       // [3..0]
        unsigned int aiaoFsclkDiv : 3;      // [6..4]
        unsigned int reserved1 : 1;          // [7]
        unsigned int aiaoCken : 1;           // [8]
        unsigned int aiaoSrstReq : 1;       // [9]
        unsigned int aiaoBclkOen : 1;       // [10]
        unsigned int aiaoBclkSel : 1;       // [11]
        unsigned int aiaoBclkinPctrl : 1;   // [12]
        unsigned int aiaoBclkoutPctrl : 1;  // [13]
        unsigned int aiaoBclkEn : 1;        // [14]
        unsigned int aiaoWsEn : 1;          // [15]
        unsigned int reserved0 : 16;         // [31..16]
    } Bits;
    unsigned int u32;
} UI2sCrgCfg1;

// define the union u_aiao_rxswitch_cfg
typedef union {
    struct {
        unsigned int innerBclkWsSelRx00 : 4;  // [3..0]
        unsigned int innerBclkWsSelRx01 : 4;  // [7..4]
        unsigned int innerBclkWsSelRx02 : 4;  // [11..8]
        unsigned int innerBclkWsSelRx03 : 4;  // [15..12]
        unsigned int innerBclkWsSelRx04 : 4;  // [19..16]
        unsigned int innerBclkWsSelRx05 : 4;  // [23..20]
        unsigned int innerBclkWsSelRx06 : 4;  // [27..24]
        unsigned int innerBclkWsSelRx07 : 4;  // [31..28]
    } Bits;
    unsigned int u32;
} UAiaoSwitchRxBclk;

// define the union u_aiao_txswitch_cfg
typedef union {
    struct {
        unsigned int innerBclkWsSelTx00 : 4;  // [3..0]
        unsigned int innerBclkWsSelTx01 : 4;  // [7..4]
        unsigned int innerBclkWsSelTx02 : 4;  // [11..8]
        unsigned int innerBclkWsSelTx03 : 4;  // [15..12]
        unsigned int innerBclkWsSelTx04 : 4;  // [19..16]
        unsigned int innerBclkWsSelTx05 : 4;  // [23..20]
        unsigned int innerBclkWsSelTx06 : 4;  // [27..24]
        unsigned int innerBclkWsSelTx07 : 4;  // [31..28]
    } Bits;
    unsigned int u32;
} UAiaoSwitchTxBclk;

// define the union u_rx_if_attri
typedef union {
    struct {
        unsigned int rxMode : 2;           // [1..0]
        unsigned int rxI2sPrecision : 2;  // [3..2]
        unsigned int rxChNum : 3;         // [6..4]
        unsigned int rxMultislotEn : 1;   // [7]
        unsigned int rxSdOffset : 8;      // [15..8]
        unsigned int rxTrackMode : 3;      // [18..16]
        unsigned int reserved0 : 1;        // [19]
        unsigned int rxSdSourceSel : 4;  // [23..20]
        unsigned int rxSd0Sel : 2;        // [25..24]
        unsigned int rxSd1Sel : 2;        // [27..26]
        unsigned int rxSd2Sel : 2;        // [29..28]
        unsigned int rxSd3Sel : 2;        // [31..30]
    } Bits;
    unsigned int u32;
} URxIfAttri;

// define the union u_tx_if_attri
typedef union {
    struct {
        unsigned int txMode : 2;            // [1..0]
        unsigned int txI2sPrecision : 2;   // [3..2]
        unsigned int txChNum : 2;          // [5..4]
        unsigned int txUnderflowCtrl : 1;  // [6]
        unsigned int txMultislotEn : 1;    // [7]
        unsigned int txSdOffset : 8;       // [15..8]
        unsigned int txTrackmode : 3;       // [18..16]
        unsigned int reserved0 : 1;         // [19]
        unsigned int txSdSourceSel : 4;   // [23..20]
        unsigned int txSd0Sel : 2;         // [25..24]
        unsigned int txSd1Sel : 2;         // [27..26]
        unsigned int txSd2Sel : 2;         // [29..28]
        unsigned int txSd3Sel : 2;         // [31..30]
    } Bits;
    unsigned int u32;
} UTxIfAttri;

// define the union u_rx_dsp_ctrl
typedef union {
    struct {
        unsigned int muteEn : 1;          // [0]
        unsigned int muteFadeEn : 1;     // [1]
        unsigned int pauseEn : 1;         // [2]
        unsigned int pauseFadeEn : 1;    // [3]
        unsigned int reserved3 : 4;       // [7..4]
        unsigned int volume : 7;           // [14..8]
        unsigned int reserved2 : 1;       // [15]
        unsigned int fadeInRate : 4;     // [19..16]
        unsigned int fadeOutRate : 4;    // [23..20]
        unsigned int reserved1 : 3;       // [26..24]
        unsigned int bypassEn : 1;        // [27]
        unsigned int rxEnable : 1;        // [28]
        unsigned int rxDisableDone : 1;  // [29]
        unsigned int reserved0 : 2;       // [31..30]
    } Bits;
    unsigned int u32;
} URxDspCtrl;

typedef union {
    struct {
        unsigned int muteEn : 1;          // [0]
        unsigned int muteFadeEn : 1;     // [1]
        unsigned int reserved3 : 6;       // [7..2]
        unsigned int volume : 7;           // [14..8]
        unsigned int reserved2 : 1;       // [15]
        unsigned int fadeInRate : 4;     // [19..16]
        unsigned int fadeOutRate : 4;    // [23..20]
        unsigned int reserved1 : 3;       // [26..24]
        unsigned int bypassEn : 1;        // [27]
        unsigned int txEnable : 1;        // [28]
        unsigned int txDisableDone : 1;  // [29]
        unsigned int reserved0 : 2;       // [31..30]
    } Bits;
    unsigned int u32;
} UTxDspCtrl;

// define the union u_tx_buff_rptr
typedef union {
    struct {
        unsigned int txBuffRptr : 24;  // [23..0]
        unsigned int reserved0 : 8;     // [31..24]
    } Bits;
    unsigned int u32;
} UTxBuffRptr;

typedef union {
    struct {
        unsigned int txBuffWptr : 24;  // [23..0]
        unsigned int reserved0 : 8;     // [31..24]
    } Bits;
    unsigned int u32;
} UTxBuffWptr;

// define the union u_tx_int_ena
typedef union {
    struct {
        unsigned int txTransIntEna : 1;        // [0]
        unsigned int txEmptyIntEna : 1;        // [1]
        unsigned int txAlemptyIntEna : 1;      // [2]
        unsigned int txBfifoEmptyIntEna : 1;  // [3]
        unsigned int txIfifoEmptyIntEna : 1;  // [4]
        unsigned int txStopIntEna : 1;         // [5]
        unsigned int txMfadeIntEna : 1;        // [6]
        unsigned int txDatBreakIntEna : 1;    // [7]
        unsigned int reserved0 : 24;             // [31..8]
    } Bits;
    unsigned int u32;
} UTxIntEna;

typedef union {
    struct {
        unsigned int txTransIntClear : 1;        // [0]
        unsigned int txEmptyIntClear : 1;        // [1]
        unsigned int txAlemptyIntClear : 1;      // [2]
        unsigned int txBfifoEmptyIntClear : 1;  // [3]
        unsigned int txIfifoEmptyIntClear : 1;  // [4]
        unsigned int txStopIntClear : 1;         // [5]
        unsigned int txMfadeIntClear : 1;        // [6]
        unsigned int txDatBreakIntClear : 1;    // [7]
        unsigned int reserved0 : 24;               // [31..8]
    } Bits;
    unsigned int u32;
} UTxIntClr;

typedef union {
    struct {
        unsigned int txBuffSize : 24;  // [23..0]
        unsigned int reserved0 : 8;     // [31..24]
    } Bits;
    unsigned int u32;
} UTxBuffSize;

typedef union {
    struct {
        unsigned int txTransSize : 24;  // [23..0]
        unsigned int reserved0 : 8;      // [31..24]
    } Bits;
    unsigned int u32;
} UTxTransSize;

// define the union u_aiao_int_ena
typedef union {
    struct {
        unsigned int rxCh0IntEna : 1;       // [0]
        unsigned int rxCh1IntEna : 1;       // [1]
        unsigned int rxCh2IntEna : 1;       // [2]
        unsigned int rxCh3IntEna : 1;       // [3]
        unsigned int rxCh4IntEna : 1;       // [4]
        unsigned int rxCh5IntEna : 1;       // [5]
        unsigned int rxCh6IntEna : 1;       // [6]
        unsigned int rxCh7IntEna : 1;       // [7]
        unsigned int reserved1 : 8;           // [15..8]
        unsigned int txCh0IntEna : 1;       // [16]
        unsigned int txCh1IntEna : 1;       // [17]
        unsigned int txCh2IntEna : 1;       // [18]
        unsigned int txCh3IntEna : 1;       // [19]
        unsigned int txCh4IntEna : 1;       // [20]
        unsigned int txCh5IntEna : 1;       // [21]
        unsigned int txCh6IntEna : 1;       // [22]
        unsigned int txCh7IntEna : 1;       // [23]
        unsigned int spdiftxCh0IntEna : 1;  // [24]
        unsigned int spdiftxCh1IntEna : 1;  // [25]
        unsigned int spdiftxCh2IntEna : 1;  // [26]
        unsigned int spdiftxCh3IntEna : 1;  // [27]
        unsigned int reserved0 : 4;           // [31..28]
    } Bits;
    unsigned int u32;
} UAiaoIntEna;

int32_t AiaoHalSysInit(void);
int32_t AiaoClockReset(void);
unsigned int AiaoHalReadReg(unsigned int offset);
int AopHalSetBuffRptr(unsigned int chnId, unsigned int value);
int AopHalSetBuffWptr(unsigned int chnId, unsigned int value);
int AopHalSetBufferAddr(unsigned int chnId, unsigned long long value);
int AipHalSetBufferAddr(unsigned int chnId, unsigned long long value);
int AipHalSetBufferSize(unsigned int chnId, unsigned int value);
int AipHalSetTransSize(unsigned int chnId, unsigned int value);
int AipHalSetRxStart(unsigned int chnId, bool en);
int AipHalSetBuffWptr(unsigned int chnId, unsigned int value);
int AipHalSetBuffRptr(unsigned int chnId, unsigned int value);
int AopHalSetBufferSize(unsigned int chnId, unsigned int value);
int AopHalSetTransSize(unsigned int chnId, unsigned int value);
int AopHalSetTxStart(unsigned int chnId, bool en);
int32_t AopHalDevEnable(unsigned int chnId);
uint32_t AipBuffRptrReg(uint32_t m);
uint32_t AipBuffWptrReg(uint32_t m);
uint32_t AopBuffRptrReg(uint32_t m);
uint32_t AopBuffWptrReg(uint32_t m);
int32_t AopSetSysCtlReg(int32_t chnId, unsigned int channelCnt, unsigned int bitWidth, unsigned int rate);
int32_t AopSetAttr(unsigned int chnId, unsigned int channelCnt, unsigned int bitWidth);
int32_t AipSetSysCtlReg(int32_t chnId, unsigned int channelCnt, unsigned int bitWidth, unsigned int rate);
int32_t AipSetAttr(unsigned int chnId, unsigned int channelCnt, unsigned int bitWidth);
void ShowAllAiaoRegister(void);
int32_t AiaoDeviceInit(unsigned int chnId);
int32_t I2sCrgCfgInit(unsigned int chnId);

#endif // __HI3516_ACODEC_H__
