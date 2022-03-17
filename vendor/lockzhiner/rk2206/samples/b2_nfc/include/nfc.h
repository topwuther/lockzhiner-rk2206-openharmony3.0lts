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

#ifndef _NFC_H_
#define _NFC_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* 信息标记 */
typedef enum {
    NDEFFirstPos, /* 起始信息标记 */
    NDEFMiddlePos, /* 中间信息标记 */
    NDEFLastPos /* 结束信息标记 */
} RecordPosEnu;

/***************************************************************
 * 函数名称: nfc_store_uri_http
 * 说    明: 向NFC写入URI信息
 * 参    数:
 *      @position：信息标识
 *      @http：需要写入的网络地址
 * 返 回 值: 返回ture为成功，false为失败
 ***************************************************************/
bool nfc_store_uri_http(RecordPosEnu position, uint8_t *http);


/***************************************************************
 * 函数名称: nfc_store_text
 * 说    明: 向NFC写入txt信息
 * 参    数:
 *      @position：信息标识
 *      @http：需要写入的文本信息
 * 返 回 值: 返回ture为成功，false为失败
 ***************************************************************/
bool nfc_store_text(RecordPosEnu position, uint8_t *text);


/***************************************************************
 * 函数名称: nfc_init
 * 说    明: NFC初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int nfc_init(void);


/***************************************************************
 * 函数名称: nfc_deinit
 * 说    明: NFC销毁
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int nfc_deinit(void);

#endif