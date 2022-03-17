# 小凌派-RK2206开发板OpenHarmonyOS开发-智慧井盖

## 实验内容

本例程演示如何在小凌派-RK2206开发板上实现智慧井盖应用案例。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![智慧井盖模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/e53_sc01_resource_map.jpg)

EEPROM 24C02的设备地址为：0x1010001* ；
六轴传感器 MPU6050 的设备地址为：0x1101000*

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在智慧井盖模块背面查看。

| 引脚名称 | 功能描述 |
| :--- | :------- | 
| MPU6050_INT | MPU6050中断输出 |
| Normal_CTR | LED控制线，低电平有效 | 
| Tilt_Warning_CTR | LED控制线，低电平有效 | 
| I2C_SCL  |I2C时钟信号线 | 
| I2C-SDA | I2C数据信号线 | 
| GND | 电源地引脚 | 
| 3V3 |3.3V电源输入引脚 | 
| GND | 电源地引脚 |

## 硬件设计

硬件电路如下图所示：
![智慧井盖模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/lz_e53_sc01_sch.jpg)

模块整体硬件电路如上图所示，电路中包含了E53接口连接器，EEPROM存储器、MPU6050传感器，LED指示灯电路，其中EEPROM存储器、MPU6050传感器为数字接口芯片，直接使用I2C总线控制，电路简单，本文不再过多说明。

### 硬件连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![智慧井盖模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/e53_sc01_connection_diagram.png)

## 程序设计

### API分析

MPU6050寄存器手册
/vendor/lockzhiner/rk2206/docs/datasheet/sensor/MPU-6000 and MPU-6050 Register Map and Descriptions.pdf

**头文件：**

/vendor/lockzhiner/rk2206/samples/e53_smart_city/include/e53_smart_city.h

#### e53_sc_init()

```c
void e53_sc_init();
```

**描述：**

E53智慧井盖模块初始化，包括初始化I2C0、LED1灯GPIO、LED2灯GPIO；初始化MPU6050传感器。

**参数：**

无

**返回值：**

无

#### mpu6050_write_reg()

```c
void mpu6050_write_reg(uint8_t reg, uint8_t data);
```

**描述：**

E53智慧井盖模块MPU6050传感器写寄存器。

**参数：**

|名字|描述|
|:--|:------| 
| reg | 寄存器 |
| data | 写入的寄存器值 |

**返回值：**

无

#### mpu6050_read_data()

```c
void mpu6050_read_data(uint8_t reg, unsigned char *buf, uint8_t length);
```

**描述：**

E53智慧井盖模块MPU6050传感器读寄存器。

**参数：**

|名字|描述|
|:--|:------| 
| reg | 寄存器 |
| buf | 读寄存器缓冲区指针 |
| length | 读寄存器长度 |

**返回值：**

无

#### MPU6050_Read_Buffer()

```c
uint8_t MPU6050_Read_Buffer(uint8_t reg, uint8_t *p_buffer, uint16_t length);
```

**描述：**

E53智慧井盖模块读取多个MPU6050寄存器值。

**参数：**

|名字|描述|
|:--|:------| 
| reg | 寄存器 |
| p_buffer | 读缓冲区指针 |
| length | 读取寄存器长度 |

**返回值：**

|返回值|描述|
|:--|:------| 
| 0 | 成功 |
| 非0值 | 失败，返回错误值 |

#### mpu6050_read_acc()

```c
void mpu6050_read_acc(short *acc_data);
```

**描述：**

E53智慧井盖模块读取MPU6050的角速度数据。

MPU6050_ACC_OUT角速度数据寄存器，只读，包括X轴、Y轴和Z轴。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_ACC_OUT.png)

**参数：**

|名字|描述|
|:--|:------| 
| acc_data | 读取的角速度数据指针，包括X轴、Y轴、Z轴数据 |

**返回值：**

无

#### mpu6050_read_id()

```c
uint8_t mpu6050_read_id();
```

**描述：**

E53智慧井盖模块读取MPU6050的ID，读取值为0x68返回成功；否则，返回失败。

MPU6050_RA_WHO_AM_I寄存器，只读，默认值0x68。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_WHO_AM_I.png)

**参数：**

无

**返回值：**

|返回值|描述|
|:--|:------| 
| 0 | 失败 |
| 1 | 成功 |

#### mpu6050_init()

```c
void mpu6050_init();
```

**描述：**

E53智慧井盖模块初始化MPU6050传感器。

MPU6050_RA_PWR_MGMT_1电源管理寄存器，DEVICE_RESET复位唤醒传感器。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_PWR_MGMT_1.png)

MPU6050_RA_INT_ENABLE中断使能寄存器，中断控制。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_INT_ENABLE.png)

MPU6050_RA_USER_CTRL用户控制寄存器，关闭I2C主模式。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_USER_CTRL.png)

MPU6050_RA_FIFO_EN FIFO使能寄存器，关闭FIFO。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_FIFO_EN.png)

MPU6050_RA_INT_PIN_CFG INT管脚/旁路控制寄存器。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_INT_PIN_CFG.png)

MPU6050_RA_CONFIG配置寄存器，配置为外部引脚采样，DLPF数字低通滤波器。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_CONFIG.png)

MPU6050_RA_ACCEL_CONFIG加速配置寄存器，配置加速度传感器量程和高通滤波器。

![寄存器](/vendor/lockzhiner/rk2206/docs/figures/e53_sc01/MPU6050_RA_ACCEL_CONFIG.png)

**参数：**

无

**返回值：**

无

#### e53_sc_read_data()

```c
void e53_sc_read_data(e53_sc_data_t *p_data);
```

**描述：**

E53智慧井盖模块读取传感器数据。

**参数：**

|名字|描述|
|:--|:------| 
| data | 读取的传感器数据指针，包括X轴、Y轴、Z轴数据 |

**返回值：**

无

#### led_d1_set()

```c
void led_d1_set(SWITCH_STATUS_ENUM status);
```

**描述：**

E53智慧井盖模块控制LED1告警状态灯开关。

**参数：**

|名字|描述|
|:--|:------| 
| status | LED灯状态，ON：开；OFF：关 |

**返回值：**

无

#### led_d2_set()

```c
void led_d2_set(SWITCH_STATUS_ENUM status);
```

**描述：**

E53智慧井盖模块控制LED2正常状态灯开关。

**参数：**

|名字|描述|
|:--|:------| 
| status | LED灯状态，ON：开；OFF：关 |

**返回值：**

无

### 主要代码分析

在e53_sc_thread函数中，每2s读取一次传感器数据并打印X轴、Y轴和Z轴数据。首次读取传感器数据时，保存X轴、Y轴和Z轴数据，用于与之后的传感器数据进行对比。当X轴、Y轴和Z轴与第一次读取保存的X轴、Y轴和Z轴数据差值大于100时，认为井盖倾斜，点亮LED2告警状态灯，关闭LED1正常状态灯；否则认为井盖没有倾斜，关闭LED2告警状态灯，打开LED1正常状态灯。

```c
void e53_sc_thread()
{
    e53_sc_data_t data;
    int x = 0, y = 0, z = 0;

    e53_sc_init();
    led_d1_set(OFF);
    led_d2_set(OFF);

    while (1)
    {
        e53_sc_read_data(&data);
        printf("x is %d\n", (int)data.accel[0]);
        printf("y is %d\n", (int)data.accel[1]);
        printf("z is %d\n", (int)data.accel[2]);
        printf("init x:%d y:%d z:%d\n", x, y, z);

        if (x == 0 && y == 0 && z == 0)
        {
            x = (int)data.accel[0];
            y = (int)data.accel[1];
            z = (int)data.accel[2];
        }
        else
        {
            if ((x + DELTA) < data.accel[0] || (x - DELTA) > data.accel[0] ||
                (y + DELTA) < data.accel[1] || (y - DELTA) > data.accel[1] ||
                (z + DELTA) < data.accel[2] || (z - DELTA) > data.accel[2])
            {
                /*倾斜告警*/
                led_d1_set(OFF);
                led_d2_set(ON);
                data.tilt_status = 1;
                printf("tilt warning \nLED1 OFF LED2 On\n");
            }
            else
            {
                led_d1_set(ON);
                led_d2_set(OFF);
                data.tilt_status = 0;
                printf("normal \nLED1 ON LED2 OFF\n");
            }
        }
        LOS_Msleep(2000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `c2_e53_smart_city` 参与编译。

```r
"./c2_e53_smart_city:e53_sc_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_sc_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -le53_sc_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，串口会打印X、Y、Z轴加速度信息、倾斜告警状态和指示灯状态；倾斜智慧井盖模块开发板，智慧井盖模块上的倾斜状态灯LED2会被点亮。

```r
x is 149
y is 21
z is 1822
init x:144 y:24 z:1819
normal 
LED1 ON LED2 OFF

x is 154
y is 749
z is 1684
init x:144 y:24 z:1819
tilt warning 
LED1 OFF LED2 On
```

