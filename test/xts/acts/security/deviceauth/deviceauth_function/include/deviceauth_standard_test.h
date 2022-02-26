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

#ifndef DEVICEAUTH_STANDARD_TEST_H
#define DEVICEAUTH_STANDARD_TEST_H
#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "device_auth.h"


const char *TEST_APP_NAME = "TestApp";
const int32_t TEMP_REQUEST_ID = 123;
const int32_t BUFFER_SIZE = 2048;
const char *CLIENT_AUTH_ID = "3C58C27533D8";
const char *SERVER_AUTH_ID = "CAF34E13190CBA510AA8DABB70CDFF8E9F623656DED400EF0D4CFD9E88FD6202";
const char *PIN_CODE = "123456";
const int32_t CLIENT_REQUEST_ID = 123;
const int32_t SERVER_REQUEST_ID = 345;
const int32_t STR_BUFF_SZ_MIN = 32;
const int32_t STR_BUFF_SZ_NORMAL = 128;
const int32_t MAX_GROUP_NUMBER = 101;


typedef enum {
    ON_REQUEST = 1,
    ON_ERROR = 2,
    ON_FINISH = 3,
    ON_SESSION_KEY_RETURNED = 4,
    ON_TRANSMIT = 5
} CallbackType;

#endif

