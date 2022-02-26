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

#include "dmslite_feature.h"

#include "dmslite_famgr.h"
#include "dmslite_log.h"
#include "dmslite_session.h"

#include "ohos_init.h"
#include "samgr_lite.h"
#include "securec.h" 

#define EMPTY_FEATURE_NAME ""

static const char *GetName(Feature *feature);
static void OnInitialize(Feature *feature, Service *parent, Identity identity);
static void OnStop(Feature *feature, Identity identity);
static BOOL OnMessage(Feature *feature, Request *request);

DmsLite g_dmslite = {
    /* feature functions */
    .GetName = GetName,
    .OnInitialize = OnInitialize,
    .OnStop = OnStop,
    .OnMessage = OnMessage,
    .identity = {-1, -1, NULL},
    /* dms interface for other subsystems */
    DEFAULT_IUNKNOWN_ENTRY_BEGIN,
    .StartRemoteAbility = StartRemoteAbilityInner,
    DEFAULT_IUNKNOWN_ENTRY_END
};

DmsLite *GetDmsLiteFeature()
{
    return &g_dmslite;
}

static const char *GetName(Feature *feature)
{
    if (feature == NULL) {
        return EMPTY_FEATURE_NAME;
    }
    return DMSLITE_FEATURE;
}

static void OnInitialize(Feature *feature, Service *parent, Identity identity)
{
    if (feature == NULL || parent == NULL) {
        return;
    }

    ((DmsLite*) feature)->identity = identity;
}

static void OnStop(Feature *feature, Identity identity)
{
    HILOGD("[Feature stop]");
}

static BOOL OnMessage(Feature *feature, Request *request)
{
    if (feature == NULL || request == NULL) {
        return FALSE;
    }

    /* process for a specific feature-level msgId can be added below */
    switch (request->msgId) {
        case START_REMOTE_ABILITY: {
            if (request->data == NULL) {
                InvokeCallback(NULL, DMS_EC_FAILURE);
                HILOGE("[START_REMOTE_ABILITY request is NULL]");
                return FALSE;
            }
            const RequestData *data = (const RequestData *)request->data;
            int32_t result = StartRemoteAbility(data->want, data->callerInfo, data->callback);
            FreeRequestData(data->want, data->callerInfo);
            if (result != DMS_EC_SUCCESS) {
                InvokeCallback(NULL, result);
            }
            break;
        }
        case SESSION_OPEN:
            HandleSessionOpened(request->msgValue);
            break;
        case SESSION_CLOSE:
            HandleSessionClosed(request->msgValue);
            break;
        case BYTES_RECEIVED:
            HandleBytesReceived(request->msgValue, request->data, request->len);
            break;
        default: {
            HILOGW("[Unkonwn msgId = %d]", request->msgId);
            break;
        }
    }
    return TRUE;
}

static void Init()
{
    BOOL result = SAMGR_GetInstance()->RegisterFeature(DISTRIBUTED_SCHEDULE_SERVICE, (Feature*) &g_dmslite);
    if (!result) {
        HILOGE("[dms register feature failed]");
    }

    result = SAMGR_GetInstance()->RegisterFeatureApi(DISTRIBUTED_SCHEDULE_SERVICE,
        DMSLITE_FEATURE, GET_IUNKNOWN(g_dmslite));
    if (!result) {
        HILOGE("[dms register feature api failed]");
    }
}
SYS_FEATURE_INIT(Init);
