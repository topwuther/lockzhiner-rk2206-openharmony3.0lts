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
#include <inttypes.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"
#include "hdf_io_service.h"
#include "osal_time.h"
#include "sample_driver_test.h"
#include "hdf_log.h"
#include "hdf_power_state.h"

using namespace testing::ext;

struct Eventlistener {
    struct HdfDevEventlistener listener;
    int32_t eventCount;
};

class IoServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static int OnDevEventReceived(struct HdfDevEventlistener *listener, struct HdfIoService *service, uint32_t id,
        struct HdfSBuf *data);

    static struct Eventlistener listener0;
    static struct Eventlistener listener1;
    const char *testSvcName = SAMPLE_SERVICE;
    const int eventWaitTimeUs = (150 * 1000);
    static int eventCount;
};

int IoServiceTest::eventCount = 0;

struct Eventlistener IoServiceTest::listener0;
struct Eventlistener IoServiceTest::listener1;

void IoServiceTest::SetUpTestCase()
{
    listener0.listener.onReceive = OnDevEventReceived;
    listener0.listener.priv = (void *)"listener0";

    listener1.listener.onReceive = OnDevEventReceived;
    listener1.listener.priv = (void *)"listener1";
}

void IoServiceTest::TearDownTestCase() {}

void IoServiceTest::SetUp()
{
    listener0.eventCount = 0;
    listener1.eventCount = 0;
    eventCount = 0;
}

void IoServiceTest::TearDown() {}

int IoServiceTest::OnDevEventReceived(struct HdfDevEventlistener *listener, struct HdfIoService *service, uint32_t id,
    struct HdfSBuf *data)
{
    OsalTimespec time;
    OsalGetTime(&time);
    HDF_LOGE("%s: received event[%d] from %s at %" PRIu64 ".%" PRIu64 "", (char *)listener->priv, eventCount++, (char *)service->priv,
        time.sec, time.usec);

    const char *string = HdfSbufReadString(data);
    if (string == nullptr) {
        HDF_LOGE("failed to read string in event data");
        return 0;
    }
    struct Eventlistener *l = CONTAINER_OF(listener, struct Eventlistener, listener);
    l->eventCount++;
    HDF_LOGE("%s: dev event received: %d %s", (char *)service->priv, id, string);
    return 0;
}

static int SendEvent(struct HdfIoService *serv, const char *eventData)
{
    OsalTimespec time;
    OsalGetTime(&time);

    int ret;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    if (data == nullptr) {
        HDF_LOGE("fail to obtain sbuf data");
        return HDF_FAILURE;
    }

    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (reply == nullptr) {
        HDF_LOGE("fail to obtain sbuf reply");
        HdfSBufRecycle(data);
        return HDF_DEV_ERR_NO_MEMORY;
    }

    do {
        if (!HdfSbufWriteString(data, eventData)) {
            HDF_LOGE("fail to write sbuf");
            ret = HDF_FAILURE;
            break;
        }

        ret = serv->dispatcher->Dispatch(&serv->object, SAMPLE_DRIVER_SENDEVENT_SINGLE_DEVICE, data, reply);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("fail to send service call");
            break;
        }

        int replyData = 0;
        if (!HdfSbufReadInt32(reply, &replyData)) {
            HDF_LOGE("fail to get service call reply");
            ret = HDF_ERR_INVALID_OBJECT;
        } else if (replyData != INT32_MAX) {
            HDF_LOGE("service call reply check fail, replyData=0x%x", replyData);
            ret = HDF_ERR_INVALID_OBJECT;
        }
        HDF_LOGE("send event finish at %" PRIu64 ".%" PRIu64 "", time.sec, time.usec);
    } while (0);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

/* *
 * @tc.name: HdfIoService001
 * @tc.desc: service bind test
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService001, TestSize.Level0)
{
    struct HdfIoService *testServ = HdfIoServiceBind(testSvcName);
    ASSERT_NE(testServ, nullptr);
    HdfIoServiceRecycle(testServ);
}

/* *
 * @tc.name: HdfIoService002
 * @tc.desc: service group listen test
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService002, TestSize.Level0)
{
    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv0";

    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfDeviceUnregisterEventListener(serv, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfIoServiceGroupRecycle(group);
    group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(2, listener0.eventCount);
    HdfIoServiceGroupRecycle(group);

    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService003
 * @tc.desc: remove service from service group by recycle group test
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService003, TestSize.Level0)
{
    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv0";

    struct HdfIoService *serv1 = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv1, nullptr);
    serv1->priv = (void *)"serv1";

    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfIoServiceGroupAddService(group, serv1);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfDeviceUnregisterEventListener(serv, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfDeviceRegisterEventListener(serv, &listener1.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(2, listener0.eventCount);
    ASSERT_EQ(1, listener1.eventCount);
    HdfIoServiceGroupRecycle(group);
    HdfDeviceUnregisterEventListener(serv, &listener1.listener);
    HdfIoServiceRecycle(serv);
    HdfIoServiceRecycle(serv1);
}

/* *
 * @tc.name: HdfIoService004
 * @tc.desc: single service listen test
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService004, TestSize.Level0)
{
    struct HdfIoService *serv1 = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv1, nullptr);
    serv1->priv = (void *)"serv1";

    int ret = HdfDeviceRegisterEventListener(serv1, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);
    ret = SendEvent(serv1, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfDeviceUnregisterEventListener(serv1, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);
    HdfIoServiceRecycle(serv1);
}

/* *
 * @tc.name: HdfIoService005
 * @tc.desc: service group add remove test
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService005, TestSize.Level0)
{
    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    HdfIoServiceGroupRemoveService(group, serv);

    ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(2, listener0.eventCount);
    HdfIoServiceGroupRecycle(group);
}


/* *
 * @tc.name: HdfIoService006
 * @tc.desc: service group add remove listener test
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService006, TestSize.Level0)
{
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    struct HdfIoService *serv1 = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv1, nullptr);
    serv1->priv = (void *)"serv1";

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupAddService(group, serv1);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = SendEvent(serv1, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(2, listener0.eventCount);

    HdfIoServiceGroupRemoveService(group, serv);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(2, listener0.eventCount);

    ret = SendEvent(serv1, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(3, listener0.eventCount);

    ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(4, listener0.eventCount);

    HdfIoServiceGroupRecycle(group);
    HdfIoServiceRecycle(serv);
    HdfIoServiceRecycle(serv1);
}


/* *
 * @tc.name: HdfIoService007
 * @tc.desc: duplicate remove group listener
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService007, TestSize.Level0)
{
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfIoServiceGroupUnregisterListener(group, &listener0.listener);
    EXPECT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupUnregisterListener(group, &listener0.listener);
    EXPECT_NE(ret, HDF_SUCCESS);

    HdfIoServiceGroupRecycle(group);
    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService008
 * @tc.desc: duplicate add group listener
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService008, TestSize.Level0)
{
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    EXPECT_NE(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupUnregisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfIoServiceGroupRecycle(group);
    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService008
 * @tc.desc: duplicate add service
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService009, TestSize.Level0)
{
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupAddService(group, serv);
    EXPECT_NE(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfIoServiceGroupUnregisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfIoServiceGroupRecycle(group);
    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService010
 * @tc.desc: duplicate remove service
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService010, TestSize.Level0)
{
    struct HdfIoServiceGroup *group = HdfIoServiceGroupObtain();
    ASSERT_NE(group, nullptr);

    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    int ret = HdfIoServiceGroupAddService(group, serv);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupAddService(group, serv);
    EXPECT_NE(ret, HDF_SUCCESS);

    ret = HdfIoServiceGroupRegisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    HdfIoServiceGroupRemoveService(group, serv);
    HdfIoServiceGroupRemoveService(group, serv);

    ret = HdfIoServiceGroupUnregisterListener(group, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfIoServiceGroupRecycle(group);
    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService011
 * @tc.desc: duplicate add service listener
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService011, TestSize.Level0)
{
    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    int ret = HdfDeviceRegisterEventListener(serv, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfDeviceRegisterEventListener(serv, &listener0.listener);
    EXPECT_NE(ret, HDF_SUCCESS);

    ret = HdfDeviceUnregisterEventListener(serv, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);
    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService012
 * @tc.desc: duplicate remove service listener
 * @tc.type: FUNC
 * @tc.require: AR000F869B
 */
HWTEST_F(IoServiceTest, HdfIoService012, TestSize.Level0)
{
    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);
    serv->priv = (void *)"serv";

    int ret = HdfDeviceRegisterEventListener(serv, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = SendEvent(serv, testSvcName);
    ASSERT_EQ(ret, HDF_SUCCESS);

    usleep(eventWaitTimeUs);
    ASSERT_EQ(1, listener0.eventCount);

    ret = HdfDeviceUnregisterEventListener(serv, &listener0.listener);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ret = HdfDeviceUnregisterEventListener(serv, &listener0.listener);
    EXPECT_NE(ret, HDF_SUCCESS);

    HdfIoServiceRecycle(serv);
}

/* *
 * @tc.name: HdfIoService013
 * @tc.desc: devmgr power state change test
 * @tc.type: FUNC
 */
HWTEST_F(IoServiceTest, HdfIoService013, TestSize.Level0)
{
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    ASSERT_NE(data, nullptr);

    struct HdfIoService *serv = HdfIoServiceBind(testSvcName);
    ASSERT_NE(serv, nullptr);

    HdfSbufWriteUint32(data, POWER_STATE_SUSPEND);
    int ret = serv->dispatcher->Dispatch(&serv->object, SAMPLE_DRIVER_PM_STATE_INJECT, data, NULL);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfSbufFlush(data);
    HdfSbufWriteUint32(data, POWER_STATE_RESUME);
    ret = serv->dispatcher->Dispatch(&serv->object, SAMPLE_DRIVER_PM_STATE_INJECT, data, NULL);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfSbufFlush(data);
    HdfSbufWriteUint32(data, POWER_STATE_DOZE_SUSPEND);
    ret = serv->dispatcher->Dispatch(&serv->object, SAMPLE_DRIVER_PM_STATE_INJECT, data, NULL);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfSbufFlush(data);
    HdfSbufWriteUint32(data, POWER_STATE_DOZE_RESUME);
    ret = serv->dispatcher->Dispatch(&serv->object, SAMPLE_DRIVER_PM_STATE_INJECT, data, NULL);
    ASSERT_EQ(ret, HDF_SUCCESS);
    HdfIoServiceRecycle(serv);
    HdfSBufRecycle(data);
}
