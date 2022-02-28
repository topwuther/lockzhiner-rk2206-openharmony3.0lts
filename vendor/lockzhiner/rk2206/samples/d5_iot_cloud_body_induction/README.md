# 小凌派-RK2206开发板OpenHarmonyOS开发-华为云IoT人体感应

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用人体感应模块，开发基于华为云IoT的人体感应应用。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

### API分析

#### device_info_init()

```c
void device_info_init(char *client_id, char *username, char *password);
```

**描述：**

初始化华为云设备信息。

**参数：**

|名字|描述|
|:--|:------| 
| client_id | 客户端ID |
| username | 用户名 |
| password | 密码 |

**返回值：**

无

#### oc_mqtt_init()

```c
int oc_mqtt_init(void);
```

**描述：**

初始化MQTT客户端。

**参数：**

无

**返回值：**

|返回值|描述|
|:--|:------| 
| 0 | 初始化成功 |
| -1 | 获取设备信息失败 |
| -2 | 初始化失败 |

#### oc_mqtt_profile_propertyreport()

```c
int oc_mqtt_profile_propertyreport(char *deviceid, oc_mqtt_profile_service_t *payload);
```

**描述：**

按照华为云上产品模型中定义的格式，将设备的属性数据上报到华为云。

**参数：**

|名字|描述|
|:--|:------| 
| deviceid | 设备ID |
| payload | 需要上传的消息指针 |

**返回值：**

|返回值|描述|
|:--|:------| 
| 0 | 上传成功 |
| 1 | 上传失败 |

### 主要代码分析

在iot_cloud_bi_example函数中，通过LOS_QueueCreate函数创建消息队列，用于线程间传递信息；通过LOS_TaskCreate创建e53_bi_thread和iot_cloud_bi_thread两个线程。其中，e53_bi_thread线程周期获取人体感应模块采集的信息；iot_cloud_bi_thread线程处理消息队列里的信息。

```c
void iot_cloud_bi_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_bi_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_cloud_bi_thread;
    task1.uwStackSize = 10240;
    task1.pcName = "iot_cloud_bi_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create iot_cloud_bi_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_bi_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "e53_bi_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_bi_thread ret:0x%x\n", ret);
        return;
    }
}
```

e53_bi_thread函数初始化GPIO作为输入，初始化PWM作为蜂鸣器控制源。采用轮询的方式读取GPIO电平值，当读取的GPIO值为高电平时，认为有物体靠近，开启蜂鸣器警告；当读取的GPIO值为低电平时，则为没有物体靠近，关闭蜂鸣器警告；并将获取到是否有物体靠近的告警状态数据通过消息队列发送到iot_cloud_bi_thread线程处理。

```c
void e53_bi_thread()
{
    bi_msg_t *app_msg = NULL;
    LzGpioValue val = LZGPIO_LEVEL_LOW, val_last = LZGPIO_LEVEL_LOW;
    unsigned int ret = LZ_HARDWARE_SUCCESS;

    e53_bi_init();

    while (1)
    {
        if (LzGpioGetVal(GPIO0_PA5, &val) != LZ_HARDWARE_SUCCESS)
        {
            printf("error get val\n");
        }
        if (val_last != val)
        {
            app_msg = malloc(sizeof(bi_msg_t));
            if (app_msg != NULL)
            {
                app_msg->msg_type = en_msg_report;
                app_msg->report.haveBody = val;
                if (LOS_QueueWrite(m_bi_msg_queue, (void *)app_msg, sizeof(bi_msg_t), LOS_WAIT_FOREVER) != LOS_OK)
                {
                    printf("%s LOS_QueueWrite fail\n", __func__);
                    free(app_msg);
                }
            }
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
        LOS_Msleep(100);
    }
}
```

iot_cloud_bi_thread函数主要处理消息队列里的信息。线程启动后，通过SetWifiModeOn连接WIFI；WIFI连接成功后，初始化MQTT协议栈，连接到华为云IoT服务器；成功连接华为云IoT服务器后，线程开始处理消息队列里的消息。消息队列里只有一种消息格式，e53_bi_thread函数传输过来的人体感应模块的数据，线程直接通过MQTT协议传输到华为云IoT平台上，此时平台上可以查看到最新上报的人体感应模块数据，是否有物体靠近的告警状态。

```c
void iot_cloud_bi_thread()
{
    bi_msg_t *app_msg = NULL;
    unsigned int addr;
    int ret;

    SetWifiModeOn();

    device_info_init(CLIENT_ID, USERNAME, PASSWORD);
    ret = oc_mqtt_init();
    if (ret != LOS_OK)
    {
        printf("oc_mqtt_init fail ret:%d\n", ret);
    }

    while (1)
    {
        ret = LOS_QueueRead(m_bi_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            app_msg = addr;
            switch (app_msg->msg_type)
            {
                case en_msg_report:
                    bi_deal_report_msg(&app_msg->report);
                    break;
                default:
                    break;
            }
            free(app_msg);
            app_msg = NULL;
        }
        else
        {
            LOS_Msleep(100);
        }
    }
}
```

## 编译调试

### 登录华为云

设备连接华为云前，需要做一些准备工作，请在华为云平台注册个人用户账号，并且需要实名认证后才可以正常使用。华为云IotDM地址：https://www.huaweicloud.com/product/iotdm.html

![登录华为云](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/huaweicloud_login.png)

### 华为云接入协议

选择侧边栏总览页面，点击平台接入地址。

![华为云协议地址](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/cloud_mqtt.png)

这里显示华为云平台接入的协议与域名信息，选择MQTT协议作为设备接入协议。

接入协议（端口号）：MQTT 1883

域名：a161b173a6.iot-mqtts.cn-north-4.myhuaweicloud.com

![华为云协议地址](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/cloud_address.png)

WIN + R键打开PC命令行CMD，执行如下命令获取接入域名的IP地址。

```c
ping a161b173a6.iot-mqtts.cn-north-4.myhuaweicloud.com
```

通过ping命令可以查询到接入域名的IP地址为121.36.42.100

![华为云IP地址](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/ping.png)

IP地址对应例程代码中的OC_SERVER_IP，端口号1883对于例程代码中的OC_SERVER_PORT。

```c
#define OC_SERVER_IP                    "121.36.42.100"
#define OC_SERVER_PORT                  1883
```

### 创建产品

选择侧边栏产品页面，点击右上角创建产品，添加人体感应产品

![创建产品](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/create_product.png)

单击产品详情页的自定义模型，在弹出页面中添加服务

服务ID：人体感应(必须与代码一致)

![添加服务](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/add_server.png)

选择人体感应服务，点击添加属性，添加人体感应属性

![添加属性](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/add_body.png)

### 注册设备

选择侧边栏所有设备页面，点击右上角注册设备，注册人体感应设备，勾选对应所属资源空间并选中刚刚创建的人体感应产品，注意设备认证类型选择“秘钥”，并按要求填写秘钥。

![注册设备](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/device.png)

注册完成后，选择侧边栏所有设备页面，可以看到注册完成的设备。

![设备信息](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/all_device.png)

在连接华为云前需要获取CLIENT_ID、USERNAME、PASSWORD，访问[华为云iot工具](https://iot-tool.obs-website.cn-north-4.myhuaweicloud.com/)；填写注册设备时生成的设备ID和设备密钥，生成连接信息（ClientId、Username、Password），并修改代码中对应的CLIENT_ID、USERNAME、PASSWORD。

点击进入人体感应设备，可以查看设备ID。

![设备ID](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/device_id.png)

输入设备ID和设备密匙，点击Generate生成ClientId、Username和Password。

![连接信息](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/id.png)

修改例程代码中的CLIENT_ID、USERNAME和PASSWORD为生成的ClientId、Username和Password。

```c
#define CLIENT_ID                       "61c69349078a93029b83ceff_E53_BI_0_0_2021122504"
#define USERNAME                        "61c69349078a93029b83ceff_E53_BI"
#define PASSWORD                        "82933df2133618cad436234d88fde06a196dd8ebc0d5c8cf2ee065968a469546"
```

### WIFI连接

修改例程代码中的WIFI_SSID和WIFI_PASSWORD为使用WIFI的SSID和密匙，用于连接网络，设备通过WIFI访问华为云。

```c
#define WIFI_SSID                       "lzdz"
#define WIFI_PASSWORD                   "12345678"
```

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `d5_iot_cloud_body_induction` 参与编译。

```r
"./d5_iot_cloud_body_induction:iot_cloud_bi_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-liot_cloud_bi_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -liot_cloud_bi_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，当人靠近传感器时，指示灯亮起，串口打印buzzer on，蜂鸣器响1s，串口打印buzzer off；华为云平台上同时也可以看到是否有人体感应状态信息。

```
buzzer on

buzzer off
```

登录华为云平台，选择侧边栏所有设备页面，点击进入人体感应设备查看开发板上报的数据。

![华为云](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/BI/iot_bi.png)

