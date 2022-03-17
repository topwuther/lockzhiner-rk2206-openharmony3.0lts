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
#include "e53_gesture_sensor.h"

#define MSG_QUEUE_LENGTH                16
#define BUFFER_LEN                      50

#define CLIENT_ID                       "61c69349078a93029b83ceff_E53_GS_0_0_2022020914"
#define USERNAME                        "61c69349078a93029b83ceff_E53_GS"
#define PASSWORD                        "f65f9721c8dbe96b07df7f273f74e2e25260f739c41e6f0076e78a7f4ff1bbed"

typedef struct
{
    int up;
    int down;
    int left;
    int right;
    int forward;
    int backward;
    int clockwise;
    int anti_clockwise;
    int wave;
} gs_report_t;

typedef struct
{
    en_msg_type_t msg_type;
    gs_report_t report;
} gs_msg_t;

static unsigned int m_gs_msg_queue;
static gs_report_t m_gs_report;

/***************************************************************
* 函数名称: gs_deal_report_msg
* 说    明: 上报手势感应模块数据到华为云上
* 参    数: report：手势感应上报消息指针
* 返 回 值: 无
***************************************************************/
void gs_deal_report_msg(gs_report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t up;
    oc_mqtt_profile_kv_t down;
    oc_mqtt_profile_kv_t left;
    oc_mqtt_profile_kv_t right;
    oc_mqtt_profile_kv_t forward;
    oc_mqtt_profile_kv_t backward;
    oc_mqtt_profile_kv_t clockwise;
    oc_mqtt_profile_kv_t anti_clockwise;
    oc_mqtt_profile_kv_t wave;

    service.event_time = NULL;
    service.service_id = "手势感应";
    service.service_property = &up;
    service.nxt = NULL;

    up.key = "向上";
    up.value = report->up?"是":"否";
    up.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    up.nxt = &down;

    down.key = "向下";
    down.value = report->down?"是":"否";
    down.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    down.nxt = &left;

    left.key = "向左";
    left.value = report->left?"是":"否";
    left.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    left.nxt = &right;

    right.key = "向右";
    right.value = report->right?"是":"否";
    right.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    right.nxt = &forward;

    forward.key = "向前";
    forward.value = report->forward?"是":"否";
    forward.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    forward.nxt = &backward;

    backward.key = "向后";
    backward.value = report->backward?"是":"否";
    backward.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    backward.nxt = &clockwise;

    clockwise.key = "顺时针转动";
    clockwise.value = report->clockwise?"是":"否";
    clockwise.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    clockwise.nxt = &anti_clockwise;

    anti_clockwise.key = "逆时针转动";
    anti_clockwise.value = report->anti_clockwise?"是":"否";
    anti_clockwise.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    anti_clockwise.nxt = &wave;

    wave.key = "挥动";
    wave.value = report->wave?"是":"否";
    wave.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    wave.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
}

/***************************************************************
* 函数名称: iot_cloud_gs_thread
* 说    明: 华为云消息线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int iot_cloud_gs_thread()
{
    gs_msg_t *app_msg = NULL;
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
        ret = LOS_QueueRead(m_gs_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            app_msg = addr;
            switch (app_msg->msg_type)
            {
                case en_msg_report:
                    gs_deal_report_msg(&app_msg->report);
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
* 函数名称: e53_gs_thread
* 说    明: E53手势感应线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_gs_thread()
{
    unsigned int ret = 0;
    unsigned short flag = 0;
    gs_msg_t *app_msg;

    e53_gs_init();

    while (1)
    {
        ret = e53_gs_get_gesture_state(&flag);
        if (ret != 0)
        {
            printf("Get Gesture Statu: 0x%x\n", flag);
                
            e53_gs_led_up_set((flag & GES_UP) ? (1) : (0));
            m_gs_report.up = (flag & GES_UP) ? (1) : (0);

            e53_gs_led_down_set((flag & GES_DOWM) ? (1) : (0));
            m_gs_report.down= (flag & GES_DOWM) ? (1) : (0);
            
            e53_gs_led_left_set((flag & GES_LEFT) ? (1) : (0));
            m_gs_report.left = (flag & GES_LEFT) ? (1) : (0);
            
            e53_gs_led_right_set((flag & GES_RIGHT) ? (1) : (0));
            m_gs_report.right = (flag & GES_RIGHT) ? (1) : (0);
            
            e53_gs_led_forward_set((flag & GES_FORWARD) ? (1) : (0));
            m_gs_report.forward = (flag & GES_FORWARD) ? (1) : (0);
            
            e53_gs_led_backward_set((flag & GES_BACKWARD) ? (1) : (0));
            m_gs_report.backward = (flag & GES_BACKWARD) ? (1) : (0);
            
            e53_gs_led_cw_set((flag & GES_CLOCKWISE) ? (1) : (0));
            m_gs_report.clockwise = (flag & GES_CLOCKWISE) ? (1) : (0);
            
            e53_gs_led_ccw_set((flag & GES_COUNT_CLOCKWISE) ? (1) : (0));
            m_gs_report.anti_clockwise = (flag & GES_COUNT_CLOCKWISE) ? (1) : (0);
            
            e53_gs_led_wave_set((flag & GES_WAVE) ? (1) : (0));
            m_gs_report.wave = (flag & GES_WAVE) ? (1) : (0);

            app_msg = malloc(sizeof(gs_msg_t));
            if (app_msg != NULL)
            {
                app_msg->msg_type = en_msg_report;
                memcpy(&app_msg->report, &m_gs_report, sizeof(gs_report_t));
                if (LOS_QueueWrite(m_gs_msg_queue, (void *)app_msg, sizeof(gs_msg_t), LOS_WAIT_FOREVER) != LOS_OK)
                {
                    printf("%s LOS_QueueWrite fail\n", __func__);
                    free(app_msg);
                }
            }
        }
        else
        {
            /* 如果没有数据，则多等待 */
            LOS_Msleep(100);
        }
    }
}

/***************************************************************
* 函数名称: iot_cloud_gs_example
* 说    明: 华为云手势感应例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void iot_cloud_gs_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_gs_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)iot_cloud_gs_thread;
    task1.uwStackSize = 10240;
    task1.pcName = "iot_cloud_gs_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create iot_cloud_gs_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_gs_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "e53_gs_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_gs_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(iot_cloud_gs_example);
