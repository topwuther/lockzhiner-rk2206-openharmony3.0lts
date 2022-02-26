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
#include "permission_record_mgr.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "singleton.h"
#include "time_util.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionRecordMgr"};
}
namespace {
constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr int32_t PARAM0 = 0;
constexpr int32_t PARAM1 = 1;
constexpr int32_t CODE_SUCCESS = 0;
constexpr int32_t CODE_FAILED = -1;
}  // namespace

static void ConvertBundleRecord(napi_env env, const sptr<AppExecFwk::IBundleMgr> &iBundleMgr,
    napi_value objBundleRecord, const BundlePermissionUsedRecord &bundleRecord)
{
    napi_value nDeviceId;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleRecord.deviceId.c_str(), NAPI_AUTO_LENGTH, &nDeviceId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "deviceId", nDeviceId));

    napi_value nDeviceLabel;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleRecord.deviceLabel.c_str(), NAPI_AUTO_LENGTH, &nDeviceLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "deviceLabel", nDeviceLabel));
    PERMISSION_LOG_INFO(LABEL, "ConvertBundleRecord deviceLabel=%{public}s.", bundleRecord.deviceLabel.c_str());

    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleRecord.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "bundleName", nBundleName));
    PERMISSION_LOG_INFO(LABEL, "ConvertBundleRecord bundleName=%{public}s.", bundleRecord.bundleName.c_str());

    napi_value nBundleLabel;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleRecord.bundleLabel.c_str(), NAPI_AUTO_LENGTH, &nBundleLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "bundleLabel", nBundleLabel));

    AppExecFwk::BundleInfo bundleInfo;
    bool ret =
        iBundleMgr->GetBundleInfo(bundleRecord.bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (!ret) {
        PERMISSION_LOG_INFO(LABEL,
            "%{public}s cannot get bundleInfo by bundleName %{public}s",
            __func__,
            bundleRecord.bundleName.c_str());
    }
    int32_t iconId = bundleInfo.applicationInfo.iconId;
    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, iconId, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "applicationIconId", nIconId));

    int32_t labelId = bundleInfo.applicationInfo.labelId;
    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "labelId", nLabelId));

    napi_value nRecords;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nRecords));
    for (size_t idx = 0; idx < bundleRecord.permissionUsedRecords.size(); idx++) {
        napi_value objRecord;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objRecord));

        napi_value nPermissionName;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env,
                bundleRecord.permissionUsedRecords[idx].permissionName.c_str(),
                NAPI_AUTO_LENGTH,
                &nPermissionName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "permissionName", nPermissionName));

        napi_value nAccessCountBg;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int32(env, bundleRecord.permissionUsedRecords[idx].accessCountBg, &nAccessCountBg));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "accessCountBg", nAccessCountBg));

        napi_value nAccessCountFg;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int32(env, bundleRecord.permissionUsedRecords[idx].accessCountFg, &nAccessCountFg));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "accessCountFg", nAccessCountFg));

        napi_value nRejectCountBg;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int32(env, bundleRecord.permissionUsedRecords[idx].rejectCountBg, &nRejectCountBg));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "rejectCountBg", nRejectCountBg));

        napi_value nRejectCountFg;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int32(env, bundleRecord.permissionUsedRecords[idx].rejectCountFg, &nRejectCountFg));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "rejectCountFg", nRejectCountFg));

        napi_value nLastAccessTime;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int64(env, bundleRecord.permissionUsedRecords[idx].lastAccessTime, &nLastAccessTime));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "lastAccessTime", nLastAccessTime));

        napi_value nLastRejectTime;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int64(env, bundleRecord.permissionUsedRecords[idx].lastRejectTime, &nLastRejectTime));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "lastRejectTime", nLastRejectTime));

        napi_value nAccessRecordBg;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nAccessRecordBg));
        for (size_t ind = 0; ind < bundleRecord.permissionUsedRecords[idx].accessRecordBg.size(); ind++) {
            napi_value nAccessRecord;
            NAPI_CALL_RETURN_VOID(env,
                napi_create_int64(env, bundleRecord.permissionUsedRecords[idx].accessRecordBg[ind], &nAccessRecord));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nAccessRecordBg, ind, nAccessRecord));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "accessRecordBg", nAccessRecordBg));

        napi_value nAccessRecordFg;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nAccessRecordFg));
        for (size_t ind = 0; ind < bundleRecord.permissionUsedRecords[idx].accessRecordFg.size(); ind++) {
            napi_value nAccessRecord;
            NAPI_CALL_RETURN_VOID(env,
                napi_create_int64(env, bundleRecord.permissionUsedRecords[idx].accessRecordFg[ind], &nAccessRecord));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nAccessRecordFg, ind, nAccessRecord));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "accessRecordFg", nAccessRecordFg));

        napi_value nRejectRecordBg;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nRejectRecordBg));
        for (size_t ind = 0; ind < bundleRecord.permissionUsedRecords[idx].rejectRecordBg.size(); ind++) {
            napi_value nRejectRecord;
            NAPI_CALL_RETURN_VOID(env,
                napi_create_int64(env, bundleRecord.permissionUsedRecords[idx].rejectRecordBg[ind], &nRejectRecord));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nRejectRecordBg, ind, nRejectRecord));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "rejectRecordBg", nRejectRecordBg));

        napi_value nRejectRecordFg;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nRejectRecordFg));
        for (size_t ind = 0; ind < bundleRecord.permissionUsedRecords[idx].rejectRecordFg.size(); ind++) {
            napi_value nRejectRecord;
            NAPI_CALL_RETURN_VOID(env,
                napi_create_int64(env, bundleRecord.permissionUsedRecords[idx].rejectRecordFg[ind], &nRejectRecord));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nRejectRecordFg, ind, nRejectRecord));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecord, "rejectRecordFg", nRejectRecordFg));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nRecords, idx, objRecord));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleRecord, "permissionRecords", nRecords));
}

static std::string GetStringFromNAPI(napi_env env, napi_value value)
{
    std::string result;
    size_t size = 0;

    if (napi_get_value_string_utf8(env, value, nullptr, 0, &size) != napi_ok) {
        PERMISSION_LOG_ERROR(LABEL, "can not get string size");
        return "";
    }
    result.reserve(size + 1);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), (size + 1), &size) != napi_ok) {
        PERMISSION_LOG_ERROR(LABEL, "can not get string value");
        return "";
    }
    return result;
}

static napi_value ParseInt(napi_env env, int &param, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    PERMISSION_LOG_INFO(LABEL, "param=%{public}d.", valuetype);
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. int32 expected.");
    int32_t value;
    napi_get_value_int32(env, args, &value);
    PERMISSION_LOG_INFO(LABEL, "param=%{public}d.", value);
    param = value;
    // create result code
    napi_value result;
    status = napi_create_int32(env, 1, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static napi_value ParseInt64(napi_env env, int64_t &param, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    PERMISSION_LOG_INFO(LABEL, "param=%{public}d.", valuetype);
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. int64 expected.");
    int64_t value;
    napi_get_value_int64(env, args, &value);
    // PERMISSION_LOG_INFO(LABEL, "param=%{public}lld.", value);
    param = value;
    // create result code
    napi_value result;
    status = napi_create_int64(env, 1, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int64 error!");
    return result;
}

static napi_value ParseString(napi_env env, std::string &param, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    param = GetStringFromNAPI(env, args);
    PERMISSION_LOG_INFO(LABEL, "param=%{public}s.", param.c_str());
    // create result code
    napi_value result;
    status = napi_create_int32(env, 1, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static napi_value ParseStringArray(napi_env env, std::vector<std::string> &hapFiles, napi_value args)
{
    PERMISSION_LOG_INFO(LABEL, "ParseStringArray called");
    bool isArray = false;
    uint32_t arrayLength = 0;
    napi_value valueAry = 0;
    napi_valuetype valueAryType = napi_undefined;
    NAPI_CALL(env, napi_is_array(env, args, &isArray));
    NAPI_CALL(env, napi_get_array_length(env, args, &arrayLength));
    PERMISSION_LOG_INFO(LABEL, "ParseStringArray args is array, length=%{public}d", arrayLength);

    for (uint32_t j = 0; j < arrayLength; j++) {
        NAPI_CALL(env, napi_get_element(env, args, j, &valueAry));
        NAPI_CALL(env, napi_typeof(env, valueAry, &valueAryType));

        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, valueAry, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        hapFiles.push_back(GetStringFromNAPI(env, valueAry));
    }
    // create result code
    napi_value result;
    napi_status status;
    status = napi_create_int32(env, 1, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static napi_value ParseQuery(napi_env env, QueryPermissionUsedRequest &request, napi_value args)
{
    napi_status status;
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "param type mismatch!");
    PERMISSION_LOG_INFO(LABEL, "-----ParseQuery type1-----");
    napi_value requestProp = nullptr;
    status = napi_get_named_property(env, args, "queryData", &requestProp);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, requestProp, &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "property type mismatch!");
    PERMISSION_LOG_INFO(LABEL, "-----ParseRequest request-----");
    std::string deviceLabel;
    std::string bundleName;
    std::vector<std::string> permissionNames;
    int64_t beginTimeMillis;
    int64_t endTimeMillis;
    int flag;
    // get deviceLabel property
    napi_value property = nullptr;
    status = napi_get_named_property(env, requestProp, "deviceLabel", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, property, &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "property type mismatch!");
    ParseString(env, deviceLabel, property);
    request.deviceLabel = deviceLabel;
    PERMISSION_LOG_INFO(LABEL, "ParseRequest deviceLabel=%{public}s.", deviceLabel.c_str());
    // get bundleName property
    property = nullptr;
    status = napi_get_named_property(env, requestProp, "bundleName", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, property, &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "property type mismatch!");
    ParseString(env, bundleName, property);
    request.bundleName = bundleName;
    PERMISSION_LOG_INFO(LABEL, "ParseRequest bundleName=%{public}s.", bundleName.c_str());
    // get permissionNames property
    property = nullptr;
    status = napi_get_named_property(env, requestProp, "permissionNames", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    ParseStringArray(env, permissionNames, property);
    request.permissionNames = permissionNames;
    // get beginTimeMillis property
    property = nullptr;
    status = napi_get_named_property(env, requestProp, "beginTimeMillis", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    ParseInt64(env, beginTimeMillis, property);
    request.beginTimeMillis = beginTimeMillis;
    // PERMISSION_LOG_INFO(LABEL, "ParseRequest beginTimeMillis=%{public}lld.", beginTimeMillis);
    // get endTimeMillis property
    property = nullptr;
    status = napi_get_named_property(env, requestProp, "endTimeMillis", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    ParseInt64(env, endTimeMillis, property);
    request.endTimeMillis = endTimeMillis;
    // PERMISSION_LOG_INFO(LABEL, "ParseRequest endTimeMillis=%{public}lld.", endTimeMillis);
    // get flag property
    property = nullptr;
    status = napi_get_named_property(env, requestProp, "flag", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    ParseInt(env, flag, property);
    PERMISSION_LOG_INFO(LABEL, "ParseRequest flag=%{public}d.", flag);
    // create result code
    napi_value result;
    status = napi_create_int32(env, 1, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

static bool InnerGetPermissionRecords(napi_env env, int32_t &flag, std::string &deviceId,
    QueryPermissionUsedRequest &request, QueryPermissionUsedResult &queryResult)
{
    PERMISSION_LOG_INFO(LABEL, "flag%{public}d", flag);
    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);
    if (flag == Constant::ALL_RECORD_FLAG) {
        request.flag = QueryPermissionUsedRequest::FLAG_PERMISSION_USAGE_DETAIL;
    } else if (flag == Constant::LOCAL_DETAIL_FLAG || flag == Constant::DISTRIBUTED_DETAIL_FLAG) {
        if (flag == Constant::LOCAL_DETAIL_FLAG) {
            flag = Constant::LOCAL_DEVICE;
            deviceId = localDeviceId;
        } else {
            flag = Constant::DISTRIBUTED_DEVICE;
        }
        request.flag = QueryPermissionUsedRequest::FLAG_PERMISSION_USAGE_DETAIL;
    } else {
        if (flag == Constant::LOCAL_FLAG) {
            flag = Constant::LOCAL_DEVICE;
            deviceId = localDeviceId;
        } else {
            flag = Constant::DISTRIBUTED_DEVICE;
        }
        request.flag = QueryPermissionUsedRequest::FLAG_PERMISSION_USAGE_SUMMARY;
    }
    if (request.beginTimeMillis < 0 || request.endTimeMillis < 0) {
        return false;
    }
    if (request.beginTimeMillis == 0 && request.endTimeMillis == 0) {
        int64_t beginTime = TimeUtil::GetTimestamp() - Constant::RECORD_TIME;
        request.beginTimeMillis = beginTime < 0 ? 0 : beginTime;
        request.endTimeMillis = TimeUtil::GetTimestamp();
    }
    int ret = DistributedPermissionManagerClient::GetInstance().GetPermissionUsedRecords(request, queryResult);
    return ret == Constant::SUCCESS ? true : false;
}

static void ProcessQueryResult(napi_env env, napi_value result, const int32_t flag, const std::string deviceId,
    const QueryPermissionUsedResult &queryResult)
{
    PERMISSION_LOG_INFO(
        LABEL, "--------------------local deviceId is -----%{public}s", Constant::EncryptDevId(deviceId).c_str());
    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    sptr<AppExecFwk::IBundleMgr> iBundleMgr;
    iBundleMgr = externalDeps->GetBundleManager(iBundleMgr);
    if (!iBundleMgr) {
        PERMISSION_LOG_ERROR(LABEL, "can not get iBundleMgr");
    }
    napi_value nCode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, queryResult.code, &nCode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "code", nCode));
    PERMISSION_LOG_INFO(LABEL, "ProcessQueryResult code=%{public}d.", queryResult.code);

    napi_value nBeginTimeMillis;
    NAPI_CALL_RETURN_VOID(env, napi_create_int64(env, queryResult.beginTimeMillis, &nBeginTimeMillis));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "beginTimeMillis", nBeginTimeMillis));
    // PERMISSION_LOG_INFO(LABEL, "ProcessQueryResult code=%{public}lld.", queryResult.beginTimeMillis);

    napi_value nEndTimeMillis;
    NAPI_CALL_RETURN_VOID(env, napi_create_int64(env, queryResult.endTimeMillis, &nEndTimeMillis));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "endTimeMillis", nEndTimeMillis));
    // PERMISSION_LOG_INFO(LABEL, "ProcessQueryResult code=%{public}lld.", queryResult.endTimeMillis);

    napi_value objBundleRecords;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &objBundleRecords));
    if (queryResult.bundlePermissionUsedRecords.size() > 0) {
        size_t index = 0;
        for (const auto &bundle : queryResult.bundlePermissionUsedRecords) {
            PERMISSION_LOG_INFO(LABEL, "deviceId = %{public}s", Constant::EncryptDevId(bundle.deviceId).c_str());
            PERMISSION_LOG_INFO(LABEL, "deviceLabel = %{public}s ", bundle.deviceLabel.c_str());
            PERMISSION_LOG_INFO(LABEL, "bundleName = %{public}s ", bundle.bundleName.c_str());
            PERMISSION_LOG_INFO(LABEL, "bundleLabel = %{public}s ", bundle.bundleLabel.c_str());
            if (flag == Constant::DISTRIBUTED_DEVICE && deviceId == bundle.deviceId) {
                continue;
            }
            // for (const auto &record : bundle.permissionUsedRecords) {
            //     PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s ", record.permissionName.c_str());
            //     PERMISSION_LOG_INFO(LABEL, "accessCountFg = %{public}d ", record.accessCountFg);
            //     PERMISSION_LOG_INFO(LABEL, "rejectCountFg = %{public}d ", record.rejectCountFg);
            //     PERMISSION_LOG_INFO(LABEL, "accessCountBg = %{public}d ", record.accessCountBg);
            //     PERMISSION_LOG_INFO(LABEL, "rejectCountBg = %{public}d ", record.rejectCountBg);
            //     PERMISSION_LOG_INFO(LABEL, "lastAccessTime = %{public}lld ", record.lastAccessTime);
            //     PERMISSION_LOG_INFO(LABEL, "lastRejectTime = %{public}lld ", record.lastRejectTime);
            // }
            napi_value objBundleRecord;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objBundleRecord));
            ConvertBundleRecord(env, iBundleMgr, objBundleRecord, bundle);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, objBundleRecords, index, objBundleRecord));
            index++;
        }
    } else {
        PERMISSION_LOG_INFO(LABEL, "-----result is null-----");
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "bundlePermissionRecords", objBundleRecords));
}

/**
 * Promise
 */
napi_value GetPermissionRecord(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    // PERMISSION_LOG_INFO(LABEL, "ARGCSIZE is =%{public}d.", argc);
    QueryPermissionUsedRequest request;
    ParseQuery(env, request, argv[0]);

    AsyncRecordsCallbackInfo *asyncCallbackInfo = new AsyncRecordsCallbackInfo{
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .flag = request.flag, .request = request};

    PERMISSION_LOG_INFO(LABEL, "GetPermissionRecord promise.");
    napi_deferred deferred;
    napi_value promise;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    napi_value resourceName;
    napi_create_string_latin1(env, "GetApplicationInfos", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncRecordsCallbackInfo *asyncCallbackInfo = (AsyncRecordsCallbackInfo *)data;
            asyncCallbackInfo->ret = InnerGetPermissionRecords(env,
                asyncCallbackInfo->flag,
                asyncCallbackInfo->deviceId,
                asyncCallbackInfo->request,
                asyncCallbackInfo->queryResult);
        },
        [](napi_env env, napi_status status, void *data) {
            PERMISSION_LOG_INFO(LABEL, "=================load=================");
            AsyncRecordsCallbackInfo *asyncCallbackInfo = (AsyncRecordsCallbackInfo *)data;
            napi_value result;
            napi_create_array(env, &result);
            ProcessQueryResult(
                env, result, asyncCallbackInfo->flag, asyncCallbackInfo->deviceId, asyncCallbackInfo->queryResult);
            napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * Async callback
 */
napi_value GetPermissionRecordAsync(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    // PERMISSION_LOG_INFO(LABEL, "ARGCSIZE is =%{public}d.", argc);
    QueryPermissionUsedRequest request;
    ParseQuery(env, request, argv[0]);

    AsyncRecordsCallbackInfo *asyncCallbackInfo = new AsyncRecordsCallbackInfo{
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .flag = request.flag, .request = request};

    PERMISSION_LOG_INFO(LABEL, "GetPermissionRecord asyncCallback.");
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, "GetPermissionRecord", NAPI_AUTO_LENGTH, &resourceName));
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
    NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], 1, &asyncCallbackInfo->callback));

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncRecordsCallbackInfo *asyncCallbackInfo = (AsyncRecordsCallbackInfo *)data;
            asyncCallbackInfo->ret = InnerGetPermissionRecords(env,
                asyncCallbackInfo->flag,
                asyncCallbackInfo->deviceId,
                asyncCallbackInfo->request,
                asyncCallbackInfo->queryResult);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncRecordsCallbackInfo *asyncCallbackInfo = (AsyncRecordsCallbackInfo *)data;
            napi_value result[ARGS_SIZE_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            napi_create_array(env, &result[PARAM1]);
            ProcessQueryResult(env,
                result[PARAM1],
                asyncCallbackInfo->flag,
                asyncCallbackInfo->deviceId,
                asyncCallbackInfo->queryResult);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
            napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
