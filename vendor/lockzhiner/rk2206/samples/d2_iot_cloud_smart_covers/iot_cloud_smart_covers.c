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
#include "e53_smart_covers.h"

#define MSG_QUEUE_LENGTH                16
#define BUFFER_LEN                      50

#define CLIENT_ID                       "61cd273b078a93029b84e9ce_E53_SC_0_0_2021123003"
#define USERNAME                        "61cd273b078a93029b84e9ce_E53_SC"
#define PASSWORD                        "53abbf23b0449011d2cefef2b198a398523bbacf8a58449c6002988d8b042037"

typedef struct
{
    int x;
    int y;
    int z;
    unsigned int tilt_status;
} sc_report_t;

typedef struct
{
    en_msg_type_t msg_type;
    sc_report_t report;
} sc_msg_t;

static unsigned int m_sc_msg_queue;

/***************************************************************
* 函数名称: sc_deal_report_msg
* 说    明: 上报智慧井盖模块数据到华为云上
* 参    数: report：智慧农业上报消息指针
* 返 回 值: 无
***************************************************************/
void sc_deal_report_msg(sc_report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t x;
    oc_mqtt_profile_kv_t y;
    oc_mqtt_profile_kv_t z;
    oc_mqtt_profile_kv_t tilt_status;

    service.event_time = NULL;
    service.service_id = "智慧井盖";
    service.service_property = &x;
    service.nxt = NULL;

    x.key = "X";
    x.value = &report->x;
    x.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    x.nxt = &y;

    y.key = "Y";
    y.value = &report->y;
    y.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    y.nxt = &z;

    z.key = "Z";
    z.value = &report->z;
    z.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    z.nxt = &tilt_status;

    tilt_status.key = "倾斜告警";
    tilt_status.value = report->tilt_status?"是":"否";
    tilt_status.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    tilt_status.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
    return;
}

/***************************************************************
* 函数名称: iot_cloud_sc_thread
* 说    明: 华为云消息线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int iot_cloud_sc_thread()
{
    sc_msg_t *app_msg = NULL;
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
        ret = LOS_QueueRead(m_sc_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            app_msg = addr;
            switch (app_msg->msg_type)
            {
                case en_msg_report:
                    sc_deal_report_msg(&app_msg->report);
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
* 函数名称: e53_sc_thread
* 说    明: E53智慧井盖线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_sc_thread()
{
    sc_msg_t *app_msg = NULL;
    e53_sc_data_t data;
    int x = 0, y = 0, z = 0;

    e53_sc_init();
    led_d1_set(OFF);
    led_d2_set(OFF);

    while (1)
    {
        e53_sc_read_data(&data);
        printf("x is  %d\n", (int)data.accel[0]);
        printf("y is  %d\n", (int)data.accel[1]);
        printf("z is  %d\n", (int)data.accel[2]);
        printf("init x:%d y:%d z:%d", x, y, z);
        if (x == 0 && y == 0 && z == 0)
        {
            x = (int)data.accel[0];
            y = (int)data.accel[1];
            z = (int)data.accel[2];
        }
        else
        {
            if ((x + DELTA) < data.accel[0] || (x - DELTA) > data.accel[0] ||
                (y + DELTA) < data.accel[1] || (y - DELTA) > data.accel[1] ||
                (z + DELTA) < data.accel[2] || (z - DELTA) > data.accel[2])
            {
                /*倾斜告警*/
                led_d1_set(OFF);
                led_d2_set(ON);
                data.tilt_status = 1;
                printf("tilt warning \nLED1 OFF LED2 On\n");
            }
            else
            {
                led_d1_set(ON);
                led_d2_set(OFF);
                data.tilt_status = 0;
                printf("normal \nLED1 ON LED2 OFF\n");
            }
        }
        app_msg = malloc(sizeof(sc_msg_t));
        if (app_msg != NULL)
        {
            app_msg->msg_type = en_msg_report;
            app_msg->report.x = (int)data.accel[0];
            app_msg->report.y = (int)data.accel[1];
            app_msg->report.z = (int)data.accel[2];
            app_msg->report.tilt_status = data.tilt_status;
            if (LOS_QueueWrite(m_sc_msg_queue, (void *)app_msg, sizeof(sc_msg_t), LOS_WAIT_FOREVER) != LOS_OK)
            {
                printf("%s LOS_QueueWrite fail\n", __func__);
                free(app_msg);
            }
        }
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: iot_cloud_sc_example
* 说    明: 华为云智慧井盖例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void iot_cloud_sc_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_sc_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_cloud_sc_thread;
    task1.uwStackSize = 10240;
    task1.pcName = "iot_cloud_sc_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create iot_cloud_sc_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_sc_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "e53_sc_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_sc_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(iot_cloud_sc_example);
