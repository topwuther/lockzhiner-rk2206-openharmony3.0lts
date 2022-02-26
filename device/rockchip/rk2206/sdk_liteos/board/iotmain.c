/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Copyright (c) 2021 Lockzhiner Electronics Co., Ltd.
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

#include "lz_hardware.h"

#define IOT_TAG      "IOT"


static void IotProcess(void *arg)
{
    uint32_t ret = LZ_HARDWARE_SUCCESS;

    LZ_HARDWARE_LOGD(IOT_TAG, "%s: start ....", __func__);

    while (1)
    {
        //printf("%s: sleep 5 sec!\n", __func__);
        LOS_Msleep(5000);
    }
}

void IotInit(void)
{
    unsigned int threadID;

    LZ_HARDWARE_LOGD(IOT_TAG, "%s: start ....", __func__);

    CreateThread(&threadID, IotProcess, NULL, "iot process");
}
