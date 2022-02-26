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

#include <ctime>
#include <gtest/gtest.h>
#include "common_defs.h"
#include "json_utils.h"
#include "hc_condition.h"
#include "hc_mutex.h"
#include "hc_types.h"
#include "ipc_sdk.h"
#include "device_auth.h"
#include "database_manager.h"
#include "device_auth_defines.h"
#include "deviceauth_test_mock.h"
#include "deviceauth_standard_test.h"

using namespace std;
using namespace testing::ext;

namespace
{
    static bool g_isNeedContinue = false;
    static int64_t g_requestId = 0L;
    static int g_operationCode = -1;
    static int g_errorCode = 1;
    static char *g_tempStr = nullptr;
    static int g_messageCode = -1;
    static char g_dataBuffer[BUFFER_SIZE] = { 0 };
    static HcCondition g_testCondition;
    static const DeviceGroupManager *g_testGm = nullptr;
    static const GroupAuthManager *g_testGa = nullptr;
    static DeviceAuthCallback g_gaCallback;
    static int delaytime = 500;
    static int expireTime = 90;
    static int UN_expireTime = 91;



static void DelayWithMSec(int32_t mSecs)
{
    int times = 1000;
    struct timeval out;
    out.tv_sec = 0;
    out.tv_usec = mSecs * times;
    (void)select(1, NULL, NULL, NULL, &out);
    return;
}

void ClearTempValue()
{
    g_isNeedContinue = false;
    g_requestId = 0L;
    g_operationCode = -1;
    g_errorCode = 1;
    g_tempStr = nullptr;
    g_messageCode = -1;
}

/* delete file path success */
static int32_t RemoveDir(const char *path)
{
    char strBuf[BUFFER_SIZE] = {0};
    if (path == nullptr) {
        return -1;
    }
    sprintf_s(strBuf, sizeof(strBuf) - 1, "rm -rf %s", path);
    cout << strBuf << endl;
    system(strBuf);
    return 0;
}

static void RemoveHuks(void)
{
    int ret = RemoveDir("/data/data/maindata");
    cout << "[Clear] clear huks:maindata done: " << ret << endl;
    ret = RemoveDir("/data/data/bakdata");
    cout << "[Clear] clear huks:bakdata done: " << ret << endl;
}

static int DeleteDatabase()
{
    const char *groupPath = "/data/data/deviceauth/hcgroup.dat";
    int ret = RemoveDir(groupPath);
    cout << "[Clear] clear db: done: " << ret << endl;
    RemoveHuks();
    /* wait for delete data */
    DelayWithMSec(delaytime);
    return 0;
}

static char *OnRequestNormal(int64_t requestId, int operationCode, const char *reqParams)
{
    g_messageCode = ON_REQUEST;
    g_requestId = requestId;
    g_operationCode = operationCode;
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_CONFIRMATION, REQUEST_ACCEPTED);
    AddStringToJson(json, FIELD_PIN_CODE, "123456");
    AddStringToJson(json, FIELD_DEVICE_ID, SERVER_AUTH_ID);
    AddIntToJson(json, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(json, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(json, FIELD_EXPIRE_TIME, expireTime);
    char *returnDataStr = PackJsonToString(json);
    FreeJson(json);
    return returnDataStr;
}

static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    (void)errorReturn;
    g_messageCode = ON_ERROR;
    g_requestId = requestId;
    g_operationCode = operationCode;
    g_errorCode = errorCode;
    g_testCondition.notify(&g_testCondition);
}

static void OnError2(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    (void)errorReturn;
    g_messageCode = ON_ERROR;
    g_requestId = requestId;
    g_operationCode = operationCode;
    g_errorCode = errorCode;
}

static void OnFinish(int64_t requestId, int operationCode, const char *returnData)
{
    g_messageCode = ON_FINISH;
    g_requestId = requestId;
    g_operationCode = operationCode;
    if (operationCode == GROUP_CREATE) {
        CJson *json = CreateJsonFromString(returnData);
        const char *groupId = GetStringFromJson(json, FIELD_GROUP_ID);
        (void)memcpy_s(g_dataBuffer, BUFFER_SIZE, groupId, strlen(groupId));
        FreeJson(json);
    }
    g_testCondition.notify(&g_testCondition);
}

static void OnFinish2(int64_t requestId, int operationCode, const char *returnData)
{
    g_messageCode = ON_FINISH;
    g_requestId = requestId;
    g_operationCode = operationCode;
}

static void OnSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    g_messageCode = ON_SESSION_KEY_RETURNED;
    g_requestId = requestId;
}

static bool OnTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    g_messageCode = ON_TRANSMIT;
    g_requestId = requestId;
    g_isNeedContinue = true;
    memset_s(g_dataBuffer, BUFFER_SIZE, 0, BUFFER_SIZE);
    memcpy_s(g_dataBuffer, BUFFER_SIZE, data, dataLen);
    return true;
}

static char *GaOnRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    g_messageCode = ON_REQUEST;
    g_requestId = requestId;
    g_operationCode = operationCode;
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_CONFIRMATION, REQUEST_ACCEPTED);
    AddStringToJson(json, FIELD_SERVICE_PKG_NAME, TEST_APP_NAME);
    AddStringToJson(json, FIELD_PEER_CONN_DEVICE_ID, "udid1");
    char *returnDataStr = PackJsonToString(json);
    FreeJson(json);
    return returnDataStr;
}

enum {
    GROUP_CREATED = 0,
    GROUP_DELETED,
    DEVICE_BOUND,
    DEVICE_UNBOUND,
    DEVICE_NOT_TRUSTED,
    LAST_GROUP_DELETED,
    TRUSTED_DEVICE_NUM_CHANGED
};

static int g_receivedMessageNum[7] = {0};

void OnGroupCreated(const char *groupInfo)
{
    g_receivedMessageNum[GROUP_CREATED]++;
}

void OnGroupDeleted(const char *groupInfo)
{
    g_receivedMessageNum[GROUP_DELETED]++;
}

void OnDeviceBound(const char *peerUdid, const char *groupInfo)
{
    g_receivedMessageNum[DEVICE_BOUND]++;
}

void OnDeviceUnBound(const char *peerUdid, const char *groupInfo)
{
    g_receivedMessageNum[DEVICE_UNBOUND]++;
}

void OnDeviceNotTrusted(const char *peerUdid)
{
    g_receivedMessageNum[DEVICE_NOT_TRUSTED]++;
}

void OnLastGroupDeleted(const char *peerUdid, int groupType)
{
    g_receivedMessageNum[LAST_GROUP_DELETED]++;
}

void OnTrustedDeviceNumChanged(int curTrustedDeviceNum)
{
    g_receivedMessageNum[TRUSTED_DEVICE_NUM_CHANGED]++;
}

static void InitCaseResource(void)
{
    const DeviceGroupManager *gmTmp = nullptr;
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret == HC_SUCCESS, true);
    cout << "InitDeviceAuthService: " << ret << endl;
    gmTmp = GetGmInstance();
    ASSERT_NE(gmTmp, nullptr);
    g_testGm = gmTmp;
    g_gaCallback.onRequest = OnRequestNormal;
    g_gaCallback.onTransmit = OnTransmit;
    g_gaCallback.onFinish = OnFinish;
    g_gaCallback.onError = OnError;
    g_gaCallback.onSessionKeyReturned = OnSessionKeyReturned;
    g_testGm->regCallback(TEST_APP_NAME, &g_gaCallback);
    DelayWithMSec(delaytime);
}

static void DeInitCaseResource(void)
{
    DestroyDeviceAuthService();
    ClearTempValue();
    g_testGm = nullptr;
}

/* test suit - GET_INSTANCE */
class GET_INSTANCE : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void GET_INSTANCE::SetUpTestCase()
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret == HC_SUCCESS, true);
}

void GET_INSTANCE::TearDownTestCase()
{
    DestroyDeviceAuthService();
    ClearTempValue();
}

void GET_INSTANCE::SetUp()
{
}

void GET_INSTANCE::TearDown()
{
}

class REGISTER_CALLBACK : public testing::Test {
public:
    const DeviceGroupManager *gm = GetGmInstance();
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};
/* test suit - REGISTER_CALLBACK */

void REGISTER_CALLBACK::SetUpTestCase()
{
}

void REGISTER_CALLBACK::TearDownTestCase()
{
}

void REGISTER_CALLBACK::SetUp()
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret == HC_SUCCESS, true);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
}

void REGISTER_CALLBACK::TearDown()
{
    DestroyDeviceAuthService();
    ClearTempValue();
    gm = nullptr;
}

class CREATE_GROUP_P2P : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

/* test suit - CREATE_GROUP_P2P */
void CREATE_GROUP_P2P::SetUpTestCase()
{
    InitHcCond(&g_testCondition, nullptr);
}

void CREATE_GROUP_P2P::TearDownTestCase()
{
    DestroyHcCond(&g_testCondition);
}
void CREATE_GROUP_P2P::SetUp()
{
    SetClient(false);
    DeleteDatabase();
    InitCaseResource();
}

void CREATE_GROUP_P2P::TearDown()
{
    DeInitCaseResource();
}

class AUTHENTICATE_GA : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

/* test suit - AUTHENTICATE_GA */
void AUTHENTICATE_GA::SetUpTestCase()
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret == HC_SUCCESS, true);
    g_gaCallback = {
        OnTransmit,
        OnSessionKeyReturned,
        OnFinish,
        OnError,
        GaOnRequest
    };
    g_testGa = GetGaInstance();
    ASSERT_NE(g_testGa, nullptr);
    SetClient(false);
}

void AUTHENTICATE_GA::TearDownTestCase()
{
    DestroyDeviceAuthService();
    DeleteDatabase();
    ClearTempValue();
    g_testGa = nullptr;
}

void AUTHENTICATE_GA::SetUp()
{
}

void AUTHENTICATE_GA::TearDown()
{
}

class REGISTER_LISTENER : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};
/* test suit - REGISTER_LISTENER */
void REGISTER_LISTENER::SetUpTestCase()
{
    DeleteDatabase();
    InitHcCond(&g_testCondition, nullptr);
}

void REGISTER_LISTENER::TearDownTestCase()
{
    DestroyHcCond(&g_testCondition);
}

void REGISTER_LISTENER::SetUp()
{
    InitDeviceAuthService();
    g_gaCallback = {
        OnTransmit,
        OnSessionKeyReturned,
        OnFinish,
        OnError,
        OnRequestNormal
    };
    g_testGm = GetGmInstance();
    g_testGm->regCallback(TEST_APP_NAME, &g_gaCallback);
    DelayWithMSec(delaytime);
}
void REGISTER_LISTENER::TearDown()
{
    DestroyDeviceAuthService();
    DeleteDatabase();
    ClearTempValue();
}

class QUERY_INTERFACE : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};
void QUERY_INTERFACE::SetUpTestCase()
{
    DeleteDatabase();
}

void QUERY_INTERFACE::TearDownTestCase() {}

void QUERY_INTERFACE::SetUp()
{
    InitDeviceAuthService();
    g_gaCallback = {
        OnTransmit,
        OnSessionKeyReturned,
        OnFinish2,
        OnError2,
        OnRequestNormal
    };
    g_testGm = GetGmInstance();
    g_testGm->regCallback(TEST_APP_NAME, &g_gaCallback);
}
void QUERY_INTERFACE::TearDown()
{
    DestroyDeviceAuthService();
    DeleteDatabase();
    ClearTempValue();
}

/* start cases */
/**
 * @tc.name: GET_INSTANCE.TC_GET_GM_INSTANCE
 * @tc.desc: Test GetGmInstance interface function;
 * @tc.type: FUNC
 */
HWTEST_F(GET_INSTANCE, TC_GET_GM_INSTANCE, TestSize.Level1)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
}

/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_01
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_01, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, 258);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, expireTime);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_02
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_02, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, expireTime);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    CJson *returnData = CreateJson();
    AddStringToJson(returnData, FIELD_GROUP_OWNER, TEST_APP_NAME);
    char *queryParams = PackJsonToString(returnData);
    FreeJson(returnData);
    char *groupVec = nullptr;
    uint32_t num = 0;
    ret = g_testGm->getGroupInfo(TEST_APP_NAME, queryParams, &groupVec, &num);
    FreeJsonString(queryParams);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(num, 1u);
    g_testGm->destroyInfo(&groupVec);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_03
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_03, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, 3);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, expireTime);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_04
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_04, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, 1);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, expireTime);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_05
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_05, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, expireTime);
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_JSON_GET, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_06
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_06, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, expireTime);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ClearTempValue();
    ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    FreeJson(createParams);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_07
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_07, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, -2);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_08
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_08, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, 0);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}
/**
 * @tc.name: CREATE_GROUP_P2P.TC_INVALID_PARAM_09
 * @tc.desc: Test CREATE_GROUP_P2P interface function;
 * @tc.type: FUNC
 */
HWTEST_F(CREATE_GROUP_P2P, TC_INVALID_PARAM_09, TestSize.Level1)
{
    CJson *createParams = CreateJson();
    AddIntToJson(createParams, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP);
    AddStringToJson(createParams, FIELD_DEVICE_ID, "3C58C27533D8");
    AddIntToJson(createParams, FIELD_USER_TYPE, DEVICE_TYPE_ACCESSORY);
    AddIntToJson(createParams, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC);
    AddIntToJson(createParams, FIELD_EXPIRE_TIME, UN_expireTime);
    AddStringToJson(createParams, FIELD_GROUP_NAME, "P2PGroup");
    char *createParamsStr = PackJsonToString(createParams);
    FreeJson(createParams);
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    FreeJsonString(createParamsStr);
    g_testCondition.wait(&g_testCondition);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ASSERT_EQ(g_messageCode, ON_ERROR);
    ASSERT_EQ(g_operationCode, GROUP_CREATE);
    ASSERT_EQ(g_requestId, TEMP_REQUEST_ID);
    ASSERT_EQ(g_errorCode == HC_ERR_INVALID_PARAMS, true);
}

/**
 * @tc.name: REGISTER_LISTENER.TC_LISTENER_01
 * @tc.desc: Test REGISTER_LISTENER interface function;
 * @tc.type: FUNC
 */

HWTEST_F(REGISTER_LISTENER, TC_LISTENER_01, TestSize.Level1)
{
    DataChangeListener listener;
    listener.onGroupCreated = OnGroupCreated;
    listener.onGroupDeleted = OnGroupDeleted;
    listener.onDeviceBound = OnDeviceBound;
    listener.onDeviceUnBound = OnDeviceUnBound;
    listener.onDeviceNotTrusted = OnDeviceNotTrusted;
    listener.onLastGroupDeleted = OnLastGroupDeleted;
    listener.onTrustedDeviceNumChanged = OnTrustedDeviceNumChanged;
    int ret = g_testGm->regDataChangeListener(TEST_APP_NAME, &listener);
    ASSERT_EQ(ret == HC_SUCCESS, true);
}
/**
 * @tc.name: REGISTER_LISTENER.TC_LISTENER_02
 * @tc.desc: Test REGISTER_LISTENER interface function;
 * @tc.type: FUNC
 */
HWTEST_F(REGISTER_LISTENER, TC_LISTENER_02, TestSize.Level1)
{
    DataChangeListener listener;
    listener.onGroupCreated = OnGroupCreated;
    listener.onGroupDeleted = OnGroupDeleted;
    listener.onDeviceBound = OnDeviceBound;
    listener.onDeviceUnBound = OnDeviceUnBound;
    listener.onDeviceNotTrusted = OnDeviceNotTrusted;
    listener.onLastGroupDeleted = OnLastGroupDeleted;
    listener.onTrustedDeviceNumChanged = OnTrustedDeviceNumChanged;
    int ret = g_testGm->regDataChangeListener(TEST_APP_NAME, &listener);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ret = g_testGm->regDataChangeListener(TEST_APP_NAME, &listener);
    ASSERT_EQ(ret == HC_SUCCESS, true);
}
/**
 * @tc.name: REGISTER_LISTENER.TC_LISTENER_03
 * @tc.desc: Test REGISTER_LISTENER interface function;
 * @tc.type: FUNC
 */
HWTEST_F(REGISTER_LISTENER, TC_LISTENER_03, TestSize.Level1)
{
    DataChangeListener listener;
    listener.onGroupCreated = OnGroupCreated;
    listener.onGroupDeleted = OnGroupDeleted;
    listener.onDeviceBound = OnDeviceBound;
    listener.onDeviceUnBound = OnDeviceUnBound;
    listener.onDeviceNotTrusted = OnDeviceNotTrusted;
    listener.onLastGroupDeleted = OnLastGroupDeleted;
    listener.onTrustedDeviceNumChanged = OnTrustedDeviceNumChanged;
    int ret = g_testGm->regDataChangeListener(TEST_APP_NAME, &listener);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ret = g_testGm->unRegDataChangeListener(TEST_APP_NAME);
    ASSERT_EQ(ret == HC_SUCCESS, true);
}
/**
 * @tc.name: REGISTER_LISTENER.TC_LISTENER_04
 * @tc.desc: Test REGISTER_LISTENER interface function;
 * @tc.type: FUNC
 */
HWTEST_F(REGISTER_LISTENER, TC_LISTENER_04, TestSize.Level1)
{
    DataChangeListener listener;
    listener.onGroupCreated = OnGroupCreated;
    listener.onGroupDeleted = OnGroupDeleted;
    listener.onDeviceBound = OnDeviceBound;
    listener.onDeviceUnBound = OnDeviceUnBound;
    listener.onDeviceNotTrusted = OnDeviceNotTrusted;
    listener.onLastGroupDeleted = OnLastGroupDeleted;
    listener.onTrustedDeviceNumChanged = OnTrustedDeviceNumChanged;
    int ret = g_testGm->regDataChangeListener(TEST_APP_NAME, &listener);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ret = g_testGm->unRegDataChangeListener(TEST_APP_NAME);
    ASSERT_EQ(ret == HC_SUCCESS, true);
    ret = g_testGm->unRegDataChangeListener(TEST_APP_NAME);
    ASSERT_EQ(ret == HC_SUCCESS, true);
}

/**
 * @tc.name: QUERY_INTERFACE.TC_QUERY_01
 * @tc.desc: Test QUERY_INTERFACE interface function;
 * @tc.type: FUNC
 */

HWTEST_F(QUERY_INTERFACE, TC_QUERY_01, TestSize.Level1)
{
    const char * createParamsStr =
        "{\"groupType\":256,\"deviceId\":\"3C58C27533D8\",\"userType\":0,\""
        "groupVisibility\":-1,\"expireTime\":90,\"groupName\":\"P2PGroup\"}";
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    DelayWithMSec(delaytime);
    char *returnDataStr = nullptr;
    g_testGm->getGroupInfoById(TEST_APP_NAME,
        "BC680ED1137A5731F4A5A90B1AACC4A0A3663F6FC2387B7273EFBCC66A54DC0B", &returnDataStr);
    CJson *json = CreateJsonFromString(returnDataStr);
    const char *groupName = GetStringFromJson(json, FIELD_GROUP_NAME);
    ret = strcmp(groupName, "P2PGroup");
    FreeJson(json);
    g_testGm->destroyInfo(&returnDataStr);
    ASSERT_EQ(ret, 0);
}
/**
 * @tc.name: QUERY_INTERFACE.TC_QUERY_02
 * @tc.desc: Test QUERY_INTERFACE interface function;
 * @tc.type: FUNC
 */
HWTEST_F(QUERY_INTERFACE, TC_QUERY_02, TestSize.Level1)
{
    const char * createParamsStr =
        "{\"groupType\":256,\"deviceId\":\"3C58C27533D8\",\"userType\":0,\""
        "groupVisibility\":-1,\"expireTime\":90,\"groupName\":\"P2PGroup\"}";
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    DelayWithMSec(delaytime);
    char *returnDataStr = nullptr;
    ret = g_testGm->getGroupInfoById(TEST_APP_NAME, nullptr, &returnDataStr);
    ASSERT_NE(ret, 0);
}
/**
 * @tc.name: QUERY_INTERFACE.TC_QUERY_03
 * @tc.desc: Test QUERY_INTERFACE interface function;
 * @tc.type: FUNC
 */
HWTEST_F(QUERY_INTERFACE, TC_QUERY_03, TestSize.Level1)
{
    const char * createParamsStr =
        "{\"groupType\":256,\"deviceId\":\"3C58C27533D8\",\"userType\":0,\""
        "groupVisibility\":-1,\"expireTime\":90,\"groupName\":\"P2PGroup\"}";
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    DelayWithMSec(delaytime);
    char *returnDataStr = nullptr;
    ret = g_testGm->getGroupInfoById(TEST_APP_NAME, "GROUPID", &returnDataStr);
    ASSERT_NE(ret, 0);
}
/**
 * @tc.name: QUERY_INTERFACE.TC_QUERY_04
 * @tc.desc: Test QUERY_INTERFACE interface function;
 * @tc.type: FUNC
 */
HWTEST_F(QUERY_INTERFACE, TC_QUERY_04, TestSize.Level1)
{
    const char * createParamsStr =
        "{\"groupType\":256,\"deviceId\":\"3C58C27533D8\",\"userType\":0,\""
        "groupVisibility\":-1,\"expireTime\":90,\"groupName\":\"P2PGroup\"}";
    int ret = g_testGm->createGroup(TEMP_REQUEST_ID, TEST_APP_NAME, createParamsStr);
    DelayWithMSec(delaytime);
    char *returnDataStr = nullptr;
    ret = g_testGm->getGroupInfoById("tesstApp2",
        "BC680ED1137A5731F4A5A90B1AACC4A0A3663F6FC2387B7273EFBCC66A54DC0B", &returnDataStr);
    ASSERT_EQ(ret, 0);
}
}