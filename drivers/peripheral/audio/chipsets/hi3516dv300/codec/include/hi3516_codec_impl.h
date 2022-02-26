/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HI3516_CODEC_IMPL_H
#define HI3516_CODEC_IMPL_H

#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "hi3516_common.h"

#define HI_NULL                 0L
#define HI_SUCCESS              0
#define HI_FAILURE              (-1)

#define CRG_REG_ADDR            0x12010000
#define PERI_CRG103             (CRG_REG_ADDR + 0x019c)
#define PERI_CRG105             (CRG_REG_ADDR + 0x01a4)
#define PERI_CRG106             (CRG_REG_ADDR + 0x01a8)
#define PERI_CRG107             (CRG_REG_ADDR + 0x01ac)

#define ACODEC_REG_BASE          0x113c0000

#define AUDIO_CODEC_ANAREG0      0x14
#define AUDIO_CODEC_MASKREG      0x48
#define ACODEC_MAX_REG_SIZE (0x1000)
#define AUDIO_ATTR_FRAMENUM 30
#define AUDIO_ATTR_POINTNUM 320
#define ACODEC_CRG_OFFSET  0x0108
#define ACODEC_REGS_OFFSET 0x0

#define ACODEC_REGS_BASE 0x113c0000
#define SYS_CRG_BASE  0x12010000

/* acodec analog register */
#define ACODEC_ANAREG0_ADDR 0x14
#define ACODEC_ANAREG1_ADDR 0x18
#define ACODEC_ANAREG2_ADDR 0x1c
#define ACODEC_ANAREG3_ADDR 0x20
#define ACODEC_ANAREG4_ADDR 0x24
#define ACODEC_ANAREG5_ADDR 0x28

/* acodec dig control register */
#define ACODEC_DIGCTRL0_ADDR 0x2c
#define ACODEC_DIGCTRL1_ADDR 0x30
#define ACODEC_DIGCTRL2_ADDR 0x34
#define ACODEC_DIGCTRL3_ADDR 0x38
#define ACODEC_DIGCTRL4_ADDR 0x3c
#define ACODEC_DIGCTRL7_ADDR 0x48

#define ACODEC_ANAREG0_DEFAULT 0x6400FCFD
#define ACODEC_ANAREG1_DEFAULT 0x00000034
#define ACODEC_ANAREG2_DEFAULT 0x4018088D
#define ACODEC_ANAREG3_DEFAULT 0x00000000

#define UINT_32_MAX 0xffffffff

typedef enum {
    ACODEC_I2S_FS_8000 = 0x18,
    ACODEC_I2S_FS_11025 = 0x18,
    ACODEC_I2S_FS_12000 = 0x18,
    ACODEC_I2S_FS_16000 = 0x19,
    ACODEC_I2S_FS_22050 = 0x19,
    ACODEC_I2S_FS_24000 = 0x19,
    ACODEC_I2S_FS_32000 = 0x1a,
    ACODEC_I2S_FS_44100 = 0x1a,
    ACODEC_I2S_FS_48000 = 0x1a,
    ACODEC_I2S_FS_64000 = 0x1b,
    ACODEC_I2S_FS_96000 = 0x1b,
    ACODEC_I2S_FS_BUTT = 0x1c,
} AcodecI2sFs;

typedef enum {
    ACODEC_ADC_MODESEL_6144 = 0x0,
    ACODEC_ADC_MODESEL_4096 = 0x1,
    ACODEC_ADC_MODESEL_BUTT = 0xff,
} AcodecAdcModeSel;

typedef enum {
    AUDIO_CODEC_BIT_WIDTH_16  = 0,   /* 16bit width */
    AUDIO_CODEC_BIT_WIDTH_18  = 1,   /* 18bit width */
    AUDIO_CODEC_BIT_WIDTH_20  = 2,   /* 20bit width */
    AUDIO_CODEC_BIT_WIDTH_24  = 3,   /* 24bit width */
    AUDIO_CODEC_BIT_WIDTH_BUTT,
} AudioCodecBitWidth;

typedef union {
    struct {
        unsigned int acodecPdVref                     : 1;    // [0]
        unsigned int acodecPdbCtcmIbias              : 1;    // [1]
        unsigned int reserved0                          : 3;    // [2:4]
        unsigned int acodecPdMicbias1                 : 1;    // [5]
        unsigned int acodecPdIbias                    : 1;    // [6]
        unsigned int acodecPdMicbias2                 : 1;    // [7]
        unsigned int acodecPdDaclDff                 : 1;    // [8]
        unsigned int acodecPdDacrDff                 : 1;    // [9]
        unsigned int reserved1                          : 1;    // [10]
        unsigned int acodecPdDacl                     : 1;    // [11]
        unsigned int acodecPdDacr                     : 1;    // [12]
        unsigned int acodecMuteDacl                   : 1;    // [13]
        unsigned int acodecMuteDacr                   : 1;    // [14]
        unsigned int reserved2                          : 1;    // [15]
        unsigned int acodecDacrPopEn                 : 1;    // [16]
        unsigned int reserved3                          : 1;    // [17]
        unsigned int acodecDaclPopEn                 : 1;    // [18]
        unsigned int acodecAnaLoop                    : 1;    // [19]
        unsigned int acodecBypChopDacVref           : 1;    // [20]
        unsigned int reserved4                          : 1;    // [21]
        unsigned int acodecSelClkChopDacVref       : 2;    // [23:22]
        unsigned int reserved5                          : 2;    // [24:25]
        unsigned int acodecIbadjDac                   : 2;    // [27:26]
        unsigned int acodecCtrlClkDacPh             : 1;    // [28]
        unsigned int acodecSelClkChopLineoutD      : 2;    // [30:29]
        unsigned int acodecIbadjCtcm                  : 1;    // [31]
    } Bits;
    unsigned int ul32;
} AcodecAnaReg0;

typedef union {
    struct {
        unsigned int acodecBypChopCtcm               : 1;    // [0]
        unsigned int acodecSelClkChopCtcm           : 1;    // [1]
        unsigned int acodecIbadjAdc                   : 2;    // [3:2]
        unsigned int reserved0                          : 2;    // [4:5]
        unsigned int acodecCtrlClkAdcPh             : 1;    // [6]
        unsigned int acodecCtrlMclkPh                : 1;    // [7]
        unsigned int acodecPgaRinManFloat           : 6;    // [13:8]
        unsigned int acodecPgaRinManEn              : 1;    // [14]
        unsigned int reserved1                          : 2;    // [15:16]
        unsigned int acodecRxCtcmPd                  : 1;    // [17]
        unsigned int reserved3                          : 2;    // [19:18]
        unsigned int acodecAdcDaIbSel              : 2;    // [21:20]
        unsigned int acodecRpgaBoost                  : 1;    // [22]
        unsigned int acodecLpgaBoost                  : 1;    // [23]
        unsigned int acodecAdcDacRlMisSelp         : 4;    // [27:24]
        unsigned int acodecAdcDacRlMisSeln         : 4;    // [31:28]
    } Bits;
    unsigned int ul32;
} AcodecAnaReg1;

typedef union {
    struct {
        unsigned int acodecLdoPd                      : 1;    // [0]
        unsigned int acodecLdoBk                      : 1;    // [1]
        unsigned int acodecLdoSel                     : 2;    // [3:2]
        unsigned int acodecAdcVref                    : 2;    // [5:4]
        unsigned int acodecAdcrModeSel               : 1;    // [6]
        unsigned int acodecAdclModeSel               : 1;    // [7]
        unsigned int reserved0                          : 5;    // [12:8]
        unsigned int acodecBypChopCtcmRx            : 1;    // [13]
        unsigned int acodecBypChopperAdc             : 2;    // [14]
        unsigned int acodecBypChopperLinein          : 2;    // [14]
        unsigned int reserved1                          : 1;    // [16]
        unsigned int acodecVrefFs                     : 1;    // [17]
        unsigned int acodecVrefSel                    : 5;    // [22:18]
        unsigned int acodecRst                         : 1;    // [23]
        unsigned int acodecMicbias1Adj                : 3;    // [26:24]
        unsigned int acodecMicbias2Adj                : 3;    // [29:27]
        unsigned int acodecPdCtcm                     : 1;    // [30]
        unsigned int reserved2                          : 1;    // [31]
    } Bits;
    unsigned int ul32;
} AcodecAnaReg2;

typedef union {
    struct {
        unsigned int acodecDacrPopDirect             : 1;    // [0]
        unsigned int reserved0                          : 1;    // [1]
        unsigned int acodecDaclPopDirect             : 1;    // [2]
        unsigned int acodecPopresSel                  : 2;    // [4:3]
        unsigned int acodecPoprampclkSel              : 2;    // [6:5]
        unsigned int acodecVrefExmode                 : 1;    // [7]
        unsigned int reserved1                          : 1;    // [8]
        unsigned int acodecRpgaMute09                : 1;    // [9]
        unsigned int acodecLpgaMute09                : 1;    // [10]
        unsigned int acodecPdAdcTune09              : 1;    // [11]
        unsigned int acodecRpgaPd09                  : 1;    // [12]
        unsigned int acodecLpgaPd09                  : 1;    // [13]
        unsigned int acodecAdcrPd09                  : 1;    // [14]
        unsigned int acodecAdclPd09                  : 1;    // [15]
        unsigned int acodecLgpaGain09                : 5;    // [20:16]
        unsigned int acodecLpgaSel09                 : 3;    // [23:21]
        unsigned int acodecRgpaGain09                : 5;    // [28:24]
        unsigned int acodecRpgaSel09                 : 3;    // [31:29]
    } Bits;
    unsigned int ul32;
} AcodecAnaReg3;

typedef union {
    struct {
        unsigned int acodecAdcDwaBps09              : 1;    // [0]
        unsigned int acodecAdcBinClkPhsel09        : 1;    // [1]
        unsigned int acodecAdcTheClkPhsel09        : 1;    // [2]
        unsigned int acodecAdcChopClkPhsel09       : 1;    // [3]
        unsigned int acodecAdcChopClkSel09         : 2;    // [5:4]
        unsigned int acodecPgaChopClkSel09         : 2;    // [7:6]
        unsigned int reserved0                          : 1;    // [8]
        unsigned int acodecCtrlClkAdcph09          : 1;    // [9]
        unsigned int acodecAdcTuneSel09             : 1;    // [10]
        unsigned int acodecAdcTuneEn09              : 1;    // [11]
        unsigned int acodecAdcTune09                 : 4;    // [15:12]
        unsigned int reserved1                          : 16;   // [31:16]
    } Bits;
    unsigned int ul32;
} AcodecAnaReg4;

typedef union {
    struct {
        unsigned int adcr2dacrVol     : 7;  // [6:0]
        unsigned int adcr2dacrEn      : 1;  // [7]
        unsigned int adcl2dacrVol     : 7;  // [14:8]
        unsigned int adcl2dacrEn      : 1;  // [15]
        unsigned int adcr2daclVol     : 7;  // [22:16]
        unsigned int adcr2daclEn      : 1;  // [23]
        unsigned int adcl2daclVol     : 7;  // [30:24]
        unsigned int adcl2daclEn      : 1;  // [31]
    } Bits;
    unsigned int ul32;
} AcodecDigCtrl5;

typedef union {
    struct {
        unsigned int adcrLrsel       : 1;  // [0]
        unsigned int adcrI2ssel      : 1;  // [1]
        unsigned int adclLrsel       : 1;  // [2]
        unsigned int adclI2ssel      : 1;  // [3]
        unsigned int reserved         : 10;  // [13:4]
        unsigned int adcrHpfEn      : 1;  // [14]
        unsigned int adclHpfEn      : 1;  // [15]
        unsigned int adcrVol         : 7;  // [22:16]
        unsigned int adcrMute        : 1;  // [23]
        unsigned int adclVol         : 7;  // [30:24]
        unsigned int adclMute        : 1;  // [31]
    } Bits;
    unsigned int ul32;
} AcodecDigCtrl4;

typedef union {
    struct {
        unsigned int dacl2dacrVol    : 7;  // [6:0]
        unsigned int dacl2dacrEn     : 1;  // [7]
        unsigned int dacr2daclVol    : 7;  // [14:8]
        unsigned int dacr2daclEn     : 1;  // [15]
        unsigned int dacrVol         : 7;  // [22:16]
        unsigned int dacrMute        : 1;  // [23]
        unsigned int daclVol         : 7;  // [30:24]
        unsigned int daclMute        : 1;  // [31]
    } Bits;
    unsigned int ul32;
} AcodecDigCtrl3;

typedef union {
    struct {
        unsigned int dacrLrsel       : 1;  // [0]
        unsigned int dacrI2ssel      : 1;  // [1]
        unsigned int daclLrsel       : 1;  // [2]
        unsigned int daclI2ssel      : 1;  // [3]
        unsigned int reserved         : 15;  // [18:4]
        unsigned int dacrDeemph      : 2;  // [20:19]
        unsigned int daclDeemph      : 2;  // [22:21]
        unsigned int muterRate       : 2;  // [24:23]
        unsigned int mutelRate       : 2;  // [26:25]
        unsigned int dacvu            : 1;  // [27]
        unsigned int sunmuter         : 1;  // [28]
        unsigned int sunmutel         : 1;  // [29]
        unsigned int smuter           : 1;  // [30]
        unsigned int smutel           : 1;  // [31]
    } Bits;
    unsigned int ul32;
} AcodecDigCtrl2;

typedef union {
    struct {
        unsigned int rst              : 1;  // [0]
        unsigned int adcFlstn        : 1;  // [1]
        unsigned int adcAdatn        : 2;  // [3:2]
        unsigned int ibadjCtcm       : 1;  // [4]
        unsigned int ibadjDac        : 1;  // [5]
        unsigned int ibadjAdc        : 2;  // [7:6]
        unsigned int i2s2FsSel      : 5;  // [12:8]
        unsigned int i2s1FsSel      : 5;  // [17:13]
        unsigned int digLoop         : 1;  // [18]
        unsigned int digBypass       : 1;  // [19]
        unsigned int i2s2DataBits   : 2;  // [21:20]
        unsigned int i2s1DataBits   : 2;  // [23:22]
        unsigned int adcrEn          : 1;  // [24]
        unsigned int adclEn          : 1;  // [25]
        unsigned int dacrEn          : 1;  // [26]
        unsigned int daclEn          : 1;  // [27]
        unsigned int adcrRstN       : 1;  // [28]
        unsigned int adclRstN       : 1;  // [29]
        unsigned int dacrRstN       : 1;  // [30]
        unsigned int daclRstN       : 1;  // [31]
    } Bits;
    unsigned int ul32;
} AcodecDigCtrl1;

typedef struct {
    volatile AcodecDigCtrl1   acodecDigctrl1;
    volatile AcodecDigCtrl2   acodecDigctrl2;
    volatile AcodecDigCtrl3   acodecDigctrl3;
} AcodecRegs;

int32_t CodecHalSysInit(void);
int AcodecDeviceInit(void);
unsigned int AcodecHalReadReg(unsigned int offset);
void AcodecHalWriteReg(uint32_t offset, uint32_t value);
long IoctlGetOutputVol(void);
int32_t AcodecSetI2s1Fs(const unsigned int rate);
int32_t AcodecSetI2s1DataWidth(const unsigned int bitWidth);
void ShowAllAcodecRegister(void);
#endif // __HI3516_ACODEC_H__
