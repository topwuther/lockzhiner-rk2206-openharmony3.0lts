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
#include "e53_body_induction.h"

#define FLAGS_MASK                      0x00000001U

static PEVENT_CB_S m_bi_event;

#define MSG_QUEUE_LENGTH                16
#define BUFFER_LEN                      50

#define CLIENT_ID                       "61e7f045de9933029be3831f_E53_BI_0_0_2022021011"
#define USERNAME                        "61e7f045de9933029be3831f_E53_BI"
#define PASSWORD                        "2eb8f50fc3a563601d9d9463c186bd7d2cb477357d781e97b50834c8c079cc8a"

typedef struct
{
    int haveBody;
} bi_report_t;

typedef struct
{
    en_msg_type_t msg_type;
    bi_report_t report;
} bi_msg_t;

static unsigned int m_bi_msg_queue;

/***************************************************************
* 函数名称: bi_deal_report_msg
* 说    明: 上报人体感应模块数据到华为云上
* 参    数: report：人体感应上报消息指针
* 返 回 值: 无
***************************************************************/
void bi_deal_report_msg(bi_report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t haveBody;

    service.event_time = NULL;
    service.service_id = "人体感应";
    service.service_property = &haveBody;
    service.nxt = NULL;

    haveBody.key = "人体感应";
    haveBody.value = report->haveBody?"是":"否";
    haveBody.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    haveBody.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
    return;
}

/***************************************************************
* 函数名称: iot_cloud_bi_thread
* 说    明: 华为云消息线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

/***************************************************************
* 函数名称: e53_bi_thread
* 说    明: 人体感应模块线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

/***************************************************************
* 函数名称: iot_cloud_bi_example
* 说    明: 华为云人体感应例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
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

APP_FEATURE_INIT(iot_cloud_bi_example);
