/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hks_ipc_slice.h"
#include "hks_ipc_serialization.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_request.h"

#define CMD_INIT_OFFSET      1 /* offset from base cmd to init cmd. */
#define CMD_UPDATE_OFFSET    2 /* offset from base cmd to update cmd. */
#define CMD_FINAL_OFFSET     3 /* offset from base cmd to final cmd. */

struct SliceParam {
    uint64_t operationHandle;
    struct HksBlob *inData;
    struct HksBlob *outData;
    const struct HksParamSet *paramSet;
};

static bool IsSliceCmd(uint32_t cmdId)
{
    return (cmdId == HKS_MSG_SIGN) || (cmdId == HKS_MSG_VERIFY) || (cmdId == HKS_MSG_ENCRYPT) ||
        (cmdId == HKS_MSG_DECRYPT) || (cmdId == HKS_MSG_MAC);
}

static uint32_t GetBlobBufSize(const struct HksBlob *blob, uint32_t *bufSize)
{
    if (IsAdditionOverflow(blob->size, DEFAULT_ALIGN_MASK_SIZE)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (IsAdditionOverflow(ALIGN_SIZE(blob->size), sizeof(blob->size))) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    *bufSize = ALIGN_SIZE(blob->size) + sizeof(blob->size);
    return HKS_SUCCESS;
}

static uint32_t GetParamSize(const struct HksBlob *key, const struct HksParamSet *paramSet, uint32_t *bufSize)
{
    if ((key->size > MAX_PROCESS_SIZE) || (paramSet->paramSetSize > MAX_PROCESS_SIZE)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    *bufSize = ALIGN_SIZE(key->size) + sizeof(key->size) + ALIGN_SIZE(paramSet->paramSetSize);
    return HKS_SUCCESS;
}

static uint32_t GetDataSize(uint32_t cmdId, const struct HksBlob *inData, const struct HksBlob *outData,
    uint32_t *bufSize)
{
    uint32_t inBuffData;
    if (GetBlobBufSize(inData, &inBuffData) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t bufOutDataSize;
    if (cmdId == HKS_MSG_VERIFY) {
        if (GetBlobBufSize(outData, &bufOutDataSize) != HKS_SUCCESS) {
            return HKS_ERROR_INVALID_ARGUMENT;
        }
    } else {
        bufOutDataSize = sizeof(outData->size);
    }

    if (IsAdditionOverflow(inBuffData, bufOutDataSize)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    *bufSize = inBuffData + bufOutDataSize;
    return HKS_SUCCESS;
}

static int32_t ProcessDataOnce(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *inData, struct HksBlob *outData)
{
    HKS_LOG_I("invoke ProcessOnce cmdId %u", cmdId);

    uint32_t paramBufSize, dataBufSize;
    if ((GetParamSize(key, paramSet, &paramBufSize) != HKS_SUCCESS) ||
        (GetDataSize(cmdId, inData, outData, &dataBufSize) != HKS_SUCCESS)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    uint32_t totalBufSize = paramBufSize + dataBufSize;
    uint8_t *buffer = (uint8_t *)HksMalloc(totalBufSize);
    if (buffer == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    struct HksBlob ipcBlob = { totalBufSize, buffer };

    uint32_t offset = 0;
    uint32_t ret = HksOnceParamPack(&ipcBlob, key, paramSet, &offset);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksOnceParamPack fail");
        HKS_FREE_BLOB(ipcBlob);
        return ret;
    }

    if (cmdId == HKS_MSG_VERIFY) {
        ret = HksOnceDataPack(&ipcBlob, inData, outData, NULL, &offset);
    } else {
        ret = HksOnceDataPack(&ipcBlob, inData, NULL, outData, &offset);
    }
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksOnceDataPack fail");
        HKS_FREE_BLOB(ipcBlob);
        return ret;
    }

    if (cmdId == HKS_MSG_VERIFY) {
        ret = HksSendRequest(cmdId, &ipcBlob, NULL, paramSet);
    } else {
        ret = HksSendRequest(cmdId, &ipcBlob, outData, paramSet);
    }
    HKS_FREE_BLOB(ipcBlob);
    return ret;
}

static int32_t SliceDataInit(uint32_t cmdId, const struct HksBlob *key, struct SliceParam *sliceParam)
{
    HKS_LOG_I("ProcessInit cmdId %u", cmdId);

    uint32_t bufSize;
    if (GetParamSize(key, sliceParam->paramSet, &bufSize) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    uint8_t *buffer = (uint8_t *)HksMalloc(bufSize);
    if (buffer == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }

    struct HksBlob inBlob = { bufSize, buffer };
    struct HksBlob outBlob = { sizeof(sliceParam->operationHandle), (uint8_t *)&sliceParam->operationHandle };
    uint32_t ret = HksInitPack(&inBlob, key, sliceParam->paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksInitPack fail");
        HKS_FREE_BLOB(inBlob);
        return ret;
    }

    ret = HksSendRequest(cmdId, &inBlob, &outBlob, sliceParam->paramSet);
    HKS_FREE_BLOB(inBlob);
    return ret;
}

static int32_t GetUpdateParamSize(uint64_t operationHandle, const struct HksBlob *inData,
    const struct HksBlob *outData, uint32_t *bufSize)
{
    /* update size is guaranteed to be align */
    *bufSize = sizeof(operationHandle) + sizeof(inData->size) + inData->size;
    if (outData->size > 0) {
        *bufSize += outData->size;
    }
    return HKS_SUCCESS;
}

static int32_t SliceDataUpdate(uint32_t cmdId, struct SliceParam *sliceParam, uint32_t *outSize)
{
    uint32_t cnt = (sliceParam->inData->size - 1) / MAX_PROCESS_SIZE; /* inSize has been checked: greater than 0 */
    struct HksBlob tmpInData = *(sliceParam->inData);
    tmpInData.size = MAX_PROCESS_SIZE;
    struct HksBlob tmpOutData = { 0, NULL };
    if (sliceParam->outData != NULL) {
        tmpOutData.data = sliceParam->outData->data;
        tmpOutData.size = MAX_PROCESS_SIZE;
    }

    struct HksBlob ipcBlob = { 0, NULL };
    if (GetUpdateParamSize(sliceParam->operationHandle, &tmpInData, &tmpOutData, &ipcBlob.size) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    ipcBlob.data = (uint8_t *)HksMalloc(ipcBlob.size);
    if (ipcBlob.data == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }

    uint32_t updateSize = 0;
    for (uint32_t i = 0; i < cnt; i++) {
        HKS_LOG_I("ProcessUpdate cmdId %u", cmdId);

        int32_t ret = HksUpdatePack(&ipcBlob, sliceParam->operationHandle, &tmpInData, &tmpOutData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksUpdatePack fail");
            HKS_FREE_BLOB(ipcBlob);
            return ret;
        }

        ret = HksSendRequest(cmdId, &ipcBlob, &tmpOutData, sliceParam->paramSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("fail to process update");
            HKS_FREE_BLOB(ipcBlob);
            return HKS_ERROR_BAD_STATE;
        }
        tmpInData.data += MAX_PROCESS_SIZE;
        tmpOutData.data += tmpOutData.size;
        updateSize += MAX_PROCESS_SIZE;
    }

    sliceParam->inData->data += updateSize;
    sliceParam->inData->size -= updateSize;
    if ((sliceParam->outData != NULL) && (outSize != NULL)) {
        sliceParam->outData->data += updateSize;
        sliceParam->outData->size -= updateSize;
        *outSize = updateSize;
    }
    HKS_FREE_BLOB(ipcBlob);
    return HKS_SUCCESS;
}

static int32_t GetFinalParamSize(uint32_t cmdId, uint64_t operationHandle, const struct HksBlob *inData,
    const struct HksBlob *outData, uint32_t *bufSize)
{
    if ((inData->size > MAX_PROCESS_SIZE) || (outData->size > MAX_PROCESS_SIZE)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (cmdId == HKS_MSG_VERIFY_FINAL) {
        *bufSize = sizeof(operationHandle) + sizeof(inData->size) + ALIGN_SIZE(inData->size) +
            sizeof(outData->size) + ALIGN_SIZE(outData->size);
    } else {
        *bufSize = sizeof(operationHandle) + sizeof(inData->size) + ALIGN_SIZE(inData->size) +
            sizeof(outData->size);
    }
    return HKS_SUCCESS;
}

static int32_t SliceDataFinal(uint32_t cmdId, struct SliceParam *sliceParam)
{
    HKS_LOG_I("enter SliceDataFinal cmdId %u", cmdId);

    struct HksBlob tmpInData = *(sliceParam->inData);
    uint32_t bufSize;
    if (GetFinalParamSize(cmdId, sliceParam->operationHandle, &tmpInData, sliceParam->outData,
        &bufSize) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    uint8_t *buffer = (uint8_t *)HksMalloc(bufSize);
    if (buffer == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }

    struct HksBlob ipcBlob = { bufSize, buffer };
    int32_t ret;
    if (cmdId == HKS_MSG_VERIFY_FINAL) {
        ret = HksFinalPack(&ipcBlob, sliceParam->operationHandle, &tmpInData, sliceParam->outData, NULL);
    } else {
        ret = HksFinalPack(&ipcBlob, sliceParam->operationHandle, &tmpInData, NULL, sliceParam->outData);
    }
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksFinalPack fail");
        HKS_FREE_BLOB(ipcBlob);
        return ret;
    }

    if (cmdId == HKS_MSG_VERIFY_FINAL) {
        ret = HksSendRequest(cmdId, &ipcBlob, NULL, sliceParam->paramSet);
    } else {
        ret = HksSendRequest(cmdId, &ipcBlob, sliceParam->outData, sliceParam->paramSet);
    }
    HKS_FREE_BLOB(ipcBlob);
    return ret;
}

static int32_t SliceData(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *inData, struct HksBlob *outData)
{
    HKS_LOG_I("enter SliceData, cmdId %u", cmdId);

    struct SliceParam sliceParam = {
        .operationHandle = 0,
        .inData = inData,
        .outData = outData,
        .paramSet = paramSet
    };
    int32_t ret = SliceDataInit(cmdId + CMD_INIT_OFFSET, key, &sliceParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("SliceDataInit fail %d", ret);
        return ret;
    }

    uint32_t outSize = 0;
    if ((cmdId == HKS_MSG_ENCRYPT) || (cmdId == HKS_MSG_DECRYPT)) {
        ret = SliceDataUpdate(cmdId + CMD_UPDATE_OFFSET, &sliceParam, &outSize);
    } else {
        sliceParam.outData = NULL;
        ret = SliceDataUpdate(cmdId + CMD_UPDATE_OFFSET, &sliceParam, NULL);
        sliceParam.outData = outData;
    }
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("SliceDataUpdate fail %d", ret);
        return ret;
    }

    ret = SliceDataFinal(cmdId + CMD_FINAL_OFFSET, &sliceParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("SliceDataFinal fail %d", ret);
        return ret;
    }

    if (IsAdditionOverflow(outData->size, outSize)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    outData->size += outSize;
    return ret;
}

static int32_t CheckRsaMode(uint32_t cmdId, const struct HksParamSet *paramSet)
{
    if ((cmdId == HKS_MSG_ENCRYPT) || (cmdId == HKS_MSG_DECRYPT)) {
        struct HksParam *algParam = NULL;
        if (HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam) != HKS_SUCCESS) {
            HKS_LOG_E("HksGetParam failed! No algorithm tag!");
            return HKS_ERROR_CHECK_GET_ALG_FAIL;
        }
        if (algParam->uint32Param == HKS_ALG_RSA) {
            HKS_LOG_E("Slice does not support RSA encrypt!");
            return HKS_ERROR_INVALID_ARGUMENT;
        }
    }
    return HKS_SUCCESS;
}

int32_t HksSliceDataEntry(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *inData, struct HksBlob *outData)
{
    if (!IsSliceCmd(cmdId)) {
        HKS_LOG_E("cmd %u not support slice!", cmdId);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t paramBufSize;
    uint32_t dataBufSize;
    if ((GetParamSize(key, paramSet, &paramBufSize) != HKS_SUCCESS) ||
        (GetDataSize(cmdId, inData, outData, &dataBufSize) != HKS_SUCCESS)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (IsAdditionOverflow(paramBufSize, dataBufSize)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t totalBufSize = paramBufSize + dataBufSize;
    if (totalBufSize <= MAX_PROCESS_SIZE) {
        return ProcessDataOnce(cmdId, key, paramSet, inData, outData);
    }

    int32_t ret = CheckRsaMode(cmdId, paramSet);
    if (ret != HKS_SUCCESS) {
        return ret;
    }
    return SliceData(cmdId, key, paramSet, inData, outData);
}
