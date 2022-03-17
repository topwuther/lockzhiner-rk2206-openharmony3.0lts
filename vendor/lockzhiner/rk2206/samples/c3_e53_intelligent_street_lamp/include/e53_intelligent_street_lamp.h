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
#ifndef __E53_INTELLIGENT_STREET_LAMP_H__
#define __E53_INTELLIGENT_STREET_LAMP_H__

#include "lz_hardware.h"

typedef enum
{
    OFF = 0,
    ON
} SWITCH_STATUS_ENUM;

void e53_isl_init(void);
float e53_isl_read_data();
void isl_light_set_status(SWITCH_STATUS_ENUM status);

#endif/*__E53_INTELLIGENT_STREET_LAMP_H__*/
