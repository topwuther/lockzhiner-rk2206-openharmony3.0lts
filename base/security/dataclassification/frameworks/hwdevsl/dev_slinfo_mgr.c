/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "dev_slinfo_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEVSL_API __attribute__ ((visibility ("default")))

DEVSL_API int32_t DEVSL_OnStart(int32_t maxDevNum)
{
    return DEVSL_SUCCESS;
}

DEVSL_API void DEVSL_ToFinish(void)
{
    return;
}

DEVSL_API int32_t DEVSL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo)
{
    *levelInfo = DATA_SEC_LEVEL3;
    return DEVSL_SUCCESS;
}

DEVSL_API int32_t DEVSL_GetLocalCertData(uint8_t *buff, uint32_t bufSz, uint32_t *dataLen)
{
    buff[0] = 0;
    *dataLen = 0;
    return DEVSL_SUCCESS;
}
#ifdef __cplusplus
}
#endif

