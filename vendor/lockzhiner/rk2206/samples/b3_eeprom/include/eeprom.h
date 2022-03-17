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
#ifndef _EEPROM_H_
#define _EEPROM_H_

/***************************************************************
* 函数名称: eeprom_init
* 说    明: EEPROM初始化
* 参    数: 无
* 返 回 值: 0为成功，反之失败
***************************************************************/
unsigned int eeprom_init();

/***************************************************************
* 函数名称: eeprom_deinit
* 说    明: EEPROM退出
* 参    数: 无
* 返 回 值: 0为成功，反之失败
***************************************************************/
unsigned int eeprom_deinit();

/***************************************************************
* 函数名称: eeprom_get_blocksize
* 说    明: EEPROM获取页大小
* 参    数: 无
* 返 回 值: 返回页大小
***************************************************************/
unsigned int eeprom_get_blocksize();

/***************************************************************
* 函数名称: eeprom_readbyte
* 说    明: EEPROM读一个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 存放EERPOM的数据指针
* 返 回 值: 返回读取字节的长度，反之为错误
***************************************************************/
unsigned int eeprom_readbyte(unsigned int addr, unsigned char *data);

/***************************************************************
* 函数名称: eeprom_writebyte
* 说    明: EEPROM写一个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 写ERPOM的数据
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_writebyte(unsigned int addr, unsigned char data);

/***************************************************************
* 函数名称: eeprom_writepage
* 说    明: EEPROM写1个页字节
* 参    数: 
*           @addr: EEPROM存储地址，必须是页地址
*           @data: 写ERPOM的数据指针
*           @data_len: 写EEPROM数据的长度，必须是小于或等于1个页大小
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_writepage(unsigned int addr, unsigned char *data, unsigned int data_len);

/***************************************************************
* 函数名称: eeprom_read
* 说    明: EEPROM读多个字节
* 参    数: 
*           @addr: EERPOM存储地址
*           @data: 存放EERPOM的数据指针
*           @data_len: 读取EERPOM数据的长度
* 返 回 值: 返回读取字节的长度，反之为错误
***************************************************************/
unsigned int eeprom_read(unsigned int addr, unsigned char *data, unsigned int data_len);

/***************************************************************
* 函数名称: eeprom_write
* 说    明: EEPROM写多个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 写ERPOM的数据指针
*           @data_len: 写EEPROM数据的长度
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_write(unsigned int addr, unsigned char *data, unsigned int data_len);


#endif /* _EEPROM_H_ */
/** @} */
