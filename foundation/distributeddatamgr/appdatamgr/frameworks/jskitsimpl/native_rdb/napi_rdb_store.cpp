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

#include "napi_rdb_store.h"
#include <inttypes.h>
#include "common.h"
#include "js_utils.h"
#include "napi_async_proxy.h"
#include "napi_rdb_predicates.h"
#include "napi_result_set.h"
#include "securec.h"

using namespace OHOS::NativeRdb;
using namespace OHOS::JsKit;

namespace OHOS {
namespace RdbJsKit {
class RdbStoreContext : public NapiAsyncProxy<RdbStoreContext>::AysncContext {
public:
    RdbStoreContext()
        : AysncContext(),
          tableName(""),
          whereClause(""),
          sql(""),
          predicatesProxy(nullptr),
          valuesBucket(nullptr),
          rowId(0)
    {
        valuesBucket = new ValuesBucket();
    }

    virtual ~RdbStoreContext()
    {
        auto *obj = reinterpret_cast<RdbStoreProxy *>(boundObj);
        if (obj != nullptr) {
            obj->Release(env);
        }
        delete valuesBucket;
    }

    void BindArgs(napi_env env, napi_value value);
    void JSNumber2NativeType(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    std::string tableName;
    std::string whereClause;
    std::vector<std::string> whereArgs;
    std::vector<std::string> selectionArgs;
    std::string sql;
    RdbPredicatesProxy *predicatesProxy;
    std::vector<std::string> columns;
    ValuesBucket *valuesBucket;
    std::map<std::string, ValueObject> numberMaps;
    std::vector<ValueObject> bindArgs;
    uint64_t rowId;
    std::unique_ptr<AbsSharedResultSet> resultSet;
};

napi_ref RdbStoreProxy::constructor_ = nullptr;

void RdbStoreContext::BindArgs(napi_env env, napi_value arg)
{
    bindArgs.clear();
    uint32_t arrLen = 0;
    napi_get_array_length(env, arg, &arrLen);
    if (arrLen == 0) {
        return;
    }
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value element;
        napi_get_element(env, arg, i, &element);
        napi_valuetype type;
        napi_typeof(env, element, &type);
        switch (type) {
            case napi_boolean: {
                bool value;
                napi_status status = napi_get_value_bool(env, element, &value);
                if (status == napi_ok) {
                    bindArgs.push_back(ValueObject(value));
                }
            } break;
            case napi_number: {
                double value;
                napi_status status = napi_get_value_double(env, element, &value);
                if (status == napi_ok) {
                    bindArgs.push_back(ValueObject(value));
                }
            } break;
            case napi_null:
                bindArgs.push_back(ValueObject());
                break;
            case napi_string:
                bindArgs.push_back(ValueObject(JSUtils::Convert2String(env, element, JSUtils::DEFAULT_BUF_SIZE)));
                break;
            case napi_object:
                bindArgs.push_back(ValueObject(JSUtils::Convert2U8Vector(env, element)));
                break;
            default:
                break;
        }
    }
}

void RdbStoreContext::JSNumber2NativeType(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    std::unique_ptr<ResultSet> result = rdbStore->QueryByStep(std::string("SELECT * FROM ") + tableName + " LIMIT 1");
    LOG_DEBUG("ValueBucket table:%{public}s", tableName.c_str());
    result->GoToFirstRow();
    for (std::map<std::string, ValueObject>::iterator it = numberMaps.begin(); it != numberMaps.end(); it++) {
        int index = -1;
        result->GetColumnIndex(it->first, index);
        ColumnType columnType = ColumnType::TYPE_FLOAT;
        result->GetColumnType(index, columnType);
        double value;
        it->second.GetDouble(value);
        switch (columnType) {
            case ColumnType::TYPE_FLOAT:
                LOG_DEBUG("JSNumber2NativeType to key:%{public}s type:float", it->first.c_str());
                valuesBucket->PutDouble(it->first, value);
                break;
            case ColumnType::TYPE_INTEGER:
                LOG_DEBUG("JSNumber2NativeType to key:%{public}s type:integer", it->first.c_str());
                valuesBucket->PutLong(it->first, int64_t(value));
                break;
            default:
                LOG_DEBUG("JSNumber2NativeType to key:%{public}s type:%{public}d", it->first.c_str(), int(columnType));
                valuesBucket->PutDouble(it->first, value);
                break;
        }
    }
    result->Close();
    result = nullptr;
    numberMaps.clear();
}

RdbStoreProxy::RdbStoreProxy() {}

RdbStoreProxy::~RdbStoreProxy()
{
    LOG_DEBUG("RdbStoreProxy destructor");
}

void RdbStoreProxy::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("delete", Delete),
        DECLARE_NAPI_FUNCTION("update", Update),
        DECLARE_NAPI_FUNCTION("insert", Insert),
        DECLARE_NAPI_FUNCTION("querySql", QuerySql),
        DECLARE_NAPI_FUNCTION("query", Query),
        DECLARE_NAPI_FUNCTION("executeSql", ExecuteSql),
    };
    LOG_DEBUG("Init RdbStoreProxy");
    napi_value cons = nullptr;
    napi_define_class(env, "RdbStore", NAPI_AUTO_LENGTH, Initialize, nullptr,
        sizeof(descriptors) / sizeof(napi_property_descriptor), descriptors, &cons);

    napi_create_reference(env, cons, 1, &constructor_);
    LOG_DEBUG("Init RdbStoreProxy end");
}

napi_value RdbStoreProxy::Initialize(napi_env env, napi_callback_info info)
{
    napi_value self;
    NAPI_CALL(env, napi_get_cb_info(env, info, NULL, NULL, &self, nullptr));
    auto finalize = [](napi_env env, void *data, void *hint) {
        RdbStoreProxy *proxy = reinterpret_cast<RdbStoreProxy *>(data);
        if (proxy->ref_ != nullptr) {
            napi_delete_reference(env, proxy->ref_);
            proxy->ref_ = nullptr;
        }
        delete proxy;
    };
    auto *proxy = new RdbStoreProxy();
    napi_status status = napi_wrap(env, self, proxy, finalize, nullptr, &proxy->ref_);
    if (status != napi_ok) {
        LOG_ERROR("RdbStoreProxy napi_wrap failed! code:%{public}d!", status);
        finalize(env, proxy, nullptr);
        return nullptr;
    }
    if (proxy->ref_ == nullptr) {
        napi_create_reference(env, self, 0, &proxy->ref_);
    }
    LOG_INFO("RdbStoreProxy constructor ref:%{public}p", proxy->ref_);
    return self;
}

napi_value RdbStoreProxy::NewInstance(napi_env env, std::shared_ptr<OHOS::NativeRdb::RdbStore> value)
{
    napi_value cons;
    napi_status status = napi_get_reference_value(env, constructor_, &cons);
    if (status != napi_ok) {
        LOG_ERROR("RdbStoreProxy get constructor failed! code:%{public}d!", status);
        return nullptr;
    }

    napi_value instance = nullptr;
    status = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (status != napi_ok) {
        LOG_ERROR("RdbStoreProxy napi_new_instance failed! code:%{public}d!", status);
        return nullptr;
    }

    RdbStoreProxy *proxy = nullptr;
    status = napi_unwrap(env, instance, reinterpret_cast<void **>(&proxy));
    if (proxy == nullptr) {
        LOG_ERROR("RdbStoreProxy native instance is nullptr! code:%{public}d!", status);
        return instance;
    }
    proxy->rdbStore_ = std::move(value);
    return instance;
}

RdbStoreProxy *RdbStoreProxy::GetNativeInstance(napi_env env, napi_value self)
{
    RdbStoreProxy *proxy = nullptr;
    napi_status status = napi_unwrap(env, self, reinterpret_cast<void **>(&proxy));
    if (proxy == nullptr) {
        LOG_ERROR("RdbStoreProxy::GetNativePredicates native instance is nullptr! code:%{public}d!", status);
        return nullptr;
    }
    uint32_t count = 0;
    {
        std::lock_guard<std::mutex> lock(proxy->mutex_);
        status = napi_reference_ref(env, proxy->ref_, &count);
    }
    if (status != napi_ok) {
        LOG_ERROR("RdbStoreProxy::GetNativePredicates napi_reference_ref(%{public}p) failed! code:%{public}d!, "
            "count:%{public}u",
            proxy->ref_, status, count);
        return proxy;
    }
    return proxy;
}

void RdbStoreProxy::Release(napi_env env)
{
    uint32_t count = 0;
    napi_status status = napi_ok;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        status = napi_reference_unref(env, ref_, &count);
    }

    if (status != napi_ok) {
        LOG_ERROR("RdbStoreProxy::Release napi_reference_unref(%{public}p) failed! code:%{public}d!, count:%{public}u",
            ref_, status, count);
    }
}

void ParseThis(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    asyncContext->boundObj = RdbStoreProxy::GetNativeInstance(env, arg);
    LOG_DEBUG("ParseThis is : %{public}p", asyncContext->boundObj);
}

void ParseTableName(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    asyncContext->tableName = JSUtils::Convert2String(env, arg, E_EMPTY_TABLE_NAME);
    LOG_DEBUG("ParseTableName is : %{public}s", asyncContext->tableName.c_str());
}

void ParsePredicates(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    LOG_DEBUG("ParsePredicates on called.");
    napi_unwrap(env, arg, reinterpret_cast<void **>(&asyncContext->predicatesProxy));
    asyncContext->tableName = asyncContext->predicatesProxy->GetPredicates()->GetTableName();
}

void ParseColumns(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    LOG_DEBUG("ParseColumns on called.");
    asyncContext->columns = JSUtils::Convert2StrVector(env, arg, JSUtils::DEFAULT_BUF_SIZE);
    LOG_DEBUG("ParseColumns columns(begin):%{public}s.", asyncContext->columns.begin()->c_str());
}

void ParseWhereClause(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    asyncContext->whereClause = JSUtils::Convert2String(env, arg, E_HAVING_CLAUSE_NOT_IN_GROUP_BY);
    LOG_DEBUG("ParseWhereClause is : %{public}s", asyncContext->whereClause.c_str());
}

void ParseWhereArgs(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    asyncContext->whereArgs = JSUtils::Convert2StrVector(env, arg, JSUtils::DEFAULT_BUF_SIZE);
    LOG_DEBUG("ParseWhereArgs is : %{public}zu", asyncContext->whereArgs.size());
}

void ParseSelectionArgs(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    asyncContext->selectionArgs = JSUtils::Convert2StrVector(env, arg, JSUtils::DEFAULT_BUF_SIZE);
    LOG_DEBUG("ParseSelectionArgs is : %{public}zu", asyncContext->selectionArgs.size());
}

void ParseSql(const napi_env &env, const napi_value &arg, RdbStoreContext *asyncContext)
{
    asyncContext->sql = JSUtils::Convert2String(env, arg, JSUtils::DEFAULT_BUF_SIZE);
    LOG_DEBUG("ParseSql is : %{public}s", asyncContext->sql.c_str());
}

void ParseValuesBucket(const napi_env &env, const napi_value &arg, RdbStoreContext *context)
{
    napi_value keys = 0;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        LOG_DEBUG("ValuesBucket errr");
        return;
    }
    LOG_DEBUG("ValuesBucket num:%{public}d ", arrLen);
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key;
        status = napi_get_element(env, keys, i, &key);
        std::string keyStr = JSUtils::Convert2String(env, key, JSUtils::DEFAULT_BUF_SIZE);
        napi_value value;
        napi_get_property(env, arg, key, &value);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, value, &valueType);
        if (valueType == napi_string) {
            std::string valueString = JSUtils::Convert2String(env, value, JSUtils::DEFAULT_BUF_SIZE);
            context->valuesBucket->PutString(keyStr, valueString);
            LOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:%{public}s", valueType, keyStr.c_str(),
                valueString.c_str());
        } else if (valueType == napi_number) {
            double valueNumber;
            napi_get_value_double(env, value, &valueNumber);
            context->numberMaps.insert(std::make_pair(keyStr, ValueObject(valueNumber)));
            LOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:%{public}lf", valueType, keyStr.c_str(),
                valueNumber);
        } else if (valueType == napi_boolean) {
            bool valueBool = false;
            napi_get_value_bool(env, value, &valueBool);
            context->valuesBucket->PutBool(keyStr, valueBool);
            LOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:%{public}d", valueType, keyStr.c_str(),
                valueBool);
        } else if (valueType == napi_null) {
            context->valuesBucket->PutNull(keyStr);
            LOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:null", valueType, keyStr.c_str());
        } else if (valueType == napi_object) {
            context->valuesBucket->PutBlob(keyStr, JSUtils::Convert2U8Vector(env, value));
            LOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:Uint8Array", valueType, keyStr.c_str());
        } else {
            LOG_WARN("valuesBucket error");
        }
    }
}

napi_value RdbStoreProxy::Insert(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<RdbStoreContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<RdbStoreContext>::InputParser> parsers;
    parsers.push_back(ParseTableName);
    parsers.push_back(ParseValuesBucket);
    proxy.ParseInputs(parsers, ParseThis);
    return proxy.DoAsyncWork(
        "Insert",
        [](RdbStoreContext *context) {
            RdbStoreProxy *obj = reinterpret_cast<RdbStoreProxy *>(context->boundObj);
            int64_t rowId = 0;
            LOG_DEBUG("Insert tableName :%{public}s", context->tableName.c_str());
            context->JSNumber2NativeType(obj->rdbStore_);
            int errCode = obj->rdbStore_->Insert(rowId, context->tableName, *(context->valuesBucket));
            context->rowId = rowId;
            LOG_DEBUG("Insert rowId :%{public}" PRIu64, context->rowId);
            return errCode;
        },
        [](RdbStoreContext *context, napi_value &output) {
            LOG_DEBUG("Insert rowId :%{public}" PRIu64, context->rowId);
            napi_status status = napi_create_int64(context->env, context->rowId, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value RdbStoreProxy::Delete(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<RdbStoreContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<RdbStoreContext>::InputParser> parsers;
    parsers.push_back(ParsePredicates);
    proxy.ParseInputs(parsers, ParseThis);
    return proxy.DoAsyncWork(
        "Delete",
        [](RdbStoreContext *context) {
            LOG_DEBUG("napi Delete predicates:%{public}s",
                context->predicatesProxy->GetPredicates()->ToString().c_str());
            RdbStoreProxy *obj = reinterpret_cast<RdbStoreProxy *>(context->boundObj);
            int temp = 0;
            int errCode = obj->rdbStore_->Delete(temp, *(context->predicatesProxy->GetPredicates()));
            context->rowId = temp;
            LOG_DEBUG("napi Delete");
            return errCode;
        },
        [](RdbStoreContext *context, napi_value &output) {
            napi_status status = napi_create_int64(context->env, context->rowId, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value RdbStoreProxy::Update(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<RdbStoreContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<RdbStoreContext>::InputParser> parsers;
    parsers.push_back(ParseValuesBucket);
    parsers.push_back(ParsePredicates);
    proxy.ParseInputs(parsers, ParseThis);
    return proxy.DoAsyncWork(
        "Update",
        [](RdbStoreContext *context) {
            LOG_DEBUG("napi Update predicates:%{public}s",
                context->predicatesProxy->GetPredicates()->ToString().c_str());
            RdbStoreProxy *obj = reinterpret_cast<RdbStoreProxy *>(context->boundObj);
            int temp = 0;
            context->JSNumber2NativeType(obj->rdbStore_);
            int errCode =
                obj->rdbStore_->Update(temp, *(context->valuesBucket), *(context->predicatesProxy->GetPredicates()));
            context->rowId = temp;
            return errCode;
        },
        [](RdbStoreContext *context, napi_value &output) {
            napi_status status = napi_create_int64(context->env, context->rowId, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}

napi_value RdbStoreProxy::Query(napi_env env, napi_callback_info info)
{
    LOG_DEBUG("Query on called.");
    NapiAsyncProxy<RdbStoreContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<RdbStoreContext>::InputParser> parsers;
    parsers.push_back(ParsePredicates);
    parsers.push_back(ParseColumns);
    proxy.ParseInputs(parsers, ParseThis);
    return proxy.DoAsyncWork(
        "Query",
        [](RdbStoreContext *context) {
            LOG_DEBUG("napi Query predicates:%{public}s",
                context->predicatesProxy->GetPredicates()->ToString().c_str());
            RdbStoreProxy *obj = reinterpret_cast<RdbStoreProxy *>(context->boundObj);
            context->resultSet = obj->rdbStore_->Query(*(context->predicatesProxy->GetPredicates()), context->columns);
            LOG_DEBUG("Query result is nullptr ? %{public}d", (context->resultSet == nullptr));
            return (context->resultSet != nullptr) ? OK : ERR;
        },
        [](RdbStoreContext *context, napi_value &output) {
            output = ResultSetProxy::NewInstance(context->env,
                                                 std::shared_ptr<AbsSharedResultSet>(context->resultSet.release()));
            return (output != nullptr) ? OK : ERR;
        });
}

napi_value RdbStoreProxy::QuerySql(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<RdbStoreContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<RdbStoreContext>::InputParser> parsers;
    parsers.push_back(ParseSql);
    parsers.push_back(ParseSelectionArgs);
    proxy.ParseInputs(parsers, ParseThis);
    return proxy.DoAsyncWork(
        "QuerySql",
        [](RdbStoreContext *context) {
            RdbStoreProxy *obj = reinterpret_cast<RdbStoreProxy *>(context->boundObj);
            context->resultSet = obj->rdbStore_->QuerySql(context->sql, context->selectionArgs);
            LOG_DEBUG("Queried Sql: %{public}s, result == null ? %{public}d", context->sql.c_str(),
                (context->resultSet != nullptr));
            return (context->resultSet != nullptr) ? OK : ERR;
        },
        [](RdbStoreContext *context, napi_value &output) {
            output = ResultSetProxy::NewInstance(context->env,
                                                 std::shared_ptr<AbsSharedResultSet>(context->resultSet.release()));
            return (output != nullptr) ? OK : ERR;
        });
}

napi_value RdbStoreProxy::ExecuteSql(napi_env env, napi_callback_info info)
{
    NapiAsyncProxy<RdbStoreContext> proxy;
    proxy.Init(env, info);
    std::vector<NapiAsyncProxy<RdbStoreContext>::InputParser> parsers;
    parsers.push_back(ParseSql);
    parsers.push_back(
        [](const napi_env &env, const napi_value &arg, RdbStoreContext *ctx) { ctx->BindArgs(env, arg); });
    proxy.ParseInputs(parsers, ParseThis);
    return proxy.DoAsyncWork(
        "ExecuteSql",
        [](RdbStoreContext *context) {
            RdbStoreProxy *obj = reinterpret_cast<RdbStoreProxy *>(context->boundObj);
            int errCode = obj->rdbStore_->ExecuteSql(context->sql, context->bindArgs);
            LOG_DEBUG("Executed Sql:%{public}s", context->sql.c_str());
            return errCode;
        },
        [](RdbStoreContext *context, napi_value &output) {
            napi_status status = napi_get_undefined(context->env, &output);
            return (status == napi_ok) ? OK : ERR;
        });
}
} // namespace RdbJsKit
} // namespace OHOS
