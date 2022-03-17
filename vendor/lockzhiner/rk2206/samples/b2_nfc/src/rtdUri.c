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

#include "rtdUri.h"
#include <string.h>
#include "rtdTypes.h"

static RTDUriTypeStr uri;

uint8_t addRtdUriRecord(const NDEFDataStr *ndef, RTDUriTypeStr *uriType)
{
    uriType->type = ((RTDUriTypeStr*) ndef->specificRtdData)->type;
    
    return 1;
}

void prepareUrihttp(NDEFDataStr *data, RecordPosEnu position, uint8_t *text)
{
    data->ndefPosition = position;
    data->rtdType = RTD_URI;
    data->rtdPayload = text;
    data->rtdPayloadlength = strlen((const char *)text);
    
    uri.type = httpWWW;
    data->specificRtdData = &uri;
}
