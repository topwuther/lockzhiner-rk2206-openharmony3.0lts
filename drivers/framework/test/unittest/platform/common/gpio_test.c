/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_test.h"
#include "device_resource_if.h"
#include "gpio_if.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_irq.h"
#include "osal_time.h"

#define HDF_LOG_TAG gpio_test

#define GPIO_TEST_IRQ_TIMEOUT 1000
#define GPIO_TEST_IRQ_DELAY   200

static int32_t GpioTestSetUp(struct GpioTester *tester)
{
    int32_t ret;
    if (tester == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    ret = GpioGetDir(tester->gpio, &tester->oldDir);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get old dir fail! ret:%d", __func__, ret);
        return ret;
    }
    ret = GpioRead(tester->gpio, &tester->oldVal);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read old val fail! ret:%d", __func__, ret);
        return ret;
    }

    tester->fails = 0;
    tester->irqCnt = 0;
    tester->irqTimeout = GPIO_TEST_IRQ_TIMEOUT;
    return HDF_SUCCESS;
}

static int32_t GpioTestTearDown(struct GpioTester *tester)
{
    int ret;
    if (tester == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    ret = GpioSetDir(tester->gpio, tester->oldDir);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set old dir fail! ret:%d", __func__, ret);
        return ret;
    }
    if (tester->oldDir == GPIO_DIR_IN) {
        return HDF_SUCCESS;
    }
    ret = GpioWrite(tester->gpio, tester->oldVal);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: write old val fail! ret:%d", __func__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t TestCaseGpioSetGetDir(struct GpioTester *tester)
{
    int32_t ret;
    uint16_t dirSet;
    uint16_t dirGet;

    dirSet = GPIO_DIR_OUT;
    dirGet = GPIO_DIR_IN;

SET_GET_DIR:
    ret = GpioSetDir(tester->gpio, dirSet);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set dir fail! ret:%d", __func__, ret);
        return ret;
    }
    ret = GpioGetDir(tester->gpio, &dirGet);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get dir fail! ret:%d", __func__, ret);
        return ret;
    }
    if (dirSet != dirGet) {
        HDF_LOGE("%s: set dir:%u, but get:%u", __func__, dirSet, dirGet);
        return HDF_FAILURE;
    }
    /* change the value and test one more time */
    if (dirSet == GPIO_DIR_OUT) {
        dirSet = GPIO_DIR_IN;
        dirGet = GPIO_DIR_OUT;
        goto SET_GET_DIR;
    }
    return HDF_SUCCESS;
}

static int32_t TestCaseGpioWriteRead(struct GpioTester *tester)
{
    int32_t ret;
    uint16_t valWrite;
    uint16_t valRead;

    ret = GpioSetDir(tester->gpio, GPIO_DIR_OUT);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set dir fail! ret:%d", __func__, ret);
        return ret;
    }
    valWrite = GPIO_VAL_LOW;
    valRead = GPIO_VAL_HIGH;

WRITE_READ_VAL:
    ret = GpioWrite(tester->gpio, valWrite);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: write val:%u fail! ret:%d", __func__, valWrite, ret);
        return ret;
    }
    ret = GpioRead(tester->gpio, &valRead);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read fail! ret:%d", __func__, ret);
        return ret;
    }
    if (valWrite != valRead) {
        HDF_LOGE("%s: write:%u, but get:%u", __func__, valWrite, valRead);
        return HDF_FAILURE;
    }
    /* change the value and test one more time */
    if (valWrite == GPIO_VAL_HIGH) {
        valWrite = GPIO_VAL_HIGH;
        valRead = GPIO_VAL_LOW;
        goto WRITE_READ_VAL;
    }
    return HDF_SUCCESS;
}

static int32_t TestCaseGpioIrqHandler(uint16_t gpio, void *data)
{
    struct GpioTester *tester = (struct GpioTester *)data;

    HDF_LOGE("%s: >>>>>>>>>>>>>>>>>>>>>enter gpio:%u<<<<<<<<<<<<<<<<<<<<<<", __func__, gpio);
    if (tester != NULL) {
        tester->irqCnt++;
        return GpioDisableIrq(gpio);
    }
    return HDF_FAILURE;
}

static inline void TestHelperGpioInverse(uint16_t gpio, uint16_t mode)
{
    uint16_t dir = 0;
    uint16_t valRead = 0;

    (void)GpioRead(gpio, &valRead);
    (void)GpioWrite(gpio, (valRead == GPIO_VAL_LOW) ? GPIO_VAL_HIGH : GPIO_VAL_LOW);
    (void)GpioRead(gpio, &valRead);
    (void)GpioGetDir(gpio, &dir);
    HDF_LOGD("%s, gpio:%u, val:%u, dir:%u, mode:%x", __func__, gpio, valRead, dir, mode);
}

static int32_t TestCaseGpioIrq(struct GpioTester *tester, uint16_t mode, bool inverse)
{
    int32_t ret;
    uint32_t timeout;

    ret = GpioSetIrq(tester->gpioIrq, mode, TestCaseGpioIrqHandler, (void *)tester);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set irq fail! ret:%d", __func__, ret);
        return ret;
    }
    ret = GpioEnableIrq(tester->gpioIrq);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: enable irq fail! ret:%d", __func__, ret);
        (void)GpioUnSetIrq(tester->gpioIrq);
        return ret;
    }

    for (timeout = 0; tester->irqCnt <= 0 && timeout <= tester->irqTimeout;
        timeout += GPIO_TEST_IRQ_DELAY) {
        if (inverse) {
            // maybe can make a inverse ...
            TestHelperGpioInverse(tester->gpioIrq, mode);
        }
        OsalMSleep(GPIO_TEST_IRQ_DELAY);
    }
    (void)GpioUnSetIrq(tester->gpioIrq);

#if defined(_LINUX_USER_) || defined(__KERNEL__)
    if (inverse) {
        HDF_LOGI("%s: do not judge edge trigger!", __func__);
        return HDF_SUCCESS;
    }
#endif
    if (tester->irqCnt <= 0) {
        HDF_LOGE("%s: set mode:%x on %u failed", __func__, mode, tester->gpioIrq);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t TestCaseGpioIrqLevel(struct GpioTester *tester)
{
    (void)tester;
    return HDF_SUCCESS;
}

static int32_t TestCaseGpioIrqEdge(struct GpioTester *tester)
{
    uint16_t mode;

    /* set dir to out for self trigger on liteos */
#if defined(_LINUX_USER_) || defined(__KERNEL__)
    (void)GpioSetDir(tester->gpioIrq, GPIO_DIR_IN);
#else
    (void)GpioSetDir(tester->gpioIrq, GPIO_DIR_OUT);
#endif
    mode = GPIO_IRQ_TRIGGER_FALLING | GPIO_IRQ_TRIGGER_RISING;
    return TestCaseGpioIrq(tester, mode, true);
}

int32_t TestCaseGpioIrqThread(struct GpioTester *tester)
{
    uint16_t mode;
    /* set dir to out for self trigger on liteos */
#if defined(_LINUX_USER_) || defined(__KERNEL__)
    (void)GpioSetDir(tester->gpioIrq, GPIO_DIR_IN);
#else
    (void)GpioSetDir(tester->gpioIrq, GPIO_DIR_OUT);
#endif
    mode = GPIO_IRQ_TRIGGER_FALLING | GPIO_IRQ_TRIGGER_RISING | GPIO_IRQ_USING_THREAD;
    return TestCaseGpioIrq(tester, mode, true);
}

static int32_t TestCaseGpioReliability(struct GpioTester *tester)
{
    uint16_t val = 0;

    (void)GpioWrite(-1, val);              /* invalid gpio number */
    (void)GpioWrite(tester->gpio, -1);     /* invalid gpio value */

    (void)GpioRead(-1, &val);              /* invalid gpio number */
    (void)GpioRead(tester->gpio, NULL);    /* invalid pointer */

    (void)GpioSetDir(-1, val);             /* invalid gpio number */
    (void)GpioSetDir(tester->gpio, -1);    /* invalid value */

    (void)GpioGetDir(-1, &val);            /* invalid gpio number */
    (void)GpioGetDir(tester->gpio, NULL);  /* invalid pointer */

    /* invalid gpio number */
    (void)GpioSetIrq(-1, OSAL_IRQF_TRIGGER_RISING, TestCaseGpioIrqHandler, (void *)tester);
    /* invalid irq mode */
    (void)GpioSetIrq(tester->gpioIrq, -1, TestCaseGpioIrqHandler, (void *)tester);
    /* invalid irq handler */
    (void)GpioSetIrq(tester->gpioIrq, OSAL_IRQF_TRIGGER_RISING, NULL, (void *)tester);
    /* invalid irq data */
    (void)GpioSetIrq(tester->gpioIrq, OSAL_IRQF_TRIGGER_RISING, TestCaseGpioIrqHandler, NULL);

    (void)GpioUnSetIrq(-1);                /* invalid gpio number */

    (void)GpioEnableIrq(-1);               /* invalid gpio number */

    (void)GpioDisableIrq(-1);              /* invalid gpio number */

    return HDF_SUCCESS;
}

static int32_t GpioTestByCmd(struct GpioTester *tester, int32_t cmd)
{
    int32_t i;

    if (cmd == GPIO_TEST_SET_GET_DIR) {
        return TestCaseGpioSetGetDir(tester);
    } else if (cmd == GPIO_TEST_WRITE_READ) {
        return TestCaseGpioWriteRead(tester);
    } else if (cmd == GPIO_TEST_IRQ_LEVEL) {
        return TestCaseGpioIrqLevel(tester);
    } else if (cmd == GPIO_TEST_IRQ_EDGE) {
        return TestCaseGpioIrqEdge(tester);
    } else if (cmd == GPIO_TEST_IRQ_THREAD) {
        return TestCaseGpioIrqThread(tester);
    } else if (cmd == GPIO_TEST_RELIABILITY) {
        return TestCaseGpioReliability(tester);
    }

    for (i = 0; i < GPIO_TEST_MAX; i++) {
        if (GpioTestByCmd(tester, i) != HDF_SUCCESS) {
            tester->fails++;
        }
    }
    HDF_LOGE("%s: **********PASS:%u  FAIL:%u**************\n\n",
        __func__, tester->total - tester->fails, tester->fails);
    return (tester->fails > 0) ? HDF_FAILURE : HDF_SUCCESS;
}

static int32_t GpioTestDoTest(struct GpioTester *tester, int32_t cmd)
{
    int32_t ret;

    if (tester == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = GpioTestSetUp(tester);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: setup fail!", __func__);
        return ret;
    }

    ret = GpioTestByCmd(tester, cmd);

    (void)GpioTestTearDown(tester);
    return ret;
}

static int32_t GpioTestReadConfig(struct GpioTester *tester, const struct DeviceResourceNode *node)
{
    int32_t ret;
    uint32_t tmp;
    struct DeviceResourceIface *drsOps = NULL;

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("%s: invalid drs ops fail!", __func__);
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint32(node, "gpio", &tmp, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read gpio fail!", __func__);
        return ret;
    }
    tester->gpio = (uint16_t)tmp;

    ret = drsOps->GetUint32(node, "gpioIrq", &tmp, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read gpioIrq fail!", __func__);
        return ret;
    }
    tester->gpioIrq = (uint16_t)tmp;
    tester->doTest = GpioTestDoTest;

    return HDF_SUCCESS;
}

static int32_t GpioTestBind(struct HdfDeviceObject *device)
{
    int32_t ret;
    static struct GpioTester tester;

    if (device == NULL || device->property == NULL) {
        HDF_LOGE("%s: device or property is null!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = GpioTestReadConfig(&tester, device->property);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read config fail!", __func__);
        return ret;
    }

    tester.total = GPIO_TEST_MAX;
    device->service = &tester.service;

#ifdef GPIO_TEST_ON_INIT
    HDF_LOGE("%s: test on init!", __func__);
    tester.doTest(&tester, -1);
#endif
    return HDF_SUCCESS;
}

static int32_t GpioTestInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void GpioTestRelease(struct HdfDeviceObject *device)
{
    if (device != NULL) {
        device->service = NULL;
    }
    return;
}

struct HdfDriverEntry g_gpioTestEntry = {
    .moduleVersion = 1,
    .Bind = GpioTestBind,
    .Init = GpioTestInit,
    .Release = GpioTestRelease,
    .moduleName = "PLATFORM_GPIO_TEST",
};
HDF_INIT(g_gpioTestEntry);
