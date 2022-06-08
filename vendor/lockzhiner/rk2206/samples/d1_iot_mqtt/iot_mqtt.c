/*
 * Copyright (c) 2021 FuZhou LOCKZHINER Electronic Co., Ltd.
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

#include <cJSON.h>
#include "ohos_init.h"
#include "los_task.h"
#include "oc_mqtt.h"
#include "MQTTClient.h"

#define IOT_MQTT_EXAMPLE                        "iot_mqtt"
#define MQTT_SERVER_IP                          "192.168.0.106"
#define MQTT_SERVER_PORT                        1883

static unsigned char sendBuf[1000];
static unsigned char readBuf[1000];

static Network network;

/***************************************************************
* 函数名称: message_receive
* 说    明: 接收MQTT数据
* 参    数: data:接收的数据指针
* 返 回 值: 无
***************************************************************/
void message_receive(MessageData* data)
{
    printf("message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
        data->message->payloadlen, data->message->payload);
}

/***************************************************************
* 函数名称: iot_mqtt_thread
* 说    明: iot mqtt线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

/***************************************************************
* 函数名称: iot_mqtt_example
* 说    明: iot mqtt物联网协议例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

APP_FEATURE_INIT(iot_mqtt_example);
