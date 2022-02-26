/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <iostream>
#include <thread>
#include <unistd.h>
#include <gtest/gtest.h>
#include <hdf_log.h>
#include <osal_mem.h>
#include <securec.h>
#include "c_test/cb_test/v1_0/client/icb_test.h"
#include "c_test/cb_test/v1_0/callback_stub.h"

using namespace OHOS;
using namespace testing::ext;

static struct ICbTest *g_testClient = nullptr;
static struct ICallback *g_callback = nullptr;

class CbTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp(){}
    void TearDown(){}
};

void CbTest::SetUpTestCase()
{
    g_testClient = HdiCbTestGet();
    if (g_testClient == nullptr) {
        printf("CbTest: get g_testClient failed.\n");
    }

    g_callback = CallbackStubObtain();
    if (g_callback == nullptr) {
        printf("CbTest: get g_callback failed.\n");
    }
}

void CbTest::TearDownTestCase()
{
    if (g_testClient != nullptr) {
        HdiCbTestRelease(g_testClient);
        g_testClient = nullptr;
    }

    if (g_callback != nullptr) {
        CallbackStubRelease(g_callback);
        g_callback = nullptr;
    }
}

HWTEST_F(CbTest, CbTest_001, TestSize.Level0)
{
    ASSERT_NE(nullptr, g_testClient);
    ASSERT_NE(nullptr, g_callback);
}

HWTEST_F(CbTest, CbTest_002, TestSize.Level0)
{
    int32_t ec = g_testClient->CallbackTest(g_testClient, g_callback);
    ASSERT_EQ(ec, HDF_SUCCESS);
}