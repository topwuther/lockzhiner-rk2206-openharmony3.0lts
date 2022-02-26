/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "module_common.h"
#include "common_defs.h"
#include "hc_log.h"
#include "hc_types.h"

int32_t InitSingleParam(Uint8Buff *param, uint32_t len)
{
    if (param->val != NULL) {
        (void)memset_s(param->val, param->length, 0, param->length);
        HcFree(param->val);
        param->val = NULL;
        param->length = 0;
    }

    param->length = len;
    param->val = (uint8_t *)HcMalloc(param->length, 0);
    if (param->val == NULL) {
        LOGE("Malloc for param failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    return HC_SUCCESS;
}