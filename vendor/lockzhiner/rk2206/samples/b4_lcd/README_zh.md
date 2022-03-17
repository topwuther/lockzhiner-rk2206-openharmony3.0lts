# 小凌派-RK2206开发板基础外设开发——LCD液晶屏显示

本示例将演示如何在小凌派-RK2206开发板上LCD液晶屏显示

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![2.4寸液晶模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/2.4inch_lcd/2.4inch_lcd_resource_map.jpg)

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在2.4寸液晶模块背面查看。

| 引脚名称 | 功能描述 |
| :--- | :------- | 
| D/C | 指令/数据选择端，L:指令，H:数据 |
| RESET | 复位信号线，低电平有效 | 
| SPI_MOSI | SPI数据输入信号线 | 
| SPI_CLK  | SPI时钟信号线 | 
| SPI_CS | SPI片选信号线，低电平有效 | 
| GND |电源地引脚 | 
| 5V | 5V电源输入引脚 |

## 硬件设计

硬件电路如下图所示：
![2.4寸液晶模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/2.4inch_lcd/lz_hm_2.4inch_lcd_sch.jpg)

### 硬件连接

安装图如下所示：
![2.4寸液晶模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/2.4inch_lcd/2.4inch_lcd_connection_diagram.jpg)

## 程序设计

### API分析

**头文件**

```r
//vendor/lockzhiner/rk2206/samples/b4_lcd/include/lcd.h
```

#### lcd_init()

```c
unsigned int lcd_init();
```

**描述：**

lcd液晶屏设备初始化。

**参数：**

无

**返回值：**

返回0为成功，反之为失败

#### lcd_deinit()

```c
unsigned int lcd_deinit();
```

**描述：**

lcd液晶屏设备注销。

**参数：**

无

**返回值：**

返回0为成功，反之为失败

#### lcd_fill()

```c
void lcd_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);
```

**描述：**

lcd液晶屏指定区域填充颜色。

**参数：**

| 名字  | 描述                  |
| :---- | :-------------------- |
| xsta  | 指定区域的起始点X坐标 |
| ysta  | 指定区域的起始点Y坐标 |
| xend  | 指定区域的结束点X坐标 |
| yend  | 指定区域的结束点Y坐标 |
| color | 指定区域的颜色        |

**返回值：**

无

#### lcd_draw_point()

```c
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
```

**描述：**

lcd液晶屏指定位置画一个点。

**参数：**

| 名字  | 描述          |
| :---- | :------------ |
| x     | 指定点的X坐标 |
| y     | 指定点的Y坐标 |
| color | 指定点的颜色  |

**返回值：**

无

#### lcd_draw_line()

```c
void lcd_draw_line(uint16_t x1, uint16_t y1, 
                uint16_t x2, uint16_t y2, 
                uint16_t color);
```

**描述：**

lcd液晶屏指定位置画一条线。

**参数：**

| 名字  | 描述                |
| :---- | :------------------ |
| x1    | 指定线的起始点X坐标 |
| y1    | 指定线的起始点Y坐标 |
| x2    | 指定线的结束点X坐标 |
| y2    | 指定线的结束点Y坐标 |
| color | 指定线的颜色        |

**返回值：**

无

#### lcd_draw_rectangle()

```c
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, 
                uint16_t x2, uint16_t y2, 
                uint16_t color);
```

**描述：**

lcd液晶屏指定位置画矩形。

**参数：**

| 名字  | 描述                  |
| :---- | :-------------------- |
| x1    | 指定矩形的起始点X坐标 |
| y1    | 指定矩形的起始点Y坐标 |
| x2    | 指定矩形的结束点X坐标 |
| y2    | 指定矩形的结束点Y坐标 |
| color | 指定矩形的颜色        |

**返回值：**

无

#### lcd_draw_circle()

```c
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
```

**描述：**

lcd液晶屏指定位置画圆。

**参数：**

| 名字  | 描述                |
| :---- | :------------------ |
| x0    | 指定圆的中心点X坐标 |
| y0    | 指定圆的中心点Y坐标 |
| r     | 指定圆的半径        |
| color | 指定圆的颜色        |

**返回值：**

无

#### lcd_show_chinese()

```c
void lcd_show_chinese(uint16_t x, 
                    uint16_t y, 
                    uint8_t *s, 
                    uint16_t fc, 
                    uint16_t bc, 
                    uint8_t sizey, 
                    uint8_t mode);
```

**描述：**

lcd液晶屏显示汉字串。

**参数：**

| 名字  | 描述                          |
| :---- | :---------------------------- |
| x     | 指定汉字串的起始位置X坐标     |
| y     | 指定汉字串的起始位置X坐标     |
| s     | 指定汉字串（该汉字串为utf-8） |
| fc    | 字的颜色                      |
| bc    | 字的背景色                    |
| sizey | 字号，可选：12、16、24、32    |
| mode  | 0为非叠加模式；1为叠加模式    |

**返回值：**

无

#### lcd_show_char()

```c
void lcd_show_char(uint16_t x, 
                    uint16_t y, 
                    uint8_t *s, 
                    uint16_t fc, 
                    uint16_t bc, 
                    uint8_t sizey, 
                    uint8_t mode);
```

**描述：**

lcd液晶屏显示一个字符。

**参数：**

| 名字  | 描述                       |
| :---- | :------------------------- |
| x     | 指定汉字串的起始位置X坐标  |
| y     | 指定汉字串的起始位置X坐标  |
| s     | 指定一个ASCII字符          |
| fc    | 字的颜色                   |
| bc    | 字的背景色                 |
| sizey | 字号，可选：12、16、24、32 |
| mode  | 0为非叠加模式；1为叠加模式 |

**返回值：**

无

#### lcd_show_string()

```c
void lcd_show_string(uint16_t x, 
                    uint16_t y, 
                    const uint8_t *p, 
                    uint16_t fc, 
                    uint16_t bc, uint8_t sizey, uint8_t mode);
```

**描述：**

lcd液晶屏显示字符串。

**参数：**

| 名字  | 描述                       |
| :---- | :------------------------- |
| x     | 指定汉字串的起始位置X坐标  |
| y     | 指定汉字串的起始位置X坐标  |
| p     | 指定一个英文字符串         |
| fc    | 字的颜色                   |
| bc    | 字的背景色                 |
| sizey | 字号，可选：12、16、24、32 |
| mode  | 0为非叠加模式；1为叠加模式 |

**返回值：**

无

#### lcd_show_int_num()

```c
void lcd_show_int_num(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);
```

**描述：**

lcd液晶屏显示一个整数。

**参数：**

| 名字  | 描述                       |
| :---- | :------------------------- |
| x     | 指定汉字串的起始位置X坐标  |
| y     | 指定汉字串的起始位置X坐标  |
| p     | 指定一个英文字符串         |
| fc    | 字的颜色                   |
| bc    | 字的背景色                 |
| sizey | 字号，可选：12、16、24、32 |
| mode  | 0为非叠加模式；1为叠加模式 |

**返回值：**

无

#### lcd_show_float_num1()

```c
void lcd_show_float_num1(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);
```

**描述：**

lcd液晶屏显示两位小数变量。

**参数：**

| 名字  | 描述                        |
| :---- | :-------------------------- |
| x     | 指定浮点变量的起始位置X坐标 |
| y     | 指定浮点变量的起始位置X坐标 |
| num   | 指定浮点变量                |
| fc    | 字的颜色                    |
| bc    | 字的背景色                  |
| sizey | 字号，可选：12、16、24、32  |

**返回值：**

无

#### lcd_show_picture()

```c
void lcd_show_picture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t *pic);
```

**描述：**

lcd液晶屏显示图片。

**参数：**

| 名字   | 描述                      |
| :----- | :------------------------ |
| x      | 指定浮图片的起始位置X坐标 |
| y      | 指定浮图片的起始位置X坐标 |
| length | 指定图片的长度            |
| width  | 指定图片的宽度            |
| pic    | 指定图片的内容            |

**返回值：**

无

### LCD液晶屏

LCD型号为ST7789V，采用SPI通信方式，数据传输协议如下：

`4-Line Serial Interface => 16-bit/pixel(RGB 5-6-5-bit input)，65K-Color，3Ah="05h"`

数据传输时序图如下：

![](/vendor/lockzhiner/rk2206/docs/figures/LCD_ST7789V/ST7789V_数据传输模式.png "LCD数据传输时序图")

LCD使用的是SPI协议，SPI_CS与GPIO0_PC0相连接，SPI_CLK与GPIO0_PC1相连接，SPI_MOSI与GPIO0_PC2相连接，RES与GPIO0_PC3相连接，DC与GPIO0_PC6相连接。

### 初始化代码分析

本程序可使用SPI或GPIO模拟SPI与LCD进行通信。以下我们主要对SPI与LCD进行通信进行代码分析。
这部分代码为SPI初始化的代码。首先用 `SpiIoInit()` 函数将GPIO0_PC0复用为SPI0_CS0n_M1，GPIO0_PC1复用为SPI0_CLK_M1，GPIO0_PC2复用为SPI0_MOSI_M1。最后调用 `LzI2cInit()`函数初始化SPI0端口。

```c
if (SpiIoInit(m_spiBus) != LZ_HARDWARE_SUCCESS) {
    printf("%s, %d: SpiIoInit failed!\n", __FILE__, __LINE__);
    return __LINE__;
}
if (LzSpiInit(LCD_SPI_BUS, m_spiConf) != LZ_HARDWARE_SUCCESS) {
    printf("%s, %d: LzSpiInit failed!\n", __FILE__, __LINE__);
    return __LINE__;
}
```

这部分代码为GPIO初始化的代码。首先用 `LzGpioInit()`函数将GPIO0_PC3初始化为GPIO引脚，然后用 `LzGpioSetDir()`将引脚设置为输出模式，最后调用 `LzGpioSetVal()`输出低电平。

```c
/* 初始化GPIO0_C3 */
LzGpioInit(LCD_PIN_RES);
LzGpioSetDir(LCD_PIN_RES, LZGPIO_DIR_OUT);
LzGpioSetVal(LCD_PIN_RES, LZGPIO_LEVEL_HIGH);

/* 初始化GPIO0_C6 */
LzGpioInit(LCD_PIN_DC);
LzGpioSetDir(LCD_PIN_DC, LZGPIO_DIR_OUT);
LzGpioSetVal(LCD_PIN_DC, LZGPIO_LEVEL_LOW);
```

### 通过SPI往LCD屏幕写数据操作

这部分的代码是向LCD写入数据，具体如下：

```c
LzSpiWrite(LCD_SPI_BUS, 0, &dat, 1);
```

### 配置ST7789V启动

```c
/* 重启lcd */
LCD_RES_Clr();
LOS_Msleep(100);
LCD_RES_Set();
LOS_Msleep(100);
LOS_Msleep(500);
lcd_wr_reg(0x11);
/* 等待LCD 100ms */
LOS_Msleep(100);
/* 启动LCD配置，设置显示和颜色配置 */
lcd_wr_reg(0X36);
if (USE_HORIZONTAL == 0)
{
    lcd_wr_data8(0x00);
}
else if (USE_HORIZONTAL == 1)
{
    lcd_wr_data8(0xC0);
}
else if (USE_HORIZONTAL == 2)
{
    lcd_wr_data8(0x70);
}
else
{
    lcd_wr_data8(0xA0);
}
lcd_wr_reg(0X3A);
lcd_wr_data8(0X05);
/* ST7789S帧刷屏率设置 */
lcd_wr_reg(0xb2);
lcd_wr_data8(0x0c);
lcd_wr_data8(0x0c);
lcd_wr_data8(0x00);
lcd_wr_data8(0x33);
lcd_wr_data8(0x33);
lcd_wr_reg(0xb7);
lcd_wr_data8(0x35);
/* ST7789S电源设置 */
lcd_wr_reg(0xbb);
lcd_wr_data8(0x35);
lcd_wr_reg(0xc0);
lcd_wr_data8(0x2c);
lcd_wr_reg(0xc2);
lcd_wr_data8(0x01);
lcd_wr_reg(0xc3);
lcd_wr_data8(0x13);
lcd_wr_reg(0xc4);
lcd_wr_data8(0x20);
lcd_wr_reg(0xc6);
lcd_wr_data8(0x0f);
lcd_wr_reg(0xca);
lcd_wr_data8(0x0f);
lcd_wr_reg(0xc8);
lcd_wr_data8(0x08);
lcd_wr_reg(0x55);
lcd_wr_data8(0x90);
lcd_wr_reg(0xd0);
lcd_wr_data8(0xa4);
lcd_wr_data8(0xa1);
/* ST7789S gamma设置 */
lcd_wr_reg(0xe0);
lcd_wr_data8(0xd0);
lcd_wr_data8(0x00);
lcd_wr_data8(0x06);
lcd_wr_data8(0x09);
lcd_wr_data8(0x0b);
lcd_wr_data8(0x2a);
lcd_wr_data8(0x3c);
lcd_wr_data8(0x55);
lcd_wr_data8(0x4b);
lcd_wr_data8(0x08);
lcd_wr_data8(0x16);
lcd_wr_data8(0x14);
lcd_wr_data8(0x19);
lcd_wr_data8(0x20);
lcd_wr_reg(0xe1);
lcd_wr_data8(0xd0);
lcd_wr_data8(0x00);
lcd_wr_data8(0x06);
lcd_wr_data8(0x09);
lcd_wr_data8(0x0b);
lcd_wr_data8(0x29);
lcd_wr_data8(0x36);
lcd_wr_data8(0x54);
lcd_wr_data8(0x4b);
lcd_wr_data8(0x0d);
lcd_wr_data8(0x16);
lcd_wr_data8(0x14);
lcd_wr_data8(0x21);
lcd_wr_data8(0x20);
lcd_wr_reg(0x29);
```

这部分代码将ST7789V配置为 `4-Line Serial Interface => 16-bit/pixel(RGB 5-6-5-bit input)，65K-Color`

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `lcd_example` 参与编译。

```r
"./b0_lcd:lcd_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-llcd_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -llcd_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，并请使用带有LCD屏幕显示如下：

```c
************Lcd Example***********

************Lcd Example***********
```

