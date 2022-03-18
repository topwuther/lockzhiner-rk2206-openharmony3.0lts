# 小凌派-RK2206开发板OpenHarmonyOS开发-华为云IoT智慧车载

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用智慧车载模块，开发基于华为云IoT的智慧车载应用。

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

在iot_cloud_iv_example函数中，通过LOS_QueueCreate函数创建消息队列，用于线程间传递信息；通过LOS_TaskCreate创建e53_iv_thread和iot_cloud_iv_thread两个线程。其中，e53_iv_thread线程周期获取智慧车载模块采集的距离信息；iot_cloud_iv_thread线程处理消息队列里的信息。

```c
void iot_cloud_iv_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_iv_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_cloud_iv_thread;
    task1.uwStackSize = 10240;
    task1.pcName = "iot_cloud_iv_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create iot_cloud_iv_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_iv_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "e53_iv_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_iv_thread ret:0x%x\n", ret);
        return;
    }
}
```

e53_iv_thread函数每2s获取一次智慧车载模块采集的距离信息；将获取到的距离数据通过消息队列发送到iot_cloud_iv_thread线程处理。

```c
void e53_iv_thread()
{
    iv_msg_t *app_msg = NULL;
    unsigned int ret = 0;
    /* 每个周期为200usec，占空比为100usec */
    unsigned int duty_ns = 160000;
    unsigned int cycle_ns = 200000;
    float distance_cm = 0.0;
    
    e53_iv01_init();

    while (1)
    {
        ret = e53_iv01_get_distance(&distance_cm);
        if (ret == 1)
        {
            printf("distance cm: %f\n", distance_cm);
            
            if (distance_cm <= 20.0)
            {
                e53_iv01_buzzer_set(1, duty_ns, cycle_ns);
                e53_iv01_led_warning_set(1);
            }
            else
            {
                e53_iv01_buzzer_set(0, duty_ns, cycle_ns);
                e53_iv01_led_warning_set(0);
            }

            app_msg = malloc(sizeof(iv_msg_t));
            if (app_msg != NULL)
            {
                app_msg->msg_type = en_msg_report;
                app_msg->report.distance_meter = (int)distance_cm;
                if (LOS_QueueWrite(m_iv_msg_queue, (void *)app_msg, sizeof(iv_msg_t), LOS_WAIT_FOREVER) != LOS_OK)
                {
                    printf("%s LOS_QueueWrite fail\n", __func__);
                    free(app_msg);
                }
            }
        }
        LOS_Msleep(2000);
    }
}
```

iot_cloud_iv_thread函数主要处理消息队列里的信息。线程启动后，通过SetWifiModeOn连接WIFI；WIFI连接成功后，初始化MQTT协议栈，连接到华为云IoT服务器；成功连接华为云IoT服务器后，线程开始处理消息队列里的消息。消息队列里只有一种消息格式，e53_iv_thread函数传输过来的智慧车载模块的数据，线程直接通过MQTT协议传输到华为云IoT平台上，此时平台上可以查看到最新上报的智慧车载模块的距离数据。

```c
int iot_cloud_iv_thread()
{
    iv_msg_t *app_msg;
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
        ret = LOS_QueueRead(m_iv_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            app_msg = addr;
            switch (app_msg->msg_type)
            {
                case en_msg_report:
                    iv_deal_report_msg(&app_msg->report);
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

IP地址对应例程/vendor/lockzhiner/rk2206/samples/d4_iot_cloud_intelligent_vehicle/include/oc_mqtt.h代码中的OC_SERVER_IP，端口号1883对于例程代码中的OC_SERVER_PORT。

```c
#define OC_SERVER_IP                    "121.36.42.100"
#define OC_SERVER_PORT                  1883
```

### 创建产品

选择侧边栏产品页面，点击右上角创建产品，添加智慧车载产品

![创建产品](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/create_product.png)

单击产品详情页的自定义模型，在弹出页面中添加服务

服务ID：智慧车载(必须与代码一致)

![添加服务](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/add_server.png)

选择智慧车载服务，点击添加属性，添加距离属性

![添加属性](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/add_distance.png)

### 注册设备

选择侧边栏所有设备页面，点击右上角注册设备，注册智慧车载设备，勾选对应所属资源空间并选中刚刚创建的智慧车载产品，注意设备认证类型选择“密钥”，并按要求填写密钥。

![注册设备](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/device.png)

注册完成后，选择侧边栏所有设备页面，可以看到注册完成的设备。

![设备信息](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/all_device.png)

在连接华为云前需要获取CLIENT_ID、USERNAME、PASSWORD，访问[华为云iot工具](https://iot-tool.obs-website.cn-north-4.myhuaweicloud.com/)；填写注册设备时生成的设备ID和设备密钥，生成连接信息（ClientId、Username、Password），并修改代码中对应的CLIENT_ID、USERNAME、PASSWORD。

点击进入智慧车载设备，可以查看设备ID。

![设备ID](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/device_id.png)

输入设备ID和设备密匙，点击Generate生成ClientId、Username和Password。

![连接信息](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/id.png)

修改例程/vendor/lockzhiner/rk2206/samples/d4_iot_cloud_intelligent_vehicle/iot_cloud_intelligent_vehicle.c代码中的CLIENT_ID、USERNAME和PASSWORD为生成的ClientId、Username和Password。

```c
#define CLIENT_ID                       "61e7f618de9933029be38412_E53_IV_0_0_2022021902"
#define USERNAME                        "61e7f618de9933029be38412_E53_IV"
#define PASSWORD                        "d29289d60c2decc3f8ff79759f47e6102103acdbf7b724e90a8c529786c26598"
```

### WIFI连接

修改例程/device/rockchip/rk2206/sdk_liteos/board/src/config_network.c代码中的SSID和PASSWORD为使用WIFI的SSID和密匙，用于连接网络，设备通过WIFI访问华为云。

```c
#define SSID                       "lzdz"
#define PASSWORD                   "12345678"
```

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `d4_iot_cloud_intelligent_vehicle` 参与编译。

```r
"./d4_iot_cloud_intelligent_vehicle:iot_cloud_iv_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-liot_cloud_iv_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -liot_cloud_iv_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志。

```
distance cm: 173.854752

distance cm: 173.646500
```

登录华为云平台，选择侧边栏所有设备页面，点击进入智慧车载设备查看开发板上报的数据。

![华为云](/vendor/lockzhiner/rk2206/docs/figures/huaweicloud/IV/iot_iv.png)

