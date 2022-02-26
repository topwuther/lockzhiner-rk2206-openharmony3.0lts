# 小凌派-RK2206开发板OpenHarmonyOS开发-MQTT协议开发

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用MQTT协议栈，开发物联网功能。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.png)

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

在Iot_Mqtt_Example函数中，通过LOS_TaskCreate函数创建Iot_Mqtt_Thread线程。

```c
void Iot_Mqtt_Example()
{
    unsigned int threadID;
    unsigned int ret = LOS_OK;

    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)Iot_Mqtt_Thread;
    task.uwStackSize = 10240;
    task.pcName = IOT_MQTT_EXAMPLE;
    task.usTaskPrio = 6;
    ret = LOS_TaskCreate(threadID, &task);
    if (LOS_OK != ret)
    {
        printf("Falied to create %s\n", IOT_MQTT_EXAMPLE);
        return;
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\samples`路径下 BUILD.gn 文件，指定 `iot_mqtt` 参与编译。

```r
"iot_mqtt",
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志。

```

```
