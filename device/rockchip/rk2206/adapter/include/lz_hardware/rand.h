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
 * @file rand.h
 */

#ifndef LZ_HARDWARE_RAND_H
#define LZ_HARDWARE_RAND_H

/**
 * @brief rand: get hardware random data.
 *
 *
 *
 * @param data Indicates the buffer pointer to save random data.
 * @param len Indicates the length of string.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the random data is got successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int GetRandData(unsigned char *data, unsigned int len);

#endif
/** @} */
