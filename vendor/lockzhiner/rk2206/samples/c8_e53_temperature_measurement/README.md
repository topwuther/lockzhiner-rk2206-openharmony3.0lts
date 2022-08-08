# 小凌派-RK2206开发板E53模块开发——红外测温

## 实验内容

本示例将演示如何在小凌派-RK2206开发板上实现红外测温的应用案例。

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

硬件资源图如下所示：
![红外测温模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_wm02/e53_wm02_resource_map.jpg)

EEPROM 24C02的设备地址为：0x1010001* ；
红外测温传感器 MLX90614 的设备地址为：0101101*

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在智慧井盖模块背面查看。

| 引脚名称 | 功能描述 |
| :--- | :------- | 
| LED_WARNING | LED控制线，低电平有效 | 
| I2C_SCL  |I2C时钟信号线 | 
| I2C-SDA | I2C数据信号线 | 
| GND | 电源地引脚 | 
| 3V3 |3.3V电源输入引脚 | 
| GND | 电源地引脚 |

## 硬件设计

硬件电路如下图所示：
![红外测温模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_wm02/lz_e53_wm02_sch.jpg)

模块整体硬件电路如上图所示，电路中包含了E53接口连接器，EEPROM存储器、MLX90614传感器，LED指示灯电路，其中EEPROM存储器、传感器为数字接口芯片，直接使用I2C总线控制，电路简单，本文不再过多说明。下面稍微介绍测温原理。
物体红外辐射能量的大小和波长的分布与其表面温度关系密切。因此，通过对物体自身
红外辐射的测量，能准确地确定其表面温度，红外测温就是利用这一原理测量温度的。红外测温器由光学系统、光电探测器、信号放大器和信号处理及输出等部分组成。 光学系统汇聚其视场内的目标红外辐射能量，视场的大小由测温仪的光学零件及其位置确定。红外能量聚焦在光电探测器上并转变为相应的电信号。该信号经过放大器和信号处理电路，并按照仪器内的算法和目标发射率校正后转变为被测目标的温度值。

### 硬件连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![红外测温模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_wm02/e53_wm02_connection_diagram.jpg)

## 程序设计

### API分析

**头文件：**
/vendor/lockzhiner/rk2206/samples/c7_e53_intelligent_smoke_sensor/include/e53_intelligent_smoke_sensation.h

#### e53_iss_init()

```c
uint32_t e53_iss_init(void);
```

**描述说明：**

E53智慧烟雾报警模块初始化，包括初始化eeprom的I2C0、LED1灯的GPIO、BEEP蜂鸣器的GPIO PWM、MQ2烟雾传感器的ADC

**参数说明：**

无

**返回值说明：**

0为成功，反之为失败

#### e53_iss_mq2_ppm_calibration()

```c
void e53_iss_mq2_ppm_calibration(void);
```

**描述说明：**

E53智慧烟雾报警模块ppm校准；

**参数说明：**

无

**返回值说明：**

无

#### e53_iss_get_mq2_ppm()

```c
float e53_iss_get_mq2_ppm(void);
```

**描述说明：**

E53智慧烟雾报警控制模块获取ppm值

**参数说明：**

无

**返回值说明：**

获得ppm结果

#### e53_iss_get_mq2_alarm_value()

```c
uint16_t e53_iss_get_mq2_alarm_value();
```

**描述说明：**

获取eeprom烟雾浓度报警阈值，大于此阈值的需要报警

**参数说明：**

无

**返回值说明：**

报警阈值

#### e53_iss_beep_status_set()

```c
void e53_iss_beep_status_set(e53_iss_status_e status);
```

**描述说明：**

E53智慧烟雾报警控制模块蜂鸣器状态设置

**参数说明：**

| 名字   | 描述                       |
| :----- | :------------------------- |
| status | 蜂鸣器状态，ON：开；OFF:关 |

**返回值说明：**

无

#### e53_iss_led_status_set()

```c
void e53_iss_led_status_set(e53_iss_status_e status);
```

**描述说明：**

E53智慧烟雾报警控制模块LED状态设置

**参数说明：**

| 名字   | 描述                    |
| :----- | :---------------------- |
| status | LED状态，ON：开；OFF:关 |

**返回值说明：**

无

### 主要代码分析

这部分代码是获取MQ2 ppm值的代码，先调用 `iss_get_voltage()`获取adc转换成电压的值。再根据公式计算ppm值
阻值R与空气中被测气体的浓度C的计算关系式
`log R = mlog C + n (m，n均为常数)`
传感器的电阻计算
`Rs = (Vc/VRL-1) X RL`
Vc为回路电压，VRL是传感器4脚6脚输出电压，RL是负载

```c
float e53_iss_get_mq2_ppm(void)
{
    float voltage, RS, ppm;
  
    voltage = iss_get_voltage();
    RS = (5 - voltage) / voltage * RL;    //计算RS
    ppm = 613.9f * pow(RS / R0, -2.074f); //计算ppm
    return ppm;
}
```

这部分代码是iss示例代码，先调用 `e53_iss_init()`函数初始化E53智慧烟雾报警模块，再调用 `e53_iss_mq2_ppm_calibration()`校准MQ2的ppm。然后循环读取MQ2值判断ppm是否超过预设值，超过预设值蜂鸣器报警LED亮起提升，低于预设值则关闭蜂鸣器报警并关闭LED。

```c
void e53_iss_thread(void *arg)
{
    float ppm = 0;
    e53_iss_init();
    /****传感器校准****/
    usleep(2000000);               // 开机2s后进行校准
    e53_iss_mq2_ppm_calibration(); // 校准传感器
  
    while (1)
    {
        printf("=======================================\r\n");
        printf("*************e53_iss_example***********\r\n");
        printf("=======================================\r\n");
        ppm = e53_iss_get_mq2_ppm();
        printf("ppm:%.3f \n", ppm);
        //判断是否达到报警阈值
        if (ppm > e53_iss_get_mq2_alarm_value())
        {
            e53_iss_led_status_set(ON);
            e53_iss_beep_status_set(ON);
        }
        else
        {
            e53_iss_led_status_set(OFF);
            e53_iss_beep_status_set(OFF);
        }
        usleep(1000000);   // 延时1s
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `c7_e53_intelligent_smoke_sensor` 参与编译。

```r
"./c7_e53_intelligent_smoke_sensor:e53_iss_example",
```

修改 `device/rockchip/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_iss_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -leeprom_example -le53_iss_example,
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```c
=======================================
*************e53_iss_example***********
=======================================
ppm:20.3
```
