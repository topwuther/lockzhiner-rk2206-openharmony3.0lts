# 小凌派-RK2206开发板基础外设开发——GPIO

本示例将演示如何在小凌派-RK2206开发板上使用GPIO做输入输出操作。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

请参考[GPIO接口]（/device/rockchip/hardware/docs/GPIO.md）

### 软件设计

#### GPIO初始化分析

这部分代码为GPIO初始化的代码。首先用 `LzGpioInit()` 函数将 `GPIO0_PA0`配置成GPIO，然后调用 `PinctrlSet()`函数将 `GPIO0_PA0`复用为 `GPIO`。

```c
void gpio_process()
{
    unsigned int cur = 0;
    LzGpioValue value = LZGPIO_LEVEL_LOW;

    LzGpioInit(GPIO_TEST);
    PinctrlSet(GPIO_TEST, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    ...
}
```

#### GPIO输出操作

这部分代码为GPIO设置输出。首先用`LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_OUT)`函数将GPIO配置为输出模式，然后调用`LzGpioSetVal(GPIO_TEST, cur)`函数输出GPIO电平值，最后调用`LzGpioGetVal(GPIO_TEST, &value)`读取输出GPIO电平值。

```c
printf("Read GPIO\n");
LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_OUT);
if (cur == 0)
{
	LzGpioSetVal(GPIO_TEST, cur);
	LzGpioGetVal(GPIO_TEST, &value);
	printf("\tgpio set %d => gpio get %d\n", cur, value);

	cur = 1;
}
else
{
	LzGpioSetVal(GPIO_TEST, cur);
	LzGpioGetVal(GPIO_TEST, &value);
	printf("\tgpio set %d => gpio get %d\n", cur, value);

	cur = 0;
}
/* 睡眠5秒 */
sleep(5);
```

#### GPIO输出操作

这部分代码为GPIO设置输出。首先用`LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_IN)`函数将GPIO配置为输入模式，然后调用`LzGpioGetVal(GPIO_TEST, &value)`函数读取输入GPIO电平值。

```c
printf("Read GPIO\n");
LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_IN);
LzGpioGetVal(GPIO_TEST, &value);
printf("\tgpio get %d\n", value);
/* 睡眠5秒 */
sleep(5);
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `gpio_example` 参与编译。

```r
"./b9_gpio:gpio_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lgpio_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lgpio_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```r
***************GPIO Example*************
Write GPIO
        gpio set 0 => gpio get 0
Read GPIO
        gpio get 1

***************GPIO Example*************
Write GPIO
        gpio set 1 => gpio get 1
Read GPIO
        gpio get 1

......
```

