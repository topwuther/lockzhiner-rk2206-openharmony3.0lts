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

#ifndef LOCAL_DATABASE_OPER_H
#define LOCAL_DATABASE_OPER_H

#include "database_oper.h"
#include "sqlite_local_kvdb.h"

namespace DistributedDB {
class LocalDatabaseOper : public DatabaseOper {
public:
    LocalDatabaseOper(SQLiteLocalKvDB *localKvDb, SQLiteStorageEngine *storageEngine);
    ~LocalDatabaseOper() override {};

    int Rekey(const CipherPassword &passwd) override;

    int Import(const std::string &filePath, const CipherPassword &passwd) override;

    int Export(const std::string &filePath, const CipherPassword &passwd) const override;

protected:
    bool RekeyPreHandle(const CipherPassword &passwd, int &errCode) override;

    int BackupDb(const CipherPassword &passwd) const override;

    int CloseStorages() override;

    int RekeyPostHandle(const CipherPassword &passwd) override;

    int ExportAllDatabases(const std::string &currentDir, const CipherPassword &passwd,
        const std::string &dbDir) const override;

    int BackupCurrentDatabase(const ImportFileInfo &info) const override;

    int ImportUnpackedDatabase(const ImportFileInfo &info, const CipherPassword &srcPasswd) const override;

    int ImportPostHandle() const override;

private:
    SQLiteLocalKvDB *localKvDb_;
    SQLiteStorageEngine *storageEngine_;
};
} // namespace DistributedDB
#endif // LOCAL_DATABASE_OPER_H