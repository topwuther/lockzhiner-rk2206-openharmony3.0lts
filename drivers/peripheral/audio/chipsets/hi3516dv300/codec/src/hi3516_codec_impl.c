/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hi3516_codec_impl.h"
#include <asm/io.h>
#include "audio_device_log.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_io.h"

#define HDF_LOG_TAG hi3516_codec_impl

static void *g_regAcodecBase = NULL; // CODEC Reg Base Addr
const int HALF_MINUTE = 30;
const int VOLUME_DB_MAX = 6;
const int VOLUME_DB_MIN = -121;

int32_t CodecHalSysInit(void)
{
    // ACODEC REMAP
    if (g_regAcodecBase == NULL) {
        g_regAcodecBase = OsalIoRemap(ACODEC_REG_BASE, ACODEC_MAX_REG_SIZE); // ACODEC_MAX_REG_SIZE
        if (g_regAcodecBase == NULL) {
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

void AcodecHalWriteReg(unsigned int offset, unsigned int value)
{
    *(volatile unsigned int *)((unsigned char *)(g_regAcodecBase) + (unsigned int)(offset)) = value;
}

unsigned int AcodecHalReadReg(unsigned int offset)
{
    return (*(volatile unsigned int *)((unsigned char *)g_regAcodecBase + (unsigned int)offset));
}

static int AcodecSoftReset(void)
{
    AcodecDigCtrl1 acodecDigctrl1;
    AcodecDigCtrl2 acodecDigctrl2;
    AcodecDigCtrl3 acodecDigctrl3;
    AcodecDigCtrl4 acodecDigctrl4;

    AcodecAnaReg0 acodecAnareg0;
    AcodecAnaReg1 acodecAnareg1;
    AcodecAnaReg2 acodecAnareg2;
    AcodecAnaReg3 acodecAnareg3;
    AcodecAnaReg4 acodecAnareg4;

    if (g_regAcodecBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("haven't ioremap acodec regs.");
        return HDF_FAILURE;
    }

    acodecAnareg0.ul32 = 0x04000002;
    AcodecHalWriteReg(ACODEC_ANAREG0_ADDR, acodecAnareg0.ul32);
    acodecAnareg1.ul32 = 0xFD200004;
    AcodecHalWriteReg(ACODEC_ANAREG1_ADDR, acodecAnareg1.ul32);
    acodecAnareg2.ul32 = 0x00180018;
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnareg2.ul32);
    acodecAnareg3.ul32 = 0x83830028;
    AcodecHalWriteReg(ACODEC_ANAREG3_ADDR, acodecAnareg3.ul32);
    acodecAnareg4.ul32 = 0x00005C5C;
    AcodecHalWriteReg(ACODEC_ANAREG4_ADDR, acodecAnareg4.ul32);
    // offset 0x28
    AcodecHalWriteReg(ACODEC_ANAREG5_ADDR, 0x130000);

    acodecDigctrl1.ul32 = 0xff035a00;
    AcodecHalWriteReg(ACODEC_DIGCTRL1_ADDR, acodecDigctrl1.ul32);
    acodecDigctrl2.ul32 = 0x08000001;
    AcodecHalWriteReg(ACODEC_DIGCTRL2_ADDR, acodecDigctrl2.ul32);
    acodecDigctrl3.ul32 = 0x06062424;
    AcodecHalWriteReg(ACODEC_DIGCTRL3_ADDR, acodecDigctrl3.ul32);
    acodecDigctrl4.ul32 = 0x1e1ec001;
    AcodecHalWriteReg(ACODEC_DIGCTRL4_ADDR, acodecDigctrl4.ul32);

    return HDF_SUCCESS;
}

/* 0x14, 0x18, 0x1c, 0x20 */
static void AcodecInitInner(AcodecAnaReg0 *acodecAnareg0, AcodecAnaReg1 *acodecAnareg1,
                            AcodecAnaReg2 *acodecAnareg2, AcodecAnaReg3 *acodecAnareg3)
{
    acodecAnareg2->Bits.acodecRst = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnareg2->ul32);

    acodecAnareg3->Bits.acodecPopresSel = 0x1;
    acodecAnareg3->Bits.acodecPoprampclkSel = 0x1;
    AcodecHalWriteReg(ACODEC_ANAREG3_ADDR, acodecAnareg3->ul32);
    acodecAnareg2->Bits.acodecVrefSel = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnareg2->ul32);

    acodecAnareg0->Bits.acodecPdbCtcmIbias = 0x1;
    acodecAnareg0->Bits.acodecPdIbias = 0x1;
    AcodecHalWriteReg(ACODEC_ANAREG0_ADDR, acodecAnareg0->ul32);

    acodecAnareg1->Bits.acodecRxCtcmPd = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG1_ADDR, acodecAnareg1->ul32);

    acodecAnareg2->Bits.acodecLdoPd = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnareg2->ul32);

    acodecAnareg0->Bits.acodecPdVref = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG0_ADDR, acodecAnareg0->ul32);

    acodecAnareg3->Bits.acodecDacrPopDirect = 0x1;
    acodecAnareg3->Bits.acodecDaclPopDirect = 0x1;
    AcodecHalWriteReg(ACODEC_ANAREG3_ADDR, acodecAnareg3->ul32);

    OsalMSleep(HALF_MINUTE);

    acodecAnareg0->Bits.acodecPdDacr = 0x0;
    acodecAnareg0->Bits.acodecPdDacl = 0x0;
    acodecAnareg0->Bits.acodecMuteDacr = 0x0;
    acodecAnareg0->Bits.acodecMuteDacl = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG0_ADDR, acodecAnareg0->ul32);

    acodecAnareg0->Bits.acodecDacrPopEn = 0x0;
    acodecAnareg0->Bits.acodecDaclPopEn = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG0_ADDR, acodecAnareg0->ul32);
}

int AcodecDeviceInit(void)
{
    int ret;
    const int tenSeconds = 10;
    AcodecAnaReg0 acodecAnareg0;
    AcodecAnaReg1 acodecAnareg1;
    AcodecAnaReg2 acodecAnareg2;
    AcodecAnaReg3 acodecAnareg3;
    AcodecAnaReg4 acodecAnareg4;
    OsalMSleep(tenSeconds); /* sleep 10 ms */
    /* 0x14, 0x18, 0x1c, 0x20, 0x24 */
    acodecAnareg0.ul32 = 0x040578E1;
    AcodecHalWriteReg(ACODEC_ANAREG0_ADDR, acodecAnareg0.ul32);
    acodecAnareg1.ul32 = 0xFD220004;
    AcodecHalWriteReg(ACODEC_ANAREG1_ADDR, acodecAnareg1.ul32);
    acodecAnareg2.ul32 = 0x4098001B;
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnareg2.ul32);
    acodecAnareg3.ul32 = 0x8383FE00;
    AcodecHalWriteReg(ACODEC_ANAREG3_ADDR, acodecAnareg3.ul32);
    acodecAnareg4.ul32 = 0x0000505C;
    AcodecHalWriteReg(ACODEC_ANAREG4_ADDR, acodecAnareg4.ul32);
    AcodecHalWriteReg(ACODEC_ANAREG5_ADDR, 0x0);
    AcodecInitInner(&acodecAnareg0, &acodecAnareg1, &acodecAnareg2, &acodecAnareg3);
    acodecAnareg2.Bits.acodecLdoBk = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnareg2.ul32);
    acodecAnareg3.Bits.acodecPdAdcTune09 = 0x0;
    AcodecHalWriteReg(ACODEC_ANAREG3_ADDR, acodecAnareg3.ul32);
    acodecAnareg4.Bits.acodecAdcTuneSel09 = 0x1;
    AcodecHalWriteReg(ACODEC_ANAREG4_ADDR, acodecAnareg4.ul32);
    acodecAnareg4.Bits.acodecAdcTuneEn09 = 0x1;
    AcodecHalWriteReg(ACODEC_ANAREG4_ADDR, acodecAnareg4.ul32);

    ret = AcodecSoftReset();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_DEBUG("AcodecSoftReset fail");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

void ShowAllAcodecRegister(void)
{
    volatile unsigned int val;

    val = AcodecHalReadReg(ACODEC_ANAREG0_ADDR);
    AUDIO_DEVICE_LOG_DEBUG("ACODEC REG: ACODEC_ANAREG0_ADDR 0014 = [%08x]", val);

    val = AcodecHalReadReg(ACODEC_ANAREG3_ADDR);
    AUDIO_DEVICE_LOG_DEBUG("ACODEC REG: ACODEC_ANAREG0_ADDR 0020 = [%08x]", val);

    // ACODEC REG 0030
    val = AcodecHalReadReg(ACODEC_DIGCTRL1_ADDR);
    AUDIO_DEVICE_LOG_DEBUG("ACODEC REG: ACODEC_DIGCTRL1_ADDR 0030 = [%08x]", val);

    val = AcodecHalReadReg(ACODEC_DIGCTRL3_ADDR);
    AUDIO_DEVICE_LOG_DEBUG("ACODEC REG: ACODEC_DIGCTRL3_ADDR 0038 = [%08x]", val);

    // ACODEC REG 0048
    val = AcodecHalReadReg(AUDIO_CODEC_MASKREG);
    AUDIO_DEVICE_LOG_DEBUG("ACODEC REG: AUDIO_CODEC_MASKREG 0048 = [%08x]", val);
}

static unsigned int AcodecGetI2sFs(const unsigned int rate)
{
    switch (rate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_11025:
        case AUDIO_SAMPLE_RATE_12000:
            return ACODEC_I2S_FS_8000;
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_22050:
        case AUDIO_SAMPLE_RATE_24000:
            return ACODEC_I2S_FS_16000;
        case AUDIO_SAMPLE_RATE_32000:
        case AUDIO_SAMPLE_RATE_44100:
        case AUDIO_SAMPLE_RATE_48000:
            return ACODEC_I2S_FS_32000;
        case AUDIO_SAMPLE_RATE_64000:
        case AUDIO_SAMPLE_RATE_96000:
            return ACODEC_I2S_FS_64000;
        default:
            AUDIO_DEVICE_LOG_DEBUG("unsupport samplerate %d\n", rate);
            return ACODEC_I2S_FS_BUTT;
    }
}

static unsigned int AcodecGetAdcModeSel(const unsigned int rate)
{
    switch (rate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_32000:
        case AUDIO_SAMPLE_RATE_64000:
            return ACODEC_ADC_MODESEL_4096;
        case AUDIO_SAMPLE_RATE_11025:
        case AUDIO_SAMPLE_RATE_12000:
        case AUDIO_SAMPLE_RATE_22050:
        case AUDIO_SAMPLE_RATE_24000:
        case AUDIO_SAMPLE_RATE_44100:
        case AUDIO_SAMPLE_RATE_48000:
        case AUDIO_SAMPLE_RATE_96000:
            return ACODEC_ADC_MODESEL_6144;
        default:
            AUDIO_DEVICE_LOG_DEBUG("unsupport samplerate %d.\n", rate);
            return ACODEC_I2S_FS_BUTT;
    }
}

int32_t AcodecSetI2s1Fs(const unsigned int rate)
{
    AcodecDigCtrl1 acodecDigctrl1;
    AcodecAnaReg2 acodecAnaReg2;
    AcodecAnaReg4 acodecAnaReg4;

    if (rate >= AUDIO_SAMPLE_RATE_BUTT) {
        AUDIO_DEVICE_LOG_ERR("bad value, please use acodec_fs define\n");
        return HDF_FAILURE;
    }

    acodecDigctrl1.ul32 = AcodecHalReadReg(ACODEC_DIGCTRL1_ADDR);
    acodecDigctrl1.Bits.i2s1FsSel = AcodecGetI2sFs(rate);
    AcodecHalWriteReg(ACODEC_DIGCTRL1_ADDR, acodecDigctrl1.ul32);

    acodecAnaReg2.ul32 = AcodecHalReadReg(ACODEC_ANAREG2_ADDR);
    acodecAnaReg2.Bits.acodecAdcrModeSel = AcodecGetAdcModeSel(rate);
    acodecAnaReg2.Bits.acodecAdclModeSel = AcodecGetAdcModeSel(rate);
    AcodecHalWriteReg(ACODEC_ANAREG2_ADDR, acodecAnaReg2.ul32);

    /* rctune */
    acodecAnaReg4.ul32 = AcodecHalReadReg(ACODEC_ANAREG4_ADDR);
    acodecAnaReg4.Bits.acodecAdcTuneEn09 = 0;
    AcodecHalWriteReg(ACODEC_ANAREG4_ADDR, acodecAnaReg4.ul32);

    OsalUDelay(HALF_MINUTE); /* wait 30 us. */
    acodecAnaReg4.ul32 = AcodecHalReadReg(ACODEC_ANAREG4_ADDR);
    acodecAnaReg4.Bits.acodecAdcTuneEn09 = 1;
    AcodecHalWriteReg(ACODEC_ANAREG4_ADDR, acodecAnaReg4.ul32);

    return HDF_SUCCESS;
}

int32_t AcodecSetI2s1DataWidth(const unsigned int bitWidth)
{
    AcodecDigCtrl1 acodecDigctrl1;
    AudioCodecBitWidth codecBitWidth;
    codecBitWidth = AUDIO_CODEC_BIT_WIDTH_16;
    switch (bitWidth) {
        case BIT_WIDTH16:
            codecBitWidth = AUDIO_CODEC_BIT_WIDTH_16;
            break;
        case BIT_WIDTH18:
            codecBitWidth = AUDIO_CODEC_BIT_WIDTH_18;
            break;
        case BIT_WIDTH20:
            codecBitWidth = AUDIO_CODEC_BIT_WIDTH_20;
            break;
        case BIT_WIDTH24:
            codecBitWidth = AUDIO_CODEC_BIT_WIDTH_24;
            break;
        default:
            return HDF_FAILURE;
            break;
    }

    acodecDigctrl1.ul32 = AcodecHalReadReg(ACODEC_DIGCTRL1_ADDR);
    acodecDigctrl1.Bits.i2s1DataBits = codecBitWidth;
    AcodecHalWriteReg(ACODEC_DIGCTRL1_ADDR, acodecDigctrl1.ul32);
    return HDF_SUCCESS;
}
