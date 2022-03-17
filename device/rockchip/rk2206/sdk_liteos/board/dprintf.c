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

#include <stdarg.h>
#include <stdio.h>
#include "lz_hardware.h"

#define DEBUG_PORT              1

int printf(char const  *fmt, ...)
{
    va_list ap;
    char buffer[256];

    va_start(ap, fmt);
    vsnprintf(buffer, 256, fmt, ap);
    DebugWrite(DEBUG_PORT, (const unsigned char *)buffer, strlen(buffer));
    DebugPutc(DEBUG_PORT, '\r');
    va_end(ap);
    return 0;
}

