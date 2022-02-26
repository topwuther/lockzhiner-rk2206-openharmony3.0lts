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

#include "deviceauth_test_mock.h"
#include <cstring>
#include "securec.h"

static bool g_testForClient = false;

void SetClient(bool tag)
{
    g_testForClient = tag;
}

bool GetClient()
{
    return g_testForClient;
}

int32_t HcGetUdid(uint8_t *udid, int32_t udidLen)
{
    const char *clientUdid = "D6350E39AD8F11963C181BEEDC11AC85158E04466B68F1F4E6D895237E0FE81C";
    const char *serverUdid = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00";
    if (g_testForClient) {
        (void)memcpy_s(udid, udidLen - 1, clientUdid, strlen(clientUdid));
        return 0;
    }
    (void)memcpy_s(udid, udidLen - 1, serverUdid, strlen(serverUdid));
    return 0;
}

const char *GetStoragePath()
{
    return "/data/data/deviceauth/hcgroup.dat";
}
