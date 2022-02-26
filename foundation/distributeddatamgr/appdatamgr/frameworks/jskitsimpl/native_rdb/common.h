/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RDB_JSKIT_COMMON_H
#define RDB_JSKIT_COMMON_H

#include <string>
#include <vector>

#include "hilog/log.h"
#include "rdb_errno.h"

namespace OHOS {
namespace RdbJsKit {
static const OHOS::HiviewDFX::HiLogLabel PREFIX_LABEL = { LOG_CORE, 0xD001650, "RdbJsKit" };

#define LOG_DEBUG(...) ((void)OHOS::HiviewDFX::HiLog::Debug(PREFIX_LABEL, __VA_ARGS__))
#define LOG_INFO(...) ((void)OHOS::HiviewDFX::HiLog::Info(PREFIX_LABEL, __VA_ARGS__))
#define LOG_WARN(...) ((void)OHOS::HiviewDFX::HiLog::Warn(PREFIX_LABEL, __VA_ARGS__))
#define LOG_ERROR(...) ((void)OHOS::HiviewDFX::HiLog::Error(PREFIX_LABEL, __VA_ARGS__))
#define LOG_FATAL(...) ((void)OHOS::HiviewDFX::HiLog::Fatal(PREFIX_LABEL, __VA_ARGS__))

static const std::vector<std::string> ERR_INFO {
    "E_ERROR",
    "E_CANNOT_UPDATE_READONLY",
    "E_REMOVE_FILE",
    "E_EMPTY_FILE_NAME",
    "E_EMPTY_TABLE_NAME",
    "E_EMPTY_VALUES_BUCKET",
    "E_INVALID_STATEMENT",
    "E_INVALID_COLUMN_INDEX",
    "E_INVALID_COLUMN_TYPE",
    "E_INVALID_COLUMN_NAME",
    "E_QUERY_IN_EXECUTE",
    "E_TRANSACTION_IN_EXECUTE",
    "E_EXECUTE_IN_STEP_QUERY",
    "E_EXECUTE_WRITE_IN_READ_CONNECTION",
    "E_BEGIN_TRANSACTION_IN_READ_CONNECTION",
    "E_NO_TRANSACTION_IN_SESSION",
    "E_MORE_STEP_QUERY_IN_ONE_SESSION",
    "E_NO_ROW_IN_QUERY",
    "E_INVALID_BIND_ARGS_COUNT",
    "E_INVALID_OBJECT_TYPE",
    "E_INVALID_CONFLICT_FLAG",
    "E_HAVING_CLAUSE_NOT_IN_GROUP_BY",
    "E_NOT_SUPPORTED_BY_STEP_RESULT_SET",
    "E_STEP_RESULT_SET_CROSS_THREADS",
    "E_STEP_RESULT_QUERY_NOT_EXECUTED",
    "E_STEP_RESULT_IS_AFTER_LAST",
    "E_STEP_RESULT_QUERY_EXCEEDED",
    "E_STATEMENT_NOT_PREPARED",
    "E_EXECUTE_RESULT_INCORRECT",
    "E_STEP_RESULT_CLOSED",
    "E_RELATIVE_PATH",
    "E_EMPTY_NEW_ENCRYPT_KEY",
    "E_CHANGE_UNENCRYPTED_TO_ENCRYPTED",
    "E_CHANGE_ENCRYPT_KEY_IN_BUSY",
    "E_STEP_STATEMENT_NOT_INIT",
    "E_NOT_SUPPORTED_ATTACH_IN_WAL_MODE"
};

static inline const char *GetErrStr(int err)
{
    if (err == OHOS::NativeRdb::E_OK) {
        return nullptr;
    }
    size_t index = err - OHOS::NativeRdb::E_BASE - 1;
    if (index >= ERR_INFO.size() || index < 0) {
        return "Unkown error";
    }
    return ERR_INFO.at(index).c_str();
}
} // namespace RdbJsKit
} // namespace OHOS

#endif
