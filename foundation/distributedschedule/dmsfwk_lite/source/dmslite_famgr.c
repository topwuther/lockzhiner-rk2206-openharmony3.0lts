/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "dmslite_famgr.h"

#include <malloc.h>

#include "dmslite_feature.h"
#include "dmslite_log.h"
#include "dmslite_packet.h"
#include "dmslite_permission.h"
#include "dmslite_session.h"
#include "dmslite_tlv_common.h"
#include "dmslite_utils.h"

#include "message.h"
#include "ohos_errno.h"
#include "securec.h"

#define DMS_VERSION_VALUE 200
#define ENDING_SYMBOL_LEN 1

static int32_t FillRequestData(RequestData *reqdata, const Want *want,
    const CallerInfo *callerInfo, const IDmsListener *callback);
static int32_t MarshallDmsMessage(const Want *want, const CallerInfo *callerInfo);

int32_t StartAbilityFromRemote(const char *bundleName, const char *abilityName,
    StartAbilityCallback onStartAbilityDone)
{
    return EC_SUCCESS;
}

int32_t StartRemoteAbilityInner(const Want *want, const CallerInfo *callerInfo,
    const IDmsListener *callback)
{
    if (want == NULL || want->element == NULL || callerInfo == NULL) {
        HILOGE("[param error!]");
        return DMS_EC_FAILURE;
    }
    RequestData *reqdata = (RequestData *)DMS_ALLOC(sizeof(RequestData));
    if (reqdata == NULL) {
        HILOGE("[mem alloc error!]");
        return DMS_EC_FAILURE;
    }
    if (memset_s(reqdata, sizeof(RequestData), 0x00, sizeof(RequestData)) != EOK) {
        DMS_FREE(reqdata);
        HILOGE("[RequestData memset failed]");
        return DMS_EC_FAILURE;
    }

    if (FillRequestData(reqdata, want, callerInfo, callback) != DMS_EC_SUCCESS) {
        FreeRequestData(reqdata->want, reqdata->callerInfo);
        DMS_FREE(reqdata);
        HILOGE("[FillRequestData failed]");
        return DMS_EC_FAILURE;
    }

    Request request = {
        .msgId = START_REMOTE_ABILITY,
        .data = (void *)reqdata,
        .len = sizeof(RequestData),
        .msgValue = 0
    };
    int32_t result = SAMGR_SendRequest((const Identity*)&(GetDmsLiteFeature()->identity), &request, NULL);
    if (result != EC_SUCCESS) {
        FreeRequestData(reqdata->want, reqdata->callerInfo);
        DMS_FREE(reqdata);
        HILOGD("[StartRemoteAbilityInner SendRequest errCode = %d]", result);
    }
    return result;
}

int32_t StartRemoteAbility(const Want *want, CallerInfo *callerInfo, IDmsListener *callback)
{
    HILOGI("[StartRemoteAbility]");
    if (want == NULL || want->element == NULL || callerInfo == NULL) {
        return DMS_EC_INVALID_PARAMETER;
    }
    if (IsDmsBusy()) {
        HILOGI("[StartRemoteAbility dms busy]");
        return DMS_EC_FAILURE;
    }
#ifndef XTS_SUITE_TEST
    if (!PreprareBuild()) {
        return DMS_EC_FAILURE;
    }
#endif
    if (MarshallDmsMessage(want, callerInfo) != DMS_EC_SUCCESS) {
        return DMS_EC_FAILURE;
    }
#ifndef XTS_SUITE_TEST
    int32_t ret = SendDmsMessage(GetPacketBufPtr(), GetPacketSize(),
        want->element->deviceId, callback);
    return ret;
#else
    return DMS_EC_SUCCESS;
#endif
}

static int32_t MarshallDmsMessage(const Want *want, const CallerInfo *callerInfo)
{
    PACKET_MARSHALL_HELPER(Uint16, COMMAND_ID, DMS_MSG_CMD_START_FA);
    PACKET_MARSHALL_HELPER(Uint16, DMS_VERSION, DMS_VERSION_VALUE);
    PACKET_MARSHALL_HELPER(String, CALLEE_BUNDLE_NAME, want->element->bundleName);
    PACKET_MARSHALL_HELPER(String, CALLEE_ABILITY_NAME, want->element->abilityName);

    BundleInfo bundleInfo = {0};
    int32_t ret = GetCallerBundleInfo(callerInfo, &bundleInfo);
    if (ret != DMS_EC_SUCCESS) {
        HILOGE("[StartRemoteAbility GetCallerBundleInfo error = %d]", ret);
        return DMS_EC_FAILURE;
    }
    if (!MarshallString(bundleInfo.appId, CALLER_SIGNATURE)) {
        HILOGE("[StartRemoteAbility Marshall appId failed]");
        ClearBundleInfo(&bundleInfo);
        return DMS_EC_FAILURE;
    }
    ClearBundleInfo(&bundleInfo);

    if (want->data != NULL && want->dataLength > 0) {
        RAWDATA_MARSHALL_HELPER(RawData, CALLER_PAYLOAD, want->data, want->dataLength);
    }
    return DMS_EC_SUCCESS;
}

static int32_t FillCallerInfo(RequestData *reqdata, const CallerInfo *callerInfo)
{
    if (callerInfo == NULL || reqdata == NULL) {
        return DMS_EC_FAILURE;
    }
    CallerInfo *callerData = (CallerInfo *)DMS_ALLOC(sizeof(CallerInfo));
    if (callerData == NULL) {
        return DMS_EC_FAILURE;
    }
    if (memset_s(callerData, sizeof(CallerInfo), 0x00, sizeof(CallerInfo)) != EOK) {
        DMS_FREE(callerData);
        HILOGE("[CallerInfo memset error]");
        return DMS_EC_FAILURE;
    }
    reqdata->callerInfo = callerData;

    callerData->uid = callerInfo->uid;
    if (callerInfo->bundleName != NULL) {
        int32_t size = strlen(callerInfo->bundleName) + ENDING_SYMBOL_LEN;
        char *data = (char *)DMS_ALLOC(size);
        if (data == NULL) {
            return DMS_EC_FAILURE;
        }
        if (memcpy_s(data, size, callerInfo->bundleName, size) != EOK) {
            DMS_FREE(data);
            return DMS_EC_FAILURE;
        }
        data[size - ENDING_SYMBOL_LEN] = '\0';
        callerData->bundleName = data;
    }
    return DMS_EC_SUCCESS;
}

static int32_t FillRequestData(RequestData *reqdata, const Want *want,
    const CallerInfo *callerInfo, const IDmsListener *callback)
{
    Want *wantData = (Want *)DMS_ALLOC(sizeof(Want));
    if (wantData == NULL) {
        return DMS_EC_FAILURE;
    }
    if (memset_s(wantData, sizeof(Want), 0x00, sizeof(Want)) != EOK) {
        DMS_FREE(wantData);
        return DMS_EC_FAILURE;
    }
    reqdata->want = wantData;

    ElementName elementData;
    if (memset_s(&elementData, sizeof(ElementName), 0x00, sizeof(ElementName)) != EOK) {
        return DMS_EC_FAILURE;
    }
    if (!(SetElementBundleName(&elementData, want->element->bundleName)
        && SetElementAbilityName(&elementData, want->element->abilityName)
        && SetElementDeviceID(&elementData, want->element->deviceId)
        && SetWantElement(wantData, elementData))) {
        HILOGE("[FillRequestData error]");
        ClearElement(&elementData);
        ClearWant(wantData);
        return DMS_EC_FAILURE;
    }
    ClearElement(&elementData);

    if (want->data != NULL && want->dataLength > 0) {
        void *data = DMS_ALLOC(want->dataLength);
        if (data == NULL) {
            return DMS_EC_FAILURE;
        }
        if (memcpy_s(data, want->dataLength, want->data, want->dataLength) != EOK) {
            DMS_FREE(data);
            return DMS_EC_FAILURE;
        }
        wantData->data = data;
        wantData->dataLength = want->dataLength;
    }

    if (FillCallerInfo(reqdata, callerInfo) != DMS_EC_SUCCESS) {
        return DMS_EC_FAILURE;
    }

    reqdata->callback = (IDmsListener *)callback;
    return DMS_EC_SUCCESS;
}

void FreeRequestData(const Want *want, CallerInfo *callerInfo)
{
    if (want != NULL) {
        ClearWant((Want *)want);
        DMS_FREE(want);
    }

    if (callerInfo != NULL) {
        DMS_FREE(callerInfo->bundleName);
        DMS_FREE(callerInfo);
    }
}