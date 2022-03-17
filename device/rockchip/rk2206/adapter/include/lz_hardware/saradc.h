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
 * @file saradc.h
 */

#ifndef LZ_HARDWARE_SARADC_H
#define LZ_HARDWARE_SARADC_H

/**
 * @brief Initializes an SARADC device.
 *
 *
 *
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the SARADC device is initialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
int LzSaradcInit(void);

/**
 * @brief Deinitializes an SARADC device.
 *
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the SARADC device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
int LzSaradcDeinit(void);

/**
 * @brief transfer data with an SARADC device.
 *
 *
 *
 * @param ch Indicates the SARADC channel number.
 * @param val Indicates the pointer to save the adc value.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the SARADC device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
int LzSaradcReadValue(unsigned int chn, unsigned int *val);

#endif
/** @} */
