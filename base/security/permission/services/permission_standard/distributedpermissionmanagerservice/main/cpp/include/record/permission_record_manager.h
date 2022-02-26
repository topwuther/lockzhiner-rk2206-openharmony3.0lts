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

#ifndef PERMISSION_MANAGER_RECORD_H
#define PERMISSION_MANAGER_RECORD_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "base64_util.h"
#include "constant.h"
#include "data_translator.h"
#include "device_info.h"
#include "device_info_manager.h"
#include "permission_log.h"
#include "external_deps.h"
#include "on_permission_used_record.h"
#include "permission_visitor.h"
#include "permission_record.h"
#include "query_permission_used_request.h"
#include "query_permission_used_result.h"
#include "singleton.h"
#include "sqlite_storage.h"
#include "time_util.h"
#include "zip_util.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionRecordManager final {
public:
    static PermissionRecordManager &GetInstance();

    virtual ~PermissionRecordManager();

    void AddPermissionsRecord(const std::string &permissionName, const std::string &deviceId, const int32_t uid,
        const int32_t sucCount, const int32_t failCount);

    int32_t GetPermissionRecordsBase(
        const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen, std::string &resultStr);

    int32_t GetPermissionRecordsAsync(const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen,
        const sptr<OnPermissionUsedRecord> &callback);

    void DeletePermissionUsedRecords(const int32_t uid);

private:
    PermissionRecordManager();

    /**
     * Add one data to permissionVisitor table.
     *
     * @param deviceId Device id.
     * @param uid The application uid of caller device.
     * @param visitorId PermissionVisitor table primary key.
     * @return true if succeeded; false if failed.
     */
    bool AddToVisitor(const std::string &deviceId, const int32_t uid, int32_t &visitorId);

    /**
     * Add one data to permissionRecord table.
     *
     * @param permissionName Permission name.
     * @param visitorId PermissionVisitor table primary key.
     * @param uid The application uid of caller device.
     * @param sucCount Succeed count.
     * @param failCount Failed count.
     * @return true if succeeded; false if failed.
     */
    bool AddToRecord(const std::string &permissionName, const int32_t visitorId, const int32_t uid,
        const int32_t sucCount, const int32_t failCount);

    /**
     * Combine bundlePermissionUsedRecord.
     *
     * @param request Request dto for permission used records.
     * @param bundle BundlePermissionUsedRecord vector.
     * @param queryResult Response dto for permission used records.
     * @return true if succeeded; false if failed.
     */
    bool GetBundlePermissionUsedRecord(const QueryPermissionUsedRequest &request,
        std::vector<BundlePermissionUsedRecord> &bundle, QueryPermissionUsedResult &queryResult);

    /**
     * Translate data form permissionRecord table into permissionUsedRecord.
     *
     * @param allFlag Query flag.
     * @param recordValues The data from permissionRecord table.
     * @param bundleRecord BundlePermissionUsedRecord.
     * @param queryResult Response dto for permission used records.
     * @return true if succeeded; false if failed.
     */
    bool GetRecordFromDB(const int32_t allFlag, const std::vector<GenericValues> &recordValues,
        BundlePermissionUsedRecord &bundleRecord, QueryPermissionUsedResult &queryResult);

    /**
     * Get permission used records.
     *
     * @param queryJsonStr Request ziped json string.
     * @param queryResult Response dto for permission used records.
     * @return 0 if succeeded; -1 if failed.
     */
    int32_t GetPermissionRecords(const std::string &queryJsonStr, QueryPermissionUsedResult &queryResult);

    /**
     * Delete permission used records older than days.
     *
     * @param days Days.
     * @return 0 if succeeded; -1 if failed.
     */
    int32_t DeletePermissionRecords(const int32_t days);

    /**
     * Produce permissionRecord dto.
     *
     * @param deviceId Device id.
     * @param uid The application uid of caller device.
     * @param PermissionVisitor PermissionVisitor dto.
     * @return true if succeeded; false if failed.
     */
    bool GetPermissionVisitor(const std::string &deviceId, const int32_t uid, PermissionVisitor &permissionVisitor);

    /**
     * Produce permissionRecord dto.
     *
     * @param permissionName Permission name.
     * @param visitorId PermissionVisitor table primary key.
     * @param uid The application uid of caller device.
     * @param sucCount Succeed count.
     * @param failCount Failed count.
     * @param PermissionRecord permissionRecord dto.
     * @return true if succeeded; false if failed.
     */
    bool GetPermissionRecord(const std::string &permissionName, const int32_t visitorId, const int32_t uid,
        const int32_t sucCount, const int32_t failCount, PermissionRecord &permissionRecord);

    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_MANAGER_USED_RECORD_H
