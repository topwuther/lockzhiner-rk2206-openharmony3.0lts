# GPIO概述

## 简介

- GPIO（General-purpose input/output）即通用型输入输出。通常，GPIO控制器通过分组的方式管理所有GPIO管脚，每组GPIO有一个或多个寄存器与之关联，通过读写寄存器完成对GPIO管脚的操作。
- GPIO接口定义了操作GPIO管脚的标准方法集合，包括
  
  - 设置管脚方向： 方向可以是输入或者输出\(暂不支持高阻态\)
  - 读写管脚电平值： 电平值可以是低电平或高电平
  - 设置管脚中断服务函数：设置一个管脚的中断响应函数，以及中断触发方式
  - 使能和禁止管脚中断：禁止或使能管脚中

## 接口说明

### 包含头文件：

```c
#include "lz_hardware.h"
```

#### 1. GPIO初始化接口

```c
unsigned int LzGpioInit(GpioID id);
1) 参数说明：
   id： Gpio管脚id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 2. GPIO设备释放接口：

```c
unsigned int LzGpioDeinit(GpioID id);
1) 参数说明：
   id： Gpio管脚id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 3.GPIO设备配置管脚方向：

```c
unsigned int LzGpioSetDir(GpioID id, LzGpioDir dir);
1) 参数说明：
   id：  Gpio管脚id
   dir： Gpio管脚方向
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 4.GPIO设备获取管脚方向：

```c
unsigned int LzGpioGetDir(GpioID id, LzGpioDir *dir);
1) 参数说明：
   id：  Gpio管脚id
   dir： Gpio管脚方向
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 5.GPIO设备设置管脚电平值：

```c
unsigned int LzGpioSetVal(GpioID id, LzGpioValue val);
1) 参数说明：
   id：  Gpio管脚id
   val： Gpio管脚电平值
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 6.GPIO设备获取管脚电平值：

```c
unsigned int LzGpioGetVal(GpioID id, LzGpioValue *val);
1) 参数说明：
   id：  Gpio管脚id
   val： Gpio管脚电平值
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 7.GPIO设备注册管脚中断函数：

```c
unsigned int LzGpioRegisterIsrFunc(GpioID id, LzGpioIntType type, GpioIsrFunc func, void *arg);
1) 参数说明：
   id：       Gpio管脚id
   type：	 Gpio中断类型
   func:      Gpio中断函数
   arg:       Gpio中断函数参数
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 8.GPIO设备注销管脚中断函数：

```c
unsigned int LzGpioUnregisterIsrFunc(GpioID id);
1) 参数说明：
   id：       Gpio管脚id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 9.GPIO设备使能管脚中断：

```c
unsigned int LzGpioEnableIsr(GpioID id);
1) 参数说明：
   id：       Gpio管脚id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 10.GPIO设备关闭管脚中断：

```c
unsigned int LzGpioDisableIsr(GpioID id);
1) 参数说明：
   id：       Gpio管脚id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

## 使用实例

```c
#include "lz_hardware.h"

#定义从机地址
#define TEST_GPIO   GPIO0_PC6

void GpioIrqFunc()
{
    printf("enter GpioIrqFunc\n");
}

unsigned int gpio_sample()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    uint16_t val = 0;
    if (LzGpioInit(TEST_GPIO) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    //设置GPIO管脚为输入
    ret = LzGpioSetDir(TEST_GPIO, LZGPIO_DIR_IN);  //input
    //拉高GPIO管脚电平值
    ret = LzGpioSetVal(TEST_GPIO, LZGPIO_LEVEL_HIGH);
    //注册Gpio中断,设置Gpio管脚中断为下降沿触发
    LzGpioRegisterIsrFunc(TEST_GPIO, LZGPIO_INT_EDGE_FALLING, GpioIrqFunc, NULL);
    //拉低GPIO管脚电平值
    ret = LzGpioSetVal(TEST_GPIO, LZGPIO_LEVEL_LOW);
    //获取GPIO管脚电平值
    ret = LzGpioGetVal(TEST_GPIO, &val);
    return LZ_HARDWARE_SUCCESS;
}
```

