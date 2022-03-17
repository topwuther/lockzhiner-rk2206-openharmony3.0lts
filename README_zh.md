## docs 资料包目录结构

| 目录名    | 描述                                                                                                                                                 |
| --------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| board     | [小凌派-RK2206开发板原理图](/vendor/lockzhiner/rk2206/docs/board/硬件原理图/小凌派开发板原理图/小凌派开发板底板原理图.pdf)、[硬件资料](/vendor/lockzhiner/rk2206/docs/board/硬件原理图/小凌派开发板原理图)、[视频课程配套PPT](/vendor/lockzhiner/rk2206/docs/board/课程配套PPT) 等等 |
| datasheet | [数据手册](/vendor/lockzhiner/rk2206/docs/datasheet)                                                                                                                                             |
| figures   | 资料图库                                                                                                                                             |

## 资料导航

| No | 类别           | 路径                                                         |
| -- | -------------- | ------------------------------------------------------------ |
| 1  | 开发板硬件资料 | [硬件原理图](/vendor/lockzhiner/rk2206/docs/board/硬件原理图)           |
| 2  | 快速上手教程   | [教程](/vendor/lockzhiner/rk2206/README_zh.md) |
| 3  | 案例代码列表   | [案例](/vendor/lockzhiner/rk2206/samples/README_zh.md)                         |
| 4  | 视频学习地址   | [视频地址](https://www.bilibili.com/medialist/play/1360647720?from=space&business=space_series&business_id=2088027&desc=1)                                    |
| 5  | 视频课程PPT    | [课程配套课件](/vendor/lockzhiner/rk2206/docs/board/课程配套课件)          |
| 6  | 常见问题       | 暂无                                                          |

## 一、小凌派-RK2206开发板介绍

[小凌派-RK2206](https://item.taobao.com/item.htm?id=664707670233)是一款由福州市凌睿智捷电子有限公司专为HarmonyOS系统打造的HarmonyOS开发板。

* 采用IoTCluB的E53接口标准，可接入所有的E53案例扩展板。
* 主控为瑞芯微RK2206芯片，200MHz主频，RAM 256KB、ROM 16KB、PSRAM 8M、FLASH 8M。
* 板载NFC Forum Type 2 Tag芯片及天线，可实现HarmonyOS“碰一碰”功能。
* 支持 IEEE 802.11b/g/n 制式，AP 模式。
* USB 2.0 、SD card /SPI/I2C 接口、麦克风接口、E53 接口（包含 UART、 I2C、ADC、DAC、PWM、5 个 GPIO）。
* 一根TypeC USB线，即可实现供电、下载等功能。
* 一根TypeC USB线，可支持串口调试功能。

## 二、快速上手（十分钟上手）

如果您想快速的体验一下HarmonyOS的源码 `[获取]`、`[编译]`、`[烧录]`过程，强烈建议您学习以下教程，这能让您在环境安装环节省下不少时间。

1、文档（Gitee）

* [编译环境搭建](/vendor/lockzhiner/rk2206/README_zh.md)

## 三、手把手教程系列

如果您想学习HarmonyOS系统的环境搭建，并想了解整个从0到1的过程，建议您跟着我们一步一步的学习。
需要告诉您的是，HarmonyOS系统的环境搭建所涉及的领域及组件会比较多，您需要严格按照我们的教程执行每一步，否则可能会出现不能预知的错误。

1、[文档系列（Gitee）](/vendor/lockzhiner/rk2206/docs/board/课程配套课件)

## 四、小凌派-RK2206开发板 案例开发

小凌派-RK2206开发板提供多个案例，案例以A、B、C、D进行不同类别进行分级，方便初学者由浅入深逐步学习。您拿到工程后经过简单的编程和下载即可看到实验现象。下面依次对A/B/C/D类进行介绍：

* `A1 - A99`：内核类
* `B1 - B99`：基础外设类
* `C1 - C99`：E53传感器类
* `D1 - D99`：物联网类

例程列表如下所示：

| 编号 | 类别      | 例程名                  | 说明                                                                                                       |
| ---- | --------- | ----------------------- | ---------------------------------------------------------------------------------------------------------- |
| A1   | 内核      | thread                  | [任务](/vendor/lockzhiner/rk2206/samples/a1_kernal_task/README_zh.md)                                            |
| A2   | 内核      | semaphore               | [信号量](/vendor/lockzhiner/rk2206/samples/a2_kernel_semaphore/README_zh.md)                                     |
| A3   | 内核      | timer                   | [定时器](/vendor/lockzhiner/rk2206/samples/a3_kernel_timer/README_zh.md)                                         |
| A4   | 内核      | mutex                   | [互斥锁](/vendor/lockzhiner/rk2206/samples/a4_kernel_mutex/README_zh.md)                                         |
| A5   | 内核      | queue                   | [消息队列](/vendor/lockzhiner/rk2206/samples/a5_kernel_queue/README_zh.md)                                       |
| A6   | 内核      | event                   | [事件](/vendor/lockzhiner/rk2206/samples/a6_kernel_event/README_zh.md)                                           |
| B1   | 基础      | adc                     | [ADC按键检测](/vendor/lockzhiner/rk2206/samples/b1_adc/README_zh.md)                                             |
| B2   | 基础      | nfc                     | [NFC标签](/vendor/lockzhiner/rk2206/samples/b2_nfc/README_zh.md)                                                 |
| B3   | 基础      | eeprom                  | [EEPROM测试](/vendor/lockzhiner/rk2206/samples/b3_eeprom/README_zh.md)                                           |
| B4   | 基础      | lcd                     | [LCD显示](/vendor/lockzhiner/rk2206/samples/b4_lcd/README_zh.md)                                                 |
| B5   | 基础      | oled                    | [OLED显示](/vendor/lockzhiner/rk2206/samples/b5_oled/README_zh.md)                                               |
| C1   | E53传感器 | e53_ia_example          | [智慧农业模块](/vendor/lockzhiner/rk2206/samples/c1_e53_intelligent_agriculture/README_zh.md)                    |
| C2   | E53传感器 | e53_sc_example          | [智慧井盖](/vendor/lockzhiner/rk2206/samples/d2_iot_cloud_smart_covers/README_zh.md)                                     |
| C3   | E53传感器 | e53_isl_example         | [智慧路灯](/vendor/lockzhiner/rk2206/samples/c3_e53_intelligent_street_lamp/README_zh.md)                        |
| C4   | E53传感器 | e53_iv01_example        | [智慧车载](/vendor/lockzhiner/rk2206/samples/c4_e53_intelligent_vehicle_01/README_zh.md)                         |
| C5   | E53传感器 | e53_bi_example          | [人体感应](/vendor/lockzhiner/rk2206/samples/c5_e53_body_induction/README_zh.md)                                 |
| C6   | E53传感器 | e53_gs_example          | [智能手势](/vendor/lockzhiner/rk2206/samples/c6_e53_gesture_sensor/README_zh.md)                                 |
| D1   | 物联网    | iot_mqtt                | [使用 Paho-MQTT 软件包实现 MQTT 协议通信](/vendor/lockzhiner/rk2206/samples/d1_iot_mqtt/README_zh.md)            |
| D2   | 物联网    | iot_cloud_sc_example    | [基于华为IoT平台的智慧井盖](/vendor/lockzhiner/rk2206/samples/d2_iot_cloud_smart_city/README_zh.md)              |
| D3   | 物联网    | iot_cloud_isl_example   | [基于华为IoT平台的智慧路灯](/vendor/lockzhiner/rk2206/samples/d3_iot_cloud_intelligent_street_lamp/README_zh.md) |
| D4   | 物联网    | iot_cloud_iv_example    | [基于华为IoT平台的智慧车载](/vendor/lockzhiner/rk2206/samples/d4_iot_cloud_intelligent_vehicle/README_zh.md)     |
| D5   | 物联网    | iot_cloud_bi_example    | [基于华为IoT平台的人体感应](/vendor/lockzhiner/rk2206/samples/d5_iot_cloud_body_induction/README_zh.md)          |
| D6   | 物联网    | iot_cloud_gs_example    | [基于华为IoT平台的智能手势](/vendor/lockzhiner/rk2206/samples/d6_iot_cloud_gesture_sensor/README_zh.md)          |
| D7   | 物联网    | iot_cloud_ia_example    | [基于华为IoT平台的智慧农业](/vendor/lockzhiner/rk2206/samples/d7_iot_cloud_intelligent_agriculture/README_zh.md) |

## 五、源码目录简介

小凌派的源码目录延续OpenHarmony官方目录结构，以下是源码目录的说明：

| 目录名       | 描述                                  |
| ------------ | ------------------------------------- |
| applications | 应用程序样例                          |
| base         | 基础软件服务子系统集&硬件服务子系统集 |
| build        | 组件化编译、构建和配置脚本            |
| domains      | 增强软件服务子系统集                  |
| drivers      | 驱动子系统                            |
| foundation   | 系统基础能力子系统集                  |
| kernel       | 内核子系统                            |
| prebuilts    | 编译器及工具链子系统                  |
| test         | 测试子系统                            |
| third_party  | 开源第三方组件                        |
| utils        | 常用的工具集                          |
| vendor       | 厂商提供的软件                        |
| build.py     | 编译脚本文件                          |

## 六、开发板图片预览[（淘宝链接）](https://item.taobao.com/item.htm?id=664707670233)

[![](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)](https://item.taobao.com/item.htm?id=664707670233)

## 七、联系我们

QQ群：982739783

