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
#ifndef SQL_DATABASE_H
#define SQL_DATABASE_H

#include "sql_common.h"

namespace sql {
using std::string;
struct Sqlite3Db;
class SqlDatabase {
public:
    SqlDatabase(void);
    virtual ~SqlDatabase(void);
    const Sqlite3Db& GetHandle();
    bool Open(string fileName);
    void Close();
    bool IsOpen();
    bool BeginTransaction();
    bool CommitTransaction();

private:
    Sqlite3Db* db;
    string errMsg;
    int dbStatus;
};
}  // namespace sql

#endif
