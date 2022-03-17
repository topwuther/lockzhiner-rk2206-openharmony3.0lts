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

#include <stdbool.h>
#include "lz_hardware.h"
#include "stdint.h"
#include "rtdText.h"
#include "rtdUri.h"
#include "ndef.h"


/* 记录是否已经初始化 */
static unsigned char m_nfc_is_init = 0;

//////////////////////////////////////////////////////////////////////

/***************************************************************
 * 函数名称: nfc_store_uri_http
 * 说    明: 向NFC写入URI信息
 * 参    数:
 *      @position：信息标识
 *      @http：需要写入的网络地址
 * 返 回 值: 返回ture为成功，false为失败
 ***************************************************************/
bool nfc_store_uri_http(RecordPosEnu position, uint8_t *http)
{
    NDEFDataStr data;
    
    if (m_nfc_is_init == 0)
    {
        printf("%s, %s, %d: NFC is not init!\n", __FILE__, __func__, __LINE__);
        return 0;
    }
    
    prepareUrihttp(&data, position, http);
    return NT3HwriteRecord(&data);
}


/***************************************************************
 * 函数名称: nfc_store_text
 * 说    明: 向NFC写入txt信息
 * 参    数:
 *      @position：信息标识
 *      @http：需要写入的文本信息
 * 返 回 值: 返回ture为成功，false为失败
 ***************************************************************/
bool nfc_store_text(RecordPosEnu position, uint8_t *text)
{
    NDEFDataStr data;
    
    if (m_nfc_is_init == 0)
    {
        printf("%s, %s, %d: NFC is not init!\n", __FILE__, __func__, __LINE__);
        return 0;
    }
    
    prepareText(&data, position, text);
    return NT3HwriteRecord(&data);
}

/***************************************************************
 * 函数名称: nfc_init
 * 说    明: NFC初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int nfc_init(void)
{
    unsigned int ret = 0;
    uint32_t *pGrf = (uint32_t *)0x41050000U;
    uint32_t ulValue;
    
    if (m_nfc_is_init == 1)
    {
        printf("%s, %s, %d: Nfc readly init!\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }
    
    ret = NT3HI2cInit();
    if (ret != 0)
    {
        printf("%s, %s, %d: NT3HI2cInit failed!\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }
    
    m_nfc_is_init = 1;
    return 0;
}


/***************************************************************
 * 函数名称: nfc_deinit
 * 说    明: NFC销毁
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int nfc_deinit(void)
{
    m_nfc_is_init = 0;
    NT3HI2cDeInit();
    return 0;
}
