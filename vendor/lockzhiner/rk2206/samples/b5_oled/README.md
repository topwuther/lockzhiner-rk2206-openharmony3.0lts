# 小凌派-RK2206开发板基础外设开发——I2C控制OLED液晶屏显示

本示例将演示如何在小凌派-RK2206开发板上使用I2C控制OLED液晶屏显示

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)



## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在oled模块正面面查看。

| 引脚名称 | 功能描述 |
| :--- | :------- | 
| SCL | I2C时钟信号线 |
| SDA | I2C数据信号线 | 
| GND | 电源地引脚 | 
| VCC  | 电源输入引脚 | 


### 硬件连接


安装图如下所示：
![0.96寸OLED模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/0.96inch_oled/0.96inch_oled_connection_diagram_1.png)

![0.96寸OLED模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/0.96inch_oled/0.96inch_oled_connection_diagram_2.jpg)


## 程序设计

### API分析

#### oled_init()

```c
unsigned int oled_init();
```

**描述：**

初始化oled设备。

**参数：**

无

**返回值：**

返回0为成功，反之为失败

#### oled_deinit()

```c
unsigned int oled_deinit();
```

**描述：**

释放oled设备。

**参数：**

无

**返回值：**

返回0为成功，反之为失败

#### oled_display_on()

```c
void oled_display_on(void);
```

**描述：**

oled显示开启。

**参数：**

无

**返回值：**

无

#### oled_display_off()

```c
void oled_display_off(void);
```

**描述：**

oled显示关闭。

**参数：**

无

**返回值：**

无

#### oled_clear()

```c
void oled_clear(void);
```

**描述：**

oled清空屏幕。

**参数：**

无

**返回值：**

无

#### oled_show_char()

```c
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t chr_size);
```

**描述：**

oled显示单个英文字符。

**参数：**

| 名字     | 描述                          |
| :------- | :---------------------------- |
| x        | 字符的X轴坐标                 |
| y        | 字符的Y轴坐标                 |
| chr      | 字符                          |
| chr_size | 字符的字体，包括12/16两种字体 |

**返回值：**

无

#### oled_show_num()

```c
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
```

**描述：**

oled显示整数。

**参数：**

| 名字 | 描述                            |
| :--- | :------------------------------ |
| x    | 整数数的X轴坐标                 |
| y    | 整数数的Y轴坐标                 |
| num  | 整数                            |
| len  | 整数的位数                      |
| size | 整数字的字体，包括12/16两种字体 |

**返回值：**

无

#### oled_show_string()

```c
void oled_show_string(uint8_t x, uint8_t y, uint8_t *p, uint8_t chr_size);
```

**描述：**

oled显示英文字符串。

**参数：**

| 名字     | 描述                          |
| :------- | :---------------------------- |
| x        | 字符串的X轴坐标               |
| y        | 字符串的Y轴坐标               |
| p        | 字符串                        |
| chr_size | 字符的字体，包括12/16两种字体 |

**返回值：**

无

#### oled_draw_bmp()

```c
void oled_draw_bmp(unsigned char x0, 
                unsigned char y0, 
                unsigned char x1, 
                unsigned char y1, 
                unsigned char bmp[]);
```

**描述：**

oled显示图片。

**参数：**

| 名字 | 描述                             |
| :--- | :------------------------------- |
| x0   | 图片的起始点X轴坐标，取值为0~127 |
| y0   | 图片的起始点Y轴坐标，取值为0~63  |
| x1   | 图片的结束点X轴坐标，取值为0~127 |
| y1   | 图片的结束点Y轴坐标，取值为0~63  |
| bmp  | 图片                             |

**返回值：**

无

### OLED器件

**OLED显示屏**

OLED显示屏是0.96寸OLED显示屏，型号为SSD1306，像素为128 * 64。

OLED液晶屏采用i2c通信方式，根据芯片手册说明，从设备地址为 `0x3C`。

### 主要代码分析

这部分代码为i2c初始化的代码。首先用 `I2cIoInit()` 函数将GPIO0_PC1复用为I2C1_SDA_M1，GPIO0_PC2复用为I2C1_SCL_M1。最后调用 `LzI2cInit()`函数初始化I2C1端口。

```c
if (I2cIoInit(m_i2cBus) != LZ_HARDWARE_SUCCESS)
{
    printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
    return __FILE__;
}
if (LzI2cInit(OLED_I2C_BUS, m_i2c_freq) != LZ_HARDWARE_SUCCESS)
{
    printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
    return __FILE__;
}
```

具体i2c写操作如下：

```c
static inline void Write_IIC_Command(unsigned char IIC_Command)
{
    unsigned char buffer[2];
    unsigned int ret;

    /* 填充数据，第一个字节是通知OLED芯片，下一个字节是命令 */
    buffer[0] = 0x00;
    buffer[1] = IIC_Command;
    ret = LzI2cWrite(OLED_I2C_BUS, OLED_I2C_ADDRESS, buffer, 2);
    if (ret != 0)
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
    }
}

static inline void Write_IIC_Data(unsigned char IIC_Data)
{
    unsigned char buffer[2];
    unsigned int ret;

    /* 填充数据，第一个字节是通知OLED芯片，下一个字节是数据 */
    buffer[0] = 0x40;
    buffer[1] = IIC_Data;
    ret = LzI2cWrite(OLED_I2C_BUS, OLED_I2C_ADDRESS, buffer, 2);
    if (ret != 0)
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
    }
}
```

### 配置SSD1306启动

```c
LOS_Msleep(200);

Oled_WR_Byte(0xAE, OLED_CMD); //--display off
Oled_WR_Byte(0x00, OLED_CMD); //---set low column address
Oled_WR_Byte(0x10, OLED_CMD); //---set high column address
Oled_WR_Byte(0x40, OLED_CMD); //--set start line address
Oled_WR_Byte(0xB0, OLED_CMD); //--set page address
Oled_WR_Byte(0x81, OLED_CMD); // contract control
Oled_WR_Byte(0xFF, OLED_CMD); //--128
Oled_WR_Byte(0xA1, OLED_CMD); //set segment remap
Oled_WR_Byte(0xA6, OLED_CMD); //--normal / reverse
Oled_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
Oled_WR_Byte(0x3F, OLED_CMD); //--1/32 duty
Oled_WR_Byte(0xC8, OLED_CMD); //Com scan direction
Oled_WR_Byte(0xD3, OLED_CMD); //-set display offset
Oled_WR_Byte(0x00, OLED_CMD); //

Oled_WR_Byte(0xD5, OLED_CMD); //set osc division
Oled_WR_Byte(0x80, OLED_CMD); //

Oled_WR_Byte(0xD8, OLED_CMD); //set area color mode off
Oled_WR_Byte(0x05, OLED_CMD); //

Oled_WR_Byte(0xD9, OLED_CMD); //Set Pre-Charge Period
Oled_WR_Byte(0xF1, OLED_CMD); //

Oled_WR_Byte(0xDA, OLED_CMD); //set com pin configuartion
Oled_WR_Byte(0x12, OLED_CMD); //

Oled_WR_Byte(0xDB, OLED_CMD); //set Vcomh
Oled_WR_Byte(0x30, OLED_CMD); //

Oled_WR_Byte(0x8D, OLED_CMD); //set charge pump enable
Oled_WR_Byte(0x14, OLED_CMD); //

Oled_WR_Byte(0xAF, OLED_CMD); //--turn on oled panel
```

这部分代码将SSD1306的启动配置。

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `oled_example` 参与编译。

```r
"./b5_oled:oled_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-loled_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -loled_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，并请使用带有LCD屏幕显示如下：

```c
========= Oled Process =============

```

OLED显示结果如下所示：

![小凌派-RK2206开发板OLED显示结果](/vendor/lockzhiner/rk2206/docs/figures/OLED_SSD1306/OLED_SSD1306_显示结果.png)
