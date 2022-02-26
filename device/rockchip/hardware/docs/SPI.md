# SPI概述

## 简介

-   SPI是串行外设接口（Serial Peripheral Interface）的缩写，是一种高速的，全双工，同步的通信总线。
-    SPI是由Motorola公司开发，用于在主设备和从设备之间进行通信，常用于与闪存、实时时钟、传感器以及模数转换器等进行通信。
-   SPI以主从方式工作，通常有一个主设备和一个或者多个从设备。主设备和从设备之间一般用4根线相连，它们分别是：

    -   SCLK – 时钟信号，由主设备产生；
    -   MOSI – 主设备数据输出，从设备数据输入；
    -   MISO – 主设备数据输入，从设备数据输出；
    -   CS – 片选，从设备使能信号，由主设备控制;
-   SPI通信通常由主设备发起，通过以下步骤完成一次通信：

    -   通过CS选中要通信的从设备，在任意时刻，一个主设备上最多只能有一个从设备被选中。
    -   通过SCLK给选中的从设备提供时钟信号。
    -   基于SCLK时钟信号，主设备数据通过MOSI发送给从设备，同时通过MISO接收从设备发送的数据，完成通信。

## 接口说明

### 包含头文件： 

```c
#include "lz_hardware.h"
```

#### 1. SPI设备初始化接口

```c
unsigned int SpiIoInit(SpiBusIo io);
1) 参数说明：
   io：  SPI设备IO管脚配置
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 2. SPI设备初始化接口

```c
unsigned int LzSpiInit(unsigned int id, LzSpiConfig conf);
1) 参数说明：
   id：  SPI总线id
   conf：SPI配置信息
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 3. SPI设备释放接口：

```c
unsigned int LzSpiDeinit(unsigned int id);
1) 参数说明：
   id：SPI总线id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 4. SPI设备转发数据：

```c
unsigned int LzSpiTransfer(unsigned int id, LzSpiMsg *msg);
1) 参数说明：
   id：  SPI总线id
   msg： SPI转发数据
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 5. SPI写数据：

```c
static inline unsigned int LzSpiWrite(unsigned int id, unsigned int chn, const void *buf, unsigned int len);
1) 参数说明：
   id：  SPI总线id
   chn： SPI通道id
   buf:  写入数据
   len:  写入数据长度
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 6. SPI读数据：

```c
static inline unsigned int LzSpiRead(unsigned int id, unsigned int chn, void *buf, unsigned int len)
1) 参数说明：
   id：  SPI总线id
   chn： SPI通道id
   buf:  写入数据
   len:  写入数据长度
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

####  

#### 7. SPI设备修改配置：

```c
unsigned int LzSpiSetConfig(unsigned int id, LzSpiConfig conf);
1) 参数说明：
   id：  SPI总线id
   conf：SPI配置信息
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码  
```



## 使用实例

```c
#include "lz_hardware.h"
#define SPI_BUS 1
SpiBusIo g_spi1m1 = {
    .cs =   {.gpio = GPIO0_PB0, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .clk =  {.gpio = GPIO0_PB1, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .mosi = {.gpio = GPIO0_PB2, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    //.miso = {.gpio = GPIO0_PB3, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .miso = {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_SPI1,
    .mode = FUNC_MODE_M1,
};

LzSpiConfig g_spi1Conf = {.bitsPerWord = SPI_PERWORD_8BITS, 
                        .firstBit = SPI_MSB, 
                        .mode = SPI_MODE_0,
                        .csm = SPI_CMS_ONE_CYCLES, 
                        .speed = 5000000, 
                        .isSlave = false};

unsigned int spi_sample()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    unsigned char buf[1];
    //初始化spi
    if (SpiIoInit(g_spi1m1) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    if (LzSpiInit(SPI_BUS, g_spi1Conf) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    
    buf[0] = 'c';
    //spi写入一个字符
    ret = LzSpiWrite(SPI_BUS, 0, buf, 1);
    if(ret != LZ_HARDWARE_SUCCESS){
        LZ_HARDWARE_LOGE(OLED_TAG, "%s:spi write error ret = %d", __func__, ret);
    }
    
    if (LzSpiDeinit(SPI_BUS) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    
    return LZ_HARDWARE_SUCCESS;
}


```

