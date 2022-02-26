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
#include "doc_db.h"

#include "ejdb2.h"
#include "hilog/log.h"
namespace OHOS {
namespace HiviewDFX {
constexpr HiLogLabel LABEL = {LOG_CORE, 0xD002D10, "HiView-DOCDB"};
int DocDB::OpenDB(std::string dbFile, iwkv_openflags flag)
{
    EJDB_OPTS opts;
    opts.kv.path = dbFile.c_str();
    opts.kv.oflags = flag;
    opts.http.enabled = false;
    opts.no_wal = false;
    opts.sort_buffer_sz = 0;
    opts.document_buffer_sz = 0;

    iwrc rc = ejdb_init();
    if (rc) {
        iwlog_ecode_error3(rc);
        HiLog::Error(LABEL, "ejdb init failed");
        return -1;
    }

    rc = ejdb_open(&opts, &db_);
    if (rc) {
        iwlog_ecode_error3(rc);
        HiLog::Error(LABEL, "open ejdb failed");
        return -1;
    } else {
        HiLog::Info(LABEL, "open ejdb success");
    }
    return 0;
}

int DocDB::CloseDB()
{
    iwrc rc = ejdb_close(&db_);
    if (rc) {
        iwlog_ecode_error3(rc);
        HiLog::Error(LABEL, "close ejdb failed");
        return -1;
    } else {
        HiLog::Info(LABEL, "close ejdb success");
    }
    return 0;
}
} // HiviewDFX
} // OHOS