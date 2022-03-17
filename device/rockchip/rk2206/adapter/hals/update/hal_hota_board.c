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

#include "hal_hota_board.h"
//#include "hal_ota.h"

int HotaHalInit(void)
{
    //return HalOtaInit();
    return OHOS_SUCCESS;
}

int HotaHalDeInit(void)
{
    //return HalOtaDeinit();
    return OHOS_FAILURE;
}

int HotaHalGetUpdateIndex(unsigned int *index)
{
    //return HalOtaGetUpdateIndex(index);
    return OHOS_SUCCESS;
}
int HotaHalWrite(int partition, unsigned char *buffer, unsigned int offset, unsigned int bufLen)
{
    //return HalOtaWrite(partition, buffer, offset, bufLen);
    return OHOS_SUCCESS;
}

int HotaHalRead(int partition, unsigned int offset, unsigned int bufLen, unsigned char *buffer)
{
    //return HalOtaRead(partition, offset, bufLen, buffer);
    return OHOS_FAILURE;
}

int HotaHalSetBootSettings(void)
{
    //return HalOtaSetBootSettings();
    return OHOS_SUCCESS;
}

int HotaHalRestart(void)
{
    //return HalOtaRestart();
    return OHOS_SUCCESS;
}

int HotaHalRollback(void)
{
    //return HalOtaRollback();
    return OHOS_SUCCESS;
}

const ComponentTableInfo *HotaHalGetPartitionInfo()
{
    //return (const ComponentTableInfo *)HalOtaGetPartitionInfo();
    return NULL;
}

unsigned char *HotaHalGetPubKey(unsigned int *length)
{
    //return HalOtaGetPubKey(length);
    return NULL;
}

int HotaHalGetUpdateAbility(void)
{
    //return HalOtaGetUpdateAbility();
    return 0;
}

int HotaHalGetOtaPkgPath(char *path, int len)
{
    //return HalOtaGetOtaPkgPath(path, len);
    return 0;
}

int HotaHalIsDeviceCanReboot(void)
{
    //return HalOtaIsDeviceCanReboot();
    return 1;
}

int HotaHalGetMetaData(UpdateMetaData *metaData)
{
    //return HalOtaGetMetaData((HalUpdateMetaData *)metaData);
    return OHOS_SUCCESS;
}

int HotaHalSetMetaData(UpdateMetaData *metaData)
{
    //return HalOtaSetMetaData((HalUpdateMetaData *)metaData);
    return 0;
}

int HotaHalRebootAndCleanUserData(void)
{
    //return HalOtaRebootAndCleanUserData();
    return 0;
}

int HotaHalRebootAndCleanCache(void)
{
    //return HalOtaRebootAndCleanCache();
    return 0;
}

int HotaHalCheckVersionValid(const char *currentVersion, const char *pkgVersion, unsigned int pkgVersionLength)
{
    return 1;
}

