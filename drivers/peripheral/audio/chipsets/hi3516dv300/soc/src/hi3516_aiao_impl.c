/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hi3516_aiao_impl.h"
#include <asm/io.h>
#include "audio_device_log.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_io.h"

#define HDF_LOG_TAG hi3516_aiao_impl

void *g_regAiaoBase = NULL;   // AIAO Reg Base Addr

AiaoClkInfo g_aioClkInfo = {
    .clkSelect = HI_AUDIO_CLK_SELECT_BASE,

    .mclkDiv48k256fs = AIO_MCLK_48K_256FS_1188M,
    .mclkDiv441k256fs = AIO_MCLK_441K_256FS_1188M,
    .mclkDiv32k256fs = AIO_MCLK_32K_256FS_1188M,

    .mclkDiv48k320fs = AIO_MCLK_48K_320FS_1188M,
    .mclkDiv441k320fs = AIO_MCLK_441K_320FS_1188M,
    .mclkDiv32k320fs = AIO_MCLK_32K_320FS_1188M,
};

static uint32_t AipI2sRegCfg0(uint32_t n)
{
    return AIP_I2S_REG_CFG0 + OFFSET_MULTL * (n);
}

static uint32_t AipI2sRegCfg1(uint32_t n)
{
    return AIP_I2S_REG_CFG1 + OFFSET_MULTL * (n);
}

static uint32_t AopI2sRegCfg0(uint32_t n)
{
    return AOP_I2S_REG_CFG0 + OFFSET_MULTL * (n);
}

static uint32_t AopI2sRegCfg1(uint32_t n)
{
    return AOP_I2S_REG_CFG1 + OFFSET_MULTL * (n);
}
/* aop */
static uint32_t AopBuffSaddrReg(uint32_t m)
{
    return AOP_BUFF_SADDR_REG + OFFSET_MULTL * (m);
}

static uint32_t AopBuffSizeReg(uint32_t m)
{
    return AOP_BUFF_SIZE_REG + OFFSET_MULTL * (m);
}

uint32_t AopBuffWptrReg(uint32_t m)
{
    return AOP_BUFF_WPTR_REG + OFFSET_MULTL * (m);
}

uint32_t AopBuffRptrReg(uint32_t m)
{
    return AOP_BUFF_RPTR_REG + OFFSET_MULTL * (m);
}

static uint32_t AopTransSizeReg(uint32_t m)
{
    return AOP_TRANS_SIZE_REG + OFFSET_MULTL * (m);
}

static uint32_t AipBuffSaddrReg(uint32_t m)
{
    return AIP_BUFF_SADDR_REG + OFFSET_MULTL * (m);
}

static uint32_t AipBuffSizeReg(uint32_t m)
{
    return AIP_BUFF_SIZE_REG + OFFSET_MULTL * (m);
}

uint32_t AipBuffWptrReg(uint32_t m)
{
    return AIP_BUFF_WPTR_REG + OFFSET_MULTL * (m);
}

uint32_t AipBuffRptrReg(uint32_t m)
{
    return AIP_BUFF_RPTR_REG + OFFSET_MULTL * (m);
}

static uint32_t AipTransSizeReg(uint32_t m)
{
    return AIP_TRANS_SIZE_REG + OFFSET_MULTL * (m);
}

/* aip */
static uint32_t AipInfAttriReg(uint32_t n)
{
    return AIP_INF_ATTRI_REG + OFFSET_MULTL * (n);
}

static uint32_t AipCtrlReg(uint32_t n)
{
    return AIP_CTRL_REG + OFFSET_MULTL * (n);
}
/* aop */
static uint32_t AopInfAttriReg(uint32_t m)
{
    return AOP_INF_ATTRI_REG + OFFSET_MULTL * (m);
}

static uint32_t AopCtrlReg(uint32_t m)
{
    return AOP_CTRL_REG + OFFSET_MULTL * (m);
}

/* Mapping physical address to  virtual address, for acodec and aiao */
int32_t AiaoHalSysInit(void)
{
    // AIAO REMAP
    if (g_regAiaoBase == NULL) {
        g_regAiaoBase = OsalIoRemap(AIAO_REG_BASE, AIAO_MAX_REG_SIZE);
        if (g_regAiaoBase == NULL) {
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t AiaoClockReset(void)
{
    volatile char *vAddr = NULL;
    volatile const unsigned int crgVal = 0xa; // AIAO CLOCK RESET
    volatile unsigned int regval;

    /* RESET AIAO */
    vAddr = OsalIoRemap(PERI_CRG103, sizeof(unsigned int));
    if (vAddr == NULL) {
        AUDIO_DEVICE_LOG_ERR("vAddr is null \n");
        return HDF_FAILURE;
    }

    regval = OSAL_READL(vAddr);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO CLOCK PERI_CRG103, Val: 0x%08x", regval);

    OSAL_WRITEL(crgVal, vAddr);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO CLOCK PERI_CRG103, Val: 0x%08x", crgVal);
    OsalIoUnmap((void *)((uintptr_t)(void*)vAddr));
    return HDF_SUCCESS;
}

uint32_t AiaoHalReadReg(uint32_t offset)
{
    if (g_regAiaoBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("g_regAiaoBase is null.\n");
        return 0x0;
    }

    return (*(volatile uint32_t *)((unsigned char *)g_regAiaoBase + (unsigned int)offset));
}

static int32_t AiaoGetBclkSel(unsigned int bclkDiv, unsigned int *bclkSel)
{
    switch (bclkDiv) {
        case AIO_MCLK_TO_BCLK_RATIO_1:
            *bclkSel = SYS_AIO_BS_CLK1;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_2:
            *bclkSel = SYS_AIO_BS_CLK2;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_3:
            *bclkSel = SYS_AIO_BS_CLK3;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_4:
            *bclkSel = SYS_AIO_BS_CLK4;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_6:
            *bclkSel = SYS_AIO_BS_CLK6;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_8:
            *bclkSel = SYS_AIO_BS_CLK8;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_12:
            *bclkSel = SYS_AIO_BS_CLK12;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_16:
            *bclkSel = SYS_AIO_BS_CLK16;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_24:
            *bclkSel = SYS_AIO_BS_CLK24;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_32:
            *bclkSel = SYS_AIO_BS_CLK32;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_48:
            *bclkSel = SYS_AIO_BS_CLK48;
            break;

        case AIO_MCLK_TO_BCLK_RATIO_64:
            *bclkSel = SYS_AIO_BS_CLK64;
            break;

        default:
            AUDIO_DEVICE_LOG_ERR("not support this bclkDivision ratio\n");
            return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t AiaoGetLrclkSel(unsigned int lrclkDiv, unsigned int *lrclkSel)
{
    switch (lrclkDiv) {
        case AIO_BCLK_TO_FSCLK_RATIO_256: {
            *lrclkSel = SYS_AIO_SAMPLE_CLK256;
            break;
        }

        case AIO_BCLK_TO_FSCLK_RATIO_128: {
            *lrclkSel = SYS_AIO_SAMPLE_CLK128;
            break;
        }

        case AIO_BCLK_TO_FSCLK_RATIO_64: {
            *lrclkSel = SYS_AIO_SAMPLE_CLK64;
            break;
        }

        case AIO_BCLK_TO_FSCLK_RATIO_48: {
            *lrclkSel = SYS_AIO_SAMPLE_CLK48;
            break;
        }

        case AIO_BCLK_TO_FSCLK_RATIO_32: {
            *lrclkSel = SYS_AIO_SAMPLE_CLK32;
            break;
        }

        case AIO_BCLK_TO_FSCLK_RATIO_16: {
            *lrclkSel = SYS_AIO_SAMPLE_CLK16;
            break;
        }

        default: {
            AUDIO_DEVICE_LOG_ERR("not support this fsclk_division ratio\n");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

void AiaoHalWriteReg(unsigned int offset, unsigned int value)
{
    if (g_regAiaoBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("g_aio_base is null.\n");
        return;
    }

    *(volatile  unsigned int *)((unsigned char *)(g_regAiaoBase) + (unsigned int)(offset)) = value;
}

static void AopSetCtrlReg(unsigned int chnId)
{
    UTxDspCtrl aopCtrlReg;
    aopCtrlReg.u32 = AiaoHalReadReg(AopCtrlReg(chnId));

    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopSetCtrlReg AopSetAttrReg read = %08x\n", aopCtrlReg.u32);

    aopCtrlReg.Bits.muteEn = 0;
    aopCtrlReg.Bits.muteFadeEn = 0;
    aopCtrlReg.Bits.reserved3 = 0;
    aopCtrlReg.Bits.volume = 0x79; /* 0db */
    aopCtrlReg.Bits.reserved2 = 0;
    aopCtrlReg.Bits.fadeInRate = 0;
    aopCtrlReg.Bits.fadeOutRate = 0;
    aopCtrlReg.Bits.reserved1 = 0;
    aopCtrlReg.Bits.bypassEn = 0;
    aopCtrlReg.Bits.txEnable = 0;
    aopCtrlReg.Bits.txDisableDone = 0;
    aopCtrlReg.Bits.reserved0 = 0;

    AiaoHalWriteReg(AopCtrlReg(chnId), aopCtrlReg.u32);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopSetCtrlReg AopSetAttrReg write = %08x\n", aopCtrlReg.u32);
}

int AopHalSetBufferAddr(unsigned int chnId, unsigned long long value)
{
    unsigned int saddr;

    if (chnId >=  W_CHANNEL_ID_MAX) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    saddr = (unsigned int)(value & 0xffffffff);
    AiaoHalWriteReg(AopBuffSaddrReg(chnId), saddr); // buf start
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopHalSetBufferAddr: write AopHalSetBufferAddr = 0x%08x\n", saddr);

    return HDF_SUCCESS;
}

int AipHalSetBufferAddr(unsigned int chnId, unsigned long long value)
{
    unsigned int saddr;

    if (chnId >= W_CHANNEL_ID_MAX) {
        AUDIO_DEVICE_LOG_ERR("ai_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    saddr = (unsigned int)(value & 0xffffffff);
    AiaoHalWriteReg(AipBuffSaddrReg(chnId), saddr); // buf start
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] aip_set_ctrl_reg: write aip_hal_set_buffer_addr = 0x%08x\n", saddr);

    return HDF_SUCCESS;
}

int AipHalSetBuffWptr(unsigned int chnId, unsigned int value)
{
    UTxBuffWptr unTmp;

    if (chnId >= W_CHANNEL_ID_MAX) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }
    unTmp.u32 = AiaoHalReadReg(AipBuffWptrReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ aip_hal_set_buff_wptr: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);
    unTmp.Bits.txBuffWptr = value;
    AiaoHalWriteReg(AipBuffWptrReg(chnId), unTmp.u32);

    return HDF_SUCCESS;
}

int AipHalSetBuffRptr(unsigned int chnId, unsigned int value)
{
    UTxBuffRptr unTmp;

    if (chnId >= HI_AI_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("ai_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AipBuffRptrReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ aip_hal_set_buff_rptr: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txBuffRptr = value;
    AiaoHalWriteReg(AipBuffRptrReg(chnId), unTmp.u32);

    return HDF_SUCCESS;
}

int AipHalSetBufferSize(unsigned int chnId, unsigned int value)
{
    UTxBuffSize unTmp;

    if (chnId >= HI_AO_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AipBuffSizeReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ aip_hal_set_buffer_size: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txBuffSize = value;
    AiaoHalWriteReg(AipBuffSizeReg(chnId), unTmp.u32);

    return HDF_SUCCESS;
}

int AipHalSetTransSize(unsigned int chnId, unsigned int value)
{
    UTxTransSize unTmp;

    if (chnId >= HI_AO_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("ai_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AipTransSizeReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ aip_hal_set_trans_size: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txTransSize = value;
    AiaoHalWriteReg(AipTransSizeReg(chnId), unTmp.u32);

    return HDF_SUCCESS;
}

int AipHalSetRxStart(unsigned int chnId, bool en)
{
    URxDspCtrl unTmp;

    if (chnId >= HI_AI_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("AipHalSetRxStart chnId %d is invalid!\n", chnId);
        return -1;
    }

    unTmp.u32 = AiaoHalReadReg(AipCtrlReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AipHalSetRxStart: AIP_CTRL_REG read %08x \n", unTmp.u32);

    unTmp.Bits.rxDisableDone = 0;
    unTmp.Bits.rxEnable = en;

    AiaoHalWriteReg(AipCtrlReg(chnId), unTmp.u32);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AipHalSetRxStart: AIP_CTRL_REG write %08x \n", unTmp.u32);

    return 0;
}

int g_debugTick = 0;
int AopHalSetBuffWptr(unsigned int chnId, unsigned int value)
{
    UTxBuffWptr unTmp;
    const int tick = 5;
    if (chnId >= W_CHANNEL_ID_MAX) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AopBuffWptrReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ AopHalSetBuffWptr: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txBuffWptr = value;
    AiaoHalWriteReg(AopBuffWptrReg(chnId), unTmp.u32);

    if (g_debugTick++ > tick) {
        g_debugTick = 0;
    }

    return HDF_SUCCESS;
}

int AopHalSetBuffRptr(unsigned int chnId, unsigned int value)
{
    UTxBuffRptr unTmp;

    if (chnId >= R_CHANNEL_ID_MAX) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AopBuffRptrReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ AopHalSetBuffRptr: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txBuffRptr = value;
    AiaoHalWriteReg(AopBuffRptrReg(chnId), unTmp.u32);
    return HDF_SUCCESS;
}

int AopHalSetBufferSize(unsigned int chnId, unsigned int value)
{
    UTxBuffSize unTmp;

    if (chnId >= HI_AO_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AopBuffSizeReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ AopHalSetBufferSize: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txBuffSize = value;
    AiaoHalWriteReg(AopBuffSizeReg(chnId), unTmp.u32);

    return HDF_SUCCESS;
}

int AopHalSetTransSize(unsigned int chnId, unsigned int value)
{
    UTxTransSize unTmp;

    if (chnId >= HI_AO_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AopTransSizeReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("@@@ AopHalSetTransSize: read = 0x%08x, write = 0x%08x.", unTmp.u32, value);

    unTmp.Bits.txTransSize = value;
    AiaoHalWriteReg(AopTransSizeReg(chnId), unTmp.u32);

    return HDF_SUCCESS;
}

int AopHalSetTxStart(unsigned int chnId, bool en)
{
    UTxDspCtrl unTmp;
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopHalSetTxStart: entry.");

    if (chnId >= HI_AO_DEV_MAX_NUM) {
        AUDIO_DEVICE_LOG_ERR("ao_dev%d is invalid!\n", chnId);
        return HDF_FAILURE;
    }

    unTmp.u32 = AiaoHalReadReg(AopCtrlReg(chnId));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopHalSetTxStart read chid = %u, val = %08x \n",  chnId, unTmp.u32);

    unTmp.Bits.txEnable = en;
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopHalSetTxStart write chid = %u, val = %08x \n", chnId, unTmp.u32);
    AiaoHalWriteReg(AopCtrlReg(chnId), unTmp.u32);

    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopHalSetTxStart: success !!!");

    return HDF_SUCCESS;
}

int32_t InitAiaoGpio(void)
{
    return 0;
}

void ShowAllAiaoRegister(void)
{
    volatile unsigned int val;
    // AIAO REG 00 CLK
    val = AiaoHalReadReg(AipI2sRegCfg0(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AOP_I2S_REG_CFG0:0x0100 = [%08x]", val);
    val = AiaoHalReadReg(AipI2sRegCfg1(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AIP_I2S_REG_CFG1:0x0104 = [%08x]", val);
    val = AiaoHalReadReg(AipInfAttriReg(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AipInfAttriReg:0x1000 = [%08x]", val);
    val = AiaoHalReadReg(AipCtrlReg(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AipCtrlReg:0x1004 = [%08x]", val);

    // AIAO REG 01 CLK
    val = AiaoHalReadReg(AopI2sRegCfg0(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AOP_I2S_REG_CFG00 = [%08x]", val);
    val = AiaoHalReadReg(AopI2sRegCfg1(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AOP_I2S_REG_CFG1 = [%08x]", val);

    // AIAO REG ATTR
    val = AiaoHalReadReg(AopInfAttriReg(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AOP_INF_ATTRI_REG = [%08x]", val);

    // AIAO REG CTRL
    val = AiaoHalReadReg(AopCtrlReg(0));
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AOP_CTRL_REG = [%08x]", val);

    // AIAO AIAO_INT_ENA_REG
    val  = AiaoHalReadReg(AIAO_INT_ENA_REG);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AIAO REG: AIAO_INT_ENA_REG = [%08x]", val);
}

int32_t AopHalDevEnable(unsigned int chnId)
{
    int ret = AopHalSetTxStart(chnId, HI_TRUE);
    if (ret != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    ShowAllAiaoRegister();
    return HDF_SUCCESS;
}

static int32_t AiaoGetBclkFsclk(unsigned int fsBit, unsigned int rate,
                                int32_t mclkSel, int32_t *bclkSel, int32_t *lrclkSel)
{
    unsigned int mclkRateNum;
    int32_t ret;

    if (g_aioClkInfo.mclkDiv48k256fs == mclkSel) {
        mclkRateNum = AUDIO_SAMPLE_RATE_48000 * AIO_FIFO_BIT_WIDTH_256;
    } else if (g_aioClkInfo.mclkDiv32k256fs == mclkSel) {
        mclkRateNum = AUDIO_SAMPLE_RATE_32000 * AIO_FIFO_BIT_WIDTH_256;
    } else if (g_aioClkInfo.mclkDiv441k256fs == mclkSel) {
        mclkRateNum = AUDIO_SAMPLE_RATE_44100 * AIO_FIFO_BIT_WIDTH_256;
    } else if (g_aioClkInfo.mclkDiv48k320fs == mclkSel) {
        mclkRateNum = AUDIO_SAMPLE_RATE_48000 * AIO_FIFO_BIT_WIDTH_320;
    } else if (g_aioClkInfo.mclkDiv32k320fs == mclkSel) {
        mclkRateNum = AUDIO_SAMPLE_RATE_32000 * AIO_FIFO_BIT_WIDTH_320;
    } else if (g_aioClkInfo.mclkDiv441k320fs == mclkSel) {
        mclkRateNum = AUDIO_SAMPLE_RATE_44100 * AIO_FIFO_BIT_WIDTH_320;
    } else {
        AUDIO_DEVICE_LOG_ERR("not support this mclk\n");
        return HDF_FAILURE;
    }

    AUDIO_DEVICE_LOG_DEBUG("AiaoGetBclkFsclkDivCfg, mclkSel = %d,mclkRateNum=%d\n", mclkSel, mclkRateNum);

    if ((mclkRateNum % (fsBit * rate)) != 0) {
        AUDIO_DEVICE_LOG_ERR("can not get the bclkDivision ratio\n");
        return HDF_FAILURE;
    }

    ret = AiaoGetBclkSel(mclkRateNum / (fsBit * rate), bclkSel);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("AiaoGetBclkSel error\n");
        return ret;
    }

    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AiaoGetBclkFsclkDivCfg, bclkSel=%u\n", *bclkSel);

    ret = AiaoGetLrclkSel(fsBit, lrclkSel);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AiaoGetBclkFsclkDivCfg, fsBit = %d,lrclkSel=%u\n", fsBit, *lrclkSel);

    return ret;
}

static int32_t AiaoGetMclk(unsigned int channelCnt, unsigned int rate, uint32_t *mclkSel)
{
    switch (rate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_32000:
        case AUDIO_SAMPLE_RATE_64000: {
            *mclkSel = (channelCnt == AIAO_CHN_CNT_20) ?
                   g_aioClkInfo.mclkDiv32k320fs : g_aioClkInfo.mclkDiv32k256fs;
            break;
        }

        case AUDIO_SAMPLE_RATE_12000:
        case AUDIO_SAMPLE_RATE_24000:
        case AUDIO_SAMPLE_RATE_48000:
        case AUDIO_SAMPLE_RATE_96000: {
            *mclkSel = (channelCnt == AIAO_CHN_CNT_20) ?
                 g_aioClkInfo.mclkDiv48k320fs : g_aioClkInfo.mclkDiv48k256fs;
            break;
        }

        case AUDIO_SAMPLE_RATE_11025:
        case AUDIO_SAMPLE_RATE_22050:
        case AUDIO_SAMPLE_RATE_44100: {
            *mclkSel = (channelCnt == AIAO_CHN_CNT_20) ?
                   g_aioClkInfo.mclkDiv441k320fs : g_aioClkInfo.mclkDiv441k256fs;
            break;
        }

        default: {
            AUDIO_DEVICE_LOG_ERR("not support this sample rate\n");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

static unsigned int AiaoGetBitCnt(unsigned int bitWidth)
{
    if (bitWidth == BIT_WIDTH8) {
        return BIT_WIDTH8; /* 8:8bit */
    } else if (bitWidth == BIT_WIDTH16) {
        return BIT_WIDTH16; /* 16:16bit */
    } else {
        return BIT_WIDTH32; /* 32:32bit */
    }
}

int32_t AiaoSetSysCtlRegValue(unsigned int mclkSel, unsigned int bitWidth, unsigned int rate, UI2sCrgCfg1 *i2sCrgCfg1)
{
    int32_t ret;
    unsigned int fsBit;
    unsigned int bclkSel = 0;
    unsigned int lrClkSel = 0;
    const int dobule = 2;

    if (i2sCrgCfg1 == NULL) {
        AUDIO_DEVICE_LOG_ERR("AiaoSetSysCtlRegValue::param i2sCrgCfg1 is nullptr.");
        return HDF_ERR_INVALID_PARAM;
    }
    fsBit = AiaoGetBitCnt(bitWidth) * dobule;
    ret = AiaoGetBclkFsclk(fsBit, rate, mclkSel, &bclkSel, &lrClkSel);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("AiaoGetBclkFsclk fail");
        return HDF_FAILURE;
    }

    i2sCrgCfg1->Bits.aiaoBclkDiv = bclkSel;
    i2sCrgCfg1->Bits.aiaoFsclkDiv = lrClkSel;
    i2sCrgCfg1->Bits.aiaoCken = 1;
    i2sCrgCfg1->Bits.aiaoSrstReq = 0;
    i2sCrgCfg1->Bits.aiaoBclkEn = 1;
    i2sCrgCfg1->Bits.aiaoWsEn = 1;
    return HDF_SUCCESS;
}

int32_t AopSetSysCtlReg(int32_t chnId, unsigned int channelCnt, unsigned int bitWidth, unsigned int rate)
{
    unsigned int mclkSel = 0;
    int32_t ret;

    UI2sCrgCfg0 i2sCrgCfg0;
    UI2sCrgCfg1 i2sCrgCfg1;

    /* READ & PRINT */
    i2sCrgCfg0.u32 = AiaoHalReadReg(AopI2sRegCfg0(chnId));
    i2sCrgCfg1.u32 = AiaoHalReadReg(AopI2sRegCfg1(chnId));

    /* get clock value */
    ret = AiaoGetMclk(channelCnt, rate, &mclkSel);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("[OHOS] AopSetSysCtl AiaoGetMclkCfg checking");
        return HDF_FAILURE;
    }

    i2sCrgCfg0.Bits.aiaoMclkDiv = mclkSel;
    i2sCrgCfg0.Bits.reserved0 = 0;
    AiaoHalWriteReg(AopI2sRegCfg0(chnId), i2sCrgCfg0.u32);

    ret = AiaoSetSysCtlRegValue(mclkSel, bitWidth, rate, &i2sCrgCfg1);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("AopSetSysCtl:AiaoSetSysCtlRegValue checking failed.");
        return HDF_FAILURE;
    }

    AiaoHalWriteReg(AopI2sRegCfg1(chnId), i2sCrgCfg1.u32);

    AUDIO_DEVICE_LOG_DEBUG("AopSetSysCtl: AiaoHalWriteReg, i2sCrgCfg0 = 0x%08x  i2sCrgCfg1 = 0x%08x",
        i2sCrgCfg0.u32, i2sCrgCfg1.u32);

    return HDF_SUCCESS;
}

int32_t AipSetSysCtlReg(int32_t chnId, unsigned int channelCnt, unsigned int bitWidth, unsigned int rate)
{
    unsigned int mclkSel = 0;
    int32_t ret;

    UI2sCrgCfg0 i2sCrgCfg0;
    UI2sCrgCfg1 i2sCrgCfg1;

    /* READ & PRINT */
    i2sCrgCfg0.u32 = AiaoHalReadReg(AipI2sRegCfg0(chnId));
    i2sCrgCfg1.u32 = AiaoHalReadReg(AipI2sRegCfg1(chnId));

    /* get clock value */
    ret = AiaoGetMclk(channelCnt, rate, &mclkSel);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("[OHOS] AopSetSysCtl AiaoGetMclkCfg checking");
        return HDF_FAILURE;
    }

    i2sCrgCfg0.Bits.aiaoMclkDiv = mclkSel;
    i2sCrgCfg0.Bits.reserved0 = 0;
    AiaoHalWriteReg(AipI2sRegCfg0(chnId), i2sCrgCfg0.u32);

    ret = AiaoSetSysCtlRegValue(mclkSel, bitWidth, rate, &i2sCrgCfg1);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("AipSetSysCtlReg:AiaoSetSysCtlRegValue checking failed.");
        return HDF_FAILURE;
    }

    AiaoHalWriteReg(AipI2sRegCfg1(chnId), i2sCrgCfg1.u32);

    AUDIO_DEVICE_LOG_DEBUG("AipSetSysCtlReg: AiaoHalWriteReg, i2sCrgCfg0 = 0x%08x  i2sCrgCfg1 = 0x%08x",
        i2sCrgCfg0.u32, i2sCrgCfg1.u32);

    return HDF_SUCCESS;
}

int32_t AopSetAttr(unsigned int chnId, unsigned int channelCnt, unsigned int bitWidth)
{
    UTxIfAttri aopAttrReg;
    AUDIO_DEVICE_LOG_DEBUG("AopSetAttr entry");
    aopAttrReg.u32 = AiaoHalReadReg(AopInfAttriReg(chnId));

    if (channelCnt == AIO_CHN_CNT_1) {
        aopAttrReg.Bits.txChNum = 0x0;
    } else if (channelCnt == AIO_CHN_CNT_2) {
        aopAttrReg.Bits.txChNum = 0x1;
    } else {
        AUDIO_DEVICE_LOG_ERR("[OHOS] AopSetAttrReg: attr->channelCnt error.");
        return HDF_FAILURE;
    }

    if (bitWidth == BIT_WIDTH16) {
        aopAttrReg.Bits.txI2sPrecision = 0x1;
    } else if (bitWidth == BIT_WIDTH24) {
        aopAttrReg.Bits.txI2sPrecision = 0x2; /* 2: 24bit */
    } else {
        AUDIO_DEVICE_LOG_ERR("[OHOS] AopSetAttrReg: attr->channelCnt error bitWidth: %d.", bitWidth);
        return HDF_FAILURE;
    }

    aopAttrReg.Bits.txTrackmode = 0;
    AiaoHalWriteReg(AopInfAttriReg(chnId),  aopAttrReg.u32);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AopSetAttrReg aopAttrReg.u32 write = %08x\n", aopAttrReg.u32);

    return HDF_SUCCESS;
}

int32_t AipSetAttr(unsigned int chnId, unsigned int channelCnt, unsigned int bitWidth)
{
    URxIfAttri aipAttrReg;
    const int sourceSel = 8;
    AUDIO_DEVICE_LOG_DEBUG("AopSetAttr entry");
    aipAttrReg.u32 = AiaoHalReadReg(AipInfAttriReg(chnId));

    if (channelCnt == AIO_CHN_CNT_1) {
        aipAttrReg.Bits.rxChNum = 0x0;
    } else if (channelCnt == AIO_CHN_CNT_2) {
        aipAttrReg.Bits.rxChNum = 0x1;
    } else {
        AUDIO_DEVICE_LOG_ERR("[OHOS] AipSetAttr: attr->channelCnt error.");
        return HDF_FAILURE;
    }

    if (bitWidth == BIT_WIDTH16) {
        aipAttrReg.Bits.rxI2sPrecision = 0x1;
    } else if (bitWidth == BIT_WIDTH24) {
        aipAttrReg.Bits.rxI2sPrecision = 0x2; /* 2: 24bit */
    } else {
        AUDIO_DEVICE_LOG_ERR("[OHOS] AipSetAttr: attr->channelCnt error bitWidth: %d.", bitWidth);
        return HDF_FAILURE;
    }

    aipAttrReg.Bits.rxTrackMode = 0;
    aipAttrReg.Bits.rxSdSourceSel = sourceSel;
    AiaoHalWriteReg(AipInfAttriReg(chnId),  aipAttrReg.u32);
    AUDIO_DEVICE_LOG_DEBUG("[OHOS] AipSetAttr aopAttrReg.u32 write = %08x\n", aipAttrReg.u32);

    return HDF_SUCCESS;
}

static void AipSetCtrlReg(unsigned int chnId)
{
    URxDspCtrl aipCtrlReg;

    aipCtrlReg.u32 = AiaoHalReadReg(AipCtrlReg(chnId));
    aipCtrlReg.Bits.muteEn = 0;
    aipCtrlReg.Bits.muteFadeEn = 0;
    aipCtrlReg.Bits.pauseEn = 0;
    aipCtrlReg.Bits.pauseFadeEn = 0;
    aipCtrlReg.Bits.reserved3 = 0;
    aipCtrlReg.Bits.volume = 0;
    aipCtrlReg.Bits.reserved2 = 0;
    aipCtrlReg.Bits.fadeInRate = 0;
    aipCtrlReg.Bits.fadeOutRate = 0;
    aipCtrlReg.Bits.reserved1 = 0;
    aipCtrlReg.Bits.bypassEn = 0;
    aipCtrlReg.Bits.rxEnable = 0;
    aipCtrlReg.Bits.rxDisableDone = 0;
    aipCtrlReg.Bits.reserved0 = 0;
    AiaoHalWriteReg(AipCtrlReg(chnId), aipCtrlReg.u32);
}

int32_t I2sCrgCfgInit(unsigned int chnId)
{
    const unsigned int i2sCrgCfg0 = 0x152ef0;
    const unsigned int i2sCrgCfg1 = 0x0000c115;

    AiaoHalWriteReg(AopI2sRegCfg0(chnId), i2sCrgCfg0);
    AiaoHalWriteReg(AopI2sRegCfg1(chnId), i2sCrgCfg1);
    AiaoHalWriteReg(AipI2sRegCfg0(chnId), i2sCrgCfg0);
    AiaoHalWriteReg(AipI2sRegCfg1(chnId), i2sCrgCfg1);
    return HDF_SUCCESS;
}

int32_t AiaoDeviceInit(unsigned int chnId)
{
    const unsigned int aipAttrVal = 0xe4880014;
    const unsigned int aopAttrVal = 0xe4000054;

    AiaoHalWriteReg(AipInfAttriReg(chnId), aipAttrVal);
    AiaoHalWriteReg(AopInfAttriReg(chnId),  aopAttrVal);
    AopSetCtrlReg(chnId);
    AipSetCtrlReg(chnId);
    return HDF_SUCCESS;
}
