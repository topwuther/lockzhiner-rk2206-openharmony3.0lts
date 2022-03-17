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
#include "e53_intelligent_street_lamp.h"

#define MSG_QUEUE_LENGTH                16
#define BUFFER_LEN                      50

#define CLIENT_ID                       "61c9cb99a61a2a029cca62b4_E53_ISL_0_0_2021122714"
#define USERNAME                        "61c9cb99a61a2a029cca62b4_E53_ISL"
#define PASSWORD                        "b4c9e79c2cc2e56d994f0d53f3ad6f9aa2f9b41dd4f39dfd75765dc84a1af9e9"

typedef struct
{
    int lum;
} isl_report_t;

typedef struct
{
    en_msg_type_t msg_type;
    isl_report_t report;
} isl_msg_t;

static unsigned int m_isl_msg_queue;

/***************************************************************
* 函数名称: isl_deal_report_msg
* 说    明: 上报智慧路灯模块数据到华为云上
* 参    数: report：智慧路灯上报消息指针
* 返 回 值: 无
***************************************************************/
void isl_deal_report_msg(isl_report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t Lum;

    service.event_time = NULL;
    service.service_id = "智慧路灯";
    service.service_property = &Lum;
    service.nxt = NULL;

    Lum.key = "亮度";
    Lum.value = &report->lum;
    Lum.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    Lum.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
    return;
}

/***************************************************************
* 函数名称: iot_cloud_isl_thread
* 说    明: 华为云消息线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int iot_cloud_isl_thread()
{
    isl_msg_t *app_msg = NULL;
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
        ret = LOS_QueueRead(m_isl_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            app_msg = addr;
            switch (app_msg->msg_type)
            {
                case en_msg_report:
                    isl_deal_report_msg(&app_msg->report);
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
* 函数名称: e53_isl_tread
* 说    明: E53智慧路灯线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_isl_tread()
{
    float lum = 0;
    isl_msg_t *app_msg = NULL;


    e53_isl_init();

    while (1)
    {
        lum = e53_isl_read_data();

        printf("luminance value is %.2f\n", lum);

        if (lum < 20)
        {
            isl_light_set_status(ON);
        }
        else
        {
            isl_light_set_status(OFF);
        }

        app_msg = malloc(sizeof(isl_msg_t));
        if (app_msg != NULL)
        {
            app_msg->msg_type = en_msg_report;
            app_msg->report.lum = (int)lum;
            if (LOS_QueueWrite(m_isl_msg_queue, (void *)app_msg, sizeof(isl_msg_t), LOS_WAIT_FOREVER) != LOS_OK)
            {
                printf("%s LOS_QueueWrite fail\n", __func__);
                free(app_msg);
            }
        }
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: iot_cloud_isl_example
* 说    明: 华为云智慧路灯例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void iot_cloud_isl_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_isl_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_cloud_isl_thread;
    task1.uwStackSize = 10240;
    task1.pcName = "iot_cloud_isl_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create iot_cloud_isl_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_isl_tread;
    task2.uwStackSize = 2048;
    task2.pcName = "e53_isl_tread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_isl_tread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(iot_cloud_isl_example);
