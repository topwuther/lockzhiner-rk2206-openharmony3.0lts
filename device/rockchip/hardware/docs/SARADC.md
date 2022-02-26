# SARADC概述

## 简介

- 逐次逼近寄存器型模数转换器（Successive-Approximation Analog to Digital Converter），是一种常用的A/D转换结构，其较低的功耗表现，还不错的转换速率，在有低功耗要求（可穿戴设备、物联网）的数据采集场景下广泛应用。

  

  


## 接口说明

### 包含头文件： 

```c
#include "lz_hardware.h
```

#### 1. SARADC设备初始化接口

```c
int LzSaradcInit(void);
1) 参数说明：
	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 2. SARADC设备释放接口

```c
int LzSaradcDeinit(void);
1) 参数说明：
	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 3. SARADC设备读取接口：

```c
int LzSaradcReadValue(unsigned int chn, unsigned int *val);
1) 参数说明：
   chn：  ADC通道id
   val:  读取值
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```



## 使用实例

```c
#include "lz_hardware.h"

#define ADC_CHNN 5

DevIo g_adckey = {
    .isr =   {.gpio = INVALID_GPIO},
    .rst =   {.gpio = INVALID_GPIO},
    .ctrl1 = {.gpio = GPIO0_PC5, .func = MUX_FUNC1, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = GPIO_DIR_IN, .val = LZGPIO_LEVEL_KEEP},
    .ctrl2 = {.gpio = INVALID_GPIO},
};



unsigned int adc_sample()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    uint32_t keyValue;
    //初始化adc
    if (DevIoInit(g_adckey) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    if (LzSaradcInit() != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;

    if (LzSaradcReadValue(ADC_CHNN, &keyValue) != LZ_HARDWARE_SUCCESS){
        return LZ_HARDWARE_FAILURE;
    }
    
    if (LzSaradcDeinit() != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    
    return LZ_HARDWARE_SUCCESS;
}


```

