# 小凌派-RK2206开发板OpenHarmonyOS开发-人体感应

## 实验内容

本例程演示如何在小凌派-RK2206开发板上实现人体感应的应用案例。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![人体感应模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_bi01/e53_bi01_resource_map.jpg)

注：EEPROM 24C02的设备地址为：0x1010001*  ;

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在智慧农业模块背面查看。

| 引脚名称 | 功能描述 |
| :-- | :------ | 
| LED_ALARM | 人体感应输出，高电平有效 |
| BUZZER | 蜂鸣器控制线，推荐3KHz的方波信号 | 
| I2C_SCL | I2C时钟信号线 | 
| I2C_SDA| I2C数据信号线 | 
| GND | 电源地引脚 | 
| 3V3 | 3.3V电源输入引脚 | 
| GND | 电源地引脚 |

## 硬件设计

硬件电路如下图所示：
![人体感应模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_bi01/lz_e53_bi01_sch.jpg)

模块整体硬件电路如上图所示，电路中包含了E53接口连接器，EEPROM存储器、热释电传感器，LED指示灯电路，蜂鸣器电路。

这边简单介绍下热释电红外传感器的原理，热释电效应同压电效应类似，是指由于温度的变化而引起晶体表面荷电的现象。热释电传感器是对温度敏感的传感器。它由陶瓷氧化物或压电晶体元件组成，在元件两个表面做成电极，在传感器监测范围内温度有ΔT的变化时，热释电效应会在两个电极上会产生电荷ΔQ，即在两电极之间产生微弱的电压ΔV。由于它的输出 阻抗极高，在传感器中有一个场效应管进行阻抗变换。热释电效应所产生的电荷ΔQ会被空气中的离子所结合而消失，即当环境温度稳定不变时，ΔT=0，则传感 器无输出。当人体进入检测区，因人体温度与环境温度有差别，产生ΔT，则有ΔT输出；如果人体进入检测区后不动，则温度没有变化，传感器也没有输出了。所以这种传感器检测人体或者动物的活动传感。 由实验证明，传感器不加光学透镜(也称菲尼尔透镜)，其检测距离小于2m，而加上光学透镜后，其检测距离可大于7m。

当传感器受到红外辐射而温度升高时，表面电荷将减少，相当于释放了一部分电荷。将释放的电荷经放大器可以转化为电压输出通过检测这个电压可以转换为数字信号输出使用。D203B带有内置的光学滤波器，可以将检测到的辐射设置在人体辐射的波长范围内。辐射的变化经过传感器内部的放大后产生可以从外部测量到的模拟输出脉冲，但是该信号与输入电压VCC相比仍非常小，因此还需要通过外部电路将信号放大到可用的范围。

BISS0001是一款具有较高性能的传感信号处理集成芯片，其专门用来对热释电红外传感器输出信号进行处理，热释电红外传感器输出的信号比较微软，需要经过相应的滤波放大处理，下面介绍下BISS0001原理。

BISS0001芯片内部电路框图如下所示：
![BISS0001芯片内部电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_bi01/biss0001_diagram.png)

OP1为输入信号的第一级放大，然后经过C10耦合给运算放大器OP2进行二级放大，经过由电压比较器COP1和COP2构成的双向鉴幅器处理之后，检出有效触发信号Vs去启动延迟定时器，输出信号Vo经过晶体管Q1驱动LED灯。R8为光敏电阻，用来检测环境照度。当作为照明控制时候，若环境比较明亮，光敏电阻阻值降低，使VC脚的输入保持为低电平，从而封锁出发信号Vs。电路中1脚直接接高电平，芯片则处于可重复触发模式。模块默认不焊接光敏电阻。

时间计算：输出延时时间Tx由外部的R7和C6的大小调整；触发封锁时间Ti由外部的R6和C7的大小调整，由于模块的工作环境以及器件误差，理论时间与实际时间，有一定的差异属正常现象。关于延时时间与封锁时间定义，开发者可自行查阅手册。

三极管Q2为NPN管，基极为高电平时，三极管才能够导通，蜂鸣器需PWM波驱动，人耳可识别的频率范围为20Hz-20KHz，故PWM频率需在该范围内，我们默认使用3KHz的PWM波驱动。同理三极管Q1，D_OUT需为高电平时，三极管才能导通并驱动LED灯亮。

### 模块连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![人体感应模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_bi01/e53_bi01_connection_diagram.png)

## 程序设计

### API分析

**头文件：**

/vendor/lockzhiner/rk2206/samples/e53_body_induction/include/e53_body_induction.h

#### e53_bi_init()

```c
void e53_bi_init();
```

**描述：**

E53人体感应模块初始化，包括初始化GPIO、蜂鸣器PWM。

**参数：**

无

**返回值：**

无

#### buzzer_set_status()

```c
void buzzer_set_status(SWITCH_STATUS_ENUM status);
```

**描述：**

E53人体感应模块控制蜂鸣器开关。

**参数：**

|名字|描述|
|:--|:------| 
| status | 蜂鸣器状态，ON：开；OFF：关 |

**返回值：**

无

### 主要代码分析

在void e53_bi_thread函数中，初始化GPIO作为输入，初始化PWM作为蜂鸣器控制源。采用轮询的方式读取GPIO电平值，当读取的GPIO值为高电平时，认为有物体靠近，开启蜂鸣器警告；当读取的GPIO值为低电平时，则为没有物体靠近，关闭蜂鸣器警告。

```c
void e53_bi_thread()
{
    unsigned int ret;
    LzGpioValue val = LZGPIO_LEVEL_LOW, val_last = LZGPIO_LEVEL_LOW;

    e53_bi_init();

    while (1)
    {
        ret = LzGpioGetVal(GPIO0_PA5, &val);
        if (ret != LZ_HARDWARE_SUCCESS)
        {
            printf("get gpio value failed ret:%d\n", ret);
        }
        if (val_last != val)
        {
            if (val == LZGPIO_LEVEL_HIGH)
            {
                buzzer_set_status(ON);
                printf("buzzer on\n");
                LOS_Msleep(1000);
                buzzer_set_status(OFF);
                printf("buzzer off\n");
            }
            val_last = val;
        }
        LOS_Msleep(1000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `c5_e53_body_induction` 参与编译。

```r
"./c5_e53_body_induction:e53_bi_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_bi_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -le53_bi_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志；当人靠近传感器时，指示灯亮起，串口打印buzzer on，蜂鸣器响1s，串口打印buzzer off。

```
buzzer on

buzzer off
```

