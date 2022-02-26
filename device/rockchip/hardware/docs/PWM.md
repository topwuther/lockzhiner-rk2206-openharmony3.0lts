# PWM概述

## 简介

- 脉冲宽度调制(PWM)，是英文“Pulse Width Modulation”的缩写，简称**脉宽调制**，是利用微处理器的数字输出来对模拟电路进行控制的一种非常有效的技术，广泛应用在从测量、通信到功率控制与变换的许多领域中.

- PWM的频率：是指1秒钟内信号从高电平到低电平再回到高电平的次数(一个周期)；

- PWM占空比：是一个脉冲周期内，高电平的时间与整个周期时间的比例：

- 总结：
  PWM就是在合适的信号频率下，通过一个周期里改变占空比的方式来改变输出的有效电压

  PWM频率越大，响应越快，

  

## 接口说明

### 包含头文件： 

```c
#include "lz_hardware.h"
```

#### 1. PWM设备IO初始化

```c
unsigned int PwmIoInit(PwmBusIo io);
1) 参数说明：
   io：  PWM设备io配置
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```



#### 2. PWM设备初始化接口

```c
unsigned int LzPwmInit(unsigned int port);
1) 参数说明：
   port：  PWM设备id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 3. PWM设备释放接口

```c
unsigned int LzPwmDeinit(unsigned int port);
1) 参数说明：
   port：  PWM设备id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 4. PWM设备启动接口：

```c
unsigned int LzPwmStart(unsigned int port, unsigned int duty, unsigned int cycle);
1) 参数说明：
   port：  PWM设备id
   duty:  脉宽时间/高电平时间(ns)
   cycle:  总周期时长(ns)
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 5. PWM设备停止接口：

```c
unsigned int LzPwmStop(unsigned int port);
1) 参数说明：
   port：  PWM设备id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```



## 使用实例

```c
#include "lz_hardware.h"

#define BUZZER_IO 3

PwmBusIo g_buzzer  = {
    .pwm = {.gpio = GPIO0_PC3, .func = MUX_FUNC2, .type = PULL_DOWN, .drv = DRIVE_KEEP, .dir = GPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_PWM3,
    .mode = FUNC_MODE_NONE,
};



unsigned int pwm_sample()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    //初始化pwm
    if (PwmIoInit(g_buzzer) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    if (LzPwmInit(BUZZER_IO) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;

    //启动pwm控制蜂鸣器，响铃3秒
    LzPwmStart(BUZZER_IO, 160000, 200000);
    ToyMsleep(3000);
    //关闭蜂鸣器
    LzPwmStop(BUZZER_IO);
    
    if (LzPwmDeinit(BUZZER_IO) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    
    return LZ_HARDWARE_SUCCESS;
}


```

