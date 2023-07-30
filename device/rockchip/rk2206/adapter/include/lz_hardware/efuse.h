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

/**
 * @addtogroup Lockzhiner
 *
 * @file efuse.h
 *
 */
#ifndef LZ_HARDWARE_EFUSE_H
#define LZ_HARDWARE_EFUSE_H

/**
 * @brief Read data from an efuse device.
 *
 * @param offset .
 * @param len Indicates the bytes of the data to read.
 * @param data Indicates the data buffer to read.
 * @return Returns {@link > 0 read byte number}  successfully;
 * returns {@link <= 0} failed
 */

uint8_t LzEfuseRead(uint32_t offset, uint32_t len, uint8_t *val);

#endif