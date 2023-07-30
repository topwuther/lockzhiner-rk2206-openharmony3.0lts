# 小凌派-RK2206开发板基础外设开发——串口打印显示

本示例将演示如何在小凌派-RK2206开发板上使用额外串口打印

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)



## 硬件接口说明

引脚名称开发者可在硬件资源图中查看。

| 引脚名称 | 功能描述 |
| :--- | :------- | 
| GPIO0_PB6 | UART0_RX_M0 串口RX |
| GPIO0_PB7 | UART0_TX_M0 串口TX | 
| GND | 电源地引脚 | 


### 硬件连接

安装图如下所示：
![串口模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/uart/20220505104232-串口连接.jpg)

注意：
（1）串口GND引脚一定要连接到开发板的GND引脚上。

## 程序设计

### API分析

#### 头文件

```c
#include "lz_hardware.h"
```

注意：实际头文件可参考[uart.h](/device/rockchip/rk2206/adapter/include/lz_hardware/uart.h)

#### LzUartInit()

```c
unsigned int LzUartInit(unsigned int id, const UartAttribute *param);
```

**描述：**

初始化uart设备。

**参数：**

| 名字          | 描述           |
| :------------ | :------------- |
| id            | 串口ID         |
| param         | 串口配置参数，详情可参考[uart.h](/device/rockchip/rk2206/adapter/include/lz_hardware/uart.h) |

**返回值：**

返回LZ_HARDWARE_SUCCESS为成功，反之为失败

#### LzUartDeinit()

```c
unsigned int LzUartDeinit(unsigned int id);
```

**描述：**

释放uart设备。

**参数：**

| 名字          | 描述           |
| :------------ | :------------- |
| id            | 串口ID         |

**返回值：**

返回LZ_HARDWARE_SUCCESS为成功，反之为失败

#### LzUartRead()

```c
unsigned int LzUartRead(unsigned int id, unsigned char *data, unsigned int dataLen);
```

**描述：**

串口读操作。

**参数：**

| 名字          | 描述                      |
| :------------ | :------------------------ |
| id            | 串口ID                    |
| data          | 串口读取数据后存放的地址    |
| dataLen       | 串口读取数据的个数         |

**返回值：**

成功返回读取数据长度

#### LzUartWrite()

```c
unsigned int LzUartWrite(unsigned int id, const unsigned char *data, unsigned int dataLen);
```

**描述：**

串口写操作。

**参数：**

| 名字          | 描述                      |
| :------------ | :------------------------ |
| id            | 串口ID                    |
| data          | 串口写入数据的地址         |
| dataLen       | 串口写入数据的个数         |

**返回值：**

成功返回发送数据长度


### 主要代码分析

这部分代码为uart初始化的代码。首先用 `LzUartDeinit()` 函数将串口释放掉；其次用`PinctrlSet()`将GPIO0_PB6复用为UART0_RX_M0，GPIO0_PB7复用为UART0_TX_M0。最后调用 `LzUartInit()`函数初始化uart。

```c
LzUartDeinit(UART_ID);

attr.baudRate = 115200;
attr.dataBits = UART_DATA_BIT_8;
attr.pad = FLOW_CTRL_NONE;
attr.parity = UART_PARITY_NONE;
attr.rxBlock = UART_BLOCK_STATE_NONE_BLOCK;
attr.stopBits = UART_STOP_BIT_1;
attr.txBlock = UART_BLOCK_STATE_NONE_BLOCK;

PinctrlSet(GPIO0_PB6, MUX_FUNC2, PULL_KEEP, DRIVE_LEVEL2);
PinctrlSet(GPIO0_PB7, MUX_FUNC2, PULL_KEEP, DRIVE_LEVEL2);

ret = LzUartInit(UART_ID, &attr);
if (ret != LZ_HARDWARE_SUCCESS)
{
    printf("%s, %d: LzUartInit(%d) failed!\n", __FILE__, __LINE__, ret);
    return;
}
```

具体uart写操作如下：

```c
// LzUartWrite是异步发送，非阻塞发送
LzUartWrite(UART_ID, str, strlen(str));
// 等待发送完毕
LOS_Msleep(1000);
```

具体uart读操作如下：

```c
recv_length = 0;
memset(recv_buffer, 0, sizeof(recv_buffer));
recv_length = LzUartRead(UART_ID, recv_buffer, sizeof(recv_buffer));
printf("%s, %d: uart recv and str(%s), len(%d)\n", __FILE__, __LINE__, recv_buffer, recv_length);
printf("");
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `uart_example` 参与编译。

```r
"./b6_uart:uart_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-luart_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -luart_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，串口0显示如下：

```c
HelloWorld!
HelloWorld!
HelloWorld!
HelloWorld!
...
```

注意：如果你使用MobaXterm，并且发现串口0能打印，但是换行有问题，则：
（1）在MobaXterm界面中按住右边的Ctrl按键 + 右击鼠标，弹出菜单，选择`Change terminal settings...`，如下图所示：

![右击菜单图](/vendor/lockzhiner/rk2206/docs/figures/uart/MobaXterm_右击菜单.png)

（2）将2个选项勾选上即可。如下图所示：

![勾选图](/vendor/lockzhiner/rk2206/docs/figures/uart/MobaXterm_勾选.png)

## 注意事项

（1）RK2206的串口波特率误差率为3%。一般而言，市面上的USB转串口（如：CH430等）的允许误差率为0.3%，会导致小凌派-RK2206开发板的串口发送数据出现乱码（其实不是乱码，而是波特率有误差，导致出现乱码），请替换为接收允许误差率为3%的USB转串口芯片，比如FS232。
