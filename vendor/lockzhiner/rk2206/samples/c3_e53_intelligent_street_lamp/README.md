# 小凌派-RK2206开发板OpenHarmonyOS开发-智慧路灯

## 实验内容

本例程演示如何在小凌派-RK2206开发板上实现智慧路灯的应用案例。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![智慧路灯模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_is01/e53_is01_resource_map.jpg)

EEPROM 24C02的设备地址为：0x1010001* ；

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在智慧路灯模块背面查看。

| 引脚名称 | 功能描述 |
| :--- | :------- | 
| PWM_LED | LED控制线，高电平有效 |
| I2C_SCL  |I2C时钟信号线 | 
| I2C-SDA | I2C数据信号线 | 
| GND | 电源地引脚 | 
| 3V3 |3.3V电源输入引脚 | 
| GND | 电源地引脚 | 
| 5V | 5V电源输入引脚 |

## 硬件设计

硬件电路如下图所示：
![智能路灯模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_is01/lz_e53_is01_sch.jpg)
模块整体硬件电路如上图所示，电路中包含了E53接口连接器，EEPROM存储器、光线传感器， 大功率LED灯驱动电路，其中EEPROM存储器、光线传感器为数字接口芯片，直接使用I2C总线控制，电路简单，本文不再过多说明。

本文主要介绍大功率LED灯驱动原理，这边采用PT4211E23E芯片作为LED灯的驱动芯片，其是一款连续导通型的电感降压转换器，可以用于驱动单个或者多个串联的LED灯，输出电流高达350mA，输出电流可通过电阻R6进行调整，也可通过DIM引脚调整输出平均电流 从而达到调整LED灯的亮度。

根据案例场景应用，我们这边仅是模拟IOT物联网中的智能路灯案例， LED灯无需太亮，过亮的灯光可能对眼睛造成一定程度的影响，影响开发者进行功能调试，故这边通过电阻R6将LED灯的驱动电流限制的很小，同时有个问题，驱动器输出的是5V的驱动电压，但LED灯是3.3V标准的大功率LED灯，故我们在设计时也不能将驱动器输出功率设置的太大，否则容易影响LED灯的寿命。

### 硬件连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![智慧路灯模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_is01/e53_is01_connection_diagram.png)

## 程序设计

### API分析

**头文件：**

/vendor/lockzhiner/rk2206/samples/e53_intelligent_street_lamp/include/e53_intelligent_street_lamp.h

#### e53_isl_init()

```c
void e53_isl_init();
```

**描述：**

智慧路灯模块初始化，包括初始化I2C0、路灯控制GPIO；初始化BH1750传感器。

**参数：**

无

**返回值：**

无

#### e53_isl_read_data()

```c
float e53_isl_read_data();
```

**描述：**

智慧路灯模块读取传感器数据，得到亮度值。

**参数：**

无

**返回值：**

传感器返回的亮度值。

#### isl_light_set_status()

```c
void isl_light_set_status(SWITCH_STATUS_ENUM status);
```

**描述：**

智慧路灯模块控制路灯开关。

**参数：**

|名字|描述|
|:--|:------| 
| status | 路灯状态，ON：开；OFF：关 |

**返回值：**

无

### BH1750传感器指令集

![BH1750传感器指令集](/vendor/lockzhiner/rk2206/docs/figures/e53_is01/bh1750_cmd.png)

#### init_bh1750()

```c
void init_bh1750();
```

**描述：**

智慧路灯模块初始化bh1750传感器，通过I2C总线下发通电指令集。

**参数：**

无

**返回值：**

无

#### start_bh1750()

```c
void start_bh1750();
```

**描述：**

智慧路灯模块使能bh1750传感器开启测量，通过I2C总线下发连续H分辨率模式指令集。

**参数：**

无

**返回值：**

无

### 主要代码分析

在e53_isl_thread函数中，每2s读取一次传感器数据并打印亮度数据。当亮度小于20时，打开路灯，否则关闭路灯。

```c
void e53_isl_thread()
{
    float lum = 0;

    e53_isl_init();

    while (1)
    {
        lum = e53_isl_read_data();

        printf("luminance value is %.2f\n", lum);

        if (lum < 20)
        {
            isl_light_set_status(ON);
            printf("light on\n");
        }
        else
        {
            isl_light_set_status(OFF);
            printf("light off\n");
        }

        LOS_Msleep(2000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `c3_e53_intelligent_street_lamp` 参与编译。

```r
"./c3_e53_intelligent_street_lamp:e53_isl_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_isl_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -le53_isl_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，串口会打印光照强度信息；用手遮挡传感器，使亮度值小于20时，路灯亮起；移开遮挡，亮度值大于20时，路灯关闭。

```
luminance value is 45.83
light off
luminance value is 4.17
light on
```

