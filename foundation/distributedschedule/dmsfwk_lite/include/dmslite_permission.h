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

#ifndef OHOS_DISTRIBUTEDSCHEDULE_PERMISSION_H
#define OHOS_DISTRIBUTEDSCHEDULE_PERMISSION_H

#include <stdbool.h>
#include <stdint.h>

#include "bundle_info.h"
#include "dmsfwk_interface.h"
#include "dmslite_parser.h"
#include "dmslite_inner_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
* @brief Checks whether the remote has the permission of interaction with local FAs
* @param permissionCheckInfo parsed info required for checking remote permission
* @return DmsLiteCommonErrorCode
*/
int32_t CheckRemotePermission(const PermissionCheckInfo *permissionCheckInfo);
char* GetCallerSignature(const char *remoteName, BundleInfo *bundleInfo);
char* GetRemoteSignature(const char *remoteName, BundleInfo *bundleInfo);

/**
* @brief Get caller bundle info from bms or file
* @param callerInfo caller information, which includes uid and bundleName
* @param bundleInfo bundle information of caller
* @return DmsLiteCommonErrorCode
*/
int32_t GetCallerBundleInfo(const CallerInfo *callerInfo, BundleInfo *bundleInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // OHOS_DISTRIBUTEDSCHEDULE_PERMISSION_H
