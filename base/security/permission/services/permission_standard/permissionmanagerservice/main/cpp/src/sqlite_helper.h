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

#ifndef SQLITE_HELPER_H
#define SQLITE_HELPER_H

#include <string>

#include "statement.h"

#include "sqlite3sym.h"

namespace OHOS {
namespace Security {
namespace Permission {
class SqliteHelper {
public:
    explicit SqliteHelper(const std::string& dbName, const std::string& dbPath, int version);
    virtual ~SqliteHelper();

    void Open();
    void Close();

    int BeginTransaction() const;
    int CommitTransaction() const;
    int RollbackTransaction() const;

    Statement Prepare(const std::string& sql) const;
    int ExecuteSql(const std::string& sql) const;
    std::string SpitError() const;

    virtual void OnCreate() = 0;
    virtual void OnUpdate() = 0;

private:
    inline static const std::string PRAGMA_VERSION_COMMAND = "PRAGMA user_version";
    static const int GENERAL_ERROR = -1;

    const std::string dbName_;
    const std::string dbPath_;
    int currentVersion_;
    sqlite3* db_;

    int GetVersion() const;
    void SetVersion() const;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // SQLITE_HELPER_H
