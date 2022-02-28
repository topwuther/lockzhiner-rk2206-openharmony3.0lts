# 小凌派-RK2206开发板基础外设开发——I2C控制EEPROM读写

本示例将演示如何在小凌派-RK2206开发板上使用I2C控制EEPROM读写

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

### API分析

#### eeprom_init()

```c
unsigned int eeprom_init();
```

**描述：**

EEPROM初始化，包括i2c初始化。

**参数：**

无

**返回值：**

0为成功，反之失败

#### eeprom_deinit()

```c
unsigned int eeprom_deinit();
```

**描述：**

EEPROM注销。

**参数：**

无

**返回值：**

0为成功，反之失败

#### eeprom_get_blocksize()

```c
unsigned int eeprom_get_blocksize();
```

**描述：**

EEPROM获取页大小。

**参数：**

无

**返回值：**

返回页大小。

#### eeprom_readbyte()

```c
unsigned int eeprom_readbyte(unsigned int addr, unsigned char *data);
```

**描述：**

EEPROM读一个字节。

**参数：**

* addr：EEPROM存储地址
* data: 存放EERPOM的数据指针

**返回值：**

返回读取字节的长度，反之为错误。

#### eeprom_writebyte()

```c
unsigned int eeprom_writebyte(unsigned int addr, unsigned char data);
```

**描述：**

EEPROM写一个字节。

**参数：**

* addr：EEPROM存储地址
* data: 写ERPOM的数据

**返回值：**

返回写入字节的长度，反之为错误。

#### eeprom_writepage()

```c
unsigned int eeprom_writepage(unsigned int addr, 
                unsigned char *data, 
                unsigned int data_len);
```

**描述：**

EEPROM写1个页字节。

**参数：**

* addr: EEPROM存储地址，必须是页地址
* data: 写ERPOM的数据指针
* data_len: 写EEPROM数据的长度，必须是小于或等于1个页大小

**返回值：**

返回写入字节的长度，反之为错误。

#### eeprom_read()

```c
unsigned int eeprom_read(unsigned int addr, 
                unsigned char *data, 
                unsigned int data_len);
```

**描述：**

EEPROM读多个字节。

**参数：**

* addr: EEPROM存储地址，必须是页地址
* data: 存放EERPOM的数据指针
* data_len: 读取EERPOM数据的长度

**返回值：**

返回读取字节的长度，反之为错误。

#### eeprom_write()

```c
unsigned int eeprom_write(unsigned int addr, 
                unsigned char *data, 
                unsigned int data_len);
```

**描述：**

EEPROM写多个字节。

**参数：**

* addr: EEPROM存储地址，必须是页地址
* data: 写ERPOM的数据指针
* data_len: 写EEPROM数据的长度

**返回值：**

返回写入数据的长度，反之为错误。

### 主要代码分析

#### i2c初始化源代码分析

这部分代码为i2c初始化的代码。首先用 `I2cIoInit()` 函数将GPIO0_PA0复用为I2C0_SDA_M2，GPIO0_PA1复用为I2C1_SCL_M2。最后调用 `LzI2cInit()`函数初始化I2C1端口。

```c
if (I2cIoInit(m_i2cBus) != LZ_HARDWARE_SUCCESS) {
    printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
    return __LINE__;
}
if (LzI2cInit(EEPROM_I2C_BUS, m_i2c_freq) != LZ_HARDWARE_SUCCESS) {
    printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
    return __LINE__;
}
```

#### K24C02读操作

本模块只采用K24C02读模式的连续读数据（Sequential Read）。具体如何控制i2c读K24C02数据的操作如下：

```c
/***************************************************************
* 函数名称: eeprom_readbyte
* 说    明: EEPROM读一个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 存放EERPOM的数据指针
* 返 回 值: 返回读取字节的长度，反之为错误
***************************************************************/
unsigned int eeprom_readbyte(unsigned int addr, unsigned char *data)
{
    unsigned int ret = 0;
    unsigned char buffer[1];
    LzI2cMsg msgs[2];

    /* K24C02的存储地址是0~255 */
    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    buffer[0] = (unsigned char)addr;

    msgs[0].addr = EEPROM_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].buf = &buffer[0];
    msgs[0].len = 1;

    msgs[1].addr = EEPROM_I2C_ADDRESS;
    msgs[1].flags = I2C_M_RD;
    msgs[1].buf = data;
    msgs[1].len = 1;

   ret = LzI2cTransfer(EEPROM_I2C_BUS, msgs, 2);
   if (ret != LZ_HARDWARE_SUCCESS) {
       printf("%s, %s, %d: LzI2cTransfer failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
       return 0;
   }

    return 1;
}
```

#### K24C02写操作

本模块采用K24C02写模式的字节写操作（Byte Write）。具体如何控制i2c往K24C02写字节的操作如下：

```c
/***************************************************************
* 函数名称: eeprom_writebyte
* 说    明: EEPROM写一个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 写EERPOM的数据
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_writebyte(unsigned int addr, unsigned char data)
{
    unsigned int ret = 0;
    LzI2cMsg msgs[1];
    unsigned char buffer[2];

    /* K24C02的存储地址是0~255 */
    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    buffer[0] = (unsigned char)(addr & 0xFF);
    buffer[1] = data;

    msgs[0].addr = EEPROM_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].buf = &buffer[0];
    msgs[0].len = 2;

    ret = LzI2cTransfer(EEPROM_I2C_BUS, msgs, 1);
    if (ret != LZ_HARDWARE_SUCCESS) {
       printf("%s, %s, %d: LzI2cTransfer failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
       return 0;
    }

    /* K24C02芯片需要时间完成写操作，在此之前不响应其他操作*/
    eeprog_delay_usec(1000);

    return 1;
}
```

本模块采用K24C02写模式的页写操作（Page Write）。具体如何控制i2c往K24C02写页的操作如下：

```c
/***************************************************************
* 函数名称: eeprom_writepage
* 说    明: EEPROM写1个页字节
* 参    数: 
*           @addr: EEPROM存储地址，必须是页地址
*           @data: 写EERPOM的数据指针
*           @data_len: 写EEPROM数据的长度，必须是小于1个页大小
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_writepage(unsigned int addr, unsigned char *data, unsigned int data_len)
{
    unsigned int ret = 0;
    LzI2cMsg msgs[1];
    unsigned char buffer[EEPROM_PAGE + 1];
  
    /* K24C02的存储地址是0~255 */
    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    if ((addr % EEPROM_PAGE) != 0) {
        printf("%s, %s, %d: addr(0x%x) is not page addr(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_PAGE);
        return 0;
    }

    if ((addr + data_len) > EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr + data_len(0x%x) > EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr + data_len, EEPROM_ADDRESS_MAX);
        return 0;
    }

    if (data_len > EEPROM_PAGE) {
        printf("%s, %s, %d: data_len(%d) > EEPROM_PAGE(%d)\n", __FILE__, __func__, __LINE__, data_len, EEPROM_PAGE);
        return 0;
    }

    buffer[0] = addr;
    memcpy(&buffer[1], data, data_len);
  
    msgs[0].addr = EEPROM_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].buf = &buffer[0];
    msgs[0].len = 1 + data_len;

    ret = LzI2cTransfer(EEPROM_I2C_BUS, msgs, 1);
    if (ret != LZ_HARDWARE_SUCCESS) {
       printf("%s, %s, %d: LzI2cTransfer failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
       return 0;
    }

    /* K24C02芯片需要时间完成写操作，在此之前不响应其他操作*/
    eeprog_delay_usec(1000);

    return data_len;
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `eeprom_example` 参与编译。

```r
"./b3_eeprom:eeprom_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-leeprom_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -leeprom_example,
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，显示如下：

```c
************ Eeprom Process ************
BlockSize = 0x8
Write Byte: 3 = !
Write Byte: 4 = "
Write Byte: 5 = #
Write Byte: 6 = $
Write Byte: 7 = %
Write Byte: 8 = &
Write Byte: 9 = '
Write Byte: 10 = (
Write Byte: 11 = )
Write Byte: 12 = *
Write Byte: 13 = +
Write Byte: 14 = ,
Write Byte: 15 = -
Write Byte: 16 = .
Write Byte: 17 = /
Write Byte: 18 = 0
Write Byte: 19 = 1
Write Byte: 20 = 2
Write Byte: 21 = 3
Write Byte: 22 = 4
Write Byte: 23 = 5
Write Byte: 24 = 6
Write Byte: 25 = 7
Write Byte: 26 = 8
Write Byte: 27 = 9
Write Byte: 28 = :
Write Byte: 29 = ;
Write Byte: 30 = <
Write Byte: 31 = =
Write Byte: 32 = >
Read Byte: 3 = !
Read Byte: 4 = "
Read Byte: 5 = #
Read Byte: 6 = $
Read Byte: 7 = %
Read Byte: 8 = &
Read Byte: 9 = '
Read Byte: 10 = (
Read Byte: 11 = )
Read Byte: 12 = *
Read Byte: 13 = +
Read Byte: 14 = ,
Read Byte: 15 = -
Read Byte: 16 = .
Read Byte: 17 = /
Read Byte: 18 = 0
Read Byte: 19 = 1
Read Byte: 20 = 2
Read Byte: 21 = 3
Read Byte: 22 = 4
Read Byte: 23 = 5
Read Byte: 24 = 6
Read Byte: 25 = 7
Read Byte: 26 = 8
Read Byte: 27 = 9
Read Byte: 28 = :
Read Byte: 29 = ;
Read Byte: 30 = <
Read Byte: 31 = =
Read Byte: 32 = >
```
