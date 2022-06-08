# 小凌派-RK2206开发板OpenHarmonyOS开发-MQTT协议开发

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用MQTT协议栈，开发物联网功能。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

MQTT(消息队列遥测传输)是一个基于客户端-服务器的消息发布/订阅传输协议，是ISO标准(ISO/IEC PRF 20922)下基于发布/订阅范式的消息协议。MQTT工作在TCP/IP协议栈上，是为硬件性能低下的远程设备以及网络状况不理想的情况下而设计的发布/订阅型消息协议。由于MQTT协议的轻量、简便、开放的特点使它适用范围非常广泛。在很多情况下，包括受限的环境中使用，广泛应用于物联网（IoT）。MQTT协议在卫星链路通信传感器、医疗设备、智能家居、及一些小型化设备中已广泛使用。

### API分析

#### NetworkInit()

```c
void NetworkInit(Network* n);
```

**描述：**

网络初始化。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/liteOS/MQTTLiteOS.h

**参数：**

| 名字 | 描述                                                          |
| :--- | :------------------------------------------------------------ |
| n    | 网络结构体指针，包含socket ID，读操作函数指针和写操作函数指针 |

**返回值：**

无

#### NetworkConnect()

```c
int NetworkConnect(Network* n, char* addr, int port);
```

**描述：**

网络连接。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/liteOS/MQTTLiteOS.h

**参数：**

| 名字 | 描述                                                          |
| :--- | :------------------------------------------------------------ |
| n    | 网络结构体指针，包含socket ID，读操作函数指针和写操作函数指针 |
| addr | MQTT服务器IP地址                                              |
| port | MQTT服务器端口                                                |

**返回值：**

0：成功；-1：失败

#### NetworkDisconnect()

```c
void NetworkDisconnect(Network* n);
```

**描述：**

断开网络连接。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/liteOS/MQTTLiteOS.h

**参数：**

| 名字 | 描述                                                          |
| :--- | :------------------------------------------------------------ |
| n    | 网络结构体指针，包含socket ID，读操作函数指针和写操作函数指针 |

**返回值：**

无

#### MQTTClientInit()

```c
void MQTTClientInit(MQTTClient* client, 
                Network* network, 
                unsigned int command_timeout_ms, 
                unsigned char* sendbuf, 
                size_t sendbuf_size, 
                unsigned char* readbuf, 
                size_t readbuf_size);
```

**描述：**

MQTT客户端初始化。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/MQTTClient.h

**参数：**

| 名字               | 描述                                                          |
| :----------------- | :------------------------------------------------------------ |
| client             | MQTT客户端结构体指针                                          |
| network            | 网络结构体指针，包含socket ID，读操作函数指针和写操作函数指针 |
| command_timeout_ms | MQTT发送命令超时时间                                          |
| sendbuf            | 发送缓冲区指针                                                |
| sendbuf_size       | 发送缓冲区大小                                                |
| readbuf            | 读缓冲区指针                                                  |
| readbuf_size       | 读缓冲区大小                                                  |

**返回值：**

无

#### MQTTConnect()

```c
int MQTTConnect(MQTTClient* client, MQTTPacket_connectData* options);
```

**描述：**

连接到MQTT服务端。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/MQTTClient.h

**参数：**

| 名字    | 描述                                                                             |
| :------ | :------------------------------------------------------------------------------- |
| client  | MQTT客户端结构体指针                                                             |
| options | MQTT协议连接数据指针，包含客户端ID、客户端名称、MQTT协议版本、保活时间间隔等数据 |

**返回值：**

0：成功；-1：失败；-2：缓冲区溢出

#### MQTTDisconnect()

```c
int MQTTDisconnect(MQTTClient* c);
```

**描述：**

断开连接到MQTT服务端。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/MQTTClient.h

**参数：**

| 名字 | 描述                 |
| :--- | :------------------- |
| c    | MQTT客户端结构体指针 |

**返回值：**

0：成功；-1：失败；-2：缓冲区溢出

#### MQTTSubscribe()

```c
int MQTTSubscribe(MQTTClient* c, 
                const char* topicFilter, 
                enum QoS qos, 
                messageHandler messageHandler);
```

**描述：**

向MQTT服务端订阅消息。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/MQTTClient.h

**参数：**

| 名字           | 描述                 |
| :------------- | :------------------- |
| c              | MQTT客户端结构体指针 |
| topicFilter    | 订阅消息名称         |
| qos            | QOS等级              |
| messageHandler | 订阅消息处理函数指针 |

**返回值：**

0：成功；-1：失败；-2：缓冲区溢出

#### MQTTPublish()

```c
int MQTTPublish(MQTTClient* c, const char* topicName, MQTTMessage* message);
```

**描述：**

向MQTT服务端发送消息。

**头文件：**

//third_party/paho_mqtt/MQTTClient-C/src/MQTTClient.h

**参数：**

| 名字      | 描述                                              |
| :-------- | :------------------------------------------------ |
| c         | MQTT客户端结构体指针                              |
| topicName | 主题名称                                          |
| message   | 消息指针，包含消息内容、消息长度、消息ID和QOS等级 |

**返回值：**

0：成功；-1：失败；-2：缓冲区溢出

### 主要代码分析

在iot_mqtt_example函数中，通过LOS_TaskCreate函数创建iot_mqtt_thread线程。

```c
void iot_mqtt_example()
{
    unsigned int threadID;
    unsigned int ret = LOS_OK;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_mqtt_thread;
    task.uwStackSize = 10240;
    task.pcName = IOT_MQTT_EXAMPLE;
    task.usTaskPrio = 6;
    ret = LOS_TaskCreate(&threadID, &task);
    if (LOS_OK != ret)
    {
        printf("Falied to create %s\n", IOT_MQTT_EXAMPLE);
        return;
    }
}
```

iot_mqtt_thread函数中，开发板先连接WIFI，通过WIFI和PC端的MQTT服务器通信；WIFI连接成功后，初始化网络，初始化MQTT客户端；初始化完成后，连接MQTT服务端，注册消息接收函数message_receive；函数进入循环，5s向PC端MQTT服务器发送一次消息。

```c
void iot_mqtt_thread()
{
    int rc;
    MQTTClient client;
    MQTTMessage message;
    char payload[30];
    MQTTString clientId = MQTTString_initializer;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    /*WIFI连接*/
    SetWifiModeOn();

    printf("NetworkInit...\n");
    NetworkInit(&network);

begin:
    printf("NetworkConnect...\n");
    rc = NetworkConnect(&network, MQTT_SERVER_IP, MQTT_SERVER_PORT);

    printf("MQTTClientInit...\n");
    MQTTClientInit(&client, &network, 2000, sendBuf, sizeof(sendBuf), readBuf, sizeof(readBuf));

    clientId.cstring = "lzdz";

    data.clientID          = clientId;
    data.willFlag          = 0;
    data.MQTTVersion       = 3;
    data.keepAliveInterval = 0;
    data.cleansession      = 1;

    printf("MQTTConnect...\n");
    rc = MQTTConnect(&client, &data);
    if (rc != 0) {
        printf("MQTTConnect fail:%d\n", rc);
        NetworkDisconnect(&network);
        MQTTDisconnect(&client);
        LOS_Msleep(200);
        goto begin;
    }

    printf("MQTTSubscribe...\n");
    rc = MQTTSubscribe(&client, "substopic", 2, message_receive);
    if (rc != 0) {
        printf("MQTTSubscribe fail:%d\n", rc);
        LOS_Msleep(200);
        goto begin;
    }

    while (1)
    {
        sprintf(payload, "publish test");
        message.qos = 2;
        message.retained = 0;
        message.payload = payload;
        message.payloadlen = strlen(payload);

        if ((rc = MQTTPublish(&client, "IOT_MQTT", &message)) != 0){
            printf("Return code from MQTT publish is %d\n", rc);
            NetworkDisconnect(&network);
            MQTTDisconnect(&client);
            goto begin;
        }

        LOS_Msleep(5000);
    }
}
```

## 编译调试

### 修改MQTT服务端IP

PC端按键WIN + R键打开命令行CMD，执行如下命令获取PC端的IP地址。

```c
ipconfig
```

如下图所示，PC端的IP地址为：192.168.1.164。

![ip](/vendor/lockzhiner/rk2206/docs/figures/mqtt/ipconfig.png)

对应修改例程/vendor/lockzhiner/rk2206/samples/d1_iot_mqtt/iot_mqtt.c代码中的MQTT_SERVER_IP为查询到PC端IP地址192.168.1.164，MQTT_SERVER_PORT修改为MQTT服务器端口号1883。

```c
#define MQTT_SERVER_IP                          "192.168.1.164"
#define MQTT_SERVER_PORT                        1883
```

### 下载MQTT代理工具mosquitto

根据电脑位数选择对应的安装包，32位系统下载x86后缀的安装包，64位系统下载x64后缀的安装包，这里选择下载mosquitto-2.0.9a-install-windows版本进行测试。

32位系统下载地址：
https://mosquitto.org/files/binary/win32/

![mosquitto](/vendor/lockzhiner/rk2206/docs/figures/mqtt/mosquitto_download.png)

64位系统下载地址：
https://mosquitto.org/files/binary/win64/

![mosquitto](/vendor/lockzhiner/rk2206/docs/figures/mqtt/mosquitto_download_x64.png)

下载mosquitto安装包后，点击安装mosquitto工具；安装完毕后，需要配置mosquitto，修改mosquitto安装路径下的mosquitto.conf文件，如下图所示。

![mosquitto](/vendor/lockzhiner/rk2206/docs/figures/mqtt/mosquitto_config.png)

在mosquitto.conf文件末尾增加如下配置，其中，192.168.1.164为PC端IP地址，1883为MQTT服务器端口号，allow_anonymous为允许匿名登录。

```c
listener 1883 192.168.1.164
listener 1883 localhost

allow_anonymous true
````

mosquitto安装配置完成后，需要开启mosquitto服务，右击打开我的电脑-管理-服务和应用程序-服务，找到Mosquitto Broker服务。

![mosquitto](/vendor/lockzhiner/rk2206/docs/figures/mqtt/mosquitto_server.png)

双击打开Mosquitto Broker服务，点击启动，开启Mosquitto服务。

![mosquitto](/vendor/lockzhiner/rk2206/docs/figures/mqtt/mosquitto_start.png)

### 下载Eclipse Paho MQTT 工具

点击进入下载地址：https://repo.eclipse.org/content/repositories/paho-releases/org/eclipse/paho/org.eclipse.paho.ui.app/1.1.1/

下载Paho软件包：org.eclipse.paho.ui.app-1.1.1-win32.win32.x86_64.zip。

![paho](/vendor/lockzhiner/rk2206/docs/figures/mqtt/paho.png)

下载Paho软件包后，解压软件包，双击打开Paho.exe；进入Paho软件界面，点击右上角加号按键，添加一个MQTT服务器连接，如下图所示。

![paho](/vendor/lockzhiner/rk2206/docs/figures/mqtt/paho_ui.png)

### WIFI连接

修改例程/device/rockchip/rk2206/sdk_liteos/board/src/config_network.c代码中的SSID和PASSWORD为使用WIFI的SSID和密匙，用于连接网络，设备通过WIFI访问PC端MQTT服务器。

```c
#define SSID                                    "lzdz"
#define PASSWORD                                "12345678"
```

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `d1_iot_mqtt` 参与编译。

```r
"./d1_iot_mqtt:iot_mqtt_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-liot_mqtt_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -liot_mqtt_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，串口打印MQTT初始化、连接等日志。

```
NetworkConnect...
MQTTClientInit...
MQTTConnect...
MQTTSubscribe...
```

#### 连接

打开paho软件，点击连接，当连接成功时，连接状态显示为已连接；需要注意的是，如果出现连接失败，请尝试关闭PC防火墙再进行连接。

#### 订阅

点击订阅添加按键，新增主题IOT_MQTT（主题名称需要与例程代码MQTTPublish函数传参的主题名称一致），点击订阅按键，此时，历史记录页面就开始显示接收的开发板上报信息，如下图所示。

![paho](/vendor/lockzhiner/rk2206/docs/figures/mqtt/paho_mqtt.png)

#### 发布

在发布主题栏，输入主题名称substopic（主题名称需要与例程代码MQTTSubscribe函数传参的主题名称一致），在消息栏输入消息hello world，点击发布，此时，历史记录页面显示已发布出的消息，如下图所示。

![paho](/vendor/lockzhiner/rk2206/docs/figures/mqtt/paho_mqtt_publish.png)

同时，开发板串口会同步显示接收到的消息日志，如下所示。

```
message arrived on topic substopic: hello world
message arrived on topic substopic: hello world
```