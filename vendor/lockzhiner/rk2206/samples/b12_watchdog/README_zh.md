# 小凌派-RK2206开发板基础外设开发——Watchdog

本示例将演示如何在小凌派-RK2206开发板上使用Watchdog。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

请参考[Watchdog接口]（/device/rockchip/hardware/docs/WDT.md）

### 软件设计

#### Watchdog初始化

这部分代码为watchdog初始化的代码。调用 `LzWatchdogInit()` 函数将 芯片中的看门狗进行初始化。

```c
void watchdog_process()
{
    uint32_t current = 0;
    
    printf("%s: start\n", __func__);
    LzWatchdogInit();
	......
}
```

#### Watchdog配置喂狗

这部分代码为Watchdog配置喂狗信息。首先调用 `LzWatchdogSetTimeout()`函数设置最长喂狗时间（注意：该喂狗时间有档位限定），最后调用`LzWatchdogStart()`开启watchdog，并配置重启方式。

```c
void watchdog_process()
{
    ......
    /* 实际是1.3981013 * (2 ^ 4) = 22.3696208秒 */
    LzWatchdogSetTimeout(20);
    LzWatchdogStart(LZ_WATCHDOG_REBOOT_MODE_FIRST);
	......
}
```

#### Watchdog喂狗处理

调用`LzWatchdogKeepAlive()`负责喂狗，代码如下所示：

```c
void watchdog_process()
{
    ......
    while (1)
    {
        printf("Wathdog: current(%d)\n", ++current);
        if (current <= 10)
        {
            printf("    freedog\n");
            LzWatchdogKeepAlive();
        }
        else
        {
            printf("    not freedog\n");
        }
        
        LOS_Msleep(1000);
    }
}
```



## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `watchdog_example` 参与编译。

```r
"./b12_watchdog:watchdog_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lwatchdog_example` 参与编译。

```r
app_LIBS = -lwatchdog_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```r
watchdog_process: start
Wathdog: current(1)
    freedog
Wathdog: current(2)
    freedog
Wathdog: current(3)
    freedog
Wathdog: current(4)
    freedog
Wathdog: current(5)
    freedog
Wathdog: current(6)
    freedog
Wathdog: current(7)
    freedog
Wathdog: current(8)
    freedog
Wathdog: current(9)
    freedog
Wathdog: current(10)
    freedog
Wathdog: current(11)
    not freedog
Wathdog: current(12)
    not freedog
Wathdog: current(13)
    not freedog
Wathdog: current(14)
    not freedog
Wathdog: current(15)
    not freedog
Wathdog: current(16)
    not freedog
Wathdog: current(17)
    not freedog
Wathdog: current(18)
    not freedog
Wathdog: current(19)
    not freedog
Wathdog: current(20)
    not freedog
Wathdog: current(21)
    not freedog
Wathdog: current(22)
    not freedog
Wathdog: current(23)
    not freedog
Wathdog: current(24)
    not freedog
Wathdog: current(25)
    not freedog
Wathdog: current(26)
    not freedog
Wathdog: current(27)
    not freedog
Wathdog: current(28)
    not freedog
Wathdog: current(29)
    not freedog
Wathdog: current(30)
    not freedog
Wathdog: current(31)
    not freedog
Wathdog: current(32)
    not freedog
entering kernel init...
hilog will init.
[IOT:D]IotInit: start ....
[MAIN:D]Main: LOS_Start ...
Entering scheduler
[IOT:D]IotProcess: start ....
......
```

