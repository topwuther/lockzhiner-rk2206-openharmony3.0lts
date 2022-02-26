/*
 * Copyright (c) 2021 LOCKZHINER Electronic Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "lz_hardware.h"
#include "oled.h"
#include "oled_font.h"

/* OLED通信协议模式 ==>
 *    0 = gpio模拟i2c
 *    1 = i2c模块
 */
#define OLED_I2C_ENABLE     1

/* OLED的从设备地址 */
#define OLED_I2C_ADDRESS    0x3C

#if !OLED_I2C_ENABLE
/* GPIO0_C1 => I2C1_SDA_M1 */
#define GPIO_I2C_SDA        GPIO0_PC1
/* GPIO0_C2 => I2C1_SCL_M1 */
#define GPIO_I2C_SCL        GPIO0_PC2

/* SCLK引脚的输出高/低电平 */
#define OLED_SCLK_Clr()     LzGpioSetVal(GPIO_I2C_SCL, LZGPIO_LEVEL_LOW)
#define OLED_SCLK_Set()     LzGpioSetVal(GPIO_I2C_SCL, LZGPIO_LEVEL_HIGH)

/* SDIN引脚的输出高/低电平 */
#define OLED_SDIN_Clr()     LzGpioSetVal(GPIO_I2C_SDA, LZGPIO_LEVEL_LOW)
#define OLED_SDIN_Set()     LzGpioSetVal(GPIO_I2C_SDA, LZGPIO_LEVEL_HIGH)

/* RST引脚的输出高/低电平 */
#define OLED_RST_Clr()
#define OLED_RST_Set()
#else
#define OLED_I2C_BUS        1
static I2cBusIo m_i2cBus =
{
    .scl =  {.gpio = GPIO0_PC2, .func = MUX_FUNC5, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PC1, .func = MUX_FUNC5, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C1,
    .mode = FUNC_MODE_M1,
};
static unsigned int m_i2c_freq = 400000;
#endif

/* 定义OLED读写操作 */
#define OLED_CMD            0 // OLED的命令操作标记
#define OLED_DATA           1 // OLED的数据操作标记

//////////////////////////////////////////////////////////////


/***************************************************************
 * 函数名称: oled_pow
 * 说    明: 计算m^n
 * 参    数:
 *      @m：
 *      @n：
 * 返 回 值: 计算结果值
 ***************************************************************/
static uint32_t oled_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    
    while (n--)
    {
        result *= m;
    }
    
    return result;
}


#if !OLED_I2C_ENABLE
/***************************************************************
 * 函数名称: iic_start
 * 说    明: i2c的起始条件
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static inline void iic_start()
{
    OLED_SCLK_Set();
    OLED_SDIN_Set();
    OLED_SDIN_Clr();
    OLED_SCLK_Clr();
}


/***************************************************************
 * 函数名称: iic_stop
 * 说    明: i2c的结束条件
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static inline void iic_stop()
{
    OLED_SCLK_Set() ;
    OLED_SDIN_Clr();
    OLED_SDIN_Set();
}


/***************************************************************
 * 函数名称: iic_wait_ack
 * 说    明: i2c的等待应答
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static inline void iic_wait_ack()
{
    OLED_SCLK_Set();
    OLED_SCLK_Clr();
}


/***************************************************************
 * 函数名称: write_iic_byte
 * 说    明: i2c写单个字节
 * 参    数: 
 *      @IIC_Byte：数值
 * 返 回 值: 无
 ***************************************************************/
static inline void write_iic_byte(unsigned char iic_byte)
{
    unsigned char i;
    unsigned char m, da;
    da = iic_byte;
    OLED_SCLK_Clr();
    for (i = 0; i < 8; i++)
    {
        m = da;
        //OLED_SCLK_Clr();
        m = m & 0x80;
        if (m == 0x80)
        {
            OLED_SDIN_Set();
        }
        else
        {
            OLED_SDIN_Clr();
        }
        da = da << 1;
        OLED_SCLK_Set();
        OLED_SCLK_Clr();
    }
}


/***************************************************************
 * 函数名称: write_iic_command
 * 说    明: 通过i2c通信协议，往芯片写入一个命令
 * 参    数: 
 *      @IIC_Command：命令数值
 * 返 回 值: 无
 ***************************************************************/
static inline void write_iic_command(unsigned char iic_command)
{
    iic_start();
    /* 从设备地址 + SA0, SA0=0表示写操作 */
    write_iic_byte((OLED_I2C_ADDRESS << 1) | 0x0);
    iic_wait_ack();
    /* 通知芯片，下一个字节是命令 */
    write_iic_byte(0x00);
    iic_wait_ack();
    write_iic_byte(iic_command);
    iic_wait_ack();
    iic_stop();
}


/***************************************************************
 * 函数名称: write_iic_data
 * 说    明: 通过i2c通信协议，往芯片写入一个数据
 * 参    数: 
 *      @IIC_Data：数据数值
 * 返 回 值: 无
 ***************************************************************/
static inline void write_iic_data(unsigned char iic_data)
{
    iic_start();
    /* 从设备地址 + SA0, SA0=0表示写操作 */
    write_iic_byte((OLED_I2C_ADDRESS << 1) | 0x0);
    iic_wait_ack();
    /* 通知芯片，下一个字节是数据 */
    write_iic_byte(0x40);
    iic_wait_ack();
    write_iic_byte(iic_data);
    iic_wait_ack();
    iic_stop();
}
#else
/***************************************************************
 * 函数名称: write_iic_command
 * 说    明: 通过i2c通信协议，往芯片写入一个命令
 * 参    数: 
 *      @IIC_Command：命令数值
 * 返 回 值: 无
 ***************************************************************/
static inline void write_iic_command(unsigned char iic_command)
{
    unsigned char buffer[2];
    unsigned int ret;

    /* 填充数据，第一个字节是通知OLED芯片，下一个字节是命令 */
    buffer[0] = 0x00;
    buffer[1] = iic_command;
    ret = LzI2cWrite(OLED_I2C_BUS, OLED_I2C_ADDRESS, buffer, 2);
    if (ret != 0)
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
    }
}


/***************************************************************
 * 函数名称: write_iic_data
 * 说    明: 通过i2c通信协议，往芯片写入一个数据
 * 参    数: 
 *      @iic_data：数据数值
 * 返 回 值: 无
 ***************************************************************/
static inline void write_iic_data(unsigned char iic_data)
{
    unsigned char buffer[2];
    unsigned int ret;

    /* 填充数据，第一个字节是通知OLED芯片，下一个字节是数据 */
    buffer[0] = 0x40;
    buffer[1] = iic_data;
    ret = LzI2cWrite(OLED_I2C_BUS, OLED_I2C_ADDRESS, buffer, 2);
    if (ret != 0)
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
    }
}
#endif


/***************************************************************
 * 函数名称: oled_wr_byte
 * 说    明: 往芯片写数据
 * 参    数: 
 *      @data：数据数值
 *      @cmd：该数据是命令，还是数据
 * 返 回 值: 无
 ***************************************************************/
static inline void oled_wr_byte(unsigned dat, unsigned cmd)
{
    if (cmd == OLED_DATA)
    {
        write_iic_data(dat);
    }
    else if (cmd == OLED_CMD)
    {
        write_iic_command(dat);
    }
    else
    {
        printf("%s, %s, %d: cmd(%d) out of the range!\n", __FILE__, __func__, __LINE__, cmd);
    }
}


/***************************************************************
 * 函数名称: oled_set_pos
 * 说    明: 坐标设置
 * 参    数: 
 *      @x：X轴坐标
 *      @y：Y轴坐标
 * 返 回 值: 无
 ***************************************************************/
static inline void oled_set_pos(unsigned char x, unsigned char y)
{
    oled_wr_byte(0xb0 + y, OLED_CMD);
    oled_wr_byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    oled_wr_byte((x & 0x0f), OLED_CMD);
}

///////////////////////////////////////////////////////////////

/***************************************************************
 * 函数名称: oled_init
 * 说    明: oled初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int oled_init()
{
#if !OLED_I2C_ENABLE
    /* GPIO0_C1 => I2C1_SDA_M1 */
    LzGpioInit(GPIO_I2C_SDA);
    LzGpioSetDir(GPIO_I2C_SDA, LZGPIO_DIR_OUT);
    /* GPIO0_C2 => I2C1_SCL_M1 */
    LzGpioInit(GPIO_I2C_SCL);
    LzGpioSetDir(GPIO_I2C_SCL, LZGPIO_DIR_OUT);
#else
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
#endif
    
    LOS_Msleep(200);
    
    oled_wr_byte(0xAE, OLED_CMD); //--display off
    oled_wr_byte(0x00, OLED_CMD); //---set low column address
    oled_wr_byte(0x10, OLED_CMD); //---set high column address
    oled_wr_byte(0x40, OLED_CMD); //--set start line address
    oled_wr_byte(0xB0, OLED_CMD); //--set page address
    oled_wr_byte(0x81, OLED_CMD); // contract control
    oled_wr_byte(0xFF, OLED_CMD); //--128
    oled_wr_byte(0xA1, OLED_CMD); //set segment remap
    oled_wr_byte(0xA6, OLED_CMD); //--normal / reverse
    oled_wr_byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
    oled_wr_byte(0x3F, OLED_CMD); //--1/32 duty
    oled_wr_byte(0xC8, OLED_CMD); //Com scan direction
    oled_wr_byte(0xD3, OLED_CMD); //-set display offset
    oled_wr_byte(0x00, OLED_CMD); //
    
    oled_wr_byte(0xD5, OLED_CMD); //set osc division
    oled_wr_byte(0x80, OLED_CMD); //
    
    oled_wr_byte(0xD8, OLED_CMD); //set area color mode off
    oled_wr_byte(0x05, OLED_CMD); //
    
    oled_wr_byte(0xD9, OLED_CMD); //Set Pre-Charge Period
    oled_wr_byte(0xF1, OLED_CMD); //
    
    oled_wr_byte(0xDA, OLED_CMD); //set com pin configuartion
    oled_wr_byte(0x12, OLED_CMD); //
    
    oled_wr_byte(0xDB, OLED_CMD); //set Vcomh
    oled_wr_byte(0x30, OLED_CMD); //
    
    oled_wr_byte(0x8D, OLED_CMD); //set charge pump enable
    oled_wr_byte(0x14, OLED_CMD); //
    
    oled_wr_byte(0xAF, OLED_CMD); //--turn on oled panel
    
    return 0;
}


/***************************************************************
 * 函数名称: oled_deinit
 * 说    明: oled销毁
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int oled_deinit()
{
#if !OLED_I2C_ENABLE
    LzGpioDeinit(GPIO_I2C_SDA);
    LzGpioDeinit(GPIO_I2C_SCL);
#else
    LzI2cDeinit(OLED_I2C_BUS);
#endif
    return 0;
}


/***************************************************************
 * 函数名称: oled_clear
 * 说    明: oled清空
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void oled_clear()
{
    uint8_t i, n;
    
    for (i = 0; i < 8; i++)
    {
        oled_wr_byte(0xb0 + i, OLED_CMD); //设置页地址（0~7）
        oled_wr_byte(0x00, OLED_CMD); //设置显示位置—列低地址
        oled_wr_byte(0x10, OLED_CMD); //设置显示位置—列高地址
        for (n = 0; n < 128; n++)
        {
            oled_wr_byte(0, OLED_DATA);
        }
    }
}


/***************************************************************
 * 函数名称: oled_display_on
 * 说    明: oled显示开启
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void oled_display_on(void)
{
    oled_wr_byte(0X8D, OLED_CMD); //SET DCDC命令
    oled_wr_byte(0X14, OLED_CMD); //DCDC ON
    oled_wr_byte(0XAF, OLED_CMD); //DISPLAY ON
}


/***************************************************************
 * 函数名称: oled_display_off
 * 说    明: oled显示关闭
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void oled_display_off(void)
{
    oled_wr_byte(0X8D, OLED_CMD); //SET DCDC命令
    oled_wr_byte(0X10, OLED_CMD); //DCDC OFF
    oled_wr_byte(0XAE, OLED_CMD); //DISPLAY OFF
}


/***************************************************************
 * 函数名称: oled_show_char
 * 说    明: oled显示字符
 * 参    数: 
 *      @x：字符的X轴坐标
 *      @y：字符的Y轴坐标
 *      @chr：字符
 *      @chr_size：字符的字体，包括12/16两种字体
 * 返 回 值: 无
 ***************************************************************/
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t chr_size)
{
    unsigned char c = 0, i = 0;
    
    c = chr - ' '; //得到偏移后的值
    
    if (x > (OLED_COLUMN_MAX - 1))
    {
        x = 0;
        y = y + 2;
    }
    
    if (chr_size == 16)
    {
        oled_set_pos(x, y);
        for (i = 0; i < 8; i++)
        {
            oled_wr_byte(F8X16[c * 16 + i], OLED_DATA);
        }
        oled_set_pos(x, y + 1);
        for (i = 0; i < 8; i++)
        {
            oled_wr_byte(F8X16[c * 16 + i + 8], OLED_DATA);
        }
    }
    else
    {
        oled_set_pos(x, y);
        for (i = 0; i < 6; i++)
        {
            oled_wr_byte(F6x8[c][i], OLED_DATA);
        }
        
    }
}


/***************************************************************
 * 函数名称: oled_show_num
 * 说    明: oled显示数字
 * 参    数: 
 *      @x：数字的X轴坐标
 *      @y：数字的Y轴坐标
 *      @num：数字
 *      @len：数字的位数
 *      @size：字体大小
 * 返 回 值: 无
 ***************************************************************/
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size2)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    
    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                oled_show_char(x + (size2 / 2)*t, y, ' ', size2);
                continue;
            }
            else
            {
                enshow = 1;
            }
            
        }
        oled_show_char(x + (size2 / 2)*t, y, temp + '0', size2);
    }
}


/***************************************************************
 * 函数名称: oled_show_string
 * 说    明: oled显示字符串
 * 参    数: 
 *      @x：字符串的X轴坐标
 *      @y：字符串的Y轴坐标
 *      @p：字符串
 *      @chr_size：字符串的位数
 * 返 回 值: 无
 ***************************************************************/
void oled_show_string(uint8_t x, uint8_t y, uint8_t *chr, uint8_t chr_size)
{
    unsigned char j = 0;
    
    while (chr[j] != '\0')
    {
        oled_show_char(x, y, chr[j], chr_size);
        x += 8;
        if (x > 120)
        {
            x = 0;
            y += 2;
        }
        j++;
    }
}


/***************************************************************
 * 函数名称: oled_draw_bmp
 * 说    明: oled显示图片
 * 参    数: 
 *      @x0：图片的起始点X轴坐标，取值为0~127
 *      @y0：图片的起始点Y轴坐标，取值为0~63
 *      @x1：图片的结束点X轴坐标，取值为0~127
 *      @y1：图片的结束点Y轴坐标，取值为0~63
 *      @bmp：图片
 * 返 回 值: 无
 ***************************************************************/
void oled_draw_bmp(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char bmp[])
{
    unsigned int j = 0;
    unsigned char x, y;
    
    if (y1 % 8 == 0)
    {
        y = y1 / 8;
    }
    else
    {
        y = y1 / 8 + 1;
    }
    
    for (y = y0; y < y1; y++)
    {
        oled_set_pos(x0, y);

        for (x = x0; x < x1; x++)
        {
            oled_wr_byte(bmp[j++], OLED_DATA);
        }
    }
}
