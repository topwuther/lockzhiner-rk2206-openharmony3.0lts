# 小凌派-RK2206开发板基础外设开发——Reboot

本示例将演示如何在小凌派-RK2206开发板上使用Reboot操作。

本次案例将等待10秒后，重启系统。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

### Reboot调用

这部分代码为Reboot代码。调用 `RebootDevice()` 函数可将系统重启（注意：需要等待1.3秒左右才正式重启）。

```c
void reboot_process()
{
    uint32_t current = 0;
    
    printf("reboot: start\n");

    while (1)
    {
        printf("reboot: current(%ld)\n", ++current);
        if (current == 10)
        {
            printf("reboot already\n");
            RebootDevice(0);
        }
        
        LOS_Msleep(1000);
    }
}
```

### Reboot注意

#### （1）头文件

reset.h目录在`//base/iot_hardware/peripheral/interfaces/kits/reset.h`

所以要调用`RebootDevice()`函数必须修改目录下的BUILD.gn，添加头文件路径

```
static_library("reboot_example") {
    sources = [
        "reboot_example.c",
    ]

    include_dirs = [
        "//utils/native/lite/include",
        "//device/lockzhiner/rk2206/adapter/include",
        "include",
        "//base/iot_hardware/peripheral/interfaces/kits",	# 添加reset.h，才能调用RebootDevice()
    ]

    deps = [
        "//device/rockchip/hardware:hardware",
    ]
}
```

源代码添加头文件，具体代码如下：

```c
#include "reset.h"
```

#### （2）具体实现

RebootDevice函数具体实现可参考：`//device/rockchip/rk2206/adapter/hals/iot_hardware/wifiiot_lite/hal_reset.c`

具体代码如下：

```c
void RebootDevice(unsigned int cause)
{
    printf("RebootDevice: restart!\n");
    LzWatchdogInit();
    LzWatchdogSetTimeout(1);
    LzWatchdogStart(LZ_WATCHDOG_REBOOT_MODE_FIRST);
    LOS_Msleep(2000);
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `reboot_example` 参与编译。

```r
"./b13_reboot:reboot_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lreboot_example` 参与编译。

```r
app_LIBS = -lreboot_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```r
entering kernel init...
hilog will init.
[IOT:D]IotInit: start ....
[MAIN:D]Main: LOS_Start ...
Entering scheduler
[IOT:D]IotProcess: start ....
reboot: startuccess.
reboot: current(1)
reboot: current(2)
reboot: current(3)
reboot: current(4)
reboot: current(5)
reboot: current(6)
reboot: current(7)
reboot: current(8)
reboot: current(9)
reboot: current(10)
reboot already
RebootDevice: restart!
entering kernel init...
hilog will init.
[IOT:D]IotInit: start ....
[MAIN:D]Main: LOS_Start ...
Entering scheduler
[IOT:D]IotProcess: start ....
......
```

