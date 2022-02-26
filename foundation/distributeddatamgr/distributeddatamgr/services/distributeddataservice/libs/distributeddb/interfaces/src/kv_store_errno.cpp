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

#include "kv_store_errno.h"
#include "db_errno.h"

namespace DistributedDB {
struct DBErrnoPair {
    int errCode;
    DBStatus status;
};

namespace {
    const DBErrnoPair ERRNO_MAP[] = {
        { E_OK, OK },
        { -E_BUSY, BUSY },
        { -E_NOT_FOUND, NOT_FOUND },
        { -E_INVALID_ARGS, INVALID_ARGS },
        { -E_TIMEOUT, TIME_OUT },
        { -E_NOT_SUPPORT, NOT_SUPPORT },
        { -E_INVALID_PASSWD_OR_CORRUPTED_DB, INVALID_PASSWD_OR_CORRUPTED_DB },
        { -E_MAX_LIMITS, OVER_MAX_LIMITS },
        { -E_INVALID_FILE, INVALID_FILE },
        { -E_INVALID_PATH, NO_PERMISSION },
        { -E_READ_ONLY, READ_ONLY },
        { -E_INVALID_SCHEMA, INVALID_SCHEMA },
        { -E_SCHEMA_MISMATCH, SCHEMA_MISMATCH },
        { -E_SCHEMA_VIOLATE_VALUE, SCHEMA_VIOLATE_VALUE },
        { -E_VALUE_MISMATCH_FEILD_COUNT, INVALID_VALUE_FIELDS },
        { -E_VALUE_MISMATCH_FEILD_TYPE, INVALID_FIELD_TYPE },
        { -E_VALUE_MISMATCH_CONSTRAINT, CONSTRAIN_VIOLATION },
        { -E_INVALID_FORMAT, INVALID_FORMAT },
        { -E_STALE, STALE },
        { -E_LOCAL_DELETED, LOCAL_DELETED },
        { -E_LOCAL_DEFEAT, LOCAL_DEFEAT },
        { -E_LOCAL_COVERED, LOCAL_COVERED },
        { -E_INVALID_QUERY_FORMAT, INVALID_QUERY_FORMAT },
        { -E_INVALID_QUERY_FIELD, INVALID_QUERY_FIELD },
        { -E_ALREADY_SET, ALREADY_SET },
        { -E_EKEYREVOKED, EKEYREVOKED_ERROR },
        { -E_SECURITY_OPTION_CHECK_ERROR, SECURITY_OPTION_CHECK_ERROR},
    };
}

DBStatus TransferDBErrno(int err)
{
    for (const auto &item : ERRNO_MAP) {
        if (item.errCode == err) {
            return item.status;
        }
    }
    return DB_ERROR;
}
};
