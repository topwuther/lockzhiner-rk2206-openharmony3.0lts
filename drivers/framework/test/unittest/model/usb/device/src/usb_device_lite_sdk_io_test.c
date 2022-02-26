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

static void ReadComplete(uint8_t pipe, struct UsbFnRequest *req)
{
    if (NULL == req) {
        return;
    }
    if (req->actual) {
        uint8_t *data = (uint8_t *)req->buf;
        data[req->actual] = '\0';
        dprintf("receive [%d] bytes data: %s\n", req->actual, data);
        if (strcmp((const char *)data, "q") == 0 || \
            strcmp((const char *)data, "q\n") == 0) {
            g_acmDevice->submitExit = 1;
        }
    }
    g_acmDevice->submit = 1;
}

int32_t UsbFnDviceTestRequestAsync(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;

    ret = UsbFnSubmitRequestAsync(req);
    if (HDF_SUCCESS == ret) {
        HDF_LOGE("%s: async Request success!!", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestAsync002(void)
{
    struct UsbFnRequest *req = NULL;
    int ret = HDF_SUCCESS;
    int ret1;
    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("wait receiving data form host, please connect\n");
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    req->complete = ReadComplete;
    req->context = g_acmDevice;
    while (g_acmDevice->connect == false) {
        OsalMSleep(WAIT_100MS);
    }
    while (1) {
        g_acmDevice->submit = 0;
        ret = UsbFnSubmitRequestAsync(req);
        if (HDF_SUCCESS != ret) {
            HDF_LOGE("%s: async Request error", __func__);
            ret = HDF_FAILURE;
            break;
        }
        while (g_acmDevice->submit == 0) {
            OsalMSleep(WAIT_100MS);
        }
        if (req->actual > 0) {
            break;
        }
    }
    ret1 = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret1) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return ret;
}

int32_t UsbFnDviceTestRequestAsync003(void)
{
    struct UsbFnRequest *req = NULL;
    int ret = HDF_SUCCESS;
    int ret1;
    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("wait receiving data form host, please connect\n");
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    req->complete = ReadComplete;
    req->context = g_acmDevice;
    while (g_acmDevice->connect == false) {
        OsalMSleep(WAIT_100MS);
    }
    g_acmDevice->submitExit = 0;
    while (g_acmDevice->submitExit == 0) {
        g_acmDevice->submit = 0;
        ret = UsbFnSubmitRequestAsync(req);
        if (HDF_SUCCESS != ret) {
            HDF_LOGE("%s: async Request error", __func__);
            ret = HDF_FAILURE;
            break;
        }
        while (g_acmDevice->submit == 0) {
            OsalMSleep(WAIT_100MS);
        }
    }
    ret1 = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret1) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return ret;
}

static void WriteComplete(uint8_t pipe, struct UsbFnRequest *req)
{
    dprintf("write data status = %d\n", req->status);
    g_acmDevice->submit = 1;
}

int32_t UsbFnDviceTestRequestAsync004(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
        g_acmDevice->dataInPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    req->complete = WriteComplete;
    req->context = g_acmDevice;
    g_acmDevice->submit = 0;
    dprintf("------send \"abc\" to host------\n");
    memcpy_s(req->buf, g_acmDevice->dataInPipe.maxPacketSize, "abc", strlen("abc"));
    req->length = strlen("abc");
    ret = UsbFnSubmitRequestAsync(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: async Request error", __func__);
        return HDF_FAILURE;
    }
    while (g_acmDevice->submit == 0) {
        OsalMSleep(1);
    }
    g_acmDevice->submit = 0;
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestAsync005(void)
{
    struct UsbFnRequest *req = NULL;
    int loopTime = TEST_TIMES;
    int ret;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("------send \"xyz\" 10 times to host------\n");
    while (loopTime--) {
        req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
            g_acmDevice->dataInPipe.maxPacketSize);
        if (req == NULL) {
            HDF_LOGE("%s: alloc req fail", __func__);
            return HDF_FAILURE;
        }
        req->complete = WriteComplete;
        req->context = g_acmDevice;
        g_acmDevice->submit = 0;
        memcpy_s(req->buf, g_acmDevice->dataInPipe.maxPacketSize, "xyz", strlen("xyz"));
        req->length = strlen("xyz");
        ret = UsbFnSubmitRequestAsync(req);
        if (HDF_SUCCESS != ret) {
            HDF_LOGE("%s: async Request error", __func__);
            return HDF_FAILURE;
        }
        while (g_acmDevice->submit == 0) {
            OsalMSleep(1);
        }
        g_acmDevice->submit = 0;
        ret = UsbFnFreeRequest(req);
        if (HDF_SUCCESS != ret) {
            HDF_LOGE("%s: free Request error", __func__);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestAsync006(void)
{
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;
    ret = UsbFnSubmitRequestSync(req, 0);
    if (HDF_SUCCESS == ret) {
        HDF_LOGE("%s: sync Request success!!", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync002(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;
    uint8_t *data = NULL;
    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("wait receiving data form host\n");
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnSubmitRequestSync(req, 0);
    if (ret != 0 || req->actual <= 0 || req->status != USB_REQUEST_COMPLETED) {
        HDF_LOGE("%s: Sync Request error", __func__);
        return HDF_FAILURE;
    }
    data = (uint8_t *)req->buf;
    data[req->actual] = '\0';
    dprintf("receive data from host: %s\n", data);
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync003(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;
    int submitExit = 0;
    uint8_t *data = NULL;
    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("receive data until 'q' exit\n");
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    while (submitExit == 0) {
        ret = UsbFnSubmitRequestSync(req, 0);
        if (ret != 0 || req->actual <= 0 || req->status != USB_REQUEST_COMPLETED) {
            HDF_LOGE("%s: Sync Request error", __func__);
            break;
        }
        data = (uint8_t *)req->buf;
        data[req->actual] = '\0';
        if (strcmp((const char *)data, "q") == 0 || \
            strcmp((const char *)data, "q\n") == 0) {
            submitExit = 1;
        }
        dprintf("receive data from host: %s\n", data);
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync004(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
        g_acmDevice->dataInPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    dprintf("------send \"abc\" to host------\n");
    memcpy_s(req->buf, g_acmDevice->dataInPipe.maxPacketSize, "abc", strlen("abc"));
    req->length = strlen("abc");
    ret = UsbFnSubmitRequestSync(req, 0);
    if (HDF_SUCCESS != ret || (req->actual != strlen("abc")) || \
        (req->status != USB_REQUEST_COMPLETED)) {
        HDF_LOGE("%s: async Request error", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync005(void)
{
    struct UsbFnRequest *req = NULL;
    int loopTime = TEST_TIMES;
    int ret;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("------send \"abcdefg\" 10 times to host------\n");
    while (loopTime--) {
        req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
            g_acmDevice->dataInPipe.maxPacketSize);
        if (req == NULL) {
            HDF_LOGE("%s: alloc req fail", __func__);
            return HDF_FAILURE;
        }
        memcpy_s(req->buf, g_acmDevice->dataInPipe.maxPacketSize, "abcdefg", strlen("abcdefg"));
        req->length = strlen("abcdefg");
        ret = UsbFnSubmitRequestSync(req, 0);
        if (HDF_SUCCESS != ret || (req->actual != strlen("abcdefg")) || \
            (req->status != USB_REQUEST_COMPLETED)) {
            HDF_LOGE("%s: async Request error", __func__);
            return HDF_FAILURE;
        }
        ret = UsbFnFreeRequest(req);
        if (HDF_SUCCESS != ret) {
            HDF_LOGE("%s: free Request error", __func__);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync006(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;
    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.fn is invail", __func__);
        return HDF_FAILURE;
    }
    dprintf("test sync timeout 5s:\n");
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnSubmitRequestSync(req, SYNC_5000MS);
    if (ret == 0) {
        HDF_LOGE("%s: Sync Request success", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestRequestSync007(void)
{
    struct UsbFnRequest *req = NULL;
    int ret;

    ret = UsbFnSubmitRequestSync(req, SYNC_5000MS);
    if (ret == 0) {
        HDF_LOGE("%s: Sync Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static void TestCancelComplete(uint8_t pipe, struct UsbFnRequest *req)
{
    dprintf("%s, req->buf = 0x%x\n", __func__, (uint32_t)req->buf);
    g_acmDevice->havedSubmit = true;
}

int32_t UsbFnDviceTestCancelRequest(void)
{
    int ret;
    struct UsbFnRequest *notifyReq = NULL;
    ret = UsbFnCancelRequest(notifyReq);
    if (HDF_SUCCESS == ret) {
        HDF_LOGE("%s: cancel request success!!", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestCancelRequest002(void)
{
    int ret;
    struct UsbFnRequest *req = NULL;

    if (g_acmDevice == NULL || g_acmDevice->ctrlIface.handle == NULL) {
        HDF_LOGE("%s: CtrlIface.handle is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocCtrlRequest(g_acmDevice->ctrlIface.handle,
        sizeof(struct UsbCdcNotification));
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnCancelRequest(req);
    if (HDF_SUCCESS == ret) {
        HDF_LOGE("%s: cancel request success", __func__);
        ret = UsbFnFreeRequest(req);
        if (HDF_SUCCESS != ret) {
            HDF_LOGE("%s: free Request error", __func__);
            return HDF_FAILURE;
        }
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestCancelRequest003(void)
{
    int ret;
    struct UsbFnRequest *req = NULL;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.handle is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
        g_acmDevice->dataInPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnCancelRequest(req);
    if (HDF_SUCCESS == ret) {
        dprintf("%s: cancel request success\n", __func__);
        ret = UsbFnFreeRequest(req);
        if (HDF_SUCCESS != ret) {
            dprintf("%s: free Request error", __func__);
            return HDF_FAILURE;
        }
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestCancelRequest004(void)
{
    int ret;
    struct UsbFnRequest *req = NULL;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.handle is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
        g_acmDevice->dataInPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnCancelRequest(req);
    if (HDF_SUCCESS == ret) {
        dprintf("%s: cancel request success\n", __func__);
        ret = UsbFnFreeRequest(req);
        if (HDF_SUCCESS != ret) {
            dprintf("%s: free Request error", __func__);
            return HDF_FAILURE;
        }
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestCancelRequest005(void)
{
    int ret;
    struct UsbFnRequest *req = NULL;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.handle is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataInPipe.id,
        g_acmDevice->dataInPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    g_acmDevice->havedSubmit = false;
    req->complete = TestCancelComplete;
    req->context = g_acmDevice;
    dprintf("------send \"abc\" to host------\n");
    memcpy_s(req->buf, g_acmDevice->dataInPipe.maxPacketSize, "abc", strlen("abc"));
    req->length = strlen("abc");
    ret = UsbFnSubmitRequestAsync(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: request async error", __func__);
        return HDF_FAILURE;
    }
    while (g_acmDevice->havedSubmit == 0) {
        OsalMSleep(1);
    }
    ret = UsbFnCancelRequest(req);
    if (HDF_SUCCESS == ret) {
        dprintf("%s: cancel request error", __func__);
        return HDF_FAILURE;
    }
    g_acmDevice->havedSubmit = false;
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t UsbFnDviceTestCancelRequest006(void)
{
    int ret;
    struct UsbFnRequest *req = NULL;
    struct UsbFnRequest *req2 = NULL;

    if (g_acmDevice == NULL || g_acmDevice->dataIface.handle == NULL) {
        HDF_LOGE("%s: dataIface.handle is invail", __func__);
        return HDF_FAILURE;
    }
    req = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    g_acmDevice->havedSubmit = false;
    req->complete = TestCancelComplete;
    req->context = g_acmDevice;

    req2 = UsbFnAllocRequest(g_acmDevice->dataIface.handle, g_acmDevice->dataOutPipe.id,
        g_acmDevice->dataOutPipe.maxPacketSize);
    if (req2 == NULL) {
        HDF_LOGE("%s: alloc req fail", __func__);
        return HDF_FAILURE;
    }
    g_acmDevice->submit = false;
    req2->complete = ReadComplete;
    req2->context = g_acmDevice;
    ret = UsbFnSubmitRequestAsync(req);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: request async error", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnSubmitRequestAsync(req2);
    if (HDF_SUCCESS != ret) {
        HDF_LOGE("%s: request async error", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnCancelRequest(req2);
    if (HDF_SUCCESS != ret) {
        dprintf("%s: cancel request error", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnCancelRequest(req);
    if (HDF_SUCCESS != ret) {
        dprintf("%s: cancel request error", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req);
    if (HDF_SUCCESS != ret) {
        dprintf("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    ret = UsbFnFreeRequest(req2);
    if (HDF_SUCCESS != ret) {
        dprintf("%s: free Request error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

