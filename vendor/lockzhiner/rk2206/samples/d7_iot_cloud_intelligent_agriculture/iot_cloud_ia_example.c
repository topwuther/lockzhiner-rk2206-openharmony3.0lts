/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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
#include "e53_intelligent_agriculture.h"

#define MSG_QUEUE_LENGTH                16
#define BUFFER_LEN                      50

#define CLIENT_ID                       "61c68f24078a93029b83ce27_E53_IA_0_0_2022020914"
#define USERNAME                        "61c68f24078a93029b83ce27_E53_IA"
#define PASSWORD                        "f65f9721c8dbe96b07df7f273f74e2e25260f739c41e6f0076e78a7f4ff1bbed"

typedef struct
{
    int lum;
    int temp;
    int hum;
} ia_report_t;

typedef struct
{
    en_msg_type_t msg_type;
    cmd_t cmd;
    ia_report_t report;
} ia_msg_t;

typedef struct
{
    int connected;
    int led;
    int motor;
} ia_status_t;

static unsigned int m_ia_MsgQueue;
static ia_status_t m_app_status;

/***************************************************************
* 函数名称: ia_deal_report_msg
* 说    明: 上报智慧农业模块数据到华为云上
* 参    数: report：智慧农业上报消息指针
* 返 回 值: 无
***************************************************************/
void ia_deal_report_msg(ia_report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t temperature;
    oc_mqtt_profile_kv_t humidity;
    oc_mqtt_profile_kv_t luminance;
    oc_mqtt_profile_kv_t led;
    oc_mqtt_profile_kv_t motor;

    service.event_time = NULL;
    service.service_id = "智慧农业";
    service.service_property = &temperature;
    service.nxt = NULL;

    temperature.key = "温度";
    temperature.value = &report->temp;
    temperature.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    temperature.nxt = &humidity;

    humidity.key = "湿度";
    humidity.value = &report->hum;
    humidity.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    humidity.nxt = &luminance;

    luminance.key = "亮度";
    luminance.value = &report->lum;
    luminance.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    luminance.nxt = &led;

    led.key = "紫光灯状态";
    led.value = m_app_status.led ? "开" : "关";
    led.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    led.nxt = &motor;

    motor.key = "电机状态";
    motor.value = m_app_status.motor ? "开" : "关";
    motor.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    motor.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
}

/***************************************************************
* 函数名称: ia_cmd_response_callback
* 说    明: 华为云命令处理回调函数
* 参    数: recv_data：接收的数据指针
*           recv_size：接收数据的大小
*           resp_data：响应数据的指针
*           resp_size：响应数据的大小
* 返 回 值: 无
***************************************************************/
void ia_cmd_response_callback(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
{
    ia_msg_t *app_msg = NULL;
    int ret = 0;

    app_msg = malloc(sizeof(ia_msg_t));
    if (app_msg == NULL)
    {
        printf("malloc msg fail");
        return;
    }
    app_msg->msg_type = en_msg_cmd;
    app_msg->cmd.payload = (char *)recv_data;

    printf("recv data is %.*s", recv_size, recv_data);
    if (LOS_OK != LOS_QueueWrite(m_ia_MsgQueue, (void *)app_msg, sizeof(ia_msg_t), 0))
    {
        printf("%s LOS_QueueWrite fail\n", __func__);
        free(recv_data);
        free(app_msg);
    }
    *resp_data = NULL;
    *resp_size = 0;
}

/***************************************************************
* 函数名称: ia_deal_cmd_msg
* 说    明: 处理华为云下发的命令
* 参    数: cmd：华为云下发的命令结构体指针
* 返 回 值: 无
***************************************************************/
void ia_deal_cmd_msg(cmd_t *cmd)
{
    cJSON *obj_root;
    cJSON *obj_cmdname;
    cJSON *obj_paras;
    cJSON *obj_para;
    int cmdret = 1;
    oc_mqtt_profile_cmdresp_t cmdresp;

    obj_root = cJSON_Parse(cmd->payload);
    if (obj_root == NULL)
    {
        goto EXIT_JSONPARSE;
    }

    obj_cmdname = cJSON_GetObjectItem(obj_root, "command_name");
    if (obj_cmdname == NULL)
    {
        goto EXIT;
    }
    if (0 == strcmp(cJSON_GetStringValue(obj_cmdname), "紫光灯控制"))
    {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (obj_paras == NULL)
        {
            goto EXIT;
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Light");
        if (obj_para == NULL)
        {
            goto EXIT;
        }
        if (0 == strcmp(cJSON_GetStringValue(obj_para), "ON"))
        {
            m_app_status.led = 1;
            light_set(ON);
            printf("Light On\n");
        }
        else
        {
            m_app_status.led = 0;
            light_set(OFF);
            printf("Light Off\n");
        }
        cmdret = 0;
    }
    else if (0 == strcmp(cJSON_GetStringValue(obj_cmdname), "电机控制"))
    {
        obj_paras = cJSON_GetObjectItem(obj_root, "Paras");
        if (obj_paras == NULL)
        {
            goto EXIT;
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Motor");
        if (obj_para == NULL)
        {
            goto EXIT;
        }
        if (0 == strcmp(cJSON_GetStringValue(obj_para), "ON"))
        {
            m_app_status.motor = 1;
            motor_set_status(ON);
            printf("Motor On\n");
        }
        else
        {
            m_app_status.motor = 0;
            motor_set_status(OFF);
            printf("Motor Off\n");
        }
        cmdret = 0;
    }

EXIT:
    cJSON_Delete(obj_root);
EXIT_JSONPARSE:
    cmdresp.paras = NULL;
    cmdresp.request_id = cmd->request_id;
    cmdresp.ret_code = cmdret;
    cmdresp.ret_name = NULL;

    oc_mqtt_profile_cmdresp(NULL, &cmdresp);
}

/***************************************************************
* 函数名称: iot_cloud_ia_thread
* 说    明: 华为云智慧农业线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void iot_cloud_ia_thread()
{
    ia_msg_t *app_msg = NULL;
    unsigned int addr;
    int ret;

    SetWifiModeOn();

    device_info_init(CLIENT_ID, USERNAME, PASSWORD);
    ret = oc_mqtt_init();
    if (ret != LOS_OK)
    {
        printf("oc_mqtt_init fail ret:%d\n", ret);
    }
    oc_set_cmd_rsp_cb(ia_cmd_response_callback);

    while (1)
    {
        ret = LOS_QueueRead(m_ia_MsgQueue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            app_msg = addr;
            switch (app_msg->msg_type)
            {
                case en_msg_cmd:
                    ia_deal_cmd_msg(&app_msg->cmd);
                    break;
                case en_msg_report:
                    ia_deal_report_msg(&app_msg->report);
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

/***************************************************************
* 函数名称: e53_ia_thread
* 说    明: E53智慧农业线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_ia_thread()
{
    ia_msg_t *app_msg = NULL;
    e53_ia_data_t data;

    e53_ia_init();

    while (1)
    {
        e53_ia_read_data(&data);
        printf("Luminance:%.2f temperature:%.2f humidity:%.2f\n", data.luminance, data.temperature, data.humidity);

        app_msg = malloc(sizeof(ia_msg_t));
        if (app_msg != NULL)
        {
            app_msg->msg_type = en_msg_report;
            app_msg->report.hum = (int)data.humidity;
            app_msg->report.lum = (int)data.luminance;
            app_msg->report.temp = (int)data.temperature;
            if (LOS_QueueWrite(m_ia_MsgQueue, (void *)app_msg, sizeof(ia_msg_t), LOS_WAIT_FOREVER) != LOS_OK)
            {
                printf("%s LOS_QueueWrite fail\n", __func__);
                free(app_msg);
            }
        }
        LOS_Msleep(5000);
    }
}

/***************************************************************
* 函数名称: iot_cloud_ia_example
* 说    明: 华为云智慧农业例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void iot_cloud_ia_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_ia_MsgQueue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_cloud_ia_thread;
    task1.uwStackSize = 10240;
    task1.pcName = "iot_cloud_ia_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create iot_cloud_ia_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_ia_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "e53_ia_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_ia_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(iot_cloud_ia_example);
