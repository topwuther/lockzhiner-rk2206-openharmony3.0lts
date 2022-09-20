# 小凌派-RK2206开发板基础外设开发——GPIO中断

本示例将演示如何在小凌派-RK2206开发板上使用GPIO做输入输出操作。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

请参考[GPIO接口]（/device/rockchip/hardware/docs/GPIO.md）

### 软件设计

#### GPIO初始化

这部分代码为GPIO初始化的代码。首先用 `LzGpioInit()` 函数将 `GPIO0_PA0`配置成GPIO，然后调用 `PinctrlSet()`函数将 `GPIO0_PA0`复用为 `GPIO`，最后调用`LzGpioSetDir()`将GPIO配置成输入模式。

```c
void gpio_process()
{
    unsigned int ret;

    /* 初始化引脚为GPIO */
    LzGpioInit(GPIO_TEST);
    /* 引脚复用配置为GPIO */
    PinctrlSet(GPIO_TEST, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    /* 引脚配置为输入 */
    LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_IN);
    ...
}
```

#### GPIO配置中断

这部分代码为GPIO设置输出。首先用`LzGpioRegisterIsrFunc()`函数将GPIO配置为中断触发模式，然后调用`LzGpioEnableIsr()`函数使能中断触发。

```c
	/* 设置中断触发方式为上升沿和中断处理函数，
     * 详见//device/rockchip/rk2206/adapter/include/lz_hardware/gpio.h
     */
    ret = LzGpioRegisterIsrFunc(GPIO_TEST, LZGPIO_INT_EDGE_RISING, gpio_isr_func, NULL);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("LzGpioRegisterIsrFunc failed(%d)\n", ret);
        return;
    }

    /* 初始化中断触发次数 */
    m_gpio_interrupt_count = 0;
    /* 开启中断使能，
     * 注意：如果手动将GPIO接到接地线，则容易出现抖动，
     * 导致GPIO中断次数一下就变得很大
     */
    LzGpioEnableIsr(GPIO_TEST);
```

#### GPIO中断响应函数

中断响应函数`gpio_isr_func()`主要响应GPIO中断触发，并将中断触发次数累加1，代码如下所示：

```c
void gpio_isr_func(void *args)
{
    m_gpio_interrupt_count++;
}
```

#### GPIO中断响应效果呈现

效果呈现如下：

```c
	while (1)
    {
        printf("***************GPIO Interrupt Example*************\n");
        printf("gpio interrupt count = %d\n", m_gpio_interrupt_count);
        printf("\n");
        
        /* 睡眠1秒 */
        sleep(1);
    }
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `gpio_interrupt_example` 参与编译。

```r
"./b10_gpio_interrupt:gpio_interrupt_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lgpio_interrupt_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lgpio_interrupt_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```r
***************GPIO Interrupt Example*************
gpio interrupt count = 0

***************GPIO Interrupt Example*************
gpio interrupt count = 1

***************GPIO Interrupt Example*************
gpio interrupt count = 1

......
```

