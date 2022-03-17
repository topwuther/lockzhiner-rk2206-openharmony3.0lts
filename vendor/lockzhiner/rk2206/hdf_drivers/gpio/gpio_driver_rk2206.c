/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

#include <stdint.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal.h"
#include "gpio_core.h"

#include "lz_hardware.h"

#include "gpio_service_rk2206.h"


#define HDF_LOG_TAG     gpio_driver_rk2206


typedef struct tag_gpio_ctrl {
    struct GpioCntlr cntlr;
    uint32_t phy_base;
    uint32_t reg_step;
    uint32_t group_num;
    uint32_t bit_num;
    uint32_t irq_start;
    uint8_t irq_share;
}gpio_ctrl_s;

static gpio_ctrl_s m_gpio_ctrl = {
    .group_num = 1,
    .bit_num = 32,
};


////////////////////////////////////////////////////////////

static int32_t gpio_request(struct GpioCntlr *cntlr, uint16_t local)
{
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;

    if (local >= pgpio_ctrl->bit_num)
    {
        HDF_LOGD("%s: local(%d) > gpio_num(%d)!", __func__, local, pgpio_ctrl->bit_num);
        return HDF_ERR_INVALID_PARAM;
    }

    LzGpioInit(local);
    return HDF_SUCCESS;
}

static int32_t gpio_release(struct GpioCntlr *cntlr, uint16_t local)
{
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;

    if (local >= pgpio_ctrl->bit_num)
    {
        HDF_LOGD("%s: local(%d) > gpio_num(%d)!\n", __func__, local, pgpio_ctrl->bit_num);
        return HDF_ERR_INVALID_PARAM;
    }

    LzGpioDeinit(local);
    return HDF_SUCCESS;
}

static int32_t gpio_write(struct GpioCntlr *cnltr, uint16_t gpio, uint16_t val)
{
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;
    uint32_t ret = 0;
    LzGpioValue gpio_value;

    if (gpio >= pgpio_ctrl->bit_num)
    {
        HDF_LOGD("%s: local(%d) > gpio_num(%d)!\n", __func__, gpio, pgpio_ctrl->bit_num);
        return HDF_ERR_INVALID_PARAM;
    }
    
    if (val == 0)
    {
        gpio_value = LZGPIO_LEVEL_LOW;
    }
    else if (val == 1)
    {
        gpio_value = LZGPIO_LEVEL_HIGH;
    }
    else
    {
        HDF_LOGD("%s: val(%d) out of the range!\n", __func__, val);
        return HDF_ERR_INVALID_PARAM;
    }
    
    ret = LzGpioSetVal(gpio, gpio_value);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        HDF_LOGD("%s: LzGpioSetVal() failed!\n", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    
    return HDF_SUCCESS;
}

static int32_t gpio_read(struct GpioCntlr *cnltr, uint16_t gpio, uint16_t *val)
{
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;
    uint32_t ret = 0;
    LzGpioValue gpio_value;

    if (gpio >= pgpio_ctrl->bit_num)
    {
        HDF_LOGD("%s: local(%d) > gpio_num(%d)!\n", __func__, gpio, pgpio_ctrl->bit_num);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = LzGpioGetVal(gpio, &gpio_value);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        HDF_LOGD("%s: LzGpioGetVal() failed!\n", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    *val = (uint16_t)(gpio_value);
    
    return HDF_SUCCESS;
}

static int32_t gpio_set_dir(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t dir)
{
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;
    uint32_t ret = 0;
    LzGpioDir gpio_dir;

    if (gpio >= pgpio_ctrl->bit_num)
    {
        HDF_LOGD("%s: local(%d) > gpio_num(%d)!", __func__, gpio, pgpio_ctrl->bit_num);
        return HDF_ERR_INVALID_PARAM;
    }

    if (dir == 0)
    {
        gpio_dir = GPIO_DIR_IN;
    }
    else if (dir == 1)
    {
        gpio_dir = GPIO_DIR_OUT;
    }
    else
    {
        HDF_LOGD("%s: dir(%d) out of the range!!", __func__, dir);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = LzGpioSetDir(gpio, gpio_dir);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        HDF_LOGD("%s: LzGpioSetDir() failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    
    return HDF_SUCCESS;
}

static int32_t gpio_get_dir(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t *dir)
{
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;
    uint32_t ret = 0;
    LzGpioDir gpio_dir;

    if (gpio >= pgpio_ctrl->bit_num)
    {
        HDF_LOGD("%s: local(%d) > gpio_num(%d)!", __func__, gpio, pgpio_ctrl->bit_num);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = LzGpioGetDir(gpio, &gpio_dir);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        HDF_LOGD("%s: LzGpioGetDir() failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    *dir = gpio_dir;
    
    return HDF_SUCCESS;
}

static struct GpioMethod g_rk2206_gpioMethod = {
    .request = gpio_request,
    .release = gpio_release,
    .write = gpio_write,
    .read = gpio_read,
    .setDir = gpio_set_dir,
    .getDir = gpio_get_dir,
    .toIrq = NULL,
    .setIrq = NULL,
    .unsetIrq = NULL,
    .enableIrq = NULL,
    .disableIrq = NULL,
};

////////////////////////////////////////////////////////////
static const char *GpioServiceGetData(void)
{
    return "gpio_driver_rk2206";
}

static int32_t GpioServiceSetData(const char *data)
{
    if (data == NULL)
    {
        return HDF_ERR_INVALID_PARAM;
    }
    HDF_LOGD("%s:%s", __func__, data);
    return HDF_SUCCESS;
}

//////////////////////////////////////////////////////////////
static void HdfGpioDriverRelease(struct HdfDeviceObject *deviceObject)
{
    (void)deviceObject;
    return;
}

static int HdfGpioDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGD("%s::enter, deviceObject=%p", __func__, deviceObject);
    if (deviceObject == NULL)
    {
        return HDF_FAILURE;
    }
    static struct GpioServiceRk2206 gpioService =
    {
        .getData = GpioServiceGetData,
        .setData = GpioServiceSetData,
    };
    deviceObject->service = &gpioService.service;
    return HDF_SUCCESS;
}

static int32_t TestCaseGpioSetGetDir(struct GpioTester *tester)
{
    int32_t ret;
    uint16_t dirSet;
    uint16_t dirGet;

    printf("%s, %d: test case\n", __FILE__, __LINE__);

    dirSet = GPIO_DIR_OUT;
    dirGet = GPIO_DIR_IN;

SET_GET_DIR:
    ret = LzGpioSetDir(tester->gpio, dirSet);
    if (ret != HDF_SUCCESS) {
        printf("%s: set dir fail! ret:%d\n", __func__, ret);
        return ret;
    }
    ret = LzGpioGetDir(tester->gpio, &dirGet);
    if (ret != HDF_SUCCESS) {
        printf("%s: get dir fail! ret:%d\n", __func__, ret);
        return ret;
    }
    if (dirSet != dirGet) {
        printf("%s: set dir:%u, but get:%u\n", __func__, dirSet, dirGet);
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

    printf("%s, %d: test case\n", __FILE__, __LINE__);

    ret = LzGpioSetDir(tester->gpio, GPIO_DIR_OUT);
    if (ret != HDF_SUCCESS) {
        printf("%s: set dir fail! ret:%d\n", __func__, ret);
        return ret;
    }
    valWrite = GPIO_VAL_LOW;
    valRead = GPIO_VAL_HIGH;

WRITE_READ_VAL:
    ret = gpio_write(NULL, tester->gpio, valWrite);
    if (ret != HDF_SUCCESS) {
        printf("%s: write val:%u fail! ret:%d", __func__, valWrite, ret);
        return ret;
    }
    ret = gpio_read(NULL, tester->gpio, &valRead);
    if (ret != HDF_SUCCESS) {
        printf("%s: read fail! ret:%d", __func__, ret);
        return ret;
    }
    if (valWrite != valRead) {
        printf("%s: write:%u, but get:%u", __func__, valWrite, valRead);
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

    printf("%s, %d: test case\n", __FILE__, __LINE__);

    if (tester != NULL) {
        tester->irqCnt++;
        //return GpioDisableIrq(gpio);
    }
    return HDF_FAILURE;
}

static inline void TestHelperGpioInverse(uint16_t gpio, uint16_t mode)
{
    uint16_t dir = 0;
    uint16_t valRead = 0;

    printf("%s, %d: test case\n", __FILE__, __LINE__);

    (void)gpio_read(NULL, gpio, &valRead);
    (void)gpio_write(NULL, gpio, (valRead == GPIO_VAL_LOW) ? GPIO_VAL_HIGH : GPIO_VAL_LOW);
    (void)gpio_read(NULL, gpio, &valRead);
    (void)LzGpioGetDir(gpio, &dir);
    HDF_LOGD("%s, gpio:%u, val:%u, dir:%u, mode:%x", __func__, gpio, valRead, dir, mode);
}

static int32_t TestCaseGpioIrqLevel(struct GpioTester *tester)
{
    printf("%s, %d: test case\n", __FILE__, __LINE__);

    (void)tester;
    return HDF_SUCCESS;
}

static int32_t GpioTestByCmd(struct GpioTester *tester, int32_t cmd)
{
    int32_t i;

    if (cmd == GPIO_TEST_SET_GET_DIR) {
        return TestCaseGpioSetGetDir(tester);
    } else if (cmd == GPIO_TEST_WRITE_READ) {
        return TestCaseGpioWriteRead(tester);
    }
#if 0
    else if (cmd == GPIO_TEST_IRQ_LEVEL) {
        return TestCaseGpioIrqLevel(tester);
    } else if (cmd == GPIO_TEST_IRQ_EDGE) {
        return TestCaseGpioIrqEdge(tester);
    } else if (cmd == GPIO_TEST_IRQ_THREAD) {
        return TestCaseGpioIrqThread(tester);
    } else if (cmd == GPIO_TEST_RELIABILITY) {
        return TestCaseGpioReliability(tester);
    }
#endif
    for (i = 0; i < GPIO_TEST_MAX; i++) {
        if (GpioTestByCmd(tester, i) != HDF_SUCCESS) {
            tester->fails++;
        }
    }
    printf("%s: **********PASS:%u  FAIL:%u**************\n\n",
        __func__, tester->total - tester->fails, tester->fails);
    return (tester->fails > 0) ? HDF_FAILURE : HDF_SUCCESS;
}

static int32_t GpioTestSetUp(struct GpioTester *tester)
{
    int32_t ret;
    if (tester == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    ret = LzGpioGetDir(tester->gpio, &tester->oldDir);
    if (ret != HDF_SUCCESS) {
        printf("%s: get old dir fail! ret:%d\n", __func__, ret);
        return ret;
    }
    ret = gpio_read(NULL, tester->gpio, &tester->oldVal);
    if (ret != HDF_SUCCESS) {
        printf("%s: read old val fail! ret:%d\n", __func__, ret);
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
    ret = LzGpioSetDir(tester->gpio, tester->oldDir);
    if (ret != HDF_SUCCESS) {
        printf("%s: set old dir fail! ret:%d\n", __func__, ret);
        return ret;
    }
    if (tester->oldDir == GPIO_DIR_IN) {
        return HDF_SUCCESS;
    }
    ret = gpio_write(NULL, tester->gpio, tester->oldVal);
    if (ret != HDF_SUCCESS) {
        printf("%s: write old val fail! ret:%d\n", __func__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t GpioTestDoTest(struct GpioTester *tester, int32_t cmd)
{
    int32_t ret;

    printf("%s, %d: Hdf Gpio Driver test!\n", __FILE__, __LINE__);

    if (tester == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = GpioTestSetUp(tester);
    if (ret != HDF_SUCCESS) {
        printf("%s: setup fail!\n", __func__);
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
        printf("%s: invalid drs ops fail!\n", __func__);
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint32(node, "gpio", &tmp, 0);
    if (ret != HDF_SUCCESS) {
        printf("%s: read gpio fail!\n", __func__);
        return ret;
    }
    tester->gpio = (uint16_t)tmp;

    ret = drsOps->GetUint32(node, "gpioIrq", &tmp, 0);
    if (ret != HDF_SUCCESS) {
        printf("%s: read gpioIrq fail!\n", __func__);
        return ret;
    }
    tester->gpioIrq = (uint16_t)tmp;

    printf("%s[%d] groupNum:%u gpio:%u gpioIrq:%u\n",
        __func__, __LINE__, tester->gpio, tester->gpioIrq);

    tester->doTest = GpioTestDoTest;

    return HDF_SUCCESS;
}

static int HdfGpioDriverInit(struct HdfDeviceObject *deviceObject)
{
    int32_t ret;
    gpio_ctrl_s *pgpio_ctrl = &m_gpio_ctrl;
    static struct GpioTester tester;

    printf("%s, %d: Hdf Gpio Driver entry!\n", __FILE__, __LINE__);
    HDF_LOGD("%s::enter, deviceObject=%p", __func__, deviceObject);
    if (deviceObject == NULL)
    {
        HDF_LOGE("%s::ptr is null!", __func__);
        return HDF_FAILURE;
    }

    pgpio_ctrl->cntlr.count = pgpio_ctrl->group_num * pgpio_ctrl->bit_num;
    pgpio_ctrl->cntlr.priv = (void *)deviceObject->property;
    pgpio_ctrl->cntlr.ops = &g_rk2206_gpioMethod;
    pgpio_ctrl->cntlr.device = deviceObject;
    ret = GpioCntlrAdd(&pgpio_ctrl->cntlr);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s: err add controller: %d\n", __func__, ret);
        return ret;
    }

    ret = GpioTestReadConfig(&tester, deviceObject->property);
    if (ret != HDF_SUCCESS) {
        printf("%s: read config fail!\n", __func__);
        return ret;
    }

    LzGpioInit(tester.gpio);
    tester.total = GPIO_TEST_MAX;
    deviceObject->service = &tester.service;

#if 1//def GPIO_TEST_ON_INIT
    HDF_LOGE("%s: test on init!", __func__);
    tester.doTest(&tester, -1);
#endif

    HDF_LOGD("%s:Init success", __func__);
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_gpioDriverEntry =
{
    .moduleVersion = 1,
    .moduleName = "gpio_driver_rk2206",
    .Bind = HdfGpioDriverBind,
    .Init = HdfGpioDriverInit,
    .Release = HdfGpioDriverRelease,
};

HDF_INIT(g_gpioDriverEntry);
