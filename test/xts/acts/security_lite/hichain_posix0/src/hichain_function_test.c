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

// #include "hichain_function_test.h"

#include "log.h"
#include "distribution.h"
#include "hichain.h"
#include "hctest.h"
#include "hks_client.h"
#include "hks_types.h"
#include <securec.h>
#include <sys/time.h>
#include <sys/time.h>

#define MAX_SEND_MESSAGE_BUFF 1024
#define PRINT(...)

LITE_TEST_SUIT(security, securityData, HichainFunctionTestSuite);

/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL HichainFunctionTestSuiteSetUp(void)
{
    int32_t status;
    status = hks_init();
    if (status != 0) {
        status = hks_refresh_key_info();
    }
    TEST_ASSERT_EQUAL_INT(0, status);
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL HichainFunctionTestSuiteTearDown(void)
{
    printf("-+++++++++++++++++++++++++++++++++++++++++++++-\n");
    return TRUE;
}

static char g_sendMessageBuf[MAX_SEND_MESSAGE_BUFF];
static struct hc_auth_id g_testClientAuthId = { sizeof("authClient") - 1, "authClient" };
static struct hc_auth_id g_testServerAuthId = { sizeof("authServer") - 1, "authServer" };
static struct hc_pin g_testPin = { sizeof("123456789012345") - 1, "123456789012345" };
static uint32_t g_sendLength = 0;
static int32_t g_serviceResult = 0;
static struct session_identity g_clientIdentity = {
    0,
    { sizeof("testClient") - 1, "testClient" },
    { sizeof("testClient") - 1, "testClient" },
    0
};

static struct session_identity g_serverIdentity = {
    0,
    { sizeof("testServer") - 1, "testServer" },
    { sizeof("testServer") - 1, "testServer" },
    0
};


void TransmitDoNothing(const struct session_identity *identity, const void *data, uint32_t length)
{
    if (memcpy_s(g_sendMessageBuf, MAX_SEND_MESSAGE_BUFF, data, length) != EOK) {
        return;
    }
    g_sendMessageBuf[length] = 0;
    PRINT("Send data: %s\n", g_sendMessageBuf);
    g_sendLength = length;
    identity = identity;
}

void GetProtocolParams(const struct session_identity *identity, int32_t operationCode, struct hc_pin *pin,
    struct operation_parameter *para)
{
    (void)identity;
    (void)operationCode;
    *pin = g_testPin;
    para->key_length = 32; // 传入128计算hkdf失败， 暂时改成32
    para->peer_auth_id = g_testClientAuthId;
    para->self_auth_id = g_testServerAuthId;
}

void SetSessionKey(const struct session_identity *identity, const struct hc_session_key *sessionKey)
{
    (void)identity;
    (void)sessionKey;
}

void SetServiceResult(const struct session_identity *identity, int32_t result)
{
    (void)identity;
    g_serviceResult = result;
}

int32_t ConfirmReceiveRequestOK(const struct session_identity *identity, int32_t operationCode)
{
    (void)identity;
    (void)operationCode;
    return 0;
}

void GetProtocolParamsClient(const struct session_identity *identity, int32_t operationCode, struct hc_pin *pin,
    struct operation_parameter *para)
{
    (void)identity;
    (void)operationCode;
    *pin = g_testPin;
    para->key_length = 32; // 传入128计算hkdf失败， 暂时改成32
    para->peer_auth_id = g_testServerAuthId;
    para->self_auth_id = g_testClientAuthId;
}

static void HcLogd(const char *tag, const char *func, const char *format, ...)
{
    printf("%s Debug: %s: ", tag, func);
    va_list args;
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void HcLogi(const char *tag, const char *func, const char *format, ...)
{
    printf("%s Info: %s: ", tag, func);
    va_list args;
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void HcLogw(const char *tag, const char *func, const char *format, ...)
{
    printf("%s Warning: %s: ", tag, func);
    va_list args;
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
    printf("\n");
}

static void HcLoge(const char *tag, const char *func, const char *format, ...)
{
	PrintTime();
    printf("%s Error: %s: ", tag, func);
    va_list args;
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
    printf("\n");
}

/* *
 * @tc.name: IntentParseUriToUri001
 * @tc.desc: Binding end key exchange
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: dingjinyan
 */
LITE_TEST_CASE(HichainFunctionTestSuite, HichainBindPeer, Function | MediumTest | Level1)
{
    struct log_func_group g = {HcLogd, HcLogi, HcLogw, HcLoge};
    registe_log(&g);

    /* 设置回调函数 */
    struct hc_call_back callBack = { TransmitDoNothing, GetProtocolParams, SetSessionKey, SetServiceResult,
        ConfirmReceiveRequestOK };
    
    struct hc_call_back callBackClient = { TransmitDoNothing, GetProtocolParamsClient, SetSessionKey, SetServiceResult,
        ConfirmReceiveRequestOK };

    /* 构造函数 */
    hc_handle server = get_instance(&g_serverIdentity, HC_ACCESSORY, &callBack);
    hc_handle client = get_instance(&g_clientIdentity, HC_CENTRE, &callBackClient);

    struct operation_parameter params = { g_testClientAuthId, g_testServerAuthId, 32 };
    int32_t ret = start_pake(client, &params);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, KEY_AGREEMENT_PROCESSING);

    struct uint8_buff startRequest = { (uint8_t *)g_sendMessageBuf, g_sendLength, g_sendLength };
    ret = receive_data(server, &startRequest);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, KEY_AGREEMENT_PROCESSING);

    struct uint8_buff startResponse = { (uint8_t *)g_sendMessageBuf, g_sendLength, g_sendLength };
    ret = receive_data(client, &startResponse);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, KEY_AGREEMENT_PROCESSING);

    struct uint8_buff endRequest = { (uint8_t *)g_sendMessageBuf, g_sendLength, g_sendLength };
    ret = receive_data(server, &endRequest);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, KEY_AGREEMENT_END);

    struct uint8_buff endResponse = { (uint8_t *)g_sendMessageBuf, g_sendLength, g_sendLength };
    ret = receive_data(client, &endResponse);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, KEY_AGREEMENT_END);

    struct uint8_buff exchangeRequest = { (uint8_t *)g_sendMessageBuf, g_sendLength, g_sendLength };
    ret = receive_data(server, &exchangeRequest);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, END_SUCCESS);

    struct uint8_buff exchangeResponse = { (uint8_t *)g_sendMessageBuf, g_sendLength, g_sendLength };
    ret = receive_data(client, &exchangeResponse);
    TEST_ASSERT_EQUAL_INT(ret, HC_OK);
    TEST_ASSERT_EQUAL_INT(g_serviceResult, END_SUCCESS);

    destroy(&client);
    destroy(&server);
}

RUN_TEST_SUITE(HichainFunctionTestSuite);