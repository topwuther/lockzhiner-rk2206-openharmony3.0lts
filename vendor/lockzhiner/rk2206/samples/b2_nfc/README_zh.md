# 小凌派-RK2206开发板基础外设开发——NFC

本示例将演示如何在小凌派-RK2206开发板控制NFC

![小凌派-RK2206](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

### 头文件

```
/vendor/lockzhiner/rk2206/samples/b0_nfc/include/nfc.h
```

### API分析

#### nfc_init()

```c
unsigned int nfc_init(void);
```

**描述：**

NFC模块初始化。

**参数：**

无

**返回值：**

0为成功，反之则失败。

#### nfc_deinit()

```c
unsigned int nfc_deinit(void);
```

**描述：**

NFC模块注销。

**参数：**

无

**返回值：**

0为成功，反之则失败。

#### nfc_store_uri_http()

```c
bool nfc_store_uri_http(RecordPosEnu position, uint8_t *http);
```

**描述：**

向NFC写入URI信息。

**参数：**

| 名字     | 描述                     |
| :------- | :----------------------- |
| position | 信息标识                 |
| http     | 需要写入的网络地址字符串 |

**返回值：**

true为成功，false则失败。

#### nfc_store_text()

```c
bool nfc_store_text(RecordPosEnu position, uint8_t *text);
```

**描述：**

向NFC写入txt信息。

**参数：**

| 名字     | 描述                 |
| :------- | :------------------- |
| position | 信息标识             |
| text     | 需要写入的内容字符串 |

**返回值：**

true为成功，false则失败。

### 主要代码分析

**初始化代码分析**

这部分代码为i2c初始化的代码。首先用 `I2cIoInit()` 函数将GPIO0_PC1复用为I2C1_SDA_M1，GPIO0_PC2复用为I2C1_SCL_M1。最后调用 `LzI2cInit()`函数初始化I2C1端口。

```c
if (I2cIoInit(m_i2c2m0) != LZ_HARDWARE_SUCCESS)
{
    printf("%s, %s, %d: I2cIoInit failed!\n", __FILE__, __func__, __LINE__);
    return __LINE__;
}
if (LzI2cInit(NFC_I2C_PORT, m_i2c2_freq) != LZ_HARDWARE_SUCCESS)
{
    printf("%s, %s, %d: LzI2cInit failed!\n", __FILE__, __func__, __LINE__);
    return __LINE__;
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `nfc` 参与编译。

```r
"./b0_nfc:nfc_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lnfc_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lnfc_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，并请使用带有LCD屏幕显示如下：

```c
==============NFC Example==============
Please use the mobile phone with NFC function close to the development board!
```

