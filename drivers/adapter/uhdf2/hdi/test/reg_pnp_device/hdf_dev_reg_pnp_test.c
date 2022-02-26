/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <servmgr_hdi.h>
#include<stdlib.h>
#include "hdf_log.h"
#include "devmgr_hdi.h"
#include "hdf_remote_service.h"
#include "osal_time.h"
#include "securec.h"

static int32_t g_testItemCnt = 0;
static int32_t g_testItemFailCnt = 0;

#ifdef __VND__
#define IS_HDI 0
#else
#define IS_HDI 1
#endif

#define UT_TEST_CHECK_RET(val) do { \
    if (!(val)) { \
        printf("[HDF_TEST] %s line:%d HDF_TEST_FAIL\r\n", __func__, __LINE__); \
        g_testItemFailCnt++; \
    } else { \
        printf("[HDF_TEST] %s line:%d HDF_TEST_PASS\r\n", __func__, __LINE__); \
    } \
    g_testItemCnt++; \
} while (0)

#define HDF_LOG_TAG hdf_reg_pnp_test

#define PNP_MODULE_NAME "libpnp_driver.so"
#define PNP_SERVICE_NAME "pnp_driver_service"

void PrintTestResult()
{
    printf("[HDF_TEST] %s test items: %d\r\n", __func__, g_testItemCnt);
    printf("[HDF_TEST] %s test PASS: %d\r\n", __func__, g_testItemCnt - g_testItemFailCnt);
    printf("[HDF_TEST] %s test FAIL: %d\r\n", __func__, g_testItemFailCnt);
}

int32_t GetProcessId(const char *processName)
{
    FILE *fp = NULL;
    char buffer[80] = {0};
    char cmd[100] = "ps -ef |grep ";
    if (strcat_s(cmd, sizeof(cmd) - strlen(cmd), processName) != 0) {
        return 0;
    }
    if (strcat_s(cmd, sizeof(cmd) - strlen(cmd), " | grep -v grep | awk '{print $2}'") != 0) {
        return 0;
    }
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return 0;
    }
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);
    return atoi(buffer);
}

void KillProcessById(int32_t procssId)
{
    char cmd[100] = "kill ";
    char buf[10] = {0};
    if (sprintf_s(buf, sizeof(buf) - 1, "%d", procssId) < 0) {
        return;
    }
    if (strcat_s(cmd, sizeof(cmd) - strlen(cmd), buf) != 0) {
        return;
    }
    system(cmd);
}


static int32_t HdfRegPnpDevice(const char *moduleName, const char *serviceName)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return HDF_DEV_ERR_NO_DEVICE_SERVICE;
    }

    int32_t ret = iDevmgr->RegPnpDevice(iDevmgr, moduleName, serviceName);

    HDIDeviceManagerRelease(iDevmgr);
    return ret;
}

static int32_t HdfUnRegPnpDevice(const char *moduleName, const char *serviceName)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return HDF_DEV_ERR_NO_DEVICE_SERVICE;
    }

    int32_t ret = iDevmgr->UnRegPnpDevice(iDevmgr, moduleName, serviceName);

    HDIDeviceManagerRelease(iDevmgr);
    return ret;
}

/*
* @tc.name: HDF_PNP_DEVICE_BaseFunction_AR_001
* @tc.desc: reg pnp driver
* @tc.type: FUNC
* @tc.require: SR000F86A5
*/
static void HdfRegPnpDriverTest_001()
{
    int32_t ret;
    ret = HdfRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    ret = HdfRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret != HDF_SUCCESS);
}

/*
* @tc.name: HDF_PNP_DEVICE_BaseFunction_AR_002
* @tc.desc: unreg pnp driver
* @tc.type: FUNC
* @tc.require: SR000F86A5
*/
static void HdfRegPnpDriverTest_002()
{
    int32_t ret;
    ret = HdfUnRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    ret = HdfUnRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret != HDF_SUCCESS);
}

/*
* @tc.name: HDF__PNP_DEVICE_BaseFunction_AR_003
* @tc.desc: reg pnp driver
* @tc.type: FUNC
* @tc.require: AR000F86A6 AR000F86A7 AR000F86A8 AR000F86A9
*/
static void HdfRegPnpDriverTest_003()
{
    int32_t ret;
    ret = HdfRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    struct HDIServiceManager *hdiServMgr = HDIServiceManagerGet();
    UT_TEST_CHECK_RET(hdiServMgr != NULL);

    struct HdfRemoteService *remote = hdiServMgr->GetService(hdiServMgr, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(remote != NULL);
    if (remote != NULL) {
        struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
        struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
        HdfSbufWriteString(data, "test for pnp dispatch!");
        int32_t processIdBefor = GetProcessId("pnp_host");
        ret = remote->dispatcher->Dispatch(remote, 0, data, reply);
        int32_t processIdAfter = GetProcessId("pnp_host");
        UT_TEST_CHECK_RET(processIdBefor == processIdAfter);
        int32_t result = 0;
        HdfParcelReadInt(reply, &result);
        UT_TEST_CHECK_RET(result == 1);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
    }
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    HDIServiceManagerRelease(hdiServMgr);
}

/*
* @tc.name: HDF__PNP_DEVICE_BaseFunction_AR_004
* @tc.desc: unreg pnp driver
* @tc.type: FUNC
* @tc.require: AR000F86A6 AR000F86A7 AR000F86A8 AR000F86A9
*/
static void HdfRegPnpDriverTest_004()
{
    int ret = HdfUnRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    struct HdfRemoteService *remote = HdfRemoteServiceGet(PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(remote == NULL);
}

/*
* @tc.name: HDF__PNP_DEVICE_BaseFunction_AR_005
* @tc.desc: reg fault pnp driver
* @tc.type: FUNC
* @tc.require: AR000F86A6 AR000F86A7 AR000F86A8 AR000F86A9
*/
static void HdfRegPnpDriverTest_005()
{
    int ret = HdfRegPnpDevice("libpnp_driver1.so", "pnp_driver_service_1");
    struct HdfRemoteService *remote = HdfRemoteServiceGet("pnp_driver_service_1");
    UT_TEST_CHECK_RET(remote == NULL);
    if (ret == HDF_SUCCESS) {
        HdfUnRegPnpDevice("libpnp_driver1.so", "pnp_driver_service1");
    }
}

/*
* @tc.name: HDF__PNP_DEVICE_BaseFunction_AR_006
* @tc.desc: reg fault pnp driver
* @tc.type: FUNC
* @tc.require: AR000F86A6 AR000F86A7 AR000F86A8 AR000F86A9
*/
static void HdfRegPnpDriverTest_006()
{
    int32_t ret;
    uint64_t totalTime = 0;
    for (int i = 0; i < 1000; i++) {
        uint64_t timeBefor = OsalGetSysTimeMs();
        char svcName[128] = {0};
        if (sprintf_s(svcName, sizeof(svcName) - 1, "%s%d", PNP_SERVICE_NAME, i) < 0) {
            continue;
        }
        const char *serviceName = svcName;
        ret = HdfRegPnpDevice(PNP_MODULE_NAME, serviceName);
        UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
        ret = HdfUnRegPnpDevice(PNP_MODULE_NAME, serviceName);
        UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
        uint64_t timeAfter = OsalGetSysTimeMs();
        totalTime += timeAfter - timeBefor;
    }
    printf("total time is %ld \n", totalTime);
    UT_TEST_CHECK_RET((totalTime / 1000) < 20);
}

/*
* @tc.name: HdfRestore_001
* @tc.desc: restore host process
* @tc.type: FUNC
* @tc.require: AR000F86AL AR000F86AM
*/
static void HdfRestore_001()
{
    int32_t ret;
    ret = HdfRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    struct HDIServiceManager *hdiServMgr = HDIServiceManagerGet();
    UT_TEST_CHECK_RET(hdiServMgr != NULL);

    struct HdfRemoteService *remote = hdiServMgr->GetService(hdiServMgr, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(remote != NULL);
    if (remote != NULL) {
        struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
        struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
        HdfSbufWriteString(data, "test for pnp dispatch!");
        int32_t processIdBefor = GetProcessId("pnp_host");
        ret = remote->dispatcher->Dispatch(remote, 1, data, reply);
        int32_t processIdAfter = GetProcessId("pnp_host");
        UT_TEST_CHECK_RET(processIdBefor != processIdAfter);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
    }
    UT_TEST_CHECK_RET(ret != HDF_SUCCESS);
    HDIServiceManagerRelease(hdiServMgr);
    ret = HdfUnRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
}

/*
* @tc.name: HdfRestore_002
* @tc.desc: restore host process
* @tc.type: FUNC
* @tc.require: AR000F86AL AR000F86AM
*/
static void HdfRestore_002()
{
    int32_t ret;
    ret = HdfRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
    struct HDIServiceManager *hdiServMgr = HDIServiceManagerGet();
    UT_TEST_CHECK_RET(hdiServMgr != NULL);

    struct HdfRemoteService *remote = hdiServMgr->GetService(hdiServMgr, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(remote != NULL);

    int32_t processIdBefor = GetProcessId("pnp_host");
    KillProcessById(processIdBefor);
    int32_t processIdAfter = GetProcessId("pnp_host");
    UT_TEST_CHECK_RET(processIdBefor != processIdAfter);
    remote = hdiServMgr->GetService(hdiServMgr, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(remote != NULL);
    HDIServiceManagerRelease(hdiServMgr);
    ret = HdfUnRegPnpDevice(PNP_MODULE_NAME, PNP_SERVICE_NAME);
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS);
}

/*
* @tc.name: HdfRestore_003
* @tc.desc: restore manager process
* @tc.type: FUNC
* @tc.require: SR000F86AK
*/
static void HdfRestore_003()
{
    int32_t processIdBefor = GetProcessId("hdf_devmgr");
    KillProcessById(processIdBefor);
    int32_t processIdAfter = GetProcessId("hdf_devmgr");
    UT_TEST_CHECK_RET((processIdAfter != 0) && (processIdBefor != processIdAfter));
}

int main(void)
{
    HdfRegPnpDriverTest_001();
    HdfRegPnpDriverTest_002();
    HdfRegPnpDriverTest_003();
    HdfRegPnpDriverTest_004();
    HdfRegPnpDriverTest_005();
    HdfRegPnpDriverTest_006();
    HdfRestore_001();
    HdfRestore_002();
    HdfRestore_003();
    PrintTestResult();
    return 0;
}

