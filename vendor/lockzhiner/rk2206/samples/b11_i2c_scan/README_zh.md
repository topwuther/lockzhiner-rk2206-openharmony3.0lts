# 小凌派-RK2206开发板基础外设开发——i2c扫描从设备

本示例将演示如何在小凌派-RK2206开发板上使用i2c做i2c从设备地址扫描操作。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

请参考[I2C接口]（/device/rockchip/hardware/docs/I2C.md）

### 软件设计

#### i2c初始化

这部分代码为i2c初始化的代码。首先用 `I2cIoInit()` 函数将 i2c总线初始化，然后调用`LzI2cInit()`将配置i2c总线通信速率，最后调用 `PinctrlSet()`函数将 `GPIO0_PA0`和`GPIO0_PA1`复用为 `i2c。

```c
/* 初始化i2c */
I2cIoInit(m_i2cBus);
LzI2cInit(I2C_BUS, m_i2c_freq);

/* 引脚复用为i2c */
PinctrlSet(GPIO0_PA1, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
PinctrlSet(GPIO0_PA0, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
```

#### 使用i2c扫描

这部分代码为使用i2c扫描函数。用`LzI2cScan()`函数将从0x03到0x87轮流询问i2c总线挂载的从设备，如有反馈则记录该从设备地址到slaveAddr数组中。

```c
slaveAddrLen = LzI2cScan(I2C_BUS, slaveAddr, SLAVE_ADDRESS_MAXSIZE);

for (i = 0; i < slaveAddrLen; i++)
{
    printf("slave address: 0x%02x\n", slaveAddr[i]);
}
```

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `i2c_scan_example` 参与编译。

```r
"./b11_i2c_scan:i2c_scan_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-li2c_scan_example` 参与编译。

```r
app_LIBS = -li2c_scan_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```r
slave address: 0x56
slave address: 0x76
......
```

注意：智慧车载模块有带i2c设备，请将智慧车载模块连接到开发板。
