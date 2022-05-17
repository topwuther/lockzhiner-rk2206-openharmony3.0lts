# UART概述

## 简介
- 通用异步收发传输器（Universal Asynchronous Receiver/Transmitter)，通常称作UART。
- 它将要传输的资料在串行通信与并行通信之间加以转换。作为把并行输入信号转成串行输出信号的芯片，UART通常被集成于其他通讯接口的连结上。
- UART是一种通用串行数据总线，用于异步通信。该总线双向通信，可以实现全双工传输和接收。
- 主要参数
    1 波特率
    表示每秒钟传送的码元符号的个数
    2 报文格式
    起始位（1bit）+ 数据位（5-8bit）+ 奇偶校验位（1bit）+ 停止位（1,1.5,2bit）

## 接口说明

### 包含头文件： 

```c
#include "lz_hardware.h"
```

#### 1. UART设备初始化接口

```c
unsigned int UartIoInit(UartBusIo io);
1) 参数说明：
    io：  UART设备IO管脚配置
2) 返回值：
    成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 2. UART设备初始化接口

```c
unsigned int LzUartInit(unsigned int id, UartAttribute *param);
1) 参数说明：
    id：  UART总线id
    param：UART配置信息
2) 返回值：
    成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 3. UART设备释放接口：

```c
unsigned int LzUartDeinit(unsigned int id);
1) 参数说明：
    id：UART总线id
2) 返回值：
    成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 4. UART写数据：

```c
unsigned int LzUartWrite(unsigned int id, const unsigned char *data, unsigned int dataLen);
1) 参数说明：
    id：      UART总线id
    data:     写入数据
    dataLen:  写入数据长度
2) 返回值：
    成功返回写入长度 失败返回 0
```

#### 5. UART读数据：

```c
unsigned int LzUartRead(unsigned int id, unsigned char *data, unsigned int dataLen);
1) 参数说明：
    id：      UART总线id
    data:     写入数据
    dataLen:  写入数据长度
2) 返回值：
    成功返回读取长度, 失败返回 0
```

####  


## 使用实例

```c
#include "lz_hardware.h"
#define UART_TAG "UART0"

#define UART_ID   0
UartBusIo g_uart0m0 = {
    .rx   = {.gpio = GPIO0_PB6,    .func = MUX_FUNC2, .type = PULL_KEEP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .tx   = {.gpio = GPIO0_PB7,    .func = MUX_FUNC2, .type = PULL_KEEP, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .ctsn = {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP,   .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .rtsn = {.gpio = INVALID_GPIO, .func = MUX_FUNC4, .type = PULL_UP,   .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_UART0,
    .mode = FUNC_MODE_NONE,
};

UartAttribute g_uart0attr = {
    .baudRate = 115200, 
    .dataBits = UART_DATA_BIT_8, 
    .parity   = UART_PARITY_NONE,
    .stopBits = UART_STOP_BIT_1, 
};

unsigned int uart_sample()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    unsigned char buf[128] = {0};

    //初始化uart
    if (UartIoInit(g_uart0m0) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;
    if (LzUartInit(UART_ID, &g_uart0attr) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;

    while(1)
    {
        //读取串口数据
        ret = LzUartRead(UART_ID, buf, 128);
        if(ret)
        {
            LZ_HARDWARE_LOG(UART_TAG, "%s:uart0 read  cnt:%d", __func__, ret);
            //发送串口数据
            ret = LzUartWrite(UART_ID, buf, ret);
            if(ret){
                LZ_HARDWARE_LOG(UART_TAG, "%s:uart0 write cnt:%d", __func__, ret);
                break;
            }
        }
        usleep(10000);
    }
    if (LzUartDeinit(UART_ID) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;

    LzGpioDeinit(g_uart0m0.tx.gpio);
    LzGpioDeinit(g_uart0m0.rx.gpio);

    return LZ_HARDWARE_SUCCESS;
}


```

