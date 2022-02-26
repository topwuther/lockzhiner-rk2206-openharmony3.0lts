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

#include "dmslite_permission.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef WEARABLE_PRODUCT
#include "bundle_manager.h"
#else
#include "bundle_inner_interface.h"
#endif
#include "dmslite_log.h"
#include "dmslite_utils.h"
#include "samgr_lite.h"
#include "securec.h"

#define DELIMITER_LENGTH 1
#define GET_BUNDLE_WITHOUT_ABILITIES 0
#ifndef WEARABLE_PRODUCT
#define NATIVE_APPID_DIR "/system/native_appid/"
#define APPID_FILE_PREFIX "uid_"
#define APPID_FILE_SUFFIX "_appid"
#define MAX_FILE_PATH_LEN 64
#define MAX_NATIVE_SERVICE_UID 99
#endif

#ifndef WEARABLE_PRODUCT
static bool GetBmsInterface(struct BmsServerProxy **bmsInterface)
{
    IUnknown *iUnknown = SAMGR_GetInstance()->GetFeatureApi(BMS_SERVICE, BMS_FEATURE);
    if (iUnknown == NULL) {
        HILOGE("[GetFeatureApi failed]");
        return false;
    }

    int32_t errCode = iUnknown->QueryInterface(iUnknown, DEFAULT_VERSION, (void **) bmsInterface);
    if (errCode != EC_SUCCESS) {
        HILOGE("[QueryInterface failed]");
        return false;
    }

    return true;
}
#endif

int32_t CheckRemotePermission(const PermissionCheckInfo *permissionCheckInfo)
{
    if (permissionCheckInfo == NULL) {
        return DMS_EC_FAILURE;
    }

    BundleInfo bundleInfo;
    if (memset_s(&bundleInfo, sizeof(BundleInfo), 0x00, sizeof(BundleInfo)) != EOK) {
        HILOGE("[bundleInfo memset failed]");
        return DMS_EC_FAILURE;
    }

    int32_t errCode;
#ifndef WEARABLE_PRODUCT
    uid_t callerUid = getuid();
    if (callerUid == FOUNDATION_UID) {
        /* inner-process mode */
        struct BmsServerProxy *bmsInterface = NULL;
        if (!GetBmsInterface(&bmsInterface)) {
            HILOGE("[GetBmsInterface query null]");
            return DMS_EC_GET_BMS_FAILURE;
        }
        errCode = bmsInterface->GetBundleInfo(permissionCheckInfo->calleeBundleName,
            GET_BUNDLE_WITHOUT_ABILITIES, &bundleInfo);
    } else if (callerUid == SHELL_UID) {
        /* inter-process mode (mainly called in xts testsuit process started by shell) */
        errCode = GetBundleInfo(permissionCheckInfo->calleeBundleName,
            GET_BUNDLE_WITHOUT_ABILITIES, &bundleInfo);
    } else {
        errCode = EC_FAILURE;
    }
#else
    errCode = GetBundleInfo(permissionCheckInfo->calleeBundleName,
        GET_BUNDLE_WITHOUT_ABILITIES, &bundleInfo);
#endif
    if (errCode != EC_SUCCESS) {
        HILOGE("[GetBundleInfo errCode = %d]", errCode);
        return DMS_EC_GET_BUNDLEINFO_FAILURE;
    }

    /* appId: bundleName + "_" + signature */
    const char *calleeSignature = bundleInfo.appId + strlen(permissionCheckInfo->calleeBundleName)
        + DELIMITER_LENGTH;
    ClearBundleInfo(&bundleInfo);
    if ((permissionCheckInfo->callerSignature == NULL) || (calleeSignature == NULL)) {
        HILOGE("[Signature is null]");
        return DMS_EC_FAILURE;
    }

    if (strcmp(permissionCheckInfo->callerSignature, calleeSignature) != 0) {
        HILOGE("[Signature unmatched]");
        return DMS_EC_CHECK_PERMISSION_FAILURE;
    }

    return DMS_EC_SUCCESS;
}

static int32_t GetBundleInfoFromFile(const char *filePath, BundleInfo *bundleInfo)
{
    int32_t fd = open(filePath, O_RDONLY, S_IRUSR);
    if (fd < 0) {
        HILOGE("[open file failed]");
        return DMS_EC_FAILURE;
    }
    int32_t fileLen = lseek(fd, 0, SEEK_END);
    if (fileLen <= 0) {
        HILOGE("[fileLen is invalid, fileLen=%d]", fileLen);
        close(fd);
        return DMS_EC_FAILURE;
    }
    int32_t ret = lseek(fd, 0, SEEK_SET);
    if (ret < 0) {
        HILOGE("[lseek failed, ret=%d]", ret);
        close(fd);
        return DMS_EC_FAILURE;
    }
    int32_t appIdLen = fileLen + 1;
    char *appId = (char *)DMS_ALLOC(appIdLen);
    if (appId == NULL) {
        HILOGE("[DMS_ALLOC appId failed]");
        close(fd);
        return DMS_EC_FAILURE;
    }
    (void)memset_s(appId, appIdLen, 0x00, appIdLen);
    ret = read(fd, appId, fileLen);
    if (ret < 0) {
        HILOGE("[read appId failed, ret=%d]", ret);
        DMS_FREE(appId);
        close(fd);
        return DMS_EC_FAILURE;
    }
    for (; fileLen > 0; --fileLen) {
        if (appId[fileLen - 1] != '\n') {
            break;
        }
    }
    appId[fileLen] = '\0';
    bundleInfo->appId = appId;
    close(fd);
    return DMS_EC_SUCCESS;
}

static int32_t GetBundleInfoFromBms(const CallerInfo *callerInfo, BundleInfo *bundleInfo)
{
    int32_t errCode;
#ifndef WEARABLE_PRODUCT
    char *bundleName = NULL;
    uid_t callerUid = getuid();
    if (callerUid == FOUNDATION_UID) {
        /* inner-process mode */
        struct BmsServerProxy *bmsServerProxy = NULL;
        if (!GetBmsInterface(&bmsServerProxy)) {
            HILOGE("[GetBmsInterface query null]");
            return DMS_EC_GET_BMS_FAILURE;
        }
        if (bmsServerProxy->GetBundleNameForUid(callerInfo->uid, &bundleName) != EC_SUCCESS) {
            HILOGE("[GetBundleNameForUid failed]");
            return DMS_EC_FAILURE;
        }
        errCode = bmsServerProxy->GetBundleInfo(bundleName, GET_BUNDLE_WITHOUT_ABILITIES, bundleInfo);
    } else if (callerUid == SHELL_UID) {
        /* inter-process mode (mainly called in xts testsuit process started by shell) */
        if (GetBundleNameForUid(callerInfo->uid, &bundleName) != EC_SUCCESS) {
            HILOGE("[GetBundleNameForUid failed]");
            return DMS_EC_FAILURE;
        }
        errCode = GetBundleInfo(bundleName, GET_BUNDLE_WITHOUT_ABILITIES, bundleInfo);
    } else {
        errCode = DMS_EC_FAILURE;
    }
    DMS_FREE(bundleName);
#else
    errCode = GetBundleInfo(callerInfo->bundleName, GET_BUNDLE_WITHOUT_ABILITIES, bundleInfo);
#endif
    if (errCode != EC_SUCCESS) {
        HILOGE("[GetBundleInfo failed]");
        return DMS_EC_GET_BUNDLEINFO_FAILURE;
    }
    return DMS_EC_SUCCESS;
}

int32_t GetCallerBundleInfo(const CallerInfo *callerInfo, BundleInfo *bundleInfo)
{
    if ((callerInfo == NULL) || (bundleInfo == NULL)) {
        HILOGE("[invalid parameter]");
        return DMS_EC_INVALID_PARAMETER;
    }
#ifndef WEARABLE_PRODUCT
    if (callerInfo->uid <= MAX_NATIVE_SERVICE_UID) {
        char filePath[MAX_FILE_PATH_LEN] = {0};
        int32_t ret = sprintf_s(filePath, MAX_FILE_PATH_LEN, "%s%s%d%s", NATIVE_APPID_DIR, APPID_FILE_PREFIX,
            callerInfo->uid, APPID_FILE_SUFFIX);
        if (ret < 0) {
            HILOGE("[filePath sprintf failed]");
            return DMS_EC_FAILURE;
        }
        bundleInfo->uid = callerInfo->uid;
        return GetBundleInfoFromFile(filePath, bundleInfo);
    }
#endif
    return GetBundleInfoFromBms(callerInfo, bundleInfo);
}
