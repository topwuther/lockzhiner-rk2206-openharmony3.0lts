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

#ifndef MULTI_VER_DATABASE_OPER_H
#define MULTI_VER_DATABASE_OPER_H

#ifndef OMIT_MULTI_VER
#include "database_oper.h"
#include "multi_ver_natural_store.h"

namespace DistributedDB {
class MultiVerDatabaseOper : public DatabaseOper {
public:
    MultiVerDatabaseOper(MultiVerNaturalStore *multiVerNaturalStore, IKvDBMultiVerDataStorage *multiVerData,
        IKvDBCommitStorage *commitHistory, MultiVerKvDataStorage *multiVerKvStorage);
    ~MultiVerDatabaseOper() override {};

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
    int ImportDatabase(const std::string &dir, const CipherPassword &passwd) const;

    MultiVerNaturalStore *multiVerNaturalStore_;
    IKvDBMultiVerDataStorage *multiVerData_;
    IKvDBCommitStorage *commitHistory_;
    MultiVerKvDataStorage *multiVerKvStorage_;
};
} // namespace DistributedDB
#endif // MULTI_VER_DATABASE_OPER_H
#endif