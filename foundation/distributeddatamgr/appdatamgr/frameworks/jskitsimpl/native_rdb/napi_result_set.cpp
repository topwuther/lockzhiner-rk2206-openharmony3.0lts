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

#include "napi_result_set.h"
#include <functional>

#include "message_parcel.h"
#include "abs_shared_result_set.h"
#include "common.h"
#include "js_utils.h"
#include "napi_async_proxy.h"
#include "string_ex.h"

using namespace OHOS::NativeRdb;
using namespace OHOS::JsKit;

namespace OHOS {
namespace RdbJsKit {
napi_ref ResultSetProxy::ctorRef_ = nullptr;
napi_value ResultSetProxy::NewInstance(napi_env env, std::shared_ptr<AbsSharedResultSet> resultSet)
{
    napi_value cons = GetConstructor(env);
    if (cons == nullptr) {
        LOG_ERROR("NewInstance GetConstructor is nullptr!");
        return nullptr;
    }
    napi_value instance;
    napi_status status = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (status != napi_ok) {
        LOG_ERROR("NewInstance napi_new_instance failed! code:%{public}d!", status);
        return nullptr;
    }

    ResultSetProxy *proxy = nullptr;
    status = napi_unwrap(env, instance, reinterpret_cast<void **>(&proxy));
    if (proxy == nullptr) {
        LOG_ERROR("NewInstance native instance is nullptr! code:%{public}d!", status);
        return instance;
    }

    if (resultSet->GetBlock() != nullptr) {
        proxy->sharedBlockName_ = resultSet->GetBlock()->Name();
        proxy->sharedBlockAshmemFd_ = resultSet->GetBlock()->GetFd();
        LOG_INFO("{sharedBlockName:%{public}s, sharedBlockAshmemFd_:%{public}d}", proxy->sharedBlockName_.c_str(),
                 proxy->sharedBlockAshmemFd_);
    }
    *proxy = std::move(resultSet);
    return instance;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> ResultSetProxy::GetNativePredicates(
    napi_env const &env, napi_value const &arg)
{
    LOG_DEBUG("GetNativePredicates on called.");
    if (arg == nullptr) {
        LOG_ERROR("DataAbilityPredicatesProxy arg is null.");
        return nullptr;
    }
    ResultSetProxy *proxy = nullptr;
    napi_unwrap(env, arg, reinterpret_cast<void **>(&proxy));
    return proxy->resultSet_;
}

napi_value ResultSetProxy::GetConstructor(napi_env env)
{
    napi_value cons;
    if (ctorRef_ != nullptr) {
        NAPI_CALL(env, napi_get_reference_value(env, ctorRef_, &cons));
        return cons;
    }
    LOG_INFO("GetConstructor result set constructor");
    napi_property_descriptor clzDes[] = {
        DECLARE_NAPI_FUNCTION("goToRow", GoToRow),
        DECLARE_NAPI_FUNCTION("getLong", GetLong),
        DECLARE_NAPI_FUNCTION("getColumnType", GetColumnType),
        DECLARE_NAPI_FUNCTION("goTo", GoTo),
        DECLARE_NAPI_FUNCTION("getColumnIndex", GetColumnIndex),
        DECLARE_NAPI_FUNCTION("getInt", GetInt),
        DECLARE_NAPI_FUNCTION("getColumnName", GetColumnName),
        DECLARE_NAPI_FUNCTION("close", Close),
        DECLARE_NAPI_FUNCTION("goToFirstRow", GoToFirstRow),
        DECLARE_NAPI_FUNCTION("goToLastRow", GoToLastRow),
        DECLARE_NAPI_FUNCTION("goToNextRow", GoToNextRow),
        DECLARE_NAPI_FUNCTION("goToPreviousRow", GoToPreviousRow),
        DECLARE_NAPI_FUNCTION("getBlob", GetBlob),
        DECLARE_NAPI_FUNCTION("getString", GetString),
        DECLARE_NAPI_FUNCTION("getDouble", GetDouble),
        DECLARE_NAPI_FUNCTION("isColumnNull", IsColumnNull),

        DECLARE_NAPI_GETTER("columnNames", GetAllColumnNames),
        DECLARE_NAPI_GETTER("columnCount", GetColumnCount),
        DECLARE_NAPI_GETTER("isEnded", IsEnded),
        DECLARE_NAPI_GETTER("isStarted", IsBegin),
        DECLARE_NAPI_GETTER("isClosed", IsClosed),
        DECLARE_NAPI_GETTER("rowCount", GetRowCount),
        DECLARE_NAPI_GETTER("rowIndex", GetRowIndex),
        DECLARE_NAPI_GETTER("isAtFirstRow", IsAtFirstRow),
        DECLARE_NAPI_GETTER("isAtLastRow", IsAtLastRow),

        DECLARE_NAPI_GETTER("sharedBlockName", GetSharedBlockName),
        DECLARE_NAPI_GETTER("sharedBlockAshmemFd", GetSharedBlockAshmemFd),
    };
    NAPI_CALL(env, napi_define_class(env, "ResultSet", NAPI_AUTO_LENGTH, Initialize, nullptr,
        sizeof(clzDes) / sizeof(napi_property_descriptor), clzDes, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &ctorRef_));
    return cons;
}

napi_value ResultSetProxy::Initialize(napi_env env, napi_callback_info info)
{
    napi_value self = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &self, nullptr));
    auto *proxy = new ResultSetProxy();
    auto finalize = [](napi_env env, void *data, void *hint) {
        ResultSetProxy *proxy = reinterpret_cast<ResultSetProxy *>(data);
        if (proxy->ref_ != nullptr) {
            napi_delete_reference(env, proxy->ref_);
            proxy->ref_ = nullptr;
        }
        delete proxy;
    };
    napi_status status = napi_wrap(env, self, proxy, finalize, nullptr, &proxy->ref_);
    if (status != napi_ok) {
        LOG_ERROR("ResultSetProxy napi_wrap failed! code:%{public}d!", status);
        finalize(env, proxy, nullptr);
        return nullptr;
    }
    if (proxy->ref_ == nullptr) {
        napi_create_reference(env, self, 0, &proxy->ref_);
    }
    LOG_INFO("ResultSetProxy constructor ref:%{public}p", proxy->ref_);
    return self;
}

ResultSetProxy::~ResultSetProxy()
{
    LOG_INFO("ResultSetProxy destructor!");
    if (resultSet_ != nullptr && !resultSet_->IsClosed()) {
        resultSet_->Close();
    }
}

ResultSetProxy::ResultSetProxy(std::shared_ptr<AbsSharedResultSet> resultSet)
{
    if (resultSet_ == resultSet) {
        return;
    }
    resultSet_ = std::move(resultSet);
}

ResultSetProxy &ResultSetProxy::operator = (std::shared_ptr<AbsSharedResultSet> resultSet)
{
    if (resultSet_ == resultSet) {
        return *this;
    }
    resultSet_ = std::move(resultSet);
    return *this;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> &ResultSetProxy::GetInnerResultSet(napi_env env, napi_callback_info info)
{
    ResultSetProxy *resultSet = nullptr;
    napi_value self = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &self, nullptr);
    napi_unwrap(env, self, reinterpret_cast<void **>(&resultSet));
    return resultSet->resultSet_;
}

napi_value ResultSetProxy::GetAllColumnNames(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetAllColumnNames Begin!");
    std::vector<std::string> colNames;
    int errCode = GetInnerResultSet(env, info)->GetAllColumnNames(colNames);
    if (errCode != E_OK) {
        LOG_ERROR("GetAllColumnNames failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, colNames);
}

napi_value ResultSetProxy::GoToRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GoToRow Begin!");
    int32_t position;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &position));
    int errCode = GetInnerResultSet(env, info)->GoToRow(position);
    if (errCode != E_OK) {
        LOG_ERROR("GoToRow failed code:%{public}d", errCode);
    }
    napi_value output;
    napi_get_undefined(env, &output);
    return output;
}

napi_value ResultSetProxy::GetColumnCount(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetColumnCount Begin!");
    int32_t count = 0;
    int errCode = GetInnerResultSet(env, info)->GetColumnCount(count);
    if (errCode != E_OK) {
        LOG_ERROR("GetColumnCount failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, count);
}

napi_value ResultSetProxy::GetLong(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetLong Begin!");
    int32_t columnIndex;
    int64_t result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetLong(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetLong failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetColumnType(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetColumnType Begin!");
    int32_t columnIndex;
    ColumnType columnType;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetColumnType(columnIndex, columnType);
    if (errCode != E_OK) {
        LOG_ERROR("GetColumnType failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, int32_t(columnType));
}

napi_value ResultSetProxy::GoTo(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GoTo Begin!");
    int32_t columnIndex;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GoTo(columnIndex);
    if (errCode != E_OK) {
        LOG_ERROR("GoTo failed code:%{public}d", errCode);
    }
    napi_value output;
    napi_get_undefined(env, &output);
    return output;
}

napi_value ResultSetProxy::GetColumnIndex(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetColumnIndex Begin!");
    int32_t result = -1;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    std::string input = JSUtils::Convert2String(env, args[0], JSUtils::DEFAULT_BUF_SIZE);
    int errCode = GetInnerResultSet(env, info)->GetColumnIndex(input, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetColumnIndex failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetInt(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetInt Begin!");
    int32_t columnIndex;
    int32_t result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetInt(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetInt failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetColumnName(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetColumnName Begin!");
    int32_t columnIndex;
    std::string result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetColumnName(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetColumnName failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::Close(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("Close Begin!");
    int errCode = GetInnerResultSet(env, info)->Close();
    if (errCode != E_OK) {
        LOG_ERROR("Close failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, (errCode == E_OK));
}

napi_value ResultSetProxy::GetRowCount(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetRowCount Begin!");
    int32_t result;
    int errCode = GetInnerResultSet(env, info)->GetRowCount(result);
    if (errCode != E_OK) {
        LOG_ERROR("GetRowCount failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetRowIndex(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetRowIndex Begin!");
    int32_t result;
    int errCode = GetInnerResultSet(env, info)->GetRowIndex(result);
    if (errCode != E_OK) {
        LOG_ERROR("GetRowIndex failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::IsEnded(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("IsEnded Begin!");
    bool result;
    int errCode = GetInnerResultSet(env, info)->IsEnded(result);
    if (errCode != E_OK) {
        LOG_ERROR("IsEnded failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::IsBegin(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("IsBegin Begin!");
    bool result;
    int errCode = GetInnerResultSet(env, info)->IsStarted(result);
    if (errCode != E_OK) {
        LOG_ERROR("IsBegin failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GoToFirstRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GoToFirstRow Begin!");
    int errCode = GetInnerResultSet(env, info)->GoToFirstRow();
    if (errCode != E_OK) {
        LOG_ERROR("GoToFirstRow failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, (errCode == E_OK));
}

napi_value ResultSetProxy::GoToLastRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GoToLastRow Begin!");
    int errCode = GetInnerResultSet(env, info)->GoToLastRow();
    if (errCode != E_OK) {
        LOG_ERROR("GoToLastRow failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, (errCode == E_OK));
}

napi_value ResultSetProxy::GoToNextRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GoToNextRow Begin!");
    int errCode = GetInnerResultSet(env, info)->GoToNextRow();
    if (errCode != E_OK) {
        LOG_ERROR("GoToNextRow failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, (errCode == E_OK));
}

napi_value ResultSetProxy::GoToPreviousRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GoToPreviousRow Begin!");
    int errCode = GetInnerResultSet(env, info)->GoToPreviousRow();
    if (errCode != E_OK) {
        LOG_ERROR("GoToPreviousRow failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, (errCode == E_OK));
}

napi_value ResultSetProxy::IsAtFirstRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("IsAtFirstRow Begin!");
    bool result;
    int errCode = GetInnerResultSet(env, info)->IsAtFirstRow(result);
    if (errCode != E_OK) {
        LOG_ERROR("IsAtFirstRow failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::IsAtLastRow(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("IsAtLastRow Begin!");
    bool result;
    int errCode = GetInnerResultSet(env, info)->IsAtLastRow(result);
    if (errCode != E_OK) {
        LOG_ERROR("IsAtLastRow failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetBlob(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetBlob Begin!");
    int32_t columnIndex;
    std::vector<uint8_t> result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetBlob(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetBlob failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetString(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetString Begin!");
    int32_t columnIndex;
    std::string result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetString(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetString failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::GetDouble(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("GetDouble Begin!");
    int32_t columnIndex;
    double result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->GetDouble(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("GetDouble failed code:%{public}d", errCode);
    }
    return JSUtils::Convert2JSValue(env, result);
}

napi_value ResultSetProxy::IsColumnNull(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("IsColumnNull Begin!");
    int32_t columnIndex;
    bool result;
    size_t argc = MAX_INPUT_COUNT;
    napi_value args[MAX_INPUT_COUNT] = { 0 };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    NAPI_ASSERT(env, argc > 0, "Invalid argvs!");
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &columnIndex));
    int errCode = GetInnerResultSet(env, info)->IsColumnNull(columnIndex, result);
    if (errCode != E_OK) {
        LOG_ERROR("IsColumnNull failed code:%{public}d", errCode);
    }
    napi_value output;
    napi_get_boolean(env, result, &output);
    return output;
}

napi_value ResultSetProxy::IsClosed(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("IsClosed Begin!");
    int result = GetInnerResultSet(env, info)->IsClosed();
    napi_value output;
    napi_get_boolean(env, result, &output);
    return output;
}

napi_value ResultSetProxy::GetSharedBlockName(napi_env env, napi_callback_info info)
{
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    ResultSetProxy *proxy;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&proxy)));

    return JSUtils::Convert2JSValue(env, proxy->sharedBlockName_);
}

napi_value ResultSetProxy::GetSharedBlockAshmemFd(napi_env env, napi_callback_info info)
{
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    ResultSetProxy *proxy;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&proxy)));

    return JSUtils::Convert2JSValue(env, proxy->sharedBlockAshmemFd_);
}
} // namespace RdbJsKit
} // namespace OHOS

EXTERN_C_START
__attribute__((visibility("default"))) napi_value NAPI_OHOS_Data_RdbJsKit_ResultSetProxy_NewInstance(
    napi_env env, OHOS::NativeRdb::AbsSharedResultSet *resultSet)
{
    return OHOS::RdbJsKit::ResultSetProxy::NewInstance(
        env, std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet>(resultSet));
}

__attribute__((visibility("default"))) OHOS::NativeRdb::AbsSharedResultSet *
NAPI_OHOS_Data_RdbJsKit_ResultSetProxy_GetNativeObject(const napi_env &env, const napi_value &arg)
{
    // the resultSet maybe release.
    auto resultSet = OHOS::RdbJsKit::ResultSetProxy::GetNativePredicates(env, arg);
    return resultSet.get();
}
EXTERN_C_END