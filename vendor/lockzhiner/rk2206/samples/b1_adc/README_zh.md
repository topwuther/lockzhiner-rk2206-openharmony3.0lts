# 小凌派-RK2206开发板基础外设开发——ADC

本示例将演示如何在小凌派-RK2206开发板上使用ADC做按键测试。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

### API分析

#### adc_dev_init()

```c
static unsigned int adc_dev_init();
```

**描述：**

初始化ADC端口和引脚。

**参数：**

无

**返回值：**

0为成功，反之为失败。

#### adc_get_voltage()

```c
static float adc_get_voltage()
```

**描述：**

获取当前ADC采集的电压数值。

**参数：**

无

**返回值：**

实际电压数值，单位为V。

### 软件设计

#### adc初始化源代码分析

这部分代码为adc初始化的代码。首先用 `DevIoInit()` 函数将 `GPIO0_PC5`复用为 `SRADC5`。然后调用 `LzSaradcInit()`函数初始化ADC5端口。最后设置寄存器 `GRF_SOC_CON29`的 `bit[4] = 0`，即ADC的基准电压以AVDD（芯片外接地）为准。

```c
/***************************************************************
* 函数名称: adc_dev_init
* 说    明: 初始化ADC
* 参    数: 无
* 返 回 值: 0为成功，反之为失败
***************************************************************/
static unsigned int adc_dev_init()
{
    unsigned int ret = 0;
    uint32_t *pGrfSocCon29 = (uint32_t *)(0x41050000U + 0x274U);
    uint32_t ulValue;

    ret = DevIoInit(m_adcKey);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %s, %d: ADC Key IO Init fail\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }
    ret = LzSaradcInit();
    if (ret != LZ_HARDWARE_SUCCESS) {
        printf("%s, %s, %d: ADC Init fail\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }

    /* 设置saradc的电压信号，选择AVDD */
    ulValue = *pGrfSocCon29;
    ulValue &= ~(0x1 << 4);
    ulValue |= ((0x1 << 4) << 16);
    *pGrfSocCon29 = ulValue;
  
    return 0;
}
```

#### ADC读数据操作

ADC模块提供 `LzSaradcReadValue()`读取ADC数据。具体ADC读数据数据的操作如下：

```c
ret = LzSaradcReadValue(ADC_CHANNEL, &data);
if (ret != LZ_HARDWARE_SUCCESS)
{
    printf("%s, %s, %d: ADC Read Fail\n", __FILE__, __func__, __LINE__);
    return 0.0;
}
```

#### ADC数据计算成实际电压操作

ADC模块采用10位的ADC采集寄存器，可测试电压范围为0~3.3V，所以ADC采集数值换算为实际电压计算公司为：

```r
实际电压 = (ADC采集数值 / 1024) * 3.3V
```

注意：实际电压是 `float`类型，源代码计算要规范。具体源代码如下所示：

```c
return (float)(data * 3.3 / 1024.0)
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `adc_example` 参与编译。

```r
"./b0_adc:adc_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-ladc_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -ladc_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```r
***************Adc Example*************
vlt:3.297V
***************Adc Example*************
vlt:3.297V
***************Adc Example*************
vlt:3.297V
***************Adc Example*************
vlt:3.297V
***************Adc Example*************
vlt:3.297V
***************Adc Example*************
vlt:3.297V
```
