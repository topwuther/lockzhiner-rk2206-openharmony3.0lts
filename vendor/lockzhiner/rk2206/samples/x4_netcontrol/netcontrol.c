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
#include <stdio.h>
#include <stdlib.h>
#include "config_uuid.h"
#include "mqttclient.h"
#include "stdbool.h"
#include "netcontrol.h"
#include "lz_hardware.h"

extern void sg90cycle(GpioID pin, uint8_t cycle);
extern float temperature;
extern float humidity;
extern float light;
extern bool flags[5];

static uint8_t MQTTMessageBuffer[512];

/***************************************************************
 * 函数名称: message_receive
 * 说    明: 接收MQTT数据
 * 参    数: data:接收的数据指针
 * 返 回 值: 无
 ***************************************************************/
void message_receive(MessageData *data)
{
    char *payload = data->message->payload;
    size_t payloadlen = data->message->payloadlen;
    memcpy(MQTTMessageBuffer, payload, sizeof(MQTTMessageBuffer));
    MQTTMessageBuffer[payloadlen] = 0x0;
    // printf("MQTTMessage:[%s]\n",MQTTMessageBuffer);
}

/***************************************************************
 * 函数名称: netcontrol_process
 * 说    明: iot mqtt线程
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void netcontrol_process()
{
    uint8_t topic[] = "/smarthouse";
    uint32_t ret;
    char payload[1000] = {0};
    MQTTClient client;
    uint8_t uuid[37];
    MQTTMessage message;
    message.qos = QOS2;
    message.retained = 0;
    /*WIFI连接*/
    while (get_wifi_info() != 0)
    {
        printf("Wait wifi\n");
        LOS_Msleep(1000);
    }
    while(get_uuid_info() != 0){
        printf("Wait uuid\n");
        LOS_Msleep(1000);
    }
    GetUUID(&uuid);
    client = Connect();
    while (1)
    {
        if (MQTTMessageBuffer[0] == 0)
        {
            message.payload = payload;
            message.payloadlen = strlen(payload);
            ret = MQTTPublish(&client, topic, &message);
            if (ret != SUCCESS)
            {
                NetworkDisconnect(&MQTTNetwork);
                MQTTDisconnect(&client);
                client = Connect();
            }
            payload[0] = 0;
            continue;
        }
        cJSON *message = cJSON_Parse(&MQTTMessageBuffer);
        uint8_t type = cJSON_GetObjectItem(message, "type")->valueint;
        // printf("type:%d\n",type);
        cJSON *json = cJSON_CreateObject();
        switch (type)
        {
        case TYPE_STATUS:
            cJSON_AddNumberToObject(json, "type", TYPE_STATUS_RET);
            cJSON_AddStringToObject(json, "uuid", &uuid);
            cJSON_AddNumberToObject(json, "temperature", temperature);
            cJSON_AddNumberToObject(json, "humidity", humidity);
            cJSON_AddNumberToObject(json, "light", light);
            cJSON_AddBoolToObject(json, "smoke",flags[0]);
            cJSON_AddBoolToObject(json, "flame",flags[1]);
            cJSON_AddBoolToObject(json, "rain",flags[2]);
            sprintf(payload, cJSON_Print(json));
            break;
        case TYPE_SET:
            Device device = cJSON_GetObjectItem(message, "device")->valueint;
            uint8_t value = cJSON_GetObjectItem(message,"value")->valueint;
            // printf("device:[%d],value:[%d]\n",device,value);
            switch (device)
            {
            case DEVICE_DOOR:
                if(value) sg90cycle(DOOR_PIN,90);
                else sg90cycle(DOOR_PIN,0);
                break;
            case DEVICE_GARAGE:
                if(value) sg90cycle(GARAGE_PIN,90);
                else sg90cycle(GARAGE_PIN,0);
                break;
            case DEVICE_LED:
                if(value) LzGpioSetVal(LIGHT,1);
                else LzGpioSetVal(LIGHT,0);
                break;
            case DEVICE_WINDOW:
                if(value) sg90cycle(WINDOW_PIN,90);
                else sg90cycle(WINDOW_PIN,0);
                break;
            }
            cJSON_AddNumberToObject(json,"type",TYPE_SET_RET);
            sprintf(payload, cJSON_Print(json));
        }
        MQTTMessageBuffer[0] = 0;
        LOS_Msleep(100);
    }
}

/***************************************************************
 * 函数名称: netcontrol
 * 说    明: iot mqtt物联网协议例程
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void netcontrol()
{
    unsigned int threadID;
    unsigned int ret = LOS_OK;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)netcontrol_process;
    task.uwStackSize = 10240;
    task.pcName = "netcontrol";
    task.usTaskPrio = 6;
    ret = LOS_TaskCreate(&threadID, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(netcontrol);
