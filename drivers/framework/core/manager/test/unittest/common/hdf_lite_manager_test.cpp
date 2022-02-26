/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"
#include "hdf_io_service.h"
#include "osal_time.h"
#include "sample_driver_test.h"

using namespace testing::ext;

class HdfManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfManagerTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfManagerTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfManagerTest::SetUp()
{
}

void HdfManagerTest::TearDown()
{
}

/**
  * @tc.name: HdfIoServiceBind001
  * @tc.desc: service bind test
  * @tc.type: FUNC
  * @tc.require: AR000F8698 AR000F8699 AR000F869A AR000F869B AR000F869C
  */
HWTEST_F(HdfManagerTest, HdfIoServiceBind001, TestSize.Level0)
{
    const char *svcName = "HDF_TEST";
    struct HdfIoService *hdfSvc = HdfIoServiceBind(svcName);
    EXPECT_TRUE(hdfSvc != NULL);
    HdfIoServiceRecycle(hdfSvc);
}

/**
  * @tc.name: HdfIoServiceBind002
  * @tc.desc: service bind test
  * @tc.type: FUNC
  * @tc.require: AR000F8698 AR000F8699 AR000F869A AR000F869B AR000F869C
  */
HWTEST_F(HdfManagerTest, HdfIoServiceBind002, TestSize.Level0)
{
    struct HdfIoService *hdfSvc = HdfIoServiceBind(DEV_MGR_NODE);
    EXPECT_TRUE(hdfSvc != NULL);
    HdfIoServiceRecycle(hdfSvc);
}

/**
  * @tc.name: HdfRegisterDevice001
  * @tc.desc: register device
  * @tc.type: FUNC
  * @tc.require: SR000F8697
  */
HWTEST_F(HdfManagerTest, HdfRegisterDevice001, TestSize.Level0)
{
    int32_t ret = HDF_FAILURE;
    struct HdfSBuf *data = NULL;
    struct HdfIoService *ioService = HdfIoServiceBind(SAMPLE_SERVICE);
    ASSERT_TRUE(ioService != NULL);
    data = HdfSBufObtainDefaultSize();
    EXPECT_TRUE(data != NULL);
    EXPECT_TRUE(HdfSbufWriteString(data, "sample_driver"));
    EXPECT_TRUE(HdfSbufWriteString(data, "sample_service1"));
    uint64_t timeBefore = OsalGetSysTimeMs();
    ret = ioService->dispatcher->Dispatch(&ioService->object, SAMPLE_DRIVER_REGISTER_DEVICE, data, NULL);
    EXPECT_TRUE(ret == HDF_SUCCESS);
    uint64_t timeAfter = OsalGetSysTimeMs();
    EXPECT_TRUE((timeAfter - timeBefore) < 100);

    struct HdfIoService *ioService1 = HdfIoServiceBind("sample_service1");
    EXPECT_TRUE(ioService1 != NULL);
    HdfIoServiceRecycle(ioService1);

    ret = ioService->dispatcher->Dispatch(&ioService->object, SAMPLE_DRIVER_UNREGISTER_DEVICE, data, NULL);
    EXPECT_TRUE(ret == HDF_SUCCESS);

    ioService1 = HdfIoServiceBind("sample_service1");
    EXPECT_TRUE(ioService1 == NULL);
    HdfIoServiceRecycle(ioService);
    HdfIoServiceRecycle(ioService1);
    HdfSBufRecycle(data);
}

/**
  * @tc.name: HdfGetServiceNameByDeviceClass001
  * @tc.desc: get service test
  * @tc.type: FUNC
  * @tc.require: AR000F8698 AR000F8699 AR000F869A AR000F869B AR000F869C
  */
HWTEST_F(HdfManagerTest, HdfGetServiceNameByDeviceClass001, TestSize.Level0)
{
    struct HdfSBuf *data = HdfSBufObtain(1000);
    ASSERT_TRUE(data != NULL);
    int32_t ret = HdfGetServiceNameByDeviceClass(DEVICE_CLASS_DEFAULT, data);
    EXPECT_TRUE(ret == HDF_SUCCESS);
    bool flag = false;
    const char *svcName = NULL;
    while(true) {
        svcName = HdfSbufReadString(data);
        if (svcName == NULL) {
            break;
        }
        if (strcmp(svcName, "sample_service") == 0) {
            flag = true;
            break;
        }
    }
    HdfSBufRecycle(data);
    EXPECT_TRUE(flag);
}

