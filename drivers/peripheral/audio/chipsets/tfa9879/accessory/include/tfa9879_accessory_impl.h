/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef TFA9879_ACCESSORY_IMPL_H
#define TFA9879_ACCESSORY_IMPL_H

#include "audio_accessory_if.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "osal_io.h"
#include "securec.h"
#include <linux/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Hi35xx IO register address */
#define HI35XX_I2C_REG_BASE_ADDR 0x114F0000
#define HI35XX_I2S_REG_BASE_ADDR 0x112F0000

typedef enum Hi35xxRegAddr {
    IOCFG_REG36_ADDR = 0x114F0048,     // I2C6_SCL
    IOCFG_REG37_ADDR = 0x114F004C,     // I2C6_SDA
    IOCFG_REG73_ADDR = 0x112F0020,     // I2S_MCLK
    IOCFG_REG74_ADDR = 0x112F0024,     // I2S_BCLK_TX
    IOCFG_REG75_ADDR = 0x112F0028,     // I2S_WS_TX
    IOCFG_REG76_ADDR = 0x112F002C,     // I2S_SD_TX
    IOCFG_REG77_ADDR = 0x112F0030      // I2S_SD_RX
} Hi35xxRegAddr;

/* TFA9879 I2C Device address 1 1 0 1 1 A2 A1 R/W */
typedef enum Tfa9879I2cDevAddr {
    TFA9879_I2C_DEV_ADDR_ADSEL1        =    0x6D,         // 1101101
    TFA9879_I2C_DEV_ADDR_ADSEL2        =    0x6E,         // 1101110
    TFA9879_I2C_DEV_ADDR_ADSEL1_READ   =    0xDB,         // 1101 1011
    TFA9879_I2C_DEV_ADDR_ADSEL1_WRITE  =    0xDA,         // 1101 1010
    TFA9879_I2C_DEV_ADDR_ADSEL2_READ   =    0xDD,         // 1101 1101
    TFA9879_I2C_DEV_ADDR_ADSEL2_WRITE  =    0xDC          // 1101 1100
} Tfa9879I2cDevAddr;

/* TFA9879 register address */
#define TFA9879_REG_BASE_ADDR 0x00;
typedef enum Tfa9879RegAddr {
    DEVICE_CONTROL_REG_ADDR               =   0x00,
    SERIAL_INTERFACE_INPUT1_REG_ADDR      =   0x01,
    PCM_IOM2_FMT_INPUT1_REG_ADDR          =   0x02,
    SERIAL_INTERFACE_INPUT2_REG_ADDR      =   0x03,
    PCM_IOM2_FMT_INPUT2_REG_ADDR          =   0x04,
    EQUALIZER_A_WORD1_REG_ADDR            =   0x05,
    EQUALIZER_A_WORD2_REG_ADDR            =   0x06,
    EQUALIZER_B_WORD1_REG_ADDR            =   0x07,
    EQUALIZER_B_WORD2_REG_ADDR            =   0x08,
    EQUALIZER_C_WORD1_REG_ADDR            =   0x09,
    EQUALIZER_C_WORD2_REG_ADDR            =   0x0A,
    EQUALIZER_D_WORD1_REG_ADDR            =   0x0B,
    EQUALIZER_D_WORD2_REG_ADDR            =   0x0C,
    EQUALIZER_E_WORD1_REG_ADDR            =   0x0D,
    EQUALIZER_E_WORD2_REG_ADDR            =   0x0E,
    BYPASS_CONTROL_REG_ADDR               =   0x0F,
    DYNAMIC_RANGE_COMP_REG_ADDR           =   0x10,
    BASS_TREBLE_REG_ADDR                  =   0x11,
    HIGH_PASS_FILTER_REG_ADDR             =   0x12,
    VOLUME_CONTROL_REG_ADDR               =   0x13,
    DE_EMPHASIS_REG_ADDR                  =   0x14,
    MISCELLANEOUS_STATUS_REG_ADDR         =   0x15
} Tfa9879RegAddr;

enum CodecInputSel {
    ADSEL1 = 0,
    ADSEL2 = 1
};

enum CodecOpMode {
    OFF_MODE = 0,
    AMPLIFIER_MODE = 1
};

struct Tfa9879RegAttr {
    uint8_t regAddr;
    uint16_t regValue; // 16bit
};

int32_t AccessoryDeviceInit(struct AudioCard *audioCard, const struct AccessoryDevice *device);
int32_t AccessoryDeviceReadReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t *value);
int32_t AccessoryDeviceWriteReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t value);
int32_t AccessoryAiaoDeviceReadReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t *value);
int32_t AccessoryAiaoDeviceWriteReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t value);
int32_t AccessoryDaiStartup(const struct AudioCard *card, const struct DaiDevice *device);
int32_t AccessoryDaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param,
    const struct DaiDevice *device);
int32_t AccessoryDaiDeviceInit(const struct AudioCard *card, const struct DaiDevice *device);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
