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

#include "rtdText.h"
#include "rtdTypes.h"
#include <string.h>

uint8_t addRtdText(RtdTextTypeStr *typeStr)
{
    typeStr->status = 0x2;
    typeStr->language[0] = 'e';
    typeStr->language[1] = 'n';
    
    return 3;
}

void prepareText(NDEFDataStr *data, RecordPosEnu position, uint8_t *text)
{
    data->ndefPosition = position;
    data->rtdType = RTD_TEXT;
    data->rtdPayload = text;
    data->rtdPayloadlength = strlen((const char *)text);
}
