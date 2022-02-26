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
#include "dmslite_msg_handler.h"

#include "dmsfwk_interface.h"
#include "dmslite_famgr.h"
#include "dmslite_log.h"
#include "dmslite_permission.h"
#include "dmslite_session.h"
#include "dmslite_tlv_common.h"
#include "dmslite_utils.h"

int32_t StartAbilityFromRemoteHandler(const TlvNode *tlvHead, StartAbilityCallback onStartAbilityDone)
{
    const char *calleeBundleName = UnMarshallString(tlvHead, CALLEE_BUNDLE_NAME);
    const char *calleeAbilityName = UnMarshallString(tlvHead, CALLEE_ABILITY_NAME);
    const char *callerSignature = UnMarshallString(tlvHead, CALLER_SIGNATURE);

    PermissionCheckInfo permissionCheckInfo;
    permissionCheckInfo.calleeAbilityName = calleeAbilityName;
    permissionCheckInfo.calleeBundleName = calleeBundleName;
    permissionCheckInfo.callerSignature = callerSignature;
    int32_t errCode = CheckRemotePermission(&permissionCheckInfo);
    if (errCode != DMS_EC_SUCCESS) {
        HILOGE("[Remote permission check failed]");
        return errCode;
    }
    return StartAbilityFromRemote(calleeBundleName, calleeAbilityName, onStartAbilityDone);
}

int32_t ReplyMsgHandler(const TlvNode *tlvHead)
{
    int32_t ret = UnMarshallInt32(tlvHead, REPLY_ERR_CODE);
    HILOGD("[ReplyMsgHandler ret = %d]", ret);
    InvokeCallback(NULL, ret);
    CloseDMSSession();
    return ret;
}