/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "tfa9879_accessory_impl.h"
#include "audio_accessory_base.h"
#include "audio_core.h"
#include "audio_device_log.h"
#include "audio_sapm.h"
#include "gpio_if.h"
#include "hdf_log.h"
#include "i2c_if.h"

#define HDF_LOG_TAG "tfa9879_codec"

#define CTRL_REG_NUM    21

#define CHANNEL_MAX_NUM 2
#define CHANNEL_MIN_NUM 1

#define I2C_BUS_NUM         6
#define I2C_8BIT            8
#define I2C_REG_LEN         1
#define I2C_MSG_NUM         2
#define I2C_MSG_BUF_SIZE    2
#define I2C_WAIT_TIMES      10 // ms

#define VOLUME_MIN          0
#define VOLUME_MAX          100
#define MUTE__MAX           255
#define MUTE_MIN            189

#define TFA9879_MSG_SIZE        2
#define TFA9879_REG_MSGLEN      3
#define TFA9879_RIGHT_SHIFT     8
#define TFA9879_MUST_SLEEP     10
#define TFA9879_REG_MASK       0xFF
#define TFA9879_REG_MASK_4     0xF
#define TFA9879_REG_MASK_16    0xFFFF

const int MUTE_SHIFT = 9;
const int CHANNEL_SHIFT = 10;

static uint16_t g_i2cDevAddr;


/* Tfa9879 Special Region Begin */
static void SysWritel(unsigned long addr, unsigned int value)
{
    *(volatile unsigned int *)(addr) = value;
}

/* tfa9879 21 control register default value */
struct Tfa9879RegAttr g_tfa9879RegDefaultAttr[] = {
    {
        .regAddr = DEVICE_CONTROL_REG_ADDR,
        .regValue = 0x0001 // 0x0009
    }, {
        .regAddr = SERIAL_INTERFACE_INPUT1_REG_ADDR,
        .regValue = 0x0a18 // 48khz, up to 24 bits
    }, {
        .regAddr = PCM_IOM2_FMT_INPUT1_REG_ADDR,
        .regValue = 0x0007
    }, {
        .regAddr = SERIAL_INTERFACE_INPUT2_REG_ADDR,
        .regValue = 0x0a18
    }, {
        .regAddr = PCM_IOM2_FMT_INPUT2_REG_ADDR,
        .regValue = 0x0007
    }, {
        .regAddr = EQUALIZER_A_WORD1_REG_ADDR,
        .regValue = 0x59DD
    }, {
        .regAddr = EQUALIZER_A_WORD2_REG_ADDR,
        .regValue = 0xC63E
    }, {
        .regAddr = EQUALIZER_B_WORD1_REG_ADDR,
        .regValue = 0x651A
    }, {
        .regAddr = EQUALIZER_B_WORD2_REG_ADDR,
        .regValue = 0xE53E
    }, {
        .regAddr = EQUALIZER_C_WORD1_REG_ADDR,
        .regValue = 0x4616
    }, {
        .regAddr = EQUALIZER_C_WORD2_REG_ADDR,
        .regValue = 0xD33E
    }, {
        .regAddr = EQUALIZER_D_WORD1_REG_ADDR,
        .regValue = 0x4DF3
    }, {
        .regAddr = EQUALIZER_D_WORD2_REG_ADDR,
        .regValue = 0xEA3E
    }, {
        .regAddr = EQUALIZER_E_WORD1_REG_ADDR,
        .regValue = 0x5EE0
    }, {
        .regAddr = EQUALIZER_E_WORD2_REG_ADDR,
        .regValue = 0xF93E
    }, {
        .regAddr = BYPASS_CONTROL_REG_ADDR,
        .regValue = 0x0008 // 0x00ff // 0x0093
    }, {
        .regAddr = DYNAMIC_RANGE_COMP_REG_ADDR,
        .regValue = 0x92BA
    }, {
        .regAddr = BASS_TREBLE_REG_ADDR,
        .regValue = 0x12A5
    }, {
        .regAddr = HIGH_PASS_FILTER_REG_ADDR,
        .regValue = 0x0004
    }, {
        .regAddr = VOLUME_CONTROL_REG_ADDR,
        .regValue = 0x1031 // 0x101A
    }, {
        .regAddr = DE_EMPHASIS_REG_ADDR,
        .regValue = 0x0000
    }
};

/*
 * release object public function
 */
static void ReleaseObject(struct I2cMsg *msgs, int16_t msgSize, DevHandle i2cHandle)
{
    if (msgs != NULL) {
        if (msgSize == 0 && msgs->buf != NULL) {
            OsalMemFree(msgs->buf);
            msgs->buf = NULL;
        } else if (msgSize == 1 && msgs[0].buf != NULL) {
            OsalMemFree(msgs[0].buf);
            msgs[0].buf = NULL;
        } else if (msgSize >= TFA9879_MSG_SIZE) {
            if (msgs[0].buf != NULL) {
                msgs[0].buf = NULL;
            }
            if (msgs[1].buf != NULL) {
                OsalMemFree(msgs[1].buf);
                msgs[1].buf = NULL;
            }
        }
        msgs = NULL;
        AUDIO_DEVICE_LOG_DEBUG("OsalMemFree msgBuf success.\n");
    }
    // close i2c device
    if (i2cHandle != NULL) {
        I2cClose(i2cHandle);
        i2cHandle = NULL;
        AUDIO_DEVICE_LOG_DEBUG("I2cClose success.\n");
    }
}

static int Tfa9879FillMsg(const struct Tfa9879RegAttr *regAttr, uint16_t rwFlag,
                          uint8_t *regs, struct I2cMsg *msgs)
{
    uint8_t *msgBuf = NULL;
    if (rwFlag != 0 && rwFlag != I2C_FLAG_READ) {
        AUDIO_DEVICE_LOG_ERR("invalid rwFlag value: %d.", rwFlag);
        return HDF_ERR_INVALID_PARAM;
    }
    regs[0] = regAttr->regAddr;
    msgs[0].addr = g_i2cDevAddr;
    msgs[0].flags = 0;
    msgs[0].len = TFA9879_REG_MSGLEN;
    AUDIO_DEVICE_LOG_DEBUG("msgs[0].addr=0x%02x, regs[0]=0x%02x.", msgs[0].addr, regs[0]);
    if (rwFlag == 0) { // write
        // S 11011A2A1 0 A ADDR A MS1 A LS1 A <....> P
        msgBuf = OsalMemCalloc(TFA9879_REG_MSGLEN);
        if (msgBuf == NULL) {
            AUDIO_DEVICE_LOG_ERR("[write]: malloc buf fail!");
            return HDF_ERR_MALLOC_FAIL;
        }
        msgBuf[0] = regs[0];
        msgBuf[1] = (uint8_t)(regAttr->regValue >> I2C_8BIT); // High 8 bit
        msgBuf[I2C_MSG_BUF_SIZE] = (uint8_t)(regAttr->regValue & TFA9879_REG_MASK);    // Low 8 bit
        msgs[0].buf = msgBuf;
        AUDIO_DEVICE_LOG_DEBUG("msgBuf[1]=0x%02x.", msgBuf[1]);
        AUDIO_DEVICE_LOG_DEBUG("msgBuf[2]=0x%02x.", msgBuf[I2C_MSG_BUF_SIZE]);
    } else {
        // S 11011A2A1 0 A ADDR A Sr 11011A2A1 1 A MS1 A LS1 A <....> NA P
        msgBuf = OsalMemCalloc(I2C_MSG_NUM);
        if (msgBuf == NULL) {
            AUDIO_DEVICE_LOG_ERR("[read]: malloc buf fail!");
            return HDF_ERR_MALLOC_FAIL;
        }
        msgs[0].len = 1;
        msgs[0].buf = regs;
        msgs[1].addr = g_i2cDevAddr;
        msgs[1].flags = I2C_FLAG_READ;
        msgs[1].len = I2C_MSG_NUM;
        msgs[1].buf = msgBuf;
    }
    AUDIO_DEVICE_LOG_DEBUG("fill msg success.\n");
    return HDF_SUCCESS;
}

int Tfa9879RegRw(struct Tfa9879RegAttr *regAttr, uint16_t rwFlag)
{
    int ret;
    DevHandle i2cHandle;
    int16_t transferMsgCount = 1;
    uint8_t regs[I2C_REG_LEN];
    struct I2cMsg msgs[I2C_MSG_NUM];
    AUDIO_DEVICE_LOG_DEBUG("entry.\n");
    if (regAttr == NULL || rwFlag < 0 || rwFlag > 1) {
        AUDIO_DEVICE_LOG_ERR("invalid parameter.");
        return HDF_ERR_INVALID_PARAM;
    }
    i2cHandle = I2cOpen(I2C_BUS_NUM);
    if (i2cHandle == NULL) {
        AUDIO_DEVICE_LOG_ERR("open i2cBus:%u fail! i2cHandle:%p", I2C_BUS_NUM, i2cHandle);
        return HDF_FAILURE;
    }
    if (rwFlag == I2C_FLAG_READ) {
        transferMsgCount = I2C_MSG_NUM;
    }
    ret = Tfa9879FillMsg(regAttr, rwFlag, regs, msgs);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879FillMsg failed!");
        I2cClose(i2cHandle);
        return HDF_FAILURE;
    }
    ret = I2cTransfer(i2cHandle, msgs, transferMsgCount);
    if (ret != transferMsgCount) {
        AUDIO_DEVICE_LOG_ERR("I2cTransfer err:%d", ret);
        ReleaseObject(msgs, transferMsgCount, i2cHandle);
        return HDF_FAILURE;
    }
    if (rwFlag == I2C_FLAG_READ) {
        regAttr->regValue = (msgs[1].buf[0] << TFA9879_RIGHT_SHIFT) | msgs[1].buf[1]; // result value 16 bit
        AUDIO_DEVICE_LOG_DEBUG("[read]: regAttr->regValue=0x%04x.\n", regAttr->regValue);
    }
    ReleaseObject(msgs, transferMsgCount, i2cHandle);
    return HDF_SUCCESS;
}

static void Tfa9879GetStatus(void)
{
    int ret;
    int high;
    int low;
    struct Tfa9879RegAttr regAttr = {
        .regAddr = MISCELLANEOUS_STATUS_REG_ADDR,
        .regValue = 0,
    };
    ret = Tfa9879RegRw(&regAttr, I2C_FLAG_READ);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw fail.");
        return;
    }
    high = regAttr.regValue >> I2C_8BIT;    // High 8 bit
    low = regAttr.regValue & 0xFF;          // Low 8 bit
    AUDIO_DEVICE_LOG_DEBUG("regAttr.regValue=0x%02x%02x. \n", high, low);
}

// get external codec I2S frequency
static int GetCodecI2sFrequency(struct I2cMsg *msg, uint16_t *fqVal)
{
    int ret;
    struct Tfa9879RegAttr regAttr = {
        .regAddr = SERIAL_INTERFACE_INPUT1_REG_ADDR,    // 0x01
        .regValue = 0,
    };
    if (msg == NULL || msg->len < TFA9879_MSG_SIZE || fqVal == NULL) {
        AUDIO_DEVICE_LOG_ERR("input invalid parameter.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (g_i2cDevAddr == TFA9879_I2C_DEV_ADDR_ADSEL2) {
        regAttr.regAddr = SERIAL_INTERFACE_INPUT2_REG_ADDR;     // 0x03
    }
    ret = Tfa9879RegRw(&regAttr, I2C_FLAG_READ);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw fail.");
        return HDF_FAILURE;
    }
    msg->buf[0] = regAttr.regValue >> I2C_8BIT;                 // High 8 bit
    msg->buf[1] = regAttr.regValue & TFA9879_REG_MASK;          // Low 8 bit
    *fqVal = (regAttr.regValue >> 6) & TFA9879_REG_MASK_4;      // 01h/03h[9-6]
    return HDF_SUCCESS;
}

// set external codec I2S frequency
static int SetCodecI2sFrequency(uint16_t frequencyVal)
{
    int ret;
    uint16_t oldVal;
    uint16_t mask = 0x3C0;
    struct I2cMsg oldMsg;
    struct Tfa9879RegAttr regAttr;
    // get current value
    oldMsg.len = I2C_MSG_BUF_SIZE;
    oldMsg.buf = OsalMemAlloc(I2C_MSG_BUF_SIZE);
    if (oldMsg.buf == NULL) {
        AUDIO_DEVICE_LOG_ERR("oldMsg.buf is null.");
        return HDF_ERR_MALLOC_FAIL;
    }
    ret = GetCodecI2sFrequency(&oldMsg, &oldVal);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("GetCodecI2sFrequency fail.");
        ReleaseObject(&oldMsg, 0, NULL);
        return HDF_FAILURE;
    }
    // update current value
    oldVal = (oldMsg.buf[0] << TFA9879_RIGHT_SHIFT) | oldMsg.buf[1]; // 16 bit
    regAttr.regAddr = SERIAL_INTERFACE_INPUT1_REG_ADDR;
    if (g_i2cDevAddr == TFA9879_I2C_DEV_ADDR_ADSEL2) {
        regAttr.regAddr = SERIAL_INTERFACE_INPUT2_REG_ADDR; // 0x03
    }
    regAttr.regValue = ((frequencyVal << 6) & mask) | (oldVal & ~mask); // 00h[6-9]
    ret = Tfa9879RegRw(&regAttr, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw failed.");
        ReleaseObject(&oldMsg, 0, NULL);
        return HDF_FAILURE;
    }
    ReleaseObject(&oldMsg, 0, NULL);
    AUDIO_DEVICE_LOG_DEBUG("success.\n");
    return HDF_SUCCESS;
}

// get external codec I2S format
static int GetCodecI2sFormat(struct I2cMsg *msg, uint16_t *fsVal)
{
    int ret;
    struct Tfa9879RegAttr regAttr = {
        .regAddr = SERIAL_INTERFACE_INPUT1_REG_ADDR, // 0x01
        .regValue = 0,
    };
    if (msg == NULL || msg->len < TFA9879_MSG_SIZE || fsVal == NULL) {
        AUDIO_DEVICE_LOG_ERR("invalid parameter.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (g_i2cDevAddr == TFA9879_I2C_DEV_ADDR_ADSEL2) {
        regAttr.regAddr = SERIAL_INTERFACE_INPUT2_REG_ADDR; // 0x03
    }
    ret = Tfa9879RegRw(&regAttr, I2C_FLAG_READ);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw is failure.");
        return HDF_FAILURE;
    }
    msg->buf[0] = regAttr.regValue >> I2C_8BIT;                 // High 8 bit
    msg->buf[1] = regAttr.regValue & TFA9879_REG_MASK;          // Low 8 bit
    *fsVal = (regAttr.regValue >> 3) & TFA9879_REG_MASK_4;      // 01h/03h[3-5]
    return HDF_SUCCESS;
}

// set external codec I2S format
static int SetCodecI2sFormat(uint16_t formatVal)
{
    int ret;
    uint16_t oldVal;
    uint16_t mask = 0x38;
    struct I2cMsg oldMsg;
    struct Tfa9879RegAttr regAttr;
    // get current value
    oldMsg.len = I2C_MSG_BUF_SIZE;
    oldMsg.buf = OsalMemAlloc(I2C_MSG_BUF_SIZE);
    if (oldMsg.buf == NULL) {
        AUDIO_DEVICE_LOG_ERR("oldMsg.buf is null.");
        return HDF_ERR_MALLOC_FAIL;
    }
    ret = GetCodecI2sFormat(&oldMsg, &oldVal);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("GetCodecI2sFormat fail.");
        ReleaseObject(&oldMsg, 0, NULL);
        return HDF_FAILURE;
    }
    // update current value
    oldVal = (oldMsg.buf[0] << TFA9879_RIGHT_SHIFT) | oldMsg.buf[1]; // 16 bit
    regAttr.regAddr = SERIAL_INTERFACE_INPUT1_REG_ADDR;
    if (g_i2cDevAddr == TFA9879_I2C_DEV_ADDR_ADSEL2) {
        regAttr.regAddr = SERIAL_INTERFACE_INPUT2_REG_ADDR; // 0x03
    }
    regAttr.regValue = ((formatVal << 3) & mask) | (oldVal & ~mask); // 00h[3-5]
    ret = Tfa9879RegRw(&regAttr, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw fail.");
        ReleaseObject(&oldMsg, 0, NULL);
        return HDF_FAILURE;
    }
    ReleaseObject(&oldMsg, 0, NULL);
    AUDIO_DEVICE_LOG_DEBUG("success.\n");
    return HDF_SUCCESS;
}

// Init Function
/*
 * gpio0_6 pin init
*/
static int Gpio06PinInit(void)
{
    int ret;
    const uint16_t gpio = 6;
    char *regGpioBase = 0;
    regGpioBase = (void *)OsalIoRemap(HI35XX_I2S_REG_BASE_ADDR, 0x10000);
    if (regGpioBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("regGpioBase is null.");
        return HDF_FAILURE;
    }
    SysWritel((uintptr_t)regGpioBase + 0x0034, 0x0400); // GPIO0_6
    if (regGpioBase != NULL) {
        OsalIoUnmap(regGpioBase);
    }
    AUDIO_DEVICE_LOG_DEBUG("SYS_WRITEL success.");
    ret = GpioSetDir(gpio, GPIO_DIR_OUT);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("%s: set gpio dir fail! ret:%d", __func__, ret);
        return ret;
    }
    ret = GpioWrite(gpio, GPIO_VAL_HIGH);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("%s: write gpio val fail! ret:%d", __func__, ret);
        return ret;
    }
    return HDF_SUCCESS;
}

/*
 * i2c6 init
*/
static int I2c6PinInit(void)
{
    char *regI2cBase = 0;
    regI2cBase = (void *)OsalIoRemap(HI35XX_I2C_REG_BASE_ADDR, 0x10000);
    if (regI2cBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("regI2cBase is null.");
        return HDF_FAILURE;
    }
    SysWritel((uintptr_t)regI2cBase + 0x0048, 0x0473); // I2C6_SCL
    SysWritel((uintptr_t)regI2cBase + 0x004C, 0x0473); // I2C6_SDA
    if (regI2cBase != NULL) {
        OsalIoUnmap(regI2cBase);
    }
    return HDF_SUCCESS;
}

/*
 * i2s0 pin init
*/
static void I2s0PinMux(const char *regI2sBase)
{
    SysWritel((uintptr_t)regI2sBase + 0x0020, 0x673); // I2S_MCLK
    SysWritel((uintptr_t)regI2sBase + 0x0024, 0x633); // I2S_BCLK_TX
    SysWritel((uintptr_t)regI2sBase + 0x0028, 0x533); // I2S_WS_TX
    SysWritel((uintptr_t)regI2sBase + 0x002C, 0x433); // I2S_SD_TX
    SysWritel((uintptr_t)regI2sBase + 0x0030, 0x533); // I2S_SD_RX
}

/*
 * i2s init
*/
static int I2sPinInit(void)
{
    char *regI2sBase = 0;
    regI2sBase = (void *)OsalIoRemap(HI35XX_I2S_REG_BASE_ADDR, 0x10000);
    if (regI2sBase == NULL) {
        AUDIO_DEVICE_LOG_ERR("regI2sBase is null.");
        return HDF_FAILURE;
    }
    I2s0PinMux(regI2sBase);
    if (regI2sBase != NULL) {
        OsalIoUnmap(regI2sBase);
    }
    return HDF_SUCCESS;
}

/*
 * init default value
 */
static int Tfa9879RegDefaultInit()
{
    int ret, i;
    struct Tfa9879RegAttr regAttr;
    // Set current i2c dev addr
    g_i2cDevAddr = TFA9879_I2C_DEV_ADDR_ADSEL1;
    // Set codec control register(00h-14h) default value
    for (i = 0; i < CTRL_REG_NUM; i++) {
        regAttr = g_tfa9879RegDefaultAttr[i];
        AUDIO_DEVICE_LOG_DEBUG("REG = [%02d], Addr = [0x%2x]", i, regAttr.regAddr);
        ret = Tfa9879RegRw(&regAttr, 0);
        if (ret != HDF_SUCCESS) {
            AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw(write) err, regAttr.regAddr: 0x%02x.\n",
                regAttr.regAddr);
            return HDF_FAILURE;
        }
        if (i == 0) {
            OsalMSleep(TFA9879_MUST_SLEEP); // MUST > 5.6 ms
        }
        AUDIO_DEVICE_LOG_DEBUG("Tfa9879RegRw success.i=%d", i);
        OsalMSleep(I2C_WAIT_TIMES);
    }
    // WORK
    regAttr.regAddr = DEVICE_CONTROL_REG_ADDR;
    regAttr.regValue = 0x09;
    ret = Tfa9879RegRw(&regAttr, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw(write) err, regAttr.regAddr: 0x%02x.\n",
            regAttr.regAddr);
        return HDF_FAILURE;
    }
    Tfa9879GetStatus();
    AUDIO_DEVICE_LOG_DEBUG("success.\n");
    return HDF_SUCCESS;
}

/*
 * codec init
*/
static int CodecTfa9879DeviceInit(void)
{
    int ret;
    AUDIO_DEVICE_LOG_DEBUG("entry.");
    ret = Gpio06PinInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Gpio06PinInit fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("Gpio06PinInit success.");
    ret = I2c6PinInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("I2c6PinInit fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("I2c6PinInit success.");
    // Initial tfa9879 register
    ret = Tfa9879RegDefaultInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegDefaultInit fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

/* Tfa9879 Special Region End */

/* ADM Framework Region Begin */
static const struct AudioMixerControl g_tfa9879AudioRegParams[] = {
    {
        .reg = VOLUME_CONTROL_REG_ADDR, /* output volume */
        .rreg = VOLUME_CONTROL_REG_ADDR,
        .shift = 0,
        .rshift = 0,
        .min = 0x0,
        .max = 0xBC,
        .mask = 0xFF,
        .invert = 1,
    }, {
        .reg = DE_EMPHASIS_REG_ADDR, /* output mute */
        .rreg = DE_EMPHASIS_REG_ADDR, // hard mute;
        .shift = MUTE_SHIFT,
        .rshift = MUTE_SHIFT,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = SERIAL_INTERFACE_INPUT1_REG_ADDR, /* left or right channel (output) */
        .rreg = SERIAL_INTERFACE_INPUT1_REG_ADDR,
        .shift = CHANNEL_SHIFT,
        .rshift = CHANNEL_SHIFT,
        .min = 0x0,
        .max = 0x3,
        .mask = 0x3,
        .invert = 0,
    },
};

static const struct AudioKcontrol g_tfa9879AudioControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Main Playback Volume",
        .Info = AudioInfoCtrlOps,
        .Get = AudioAccessoryGetCtrlOps,
        .Set = AudioAccessorySetCtrlOps,
        .privateValue = (unsigned long) &g_tfa9879AudioRegParams[0],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Playback Mute",
        .Info = AudioInfoCtrlOps,
        .Get = AudioAccessoryGetCtrlOps,
        .Set = AudioAccessorySetCtrlOps,
        .privateValue = (unsigned long) &g_tfa9879AudioRegParams[1],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_AIAO,
        .name = "Render Channel Mode",
        .Info = AudioInfoCtrlOps,
        .Get = AudioAccessoryAiaoGetCtrlOps,
        .Set = AudioAccessoryAiaoSetCtrlOps,
        .privateValue = (unsigned long) &g_tfa9879AudioRegParams[2],
    },
};

int32_t AccessoryDeviceInit(struct AudioCard *audioCard, const struct AccessoryDevice *device)
{
    int32_t ret;
    AUDIO_DEVICE_LOG_DEBUG(" entry.");
    if ((audioCard == NULL) || (device == NULL)) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    ret = (int32_t)CodecTfa9879DeviceInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("init tfa9979 device fail.");
        return HDF_FAILURE;
    }
    ret = AudioAddControls(audioCard, g_tfa9879AudioControls, HDF_ARRAY_SIZE(g_tfa9879AudioControls));
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("add controls fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static int32_t TransformGetCtrlResult(uint32_t reg, const uint16_t *resVal)
{
    int32_t ret = HDF_SUCCESS;
    uint8_t hightByte;
    uint8_t lowByte;
    if (reg < 0 || resVal == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (reg == VOLUME_CONTROL_REG_ADDR) {
        hightByte = (*resVal) >> I2C_8BIT;  // High 8 bit
        lowByte = (*resVal) & 0xFF;       // Low 8 bit
        AUDIO_DEVICE_LOG_DEBUG("hightByte=0x%x, lowByte=0x%x.\n", hightByte, lowByte);
        if (lowByte < VOLUME_MIN && lowByte > MUTE__MAX) {
            AUDIO_DEVICE_LOG_ERR("fail\n");
            return HDF_FAILURE;
        }
        AUDIO_DEVICE_LOG_DEBUG("resVal=%d.\n",  *resVal);
    }
    return ret;
}

int32_t AccessoryDeviceReadReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    struct Tfa9879RegAttr regAttr;
    uint16_t tmpVal;
    AUDIO_DEVICE_LOG_DEBUG("entry");
    if (val == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    (void)codec;
    regAttr.regAddr = (uint8_t)reg;
    regAttr.regValue = 0;
    ret = Tfa9879RegRw(&regAttr, I2C_FLAG_READ);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw fail.");
        return HDF_FAILURE;
    }
    tmpVal = regAttr.regValue;
    ret = TransformGetCtrlResult(reg, &tmpVal);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("TransformGetCtrlResult fail, reg=0x%x.", reg);
        return HDF_FAILURE;
    }
    *val = tmpVal;
    AUDIO_DEVICE_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

static int32_t TransformSetCtrlVal(uint32_t reg, uint16_t *val)
{
    int32_t ret = HDF_SUCCESS;
    uint8_t hightByte;
    uint8_t lowByte;
    if (reg < 0 || val == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (reg == VOLUME_CONTROL_REG_ADDR) {
        hightByte = (*val) >> I2C_8BIT;  // High 8 bit
        lowByte = (*val) & 0xFF;       // Low 8 bit
        AUDIO_DEVICE_LOG_DEBUG("hightByte=0x%x, lowByte=0x%x.\n", hightByte, lowByte);
        *val = (hightByte << TFA9879_RIGHT_SHIFT) | lowByte;
        AUDIO_DEVICE_LOG_DEBUG("val=%d.\n", *val);
    }
    return ret;
}

int32_t AccessoryDeviceWriteReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t value)
{
    int32_t ret;
    struct Tfa9879RegAttr regAttr;
    uint16_t tmpVal;
    AUDIO_DEVICE_LOG_DEBUG("entry");
    (void)codec;
    tmpVal = (uint16_t)value;
    ret = TransformSetCtrlVal(reg, &tmpVal);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("TransformSetCtrlVal fail, reg=0x%x.", reg);
        return HDF_FAILURE;
    }
    regAttr.regAddr = (uint8_t)reg;
    regAttr.regValue = tmpVal;
    ret = Tfa9879RegRw(&regAttr, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("Tfa9879RegRw fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t AccessoryAiaoDeviceReadReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    ret = AccessoryDeviceReadReg(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("AccessoryDeviceReadReg fail, reg=0x%x.", reg);
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t AccessoryAiaoDeviceWriteReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t value)
{
    int32_t ret;
    AUDIO_DEVICE_LOG_DEBUG("entry");
    ret = AccessoryDeviceWriteReg(codec, reg, value);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("AccessoryDeviceWriteReg fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("success");
    return HDF_SUCCESS;
}

int32_t AccessoryDaiStartup(const struct AudioCard *card, const struct DaiDevice *device)
{
    (void)card;
    (void)device;
    return HDF_SUCCESS;
}

int32_t AccessoryDaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param,
    const struct DaiDevice *device)
{
    int ret;
    uint16_t frequency, bitWidth;
    (void)card;
    (void)device;
    AUDIO_DEVICE_LOG_DEBUG("entry.");
    if (param == NULL || param->cardServiceName == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_PARAM;
    }
    ret = I2sPinInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("I2sPinInit fail.");
    }
    ret = RateToFrequency(param->rate, &frequency);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("RateToFrequency fail.");
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = FormatToBitWidth(param->format, &bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("FormatToBitWidth fail.");
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = SetCodecI2sFrequency(frequency);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("SetCodecI2sFs fail.");
        return HDF_FAILURE;
    }
    ret = SetCodecI2sFormat(bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DEVICE_LOG_ERR("SetCodecI2sFormat fail.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("channels = %d, rate = %d, periodSize = %d, \
        periodCount = %d, format = %d, cardServiceName = %s \n",
        param->channels, param->rate, param->periodSize,
        param->periodCount, (uint32_t)param->format, param->cardServiceName);
    AUDIO_DEVICE_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t AccessoryDaiDeviceInit(const struct AudioCard *card, const struct DaiDevice *device)
{
    if (device == NULL || device->devDaiName == NULL) {
        AUDIO_DEVICE_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }
    AUDIO_DEVICE_LOG_DEBUG("codec dai device name: %s\n", device->devDaiName);
    (void)card;
    return HDF_SUCCESS;
}

