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
#ifndef CRC_16_H
#define CRC_16_H

/***************************************************************
* 函数名称: calc_crc16
* 说    明: crc16计算
* 参    数: uint8_t *frame 需要校验的数据
           uint16_t flen  需要校验的长度
* 返 回 值: crc16结果
***************************************************************/
uint16_t calc_crc16(uint8_t *frame, uint16_t flen);




#endif
