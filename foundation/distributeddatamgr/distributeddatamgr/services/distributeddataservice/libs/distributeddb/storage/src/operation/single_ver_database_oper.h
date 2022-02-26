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

#ifndef SINGLE_VER_DATABASE_OPER_H
#define SINGLE_VER_DATABASE_OPER_H

#include "database_oper.h"
#include "sqlite_single_ver_natural_store.h"

namespace DistributedDB {
class SingleVerDatabaseOper : public DatabaseOper {
public:
    SingleVerDatabaseOper(SQLiteSingleVerNaturalStore *naturalStore, SQLiteStorageEngine *storageEngine);
    ~SingleVerDatabaseOper() override {};

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
    int InitStorageEngine();

    void InitDataBaseOption(OpenDbProperties &option) const;

    int RunExportLogic(const CipherPassword &passwd, const std::string &filePrefix) const;

    int RunRekeyLogic(CipherType type, const CipherPassword &passwd);

    int ExportMainDB(const std::string &currentDir, const CipherPassword &passwd, const std::string &dbDir) const;

    int ExportMetaDB(const std::string &currentDir, const CipherPassword &passwd, const std::string &dbDir) const;

    int ClearCurrentDatabase(const ImportFileInfo &info) const;

    int ImportUnpackedMainDatabase(const ImportFileInfo &info, const CipherPassword &srcPasswd) const;

    int ImportUnpackedMetaDatabase(const ImportFileInfo &info) const;

    int SetSecOpt(const std::string &dir, bool isDir = true) const;

    int BackupDatabase(const ImportFileInfo &info) const;

    SQLiteSingleVerNaturalStore *singleVerNaturalStore_;
    SQLiteStorageEngine *storageEngine_;
};
} // namespace DistributedDB
#endif // SINGLE_VER_DATABASE_OPER_H