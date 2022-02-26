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

#ifndef HC_ERROR_H
#define HC_ERROR_H

enum {
    HAL_SUCCESS = 0,
    HAL_FAILED = -1,
    HAL_ERR_INVALID_PARAM = -2,
    HAL_ERR_INVALID_LEN = -3,
    HAL_ERR_BAD_ALLOC = -4,
    HAL_ERR_NULL_PTR = -5,
    HAL_ERR_MEMORY_COPY = -6,

    HAL_ERR_FILE_NOT_EXIST = -7,
    HAL_ERR_FILE = -8,

    HAL_ERR_JSON_FAILED = -9,
    HAL_ERR_JSON_GET = -10,
    HAL_ERR_JSON_ADD = -11,
    HAL_ERR_JSON_REPLACE = -12,
    HAL_ERR_JSON_DUPLICATE = -13,

    HAL_ERR_INIT_PARAM_SET_FAILED = -14,
    HAL_ERR_ADD_PARAM_FAILED = -15,
    HAL_ERR_GET_PARAM_FAILED = -16,
    HAL_ERR_BUILD_PARAM_SET_FAILED = -17,
    HAL_ERR_FRESH_PARAM_SET_FAILED = -18,
    HAL_ERR_INIT_FAILED = -19,
};

#endif
