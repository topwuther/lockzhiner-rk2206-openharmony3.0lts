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

#ifndef SQLITE_SINGLE_VER_DATABASE_UPGRADER_H
#define SQLITE_SINGLE_VER_DATABASE_UPGRADER_H

#include "macro_utils.h"
#include "sqlite_utils.h"
#include "single_ver_database_upgrader.h"

namespace DistributedDB {
class SQLiteSingleVerDatabaseUpgrader : virtual public SingleVerDatabaseUpgrader {
public:
    explicit SQLiteSingleVerDatabaseUpgrader(sqlite3 *db, const SecurityOption &secopt, bool isMemDb);
    ~SQLiteSingleVerDatabaseUpgrader() override;
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerDatabaseUpgrader);

    // used for transferring db file to new dir while classifycation feature in SOFTWARE_VERSION_RELEASE_3_0
    static int TransferDatabasePath(const std::string &parentDir, const OpenDbProperties &option);
    static int CreateDbDir();

    void SetMetaUpgrade(const SecurityOption &currentOpt, const SecurityOption &expectOpt, const std::string &subDir);
    void SetSubdir(const std::string &subdir);
    static int SetPathSecOptWithCheck(const std::string &path, const SecurityOption &secOption,
        const std::string &dbStore, bool isWithChecked = false);
    static int SetSecOption(const std::string &path, const SecurityOption &secOption, bool isWithChecked);
protected:
    int BeginUpgrade() override;
    int EndUpgrade(bool isSuccess) override;
    int GetDatabaseVersion(int &version) const override;
    int SetDatabaseVersion(int version) override;
    int UpgradeFromDatabaseVersion(int version) override;
    void SetUpgradeSqls(int version, std::vector<std::string> &sqls, bool &isCreateUpgradeFile) const;
    static int MoveDatabaseToNewDir(const std::string &parentDir, const std::string &upgradeLockFile);
    static int GetDbVersion(const std::string &dbPath, const OpenDbProperties &option, int &version);

    sqlite3 *db_ = nullptr;
    SecurityOption secOpt_;
    bool isMemDB_;
    bool isMetaUpgrade_;
    std::string subDir_;
};
} // namespace DistributedDB
#endif // SQLITE_SINGLE_VER_DATABASE_UPGRADER_H
