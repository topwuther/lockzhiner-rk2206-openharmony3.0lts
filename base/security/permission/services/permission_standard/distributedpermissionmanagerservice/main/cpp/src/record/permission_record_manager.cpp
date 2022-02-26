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

#include "permission_record_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionRecordManager"};
}

PermissionRecordManager &PermissionRecordManager::GetInstance()
{
    static PermissionRecordManager instance;
    return instance;
}

PermissionRecordManager::PermissionRecordManager()
{}

PermissionRecordManager::~PermissionRecordManager()
{}

// add permission used record
void PermissionRecordManager::AddPermissionsRecord(const std::string &permissionName, const std::string &deviceId,
    const int32_t uid, const int sucCount, const int failCount)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, permissionName: %{public}s, deviceId: %{public}s, uid: %{public}d, sucCount: %{public}d, "
        "failCount: %{public}d",
        __func__,
        permissionName.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        uid,
        sucCount,
        failCount);

    auto DelRecordsTask = [this]() {
        PERMISSION_LOG_INFO(LABEL, "---DeletePermissionRecords task called");
        DeletePermissionRecords(Constant::RECORD_DELETE_TIME);
    };
    std::thread recordThread(DelRecordsTask);
    recordThread.detach();

    int32_t visitorId = 0;
    if (!AddToVisitor(deviceId, uid, visitorId)) {
        return;
    }
    if (!AddToRecord(permissionName, visitorId, uid, sucCount, failCount)) {
        return;
    }
}

bool PermissionRecordManager::AddToVisitor(const std::string &deviceId, const int32_t uid, int32_t &visitorId)
{
    PermissionVisitor permissionVisitor;
    if (!GetPermissionVisitor(deviceId, uid, permissionVisitor)) {
        return false;
    }
    std::vector<GenericValues> values;
    std::vector<GenericValues> resultValues;
    GenericValues findVisitor;
    GenericValues insertVisitor;
    GenericValues nullGenericValues;
    DataTranslator::TranslationIntoGenericValues(permissionVisitor, findVisitor);
    insertVisitor = findVisitor;
    findVisitor.Remove(FIELD_BUNDLE_USER_ID);
    if (DataStorage::GetRealDataStorage().FindByConditions(
            DataStorage::PERMISSION_VISITOR, findVisitor, nullGenericValues, resultValues) != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_VISITOR table find failed!", __func__);
        return false;
    }

    if (resultValues.empty()) {
        values.emplace_back(insertVisitor);
        if (DataStorage::GetRealDataStorage().Add(DataStorage::PERMISSION_VISITOR, values) != Constant::SUCCESS) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_VISITOR table add failed!", __func__);
            return false;
        }
        if (DataStorage::GetRealDataStorage().FindByConditions(
                DataStorage::PERMISSION_VISITOR, values[0], nullGenericValues, resultValues) != Constant::SUCCESS) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_VISITOR table find failed!", __func__);
            return false;
        }
    }
    DataTranslator::TranslationIntoPermissionVisitor(resultValues[0], permissionVisitor);
    visitorId = permissionVisitor.id;
    return true;
}

bool PermissionRecordManager::AddToRecord(const std::string &permissionName, const int32_t visitorId, const int32_t uid,
    const int32_t sucCount, const int32_t failCount)
{
    PermissionRecord permissionRecord;
    if (!GetPermissionRecord(permissionName, visitorId, uid, sucCount, failCount, permissionRecord)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: PermissionName translate into opCode failed!", __func__);
        return false;
    }

    std::vector<GenericValues> values;
    std::vector<GenericValues> resultValues;
    GenericValues nullGenericValues;
    GenericValues findRecord;
    GenericValues insertRecord;
    DataTranslator::TranslationIntoGenericValues(permissionRecord, findRecord);

    insertRecord = findRecord;
    findRecord.Remove(FIELD_TIMESTAMP);
    findRecord.Remove(FIELD_ACCESS_COUNT);
    findRecord.Remove(FIELD_REJECT_COUNT);
    if (DataStorage::GetRealDataStorage().FindByConditions(
            DataStorage::PERMISSION_RECORD, findRecord, nullGenericValues, resultValues) != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_RECORD table find failed!", __func__);
        return false;
    }

    if (resultValues.size() > 0) {
        for (auto record : resultValues) {
            if (insertRecord.GetInt64(FIELD_TIMESTAMP) - record.GetInt64(FIELD_TIMESTAMP) < Constant::PRECISION) {
                int32_t accessCount = insertRecord.GetInt(FIELD_ACCESS_COUNT) + record.GetInt(FIELD_ACCESS_COUNT);
                int32_t rejectCount = insertRecord.GetInt(FIELD_REJECT_COUNT) + record.GetInt(FIELD_REJECT_COUNT);
                insertRecord.Remove(FIELD_ACCESS_COUNT);
                insertRecord.Remove(FIELD_REJECT_COUNT);

                insertRecord.Put(FIELD_ACCESS_COUNT, accessCount);
                insertRecord.Put(FIELD_REJECT_COUNT, rejectCount);
                if (DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_RECORD, record) !=
                    Constant::SUCCESS) {
                    PERMISSION_LOG_ERROR(
                        LABEL, "%{public}s: database PERMISSION_RECORD update failed!(delete)", __func__);
                    return false;
                }
                break;
            }
        }
    }
    values.emplace_back(insertRecord);
    if (DataStorage::GetRealDataStorage().Add(DataStorage::PERMISSION_RECORD, values) != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_RECORD table add failed!", __func__);
        return false;
    }
    return true;
}

int32_t PermissionRecordManager::GetPermissionRecordsBase(
    const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen, std::string &resultStr)
{
    QueryPermissionUsedResult queryResult;
    if (codeLen <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: decode length less than 0!", __func__);
        return Constant::FAILURE;
    }
    unsigned char *pOut = (unsigned char *)malloc(codeLen + 1);
    Base64Util::Decode(queryGzipStr, pOut, codeLen);
    std::string queryJsonStr;
    if (!ZipUtil::ZipUnCompress(pOut, codeLen, queryJsonStr, zipLen)) {
        return Constant::FAILURE;
    }
    if (pOut) {
        free(pOut);
        pOut = nullptr;
    }
    int32_t flag = GetPermissionRecords(queryJsonStr, queryResult);
    nlohmann::json jsonObj = queryResult.to_json(queryResult);
    std::string result = jsonObj.dump();
    zipLen = result.length();
    codeLen = compressBound(zipLen);
    if (codeLen <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: compress length less than 0!", __func__);
        return Constant::FAILURE;
    }
    unsigned char *buf = (unsigned char *)malloc(codeLen + 1);
    if (!ZipUtil::ZipCompress(result, zipLen, buf, codeLen)) {
        return Constant::FAILURE;
    }
    Base64Util::Encode(buf, codeLen, resultStr);
    if (buf) {
        free(buf);
        buf = nullptr;
    }
    return flag;
}

int32_t PermissionRecordManager::GetPermissionRecordsAsync(const std::string &queryGzipStr, unsigned long &codeLen,
    unsigned long &zipLen, const sptr<OnPermissionUsedRecord> &callback)
{
    if (codeLen <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: decode length less than 0!", __func__);
        return Constant::FAILURE;
    }
    unsigned char *pOut = (unsigned char *)malloc(codeLen + 1);
    Base64Util::Decode(queryGzipStr, pOut, codeLen);
    std::string queryJsonStr;
    ZipUtil::ZipUnCompress(pOut, codeLen, queryJsonStr, zipLen);

    if (pOut) {
        free(pOut);
        pOut = nullptr;
    }
    auto task = [queryJsonStr, callback]() {
        PERMISSION_LOG_INFO(LABEL, "---GetPermissionRecords task called");
        QueryPermissionUsedResult defaultResult;
        PermissionRecordManager::GetInstance().GetPermissionRecords(queryJsonStr, defaultResult);
        PERMISSION_LOG_INFO(
            LABEL, "%{public}s callback OnQueried called, resultCode: %{public}d", __func__, defaultResult.code);
        callback->OnQueried(defaultResult.code, defaultResult);
    };
    std::thread recordThread(task);
    recordThread.detach();

    return Constant::SUCCESS;
}

int32_t PermissionRecordManager::GetPermissionRecords(
    const std::string &queryJsonStr, QueryPermissionUsedResult &queryResult)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, queryJsonStr: %{public}s", __func__, queryJsonStr.c_str());

    auto DelRecordsTask = [this]() {
        PERMISSION_LOG_INFO(LABEL, "---DeletePermissionRecords task called");
        DeletePermissionRecords(Constant::RECORD_DELETE_TIME);
    };
    std::thread recordThread(DelRecordsTask);
    recordThread.detach();

    nlohmann::json requestJson = nlohmann::json::parse(queryJsonStr);
    QueryPermissionUsedRequest request;
    request.from_json(requestJson, request);
    std::vector<BundlePermissionUsedRecord> bundle;
    queryResult.beginTimeMillis = 0;
    queryResult.endTimeMillis = 0;
    bool flag = GetBundlePermissionUsedRecord(request, bundle, queryResult);
    queryResult.bundlePermissionUsedRecords = bundle;

    if (flag) {
        return Constant::SUCCESS;
    }
    return Constant::FAILURE;
}

bool PermissionRecordManager::GetBundlePermissionUsedRecord(const QueryPermissionUsedRequest &request,
    std::vector<BundlePermissionUsedRecord> &bundle, QueryPermissionUsedResult &queryResult)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);
    std::vector<GenericValues> recordValues;
    std::vector<GenericValues> visitorValues;
    GenericValues visitorGenericValues;
    GenericValues recordAndGenericValues;
    GenericValues recordOrGenericValues;
    GenericValues nullGenericValues;
    if (DataTranslator::TranslationIntoGenericValues(
            request, visitorGenericValues, recordAndGenericValues, recordOrGenericValues) != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: time conditions are invalid!", __func__);
        return false;
    }

    if (DataStorage::GetRealDataStorage().FindByConditions(
            DataStorage::PERMISSION_VISITOR, visitorGenericValues, nullGenericValues, visitorValues) !=
        Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_VISITOR table find failed!", __func__);
        queryResult.code = Constant::DATABASE_FAILED;
        return false;
    }
    if (visitorValues.size() <= 0) {
        queryResult.code = Constant::SUCCESS_GET_RECORD;
        PERMISSION_LOG_INFO(LABEL, "%{public}s: visitor data is null!", __func__);
        return true;
    }
    for (auto visitor : visitorValues) {
        recordValues.clear();
        BundlePermissionUsedRecord bundleRecord;
        DataTranslator::TranslationIntoBundlePermissionUsedRecord(visitor, bundleRecord);
        recordAndGenericValues.Put(FIELD_VISITOR_ID, visitor.GetInt(FIELD_ID));
        if (DataStorage::GetRealDataStorage().FindByConditions(
                DataStorage::PERMISSION_RECORD, recordAndGenericValues, recordOrGenericValues, recordValues) !=
            Constant::SUCCESS) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s: database PERMISSION_RECORD table find failed!", __func__);
            queryResult.code = Constant::DATABASE_FAILED;
            return false;
        }
        recordAndGenericValues.Remove(FIELD_VISITOR_ID);
        if (recordValues.size() > 0) {
            if (!GetRecordFromDB(request.flag, recordValues, bundleRecord, queryResult)) {
                return false;
            }
        }
        if (bundleRecord.permissionUsedRecords.size() != 0) {
            bundle.emplace_back(bundleRecord);
        }
    }
    queryResult.code = Constant::SUCCESS_GET_RECORD;
    return true;
}

bool PermissionRecordManager::GetRecordFromDB(const int32_t allFlag, const std::vector<GenericValues> &recordValues,
    BundlePermissionUsedRecord &bundleRecord, QueryPermissionUsedResult &queryResult)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);
    std::vector<PermissionUsedRecord> usedRecord;
    for (auto record : recordValues) {
        PermissionUsedRecord tempRecord;
        if (record.GetInt64(FIELD_TIMESTAMP) > queryResult.endTimeMillis) {
            queryResult.endTimeMillis = record.GetInt64(FIELD_TIMESTAMP);
        }
        if (queryResult.beginTimeMillis == 0 ? true : record.GetInt64(FIELD_TIMESTAMP) < queryResult.beginTimeMillis) {
            queryResult.beginTimeMillis = record.GetInt64(FIELD_TIMESTAMP);
        }
        record.Put(FIELD_FLAG, allFlag);
        if (DataTranslator::TranslationIntoPermissionUsedRecord(record, tempRecord) != Constant::SUCCESS) {
            PERMISSION_LOG_ERROR(
                LABEL, "%{public}s: opCode translate into PermissionName failed! Cannot recognize opCode.", __func__);
            queryResult.code = Constant::NOT_DEFINED;
            return false;
        }

        auto ite = std::find_if(usedRecord.begin(), usedRecord.end(), [tempRecord](const PermissionUsedRecord &rec) {
            return tempRecord.permissionName == rec.permissionName;
        });
        if (ite != usedRecord.end()) {
            if (allFlag == 1) {
                (*ite).updateRecordWithTime(tempRecord);
            } else {
                (*ite).updateRecord(tempRecord);
            }
        } else {
            usedRecord.emplace_back(tempRecord);
        }
    }
    bundleRecord.permissionUsedRecords = usedRecord;
    return true;
}

void PermissionRecordManager::DeletePermissionUsedRecords(const int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s ------------- begin BMS------------", __func__);
    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    sptr<AppExecFwk::IBundleMgr> iBundleMgr;
    iBundleMgr = externalDeps->GetBundleManager(iBundleMgr);

    std::string bundleName;
    bool result = iBundleMgr->GetBundleNameForUid(uid, bundleName);
    if (!result) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s cannot get bundle name by uid %{public}d", __func__, uid);
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s ------------- end BMS------------", __func__);
    std::vector<GenericValues> visitorValues;
    GenericValues andCondition;
    GenericValues orCondition;
    andCondition.Put(FIELD_BUNDLE_NAME, bundleName);
    if (DataStorage::GetRealDataStorage().FindByConditions(
            DataStorage::PERMISSION_VISITOR, andCondition, orCondition, visitorValues) == Constant::FAILURE) {
        return;
    }
    for (auto visitor : visitorValues) {
        GenericValues record;
        record.Put(FIELD_VISITOR_ID, visitor.GetInt(FIELD_ID));
        DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_RECORD, record);
    }
    DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_VISITOR, andCondition);
}

int PermissionRecordManager::DeletePermissionRecords(const int32_t days)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);
    std::vector<GenericValues> resultValues;
    int result = DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_RECORD, resultValues);
    if (result == Constant::FAILURE) {
        return Constant::FAILURE;
    }
    for (auto record : resultValues) {
        if (TimeUtil::GetTimestamp() - record.GetInt64(FIELD_TIMESTAMP) >= days) {
            result = DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_RECORD, record);
            if (result == Constant::FAILURE) {
                return Constant::FAILURE;
            }
        }
    }
    return Constant::SUCCESS;
}

bool PermissionRecordManager::GetPermissionVisitor(
    const std::string &deviceId, const int uid, PermissionVisitor &permissionVisitor)
{
    DeviceInfo deviceInfo;
    if (!DelayedSingleton<DeviceInfoManager>::GetInstance()->GetDeviceInfo(
            deviceId, DeviceIdType::UNKNOWN, deviceInfo)) {
        PERMISSION_LOG_INFO(LABEL,
            "%{public}s cannot get DeviceInfo by deviceId %{public}s",
            __func__,
            Constant::EncryptDevId(deviceId).c_str());
        return false;
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s ------------- begin BMS------------", __func__);
    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);

    std::string bundleName;
    AppExecFwk::BundleInfo bundleInfo;
    bool result = iBundleManager_->GetBundleNameForUid(uid, bundleName);
    if (!result) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s cannot get bundle name by uid %{public}d", __func__, uid);
        return false;
    }
    result = iBundleManager_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (!result) {
        PERMISSION_LOG_INFO(
            LABEL, "%{public}s cannot get bundleInfo by bundleName %{public}s", __func__, bundleName.c_str());
        return false;
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s ------------- end BMS------------", __func__);
    PermissionVisitor::SetPermissionVisitor(
        deviceId, deviceInfo.deviceName, 0, bundleName, bundleInfo.label, permissionVisitor);
    return true;
}

bool PermissionRecordManager::GetPermissionRecord(const std::string &permissionName, const int32_t visitorId,
    const int32_t uid, const int32_t sucCount, const int32_t failCount, PermissionRecord &permissionRecord)
{
    int32_t opCode = 0;
    // blocked
    // get isforeground by uid
    int even = 2;
    bool isForeground = (sucCount + failCount) % even == 1 ? true : false;
    std::string tempName = permissionName;
    if (Constant::PermissionNameToOrFromOpCode(tempName, opCode)) {
        PermissionRecord::SetPermissionRecord(
            TimeUtil::GetTimestamp(), visitorId, opCode, isForeground, sucCount, failCount, permissionRecord);
        return true;
    }
    return false;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS