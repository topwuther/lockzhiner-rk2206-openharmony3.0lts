# 小凌派-RK2206开发板OpenHarmonyOS开发-智慧农业

## 实验内容

本例程演示如何在小凌派-RK2206开发板上实现智慧农业的应用案例。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![智慧农业模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/e53_ia01_resource_map.jpg)

其中，温湿度传感器SHT30的设备地址为：0x0100010* ；
光线传感器BH1750FVI的设备地址为：0x0100011* ；
EEPROM 24C02的设备地址为：0x1010001* 。

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在智慧农业模块背面查看。

| 引脚名称 | 功能描述 |
| :-- | :------ | 
| LED_CTR | LED灯控制线，高电平有效 |
| MOTOR_CTR | 电机控制线，高电平有效 | 
| I2C_SCL | I2C时钟信号线 | 
| I2C-SDA | I2C数据信号线 | 
| GND | 电源地引脚 | 
| 3V3 | 3.3V电源输入引脚 | 
| GND | 电源地引脚 | 
| 5V | 5V电源输入引脚 |

## 硬件设计

硬件电路如下图所示：

![智慧农业模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/lz_e53_ia01_sch.jpg)

整体电路中包含了E53接口连接器，EEPROM存储器、光线传感器，温湿度传感器，LED灯以及电机控制电路。其中，EEPROM存储器、光线传感器和温湿度传感器均为数字接口芯片，直接使用I2C总线控制，电路简单，本文不做过多的说明。

本文主要介绍马达和LED灯控制原理。LED灯和马达均使用三极管S8050驱动，S8050为NPN管，当LED_CTR为高电平时三极管Q3导通，D1亮；由于马达属于感性线圈，在实际电路工作中对整个电路的干扰比较大，为保证整体系统的稳定性，马达控制增加了一路光耦隔离器，光耦隔离器采用TLP521-1GB，当控制信号MOTOR_CTR为高电平时，光耦导通，进而三极管Q1导通，驱动马达转动。

### 硬件连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![智慧农业模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/e53_ia01_connection_diagram.png)

## 程序设计

### API分析

**头文件：**

/vendor/lockzhiner/rk2206/samples/e53_intelligent_agriculture/include/e53_intelligent_agriculture.h

#### e53_ia_init()

```c
void e53_ia_init();
```

**描述：**

智慧农业模块初始化，包括初始化I2C0、紫光灯GPIO、电机GPIO；初始化BH1750传感器和SHT30传感器。

**参数：**

无

**返回值：**

无

#### e53_ia_read_data()

```c
void e53_ia_read_data(e53_ia_data_t *pData);
```

**描述：**

智慧农业模块读取传感器数据。

**参数：**

|名字|描述|
|:--|:------| 
| pData | 读取的传感器数据，包括亮度、温度和湿度 |

**返回值：**

无

#### light_set()

```c
void light_set(SWITCH_STATUS_ENUM status);
```

**描述：**

智慧农业模块控制紫光灯开关。

**参数：**

|名字|描述|
|:--|:------| 
| status | 紫光灯状态，ON：开；OFF：关 |

**返回值：**

无

#### motor_set_status()

```c
void motor_set_status(SWITCH_STATUS_ENUM status);
```

**描述：**

智慧农业模块控制电机开关。

**参数：**

|名字|描述|
|:--|:------| 
| status | 电机状态：ON：开；OFF：关 |

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

#### init_sht30()

```c
void init_sht30();
```

**描述：**

智慧农业模块初始化SHT30传感器，设置测量周期，通过I2C总线下发测量周期命令0x2236，选择高重复性测量，每秒测量2次。

![SHT30传感器测量命令](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/sht30_meas_cmd.png)

**参数：**

无

**返回值：**

无

#### sht30_check_crc()

```c
uint8_t sht30_check_crc(uint8_t *data, uint8_t nbrOfBytes, uint8_t checksum);
```

**描述：**

SHT30传感器温度、湿度数据校验。

**参数：**

|名字|描述|
|:--|:------| 
| data | 校验数据指针 |
| nbrOfBytes | 校验数据长度 |
| checksum | 校验值 |

**返回值：**

|返回值|描述|
|:--|:------| 
| 0 | 校验成功 |
| 1 | 校验失败 |

#### sht30_calc_temperature()

```c
float sht30_calc_temperature(uint16_t u16sT);
```

**描述：**

SHT30传感器计算温度值，计算公式如下。

![SHT30传感器计算温度公式](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/calc_temp.png)

**参数：**

|名字|描述|
|:--|:------| 
| u16sT | 读取到的温度原始数据 |

**返回值：**

|返回值|描述|
|:--|:------| 
| temperature | 计算出的温度 |

#### sht30_calc_RH()

```c
float sht30_calc_RH(uint16_t u16sRH);
```

**描述：**

SHT30传感器计算湿度值，计算公式如下。

![SHT30传感器计算湿度公式](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/calc_lum.png)

**参数：**

|名字|描述|
|:--|:------| 
| u16sRH | 读取到的湿度原始数据 |

**返回值：**

|返回值|描述|
|:--|:------| 
| humidityRH | 计算出的湿度 |

#### e53_ia_read_data()

```c
void e53_ia_read_data(e53_ia_data_t *pData);
```

**描述：**

智慧农业模块读取BH1750传感器两个字节数据，通过公式(MSB<<8 | LSB)/1.2计算得出亮度值。

智慧农业模块读取SHT30传感器数据，通过I2C总线下发读取数据命令0xe000；I2C读取6字节数据，其中第一字节为温度值高位，第二字节为温度值低位，第三字节为温度校验值，第四字节为湿度值高位，第五字节为湿度值低位，第六字节为湿度校验值。分别计算温度、湿度校验值，校验值与温度、湿度校验值一致，则说明数据正确；通过sht30_calc_temperature计算温度值，通过sht30_calc_RH计算湿度值。

![SHT30传感器读取数据命令](/vendor/lockzhiner/rk2206/docs/figures/e53_ia01/sht30_fetch_data_cmd.png)

**参数：**

|名字|描述|
|:--|:------| 
| pData | 读取的温度、湿度和亮度数据指针 |

**返回值：**

无

### 主要代码分析

在e53_ia_thread函数中，每2s读取一次传感器数据并打印亮度、温度和湿度。当亮度小于20时，打开紫光灯，否则关闭紫光灯；当湿度小于60或者温度大于30时，打开电机，否则关闭电机。

```c
void e53_ia_thread()
{
    e53_ia_data_t data;

    e53_ia_init();

    while (1)
    {
        e53_ia_read_data(&data);

        printf("\nLuminance is %.2f\n", data.luminance);
        printf("\nHumidity is %.2f\n", data.humidity);
        printf("\nTemperature is %.2f\n", data.temperature);

        if (data.luminance < 20)
        {
            light_set(ON);
            printf("light on\n");
        }
        else
        {
            light_set(OFF);
            printf("light off\n");
        }

        if ((data.humidity > 60) || (data.temperature > 30))
        {
            motor_set_status(ON);
            printf("motor on\n");
        }
        else
        {
            motor_set_status(OFF);
            printf("motor off\n");
        }
        LOS_Msleep(2000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `c1_e53_intelligent_agriculture` 参与编译。

```r
"./c1_e53_intelligent_agriculture:e53_ia_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_ia_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -le53_ia_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，串口会打印温度、湿度及光照强度信息；用手遮挡智慧农业模块，紫光灯会自动开启；控制温度或者湿度超标，电机会自动开启。

```r
Luminance is 153.33
Humidity is 37.69
Temperature is 21.30
light on
motor off
Luminance is 726.67
Humidity is 61.02
Temperature is 20.79
light off
motor on
Luminance is 697.50
Humidity is 58.78
Temperature is 20.94
light off
motor off
```

