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
#include "e53_intelligent_vehicle_01.h"

#define MSG_QUEUE_LENGTH                16
#define BUFFER_LEN                      50

#define CLIENT_ID                       "61e7f618de9933029be38412_E53_IV_0_0_2022021902"
#define USERNAME                        "61e7f618de9933029be38412_E53_IV"
#define PASSWORD                        "d29289d60c2decc3f8ff79759f47e6102103acdbf7b724e90a8c529786c26598"

typedef struct
{
    int distance_meter;
} iv_report_t;

typedef struct
{
    en_msg_type_t msg_type;
    iv_report_t report;
} iv_msg_t;

static unsigned int m_iv_msg_queue;

/***************************************************************
* 函数名称: iv_deal_report_msg
* 说    明: 上报智慧车载模块数据到华为云上
* 参    数: report：智慧车载上报消息指针
* 返 回 值: 无
***************************************************************/
void iv_deal_report_msg(iv_report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t distance;

    service.event_time = NULL;
    service.service_id = "智慧车载";
    service.service_property = &distance;
    service.nxt = NULL;

    distance.key = "距离";
    distance.value = &report->distance_meter;
    distance.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    distance.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
    return;
}

/***************************************************************
* 函数名称: iot_cloud_iv_thread
* 说    明: 华为云消息线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

/***************************************************************
* 函数名称: e53_iv_thread
* 说    明: E53智慧车载线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

/***************************************************************
* 函数名称: iot_cloud_iv_example
* 说    明: 华为云智慧车载例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

APP_FEATURE_INIT(iot_cloud_iv_example);
