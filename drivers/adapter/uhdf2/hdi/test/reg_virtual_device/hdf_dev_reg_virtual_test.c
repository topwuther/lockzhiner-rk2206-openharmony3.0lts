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

#include "hdf_log.h"
#include "devmgr_hdi.h"
#include "osal_time.h"

static int32_t g_testItemCnt = 0;
static int32_t g_testItemFailCnt = 0;

#define UT_TEST_CHECK_RET(val, flag) do { \
    if (!(val)) { \
        HDF_LOGE("[HDF_TEST] %{public}s line:%{public}d HDF_TEST_FAIL", __func__, __LINE__); \
        printf("[HDF_TEST] %s line:%d HDF_TEST_FAIL\r\n", __func__, __LINE__); \
        g_testItemFailCnt++; \
    } else if ((flag)) { \
        HDF_LOGD("[HDF_TEST] %{public}s line:%{public}d HDF_TEST_PASS", __func__, __LINE__); \
        printf("[HDF_TEST] %s line:%d HDF_TEST_PASS\r\n", __func__, __LINE__); \
    } \
    g_testItemCnt++; \
} while (0)

#define GET_TIME_BEGIN(time) OsalGetTime(time)
#define GET_TIME_END(incex, type, time1, time2, diff, func, line) do { \
    OsalGetTime(time2); \
    OsalDiffTime(time1, time2, diff); \
    CalcAverageTime(index, type, diff, func, line); \
} while (0)

#define HDF_LOG_TAG hdf_reg_virtual_test

#define VIRTUAL_MODULE_NAME "hdf_test_virtual_driver.so"
#define VIRTUAL_MODULE_NAME_NO "hdf_test_virtual_driver1.so"
#define VIRTUAL_SERVICE_NAME "hdf_test_virtual_driver"
#define VIRTUAL_SERVICE_NAME1 "hdf_test_virtual_driver1"
#define VIRTUAL_SERVICE_NAME2 "hdf_test_virtual_driver2"
#define VIRTUAL_SERVICE_NO_DRIVER "hdf_test_virtual_driver3"

#define SERVICE_CNT 3
#define SERVICE_NO_DRIVER_CNT 1

#define REGISTER_WAIT_TIME 100
static char *g_testServiceName[SERVICE_CNT] = { VIRTUAL_SERVICE_NAME, VIRTUAL_SERVICE_NAME1, VIRTUAL_SERVICE_NAME2 };
static char *g_testServiceNameNoDrv[SERVICE_NO_DRIVER_CNT] = { VIRTUAL_SERVICE_NO_DRIVER };
#define HDF_REG_DEVICE_ONE 0xffff
#define HDF_REG_TIME_RANGE 10
#define HDF_REG_DEVICE_MORE 200
#define HDF_MICRO_UNIT 1000000
#define TEST_ITEM_CNT 2
#define ITEM_REG 0
#define ITEM_UNREG 1

static int g_index[TEST_ITEM_CNT];
static uint64_t g_totalUsec[TEST_ITEM_CNT];

void PrintTestResult()
{
    HDF_LOGE("[HDF_TEST] %{public}s test items: %{public}d", __func__, g_testItemCnt);
    HDF_LOGE("[HDF_TEST] %{public}s test PASS: %{public}d", __func__, g_testItemCnt - g_testItemFailCnt);
    HDF_LOGE("[HDF_TEST] %{public}s test FAIL: %{public}d", __func__, g_testItemFailCnt);
    printf("[HDF_TEST] %s test items: %d\r\n", __func__, g_testItemCnt);
    printf("[HDF_TEST] %s test PASS: %d\r\n", __func__, g_testItemCnt - g_testItemFailCnt);
    printf("[HDF_TEST] %s test FAIL: %d\r\n", __func__, g_testItemFailCnt);
}

static void CalcAverageTime(int index, bool type, const OsalTimespec *diff, const char *func, int line)
{
    if (type == true) {
        HDF_LOGD("[HDF_TEST] %{public}s line:%{public}d test 1 time use time:%{public}lu s %{public}lu us",
            func, line, diff->sec, diff->usec);
        printf("[HDF_TEST] %s line:%d test 1 time use time:%lu s %lu us\r\n", func, line, diff->sec, diff->usec);
        g_totalUsec[index] += diff->sec * HDF_MICRO_UNIT + diff->usec;
    } else {
        g_totalUsec[index] += diff->sec * HDF_MICRO_UNIT + diff->usec;
    }
    g_index[index]++;
}

static void CheckAverageTime()
{
    HDF_LOGD("[HDF_TEST] %{public}s line:%{public}d test reg %{public}d time use time:%{public}lu us",
        __func__, __LINE__, g_index[ITEM_REG], g_totalUsec[ITEM_REG]);
    printf("[HDF_TEST] %s line:%d test reg %d time use time: %lu us\r\n",
        __func__, __LINE__, g_index[ITEM_REG], g_totalUsec[ITEM_REG]);

    HDF_LOGD("[HDF_TEST] %{public}s line:%{public}d test unreg %{public}d time use time:%{public}lu us",
        __func__, __LINE__, g_index[ITEM_UNREG], g_totalUsec[ITEM_UNREG]);
    printf("[HDF_TEST] %s line:%d test unreg %d time use time: %lu us\r\n",
        __func__, __LINE__, g_index[ITEM_UNREG], g_totalUsec[ITEM_UNREG]);

    UT_TEST_CHECK_RET((g_totalUsec[ITEM_REG] / HDF_KILO_UNIT / g_index[ITEM_REG]) <= HDF_REG_TIME_RANGE, true);
    UT_TEST_CHECK_RET((g_totalUsec[ITEM_UNREG] / HDF_KILO_UNIT / g_index[ITEM_UNREG]) <= HDF_REG_TIME_RANGE, true);
}

static int32_t HdfRegVirtualDevice(const char *moduleName, const char *serviceName)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return HDF_DEV_ERR_NO_DEVICE_SERVICE;
    }

    int32_t ret = iDevmgr->RegVirtualDevice(iDevmgr, moduleName, serviceName);

    HDIDeviceManagerRelease(iDevmgr);
    return ret;
}

static int32_t HdfUnRegVirtualDevice(const char *moduleName, const char *serviceName)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return HDF_DEV_ERR_NO_DEVICE_SERVICE;
    }

    int32_t ret = iDevmgr->UnRegVirtualDevice(iDevmgr, moduleName, serviceName);

    HDIDeviceManagerRelease(iDevmgr);
    return ret;
}

static void HdfRegOneVirtualTest(
    int index, bool type, const char *moduleName, const char *serviceName, const char *func, int line, int result)
{
    int32_t ret;
    OsalTimespec time1 = { 0, 0 };
    OsalTimespec time2 = { 0, 0 };
    OsalTimespec diff = { 0, 0 };
    GET_TIME_BEGIN(&time1);
    ret = HdfRegVirtualDevice(moduleName, serviceName);
    if (ret != HDF_SUCCESS) {
        ret = HDF_FAILURE;
    }
    GET_TIME_END(index, type, &time1, &time2, &diff, func, line);
    UT_TEST_CHECK_RET(ret == result, type);
    if (ret == result) {
        HDF_LOGE("[HDF_TEST] %{public}s line:%{public}d", func, line);
    } else {
        HDF_LOGE("[HDF_TEST] %{public}s line:%{public}d %{public}d", func, line, ret);
        printf("[HDF_TEST] %s line:%d %d\r\n", func, line, ret);
    }
}

static void HdfUnRegOneVirtualTest(
    int index, bool type, const char *moduleName, const char *serviceName, const char *func, int line, int result)
{
    int32_t ret;
    OsalTimespec time1 = { 0, 0 };
    OsalTimespec time2 = { 0, 0 };
    OsalTimespec diff = { 0, 0 };
    GET_TIME_BEGIN(&time1);
    ret = HdfUnRegVirtualDevice(moduleName, serviceName);
    GET_TIME_END(index, type, &time1, &time2, &diff, func, line);
    if (ret != HDF_SUCCESS) {
        ret = HDF_FAILURE;
    }

    UT_TEST_CHECK_RET(ret == result, type);
    if (ret == result) {
        HDF_LOGE("[HDF_TEST] %{public}s line:%{public}d", __func__, __LINE__);
    } else {
        HDF_LOGE("[HDF_TEST] %{public}s line:%{public}d %{public}d", __func__, __LINE__, ret);
    }
}

/*
 * HDF_VIRTUAL_DEVICE_BaseFunction_AR_001 HDF_VIRTUAL_DEVICE_BaseFunction_AR_002
 * HDF_VIRTUAL_DEVICE_BaseFunction_AR_007
 */
static void HdfRegVirtualTest(bool type)
{
    /* HDF_VIRTUAL_DEVICE_BaseFunction_AR_005 */
    HdfRegOneVirtualTest(ITEM_REG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME, __func__, __LINE__, HDF_SUCCESS);
    HdfRegOneVirtualTest(ITEM_REG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME, __func__, __LINE__, HDF_FAILURE);
    HdfRegOneVirtualTest(ITEM_REG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME1, __func__, __LINE__, HDF_SUCCESS);
    HdfRegOneVirtualTest(ITEM_REG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME2, __func__, __LINE__, HDF_SUCCESS);
    HdfRegOneVirtualTest(ITEM_REG, type, VIRTUAL_MODULE_NAME_NO, VIRTUAL_SERVICE_NO_DRIVER,
        __func__, __LINE__, HDF_FAILURE);
}

/*
 * HDF__VIRTUAL_DEVICE_BaseFunction_AR_003 HDF__VIRTUAL_DEVICE_BaseFunction_AR_004
 * HDF_VIRTUAL_DEVICE_BaseFunction_AR_008
 */
static void HdfUnRegVirtualTest(bool type)
{
    HdfUnRegOneVirtualTest(ITEM_UNREG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME,
        __func__, __LINE__, HDF_SUCCESS);
    HdfUnRegOneVirtualTest(ITEM_UNREG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME1,
        __func__, __LINE__, HDF_SUCCESS);
    HdfUnRegOneVirtualTest(ITEM_UNREG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME2,
        __func__, __LINE__, HDF_SUCCESS);
    HdfUnRegOneVirtualTest(ITEM_UNREG, type, VIRTUAL_MODULE_NAME, VIRTUAL_SERVICE_NAME2,
        __func__, __LINE__, HDF_FAILURE);
    /* HDF_VIRTUAL_DEVICE_BaseFunction_AR_006 */
    HdfUnRegOneVirtualTest(ITEM_UNREG, type, VIRTUAL_MODULE_NAME, "test_no_unregister_123",
        __func__, __LINE__, HDF_FAILURE);
    HdfUnRegOneVirtualTest(ITEM_UNREG, type, VIRTUAL_MODULE_NAME_NO, VIRTUAL_SERVICE_NO_DRIVER,
        __func__, __LINE__, HDF_FAILURE);
}

static bool CheckAllService(const char *name, bool usable)
{
    char **arr = NULL;
    int32_t serviceCnt;
    if (usable) {
        arr = g_testServiceName;
        serviceCnt = SERVICE_CNT;
    } else {
        arr = g_testServiceNameNoDrv;
        serviceCnt = SERVICE_NO_DRIVER_CNT;
    }
    for (int i = 0; i < serviceCnt; i++) {
        if (strcmp(name, arr[i]) == 0) {
            return true;
        }
    }
    return false;
}


void HdfFreeQueryDeviceList(struct DeviceInfoList *list)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return;
    }

    iDevmgr->FreeQueryDeviceList(iDevmgr, list);

    HDIDeviceManagerRelease(iDevmgr);
}

int32_t HdfQueryUsableDeviceInfo(struct DeviceInfoList *list)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return HDF_DEV_ERR_NO_DEVICE_SERVICE;
    }

    int32_t ret = iDevmgr->QueryUsableDeviceInfo(iDevmgr, list);

    HDIDeviceManagerRelease(iDevmgr);
    return ret;
}

int32_t HdfQueryUnusableDeviceInfo(struct DeviceInfoList *list)
{
    struct HDIDeviceManager *iDevmgr = HDIDeviceManagerGet();
    if (iDevmgr == NULL) {
        return HDF_DEV_ERR_NO_DEVICE_SERVICE;
    }

    int32_t ret = iDevmgr->QueryUnusableDeviceInfo(iDevmgr, list);

    HDIDeviceManagerRelease(iDevmgr);
    return ret;
}


static void RegQueryDevice(bool printFlag, bool flag, bool usable)
{
    int32_t ret;
    int32_t serviceCnt;
    struct DeviceInfoList list;
    struct DeviceInfoNode *devNode = NULL;
    int cnt = 0;
    char *debugInfo = NULL;
    if (usable) {
        ret = HdfQueryUsableDeviceInfo(&list);
        serviceCnt = SERVICE_CNT;
        debugInfo = "usable";
    } else {
        ret = HdfQueryUnusableDeviceInfo(&list);
        serviceCnt = 0;
        debugInfo = "un usable";
    }
    UT_TEST_CHECK_RET(ret == HDF_SUCCESS, printFlag);
    if (ret == HDF_SUCCESS) {
        HDF_LOGD("[HDF_TEST] %{public}s %{public}s:%{public}d line:%{public}d",
            __func__, debugInfo, list.deviceCnt, __LINE__);
        DLIST_FOR_EACH_ENTRY(devNode, &list.list, struct DeviceInfoNode, node) {
            HDF_LOGD("[HDF_TEST] %{public}s: %{public}s %{public}d", debugInfo, devNode->svcName, devNode->deviceType);
            if (CheckAllService(devNode->svcName, usable)) {
                cnt++;
            }
        }
        if (flag) {
            UT_TEST_CHECK_RET(cnt == serviceCnt, printFlag);
            if (cnt != serviceCnt) {
                HDF_LOGE("[HDF_TEST] %{public}s find usable service failed %{public}d != %{public}d",
                    __func__, cnt, serviceCnt);
                printf("[HDF_TEST] %s find usable service failed %d != %d\r\n", __func__, cnt, serviceCnt);
            }
        } else {
            UT_TEST_CHECK_RET(cnt == 0, printFlag);
            if (cnt != 0) {
                HDF_LOGE("[HDF_TEST] %{public}s find unusable service failed %{public}d != %{public}d",
                    __func__, cnt, 0);
                printf("[HDF_TEST] %s find unusable service failed %d != %d\r\n", __func__, cnt, 0);
            }
        }
    } else {
        HDF_LOGE("[HDF_TEST] %{public}s line:%{public}d", __func__, __LINE__);
        UT_TEST_CHECK_RET(false, true);
    }
    HdfFreeQueryDeviceList(&list);
}

int main(int argc, char **argv)
{
    int testCnt = 1;
    bool printFlag = true;
    (void)argv;
    if (argc != 1) {
        testCnt = HDF_REG_DEVICE_MORE;
        printFlag = false;
    }
    for (int i = 0; i < testCnt; i++) {
        HdfRegVirtualTest(printFlag);
        RegQueryDevice(printFlag, true, true);
        RegQueryDevice(printFlag, true, false);
        HdfUnRegVirtualTest(printFlag);
        RegQueryDevice(printFlag, false, true);
        RegQueryDevice(printFlag, false, false);
    }
    if (argc != 1) {
        CheckAverageTime();
    }
    PrintTestResult();
    return 0;
}

