# I2C概述

## 简介

-   I2C\(Inter Integrated Circuit\)总线是由Philips公司开发的一种简单、双向二线制同步串行总线。
-   I2C以主从方式工作，通常有一个主设备和一个或者多个从设备，主从设备通过SDA\(SerialData\)串行数据线以及SCL\(SerialClock\)串行时钟线两根线相连。

-   I2C数据的传输必须以一个起始信号作为开始条件，以一个结束信号作为传输的停止条件。数据传输以字节为单位，高位在前，逐个bit进行传输。
-   I2C总线上的每一个设备都可以作为主设备或者从设备，而且每一个设备都会对应一个唯一的地址，当主设备需要和某一个从设备通信时，通过广播的方式，将从设备地址写到总线上，如果某个从设备符合此地址，将会发出应答信号，建立传输。

-   I2C接口定义了完成I2C传输的通用方法集合，包括：

    -   I2C控制器管理:  打开或关闭I2C控制器
    -   I2C消息传输：通过消息传输结构体数组进行自定义传输

## 接口说明

### 包含头文件： 

```c
#include "lz_hardware.h"
```

#### 1. I2C驱动IO管脚配置：

```C
I2cBusIo g_i2c0m0 = {
    .scl =  {.gpio = GPIO0_PB5, .func = MUX_FUNC4, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PB4, .func = MUX_FUNC4, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M0,
};
```

#### 2. I2C驱动IO管脚初始化接口

```c
unsigned int I2cIoInit(I2cBusIo io);
1) 参数说明：
   io： io引脚相关配置
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```



#### 3.I2C设备初始化接口：

```c
unsigned int LzI2cInit(unsigned int id, unsigned int freq)；
1) 参数说明：
   id：  i2c总线id
   freq：i2c频率
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 4.I2C设备释放接口：

```c
unsigned int LzI2cDeinit(unsigned int id);
1) 参数说明：
   id： i2c总线id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 5.I2C设备设置频率：

```c
unsigned int LzI2cSetFreq(unsigned int id, unsigned int freq);
1) 参数说明：
   id：  i2c总线id
   freq：i2c频率
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 6.I2C设备Transfer接口：

```c
unsigned int LzI2cTransfer(unsigned id, LzI2cMsg *msgs, unsigned int num);
1) 参数说明：
   id：  i2c总线id
   msgs：需要转发给从设备的消息
   num:  消息数量
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 7.I2C设备读接口：

```c
static inline unsigned int LzI2cRead(unsigned int id, unsigned short slaveAddr, unsigned char *data, unsigned int len);
1) 参数说明：
   id：       i2c总线id
   slaveAddr：从机地址
   data:      需要读取的消息
   len:       读取消息长度
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码  
```

#### 8.I2C设备写接口：

```c
static inline unsigned int LzI2cWrite(unsigned int id, unsigned short slaveAddr, const unsigned char *data, unsigned int len)
1) 参数说明：
   id：       i2c总线id
   slaveAddr：从机地址
   data:      需要写入的消息
   len:       写入消息长度
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码  
```

#### 9.I2C设备读寄存器接口：

```c
static inline unsigned int LzI2cReadReg(unsigned int id, unsigned short slaveAddr,
                                      unsigned char *regAddr, unsigned int regLen,
                                      unsigned char *data, unsigned int len)
1) 参数说明：
   id：       i2c总线id
   slaveAddr：从机地址
   regAddr:   从机寄存器地址
   regLen:   从机寄存器地址长度
   data:      需要写入的消息
   len:       写入消息长度
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码  
```

#### 10.I2C设备写寄存器接口：

```c
static inline unsigned int LzI2cWriteReg(unsigned int id, unsigned short slaveAddr,
                                       unsigned char *regAddr, unsigned int regLen,
                                       unsigned char *data, unsigned int len)
1) 参数说明：
   id：       i2c总线id
   slaveAddr：从机地址
   regAddr:   从机寄存器地址
   regLen:   从机寄存器地址长度
   data:      需要写入的消息
   len:       写入消息长度
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码  
```



## 使用实例

```c
#include "lz_hardware.h"

#定义从机地址
#define ATH20_ADDR 0x38
#define I2C1_BUS   1

#io管脚配置
I2cBusIo g_i2c1m2 = {
    .scl =  {.gpio = GPIO0_PA3, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA2, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C1,
    .mode = FUNC_MODE_M2,
};

unsigned int g_i2c1_freq = 400000;

unsigned int i2c_sample()
{
    uint32_t ret = LZ_HARDWARE_SUCCESS;
    uint8_t buff[6];
    int32_t len = 6;
    #初始化I2C
    if (I2cIoInit(g_i2c1m2) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    if (LzI2cInit(I2C1, g_i2c1_freq) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    
    #i2c写数据示例
    len = 6;
    buff[0] = 0xAC;
    buff[1] = 0x33;
    buff[2] = 0x00;
    len = 3;
    ret = LzI2cWrite(I2C1_BUS, ATH20_ADDR, buff, len);
    if (ret < 0)
    {
        return LZ_HARDWARE_FAILURE;
    }
	
    #
    ToyUdelay(75);

    len = 0;
    ret = LzI2cWrite(I2C1_BUS, ATH20_ADDR, buff, len);
    if (ret < 0)
    {
        return LZ_HARDWARE_FAILURE;
    }

	#i2c读取数据示例
    buff[0] = 0x00;
    buff[1] = 0x00;
    buff[2] = 0x00;
    len = 6;
    ret = LzI2cRead(I2C1_BUS, ATH20_ADDR, buff, len);
    if (ret < 0)
    {
        LZ_HARDWARE_LOGE(LOG_FACTORY,"read error:%d",ret);
        return LZ_HARDWARE_FAILURE;
    }
    
    return LZ_HARDWARE_SUCCESS;
}


```

